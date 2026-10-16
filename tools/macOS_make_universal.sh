#!/bin/bash

# Merges an arm64 build and an x86_64 build of the plugins into universal
# binaries, ready for macOS_sign_and_notarize.sh and macOS_make_installer.sh.
#
# Build each architecture into its own folder first, e.g. from the repo root:
#   cmake --preset default -B build-arm64 -DCMAKE_BUILD_TYPE=Release
#   cmake --preset default -B build-x86_64 -DCMAKE_BUILD_TYPE=Release -DTARGET_MAC_X86=1
#   cmake --build build-arm64 -j8 && cmake --build build-x86_64 -j8
#
# Then, from tools/:
#   ./macOS_make_universal.sh ../build-arm64/bin ../build-x86_64/bin ../build-universal/bin
#   ./macOS_sign_and_notarize.sh ../build-universal/bin WAIVE-Plugins
#   ./macOS_make_installer.sh ../build-universal/bin WAIVE-Plugins
#
# Signing must happen after merging, as lipo invalidates existing signatures.

set -e -o pipefail

ARM64_BIN="$1"
X86_64_BIN="$2"
OUTPUT_BIN="$3"

# Marks an output folder as made by this script, so it is safe to replace
MARKER=".universal_build"

# Minimum macOS version a single-architecture binary runs on
min_macos() {
  otool -l "$1" | awk '/LC_BUILD_VERSION/ { found = 1 } found && !done && $1 == "minos" { print $2; done = 1 }'
}

if [[ -z "$ARM64_BIN" || -z "$X86_64_BIN" || -z "$OUTPUT_BIN" ]]; then
  echo "Usage: $0 <arm64_bin_folder> <x86_64_bin_folder> <output_bin_folder>"
  exit 1
fi

for folder in "$ARM64_BIN" "$X86_64_BIN"; do
  if [ ! -d "$folder" ]; then
    echo "Error: $folder not found."
    exit 1
  fi
  for info in version.txt build_info.txt; do
    if [ ! -f "$folder/../$info" ]; then
      echo "Error: $folder/../$info not found. Ensure CMake generates this file."
      exit 1
    fi
  done
done

# Check each build is the expected architecture and the same version
ARM64_ARCH=$(source "$ARM64_BIN/../build_info.txt" && echo "$ARCH")
X86_64_ARCH=$(source "$X86_64_BIN/../build_info.txt" && echo "$ARCH")
if [ "$ARM64_ARCH" != "arm64" ]; then
  echo "Error: $ARM64_BIN is not an arm64 build (build_info.txt has ARCH=$ARM64_ARCH)."
  exit 1
fi
if [ "$X86_64_ARCH" != "x86" ]; then
  echo "Error: $X86_64_BIN is not an x86_64 build (build_info.txt has ARCH=$X86_64_ARCH). Build it with -DTARGET_MAC_X86=1."
  exit 1
fi

APP_VERSION=$(tr -d '[:space:]' < "$ARM64_BIN/../version.txt")
X86_64_VERSION=$(tr -d '[:space:]' < "$X86_64_BIN/../version.txt")
if [ "$APP_VERSION" != "$X86_64_VERSION" ]; then
  echo "Error: the builds are different versions ($APP_VERSION and $X86_64_VERSION)."
  exit 1
fi

# The sign and installer scripts read version.txt and build_info.txt from the
# output's parent folder, so don't write those into a CMake build folder
OUTPUT_PARENT="$(dirname "$OUTPUT_BIN")"
if [ -f "$OUTPUT_PARENT/CMakeCache.txt" ]; then
  echo "Error: $OUTPUT_PARENT is a CMake build folder. Choose an output outside it, e.g. ../build-universal/bin"
  exit 1
fi

if [ -e "$OUTPUT_BIN" ]; then
  if [ -f "$OUTPUT_BIN/$MARKER" ]; then
    rm -rf "$OUTPUT_BIN"
  else
    echo "Error: $OUTPUT_BIN already exists and was not made by this script. Remove it or choose another folder."
    exit 1
  fi
fi

echo "Making universal build of v${APP_VERSION}"
echo " - arm64:  ${ARM64_BIN}"
echo " - x86_64: ${X86_64_BIN}"
echo " - output: ${OUTPUT_BIN}"
echo

# Start from a copy of the arm64 build (ditto keeps bundle structure,
# symlinks and permissions), then replace each binary with a merged one
mkdir -p "$OUTPUT_PARENT"
ditto "$ARM64_BIN" "$OUTPUT_BIN"
touch "$OUTPUT_BIN/$MARKER"

PROBLEMS=0
MERGED=0

while IFS= read -r -d '' file; do
  file="${file#./}"
  arm64_file="$ARM64_BIN/$file"
  x86_64_file="$X86_64_BIN/$file"

  if [ ! -e "$x86_64_file" ]; then
    echo "Error: $file is missing from the x86_64 build."
    PROBLEMS=$((PROBLEMS + 1))
    continue
  fi

  if file -b "$arm64_file" | grep -q "Mach-O"; then
    arm64_archs=$(lipo -archs "$arm64_file")
    x86_64_archs=$(lipo -archs "$x86_64_file")
    arm64_min=$(min_macos "$arm64_file")
    x86_64_min=$(min_macos "$x86_64_file")
    if [[ "$arm64_archs" != "arm64" || "$x86_64_archs" != "x86_64" ]]; then
      echo "Error: $file has architectures '$arm64_archs' in the arm64 build and '$x86_64_archs' in the x86_64 build."
      PROBLEMS=$((PROBLEMS + 1))
    elif [[ "$arm64_min" != "$x86_64_min" ]]; then
      # Both builds should use the deployment target from the vcpkg triplets
      echo "Error: $file needs macOS $arm64_min in the arm64 build but macOS $x86_64_min in the x86_64 build."
      PROBLEMS=$((PROBLEMS + 1))
    else
      lipo -create "$arm64_file" "$x86_64_file" -output "$OUTPUT_BIN/$file"
      echo "Merged $file (macOS $arm64_min+)"
      MERGED=$((MERGED + 1))
    fi
  elif ! cmp -s "$arm64_file" "$x86_64_file"; then
    # e.g. Info.plist, resources: these should be the same in both builds
    echo "Warning: $file differs between the builds, using the arm64 version."
  fi
done < <(cd "$ARM64_BIN" && find . -type f -print0)

while IFS= read -r -d '' file; do
  file="${file#./}"
  if [ ! -e "$ARM64_BIN/$file" ]; then
    echo "Error: $file is missing from the arm64 build."
    PROBLEMS=$((PROBLEMS + 1))
  fi
done < <(cd "$X86_64_BIN" && find . -type f -print0)

if [ "$PROBLEMS" -gt 0 ]; then
  echo
  echo "Error: ${PROBLEMS} problem(s) found, removing ${OUTPUT_BIN}." >&2
  rm -rf "$OUTPUT_BIN"
  exit 1
fi

if [ "$MERGED" -eq 0 ]; then
  echo
  echo "Error: no binaries found to merge in ${ARM64_BIN}." >&2
  rm -rf "$OUTPUT_BIN"
  exit 1
fi

cp "$ARM64_BIN/../version.txt" "$OUTPUT_PARENT/version.txt"
printf "OS=macOS\nARCH=universal\n" > "$OUTPUT_PARENT/build_info.txt"

echo
echo "Merged ${MERGED} binaries:"
while IFS= read -r -d '' file; do
  if file -b "$file" | grep -q "Mach-O"; then
    echo " - ${file#"$OUTPUT_BIN"/}: $(lipo -archs "$file")"
  fi
done < <(find "$OUTPUT_BIN" -type f -print0)

echo
echo "Universal build ready in ${OUTPUT_BIN}"
