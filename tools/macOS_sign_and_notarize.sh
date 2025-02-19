#!/bin/bash

source ./notarization_secrets.env

if [[ -z "$APPLE_ID" || -z "$TEAM_ID" || -z "$APP_SPECIFIC_PASSWORD" || -z "$DEVELOPER_NAME" || -z "$BUNDLE_ID" ]]; then
    echo "Error: Missing credentials. Make sure notarization_secrets.env exists and exports the following variables:"
    echo " - APPLE_ID (e.g. name@example.com)"
    echo " - TEAM_ID (e.g. 12AB34CD56)"
    echo " - DEVELOPER_NAME (e.g. First Last)"
    echo " - APP_SPECIFIC_PASSWORD (e.g. abcd-efgh-ijkl-mnop)"
    echo " - BUNDLE_ID (e.g. com.example.myapp)"
    exit 1
fi

DEVELOPER_ID_APPLICATION="Developer ID Application: ${DEVELOPER_NAME} (${TEAM_ID})"

FOLDER_TO_SIGN="$1"
APP_NAME="$2"
RELEASE_DIR="$3"

if [[ -z "$FOLDER_TO_SIGN" || -z "$APP_NAME" ]]; then
  echo "Usage: $0 <folder_to_sign> <app_name> [<release_dir>]"
  exit 1
fi

if [ -f "$FOLDER_TO_SIGN/../version.txt" ]; then
  APP_VERSION=$(cat "$FOLDER_TO_SIGN/../version.txt" | tr -d '[:space:]')
else
  echo "Error: version.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -f "$FOLDER_TO_SIGN/../build_info.txt" ]; then
    source "$FOLDER_TO_SIGN/../build_info.txt"
    echo " - Built for OS: $OS, Architecture: $ARCH"
else
  echo "Error: build_info.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -z "$RELEASE_DIR" ]; then
    RELEASE_DIR="../release/v${APP_VERSION}/${OS}_${ARCH}"
fi

mkdir -p "$RELEASE_DIR"

echo "Signing folder: ${FOLDER_TO_SIGN}"
echo " - APP_VERSION: ${APP_VERSION}"
echo " - DEVELOPER_ID_APPLICATION: ${DEVELOPER_ID_APPLICATION}"
echo " - APP_NAME: ${APP_NAME}"
echo " - RELEASE_DIR: ${RELEASE_DIR}"

echo "Signing all files:"
for file in "$FOLDER_TO_SIGN"/*; do
  if [[ -f "$file" || -d "$file" ]]; then
    echo "Signing $file..."
    codesign --deep --force --verify --options=runtime --timestamp --verbose --sign "${DEVELOPER_ID_APPLICATION}" "$file"
  fi
done

echo
echo "Verifing signing:"
for file in "$FOLDER_TO_SIGN"/*; do
  if [[ -f "$file" || -d "$file" ]]; then
    codesign --verify --verbose "$file"
  fi
done

ZIP_NAME="${APP_NAME}_${OS}_${ARCH}_v${APP_VERSION}"
ZIP_FILE_UNSIGNED="${ZIP_NAME}_UNSIGNED.zip"

echo
echo "Zipping files to ${RELEASE_DIR}/${ZIP_FILE_UNSIGNED}"
zip -r -q "${RELEASE_DIR}/${ZIP_FILE_UNSIGNED}" "$FOLDER_TO_SIGN"
echo "Zipped to ${RELEASE_DIR}/${ZIP_FILE_UNSIGNED}"

echo
echo "Submitting for notarization..."

xcrun notarytool submit "${RELEASE_DIR}/${ZIP_FILE_UNSIGNED}" --apple-id "$APPLE_ID" --password "$APP_SPECIFIC_PASSWORD" --team-id "$TEAM_ID" --wait
if [ $? -eq 0 ] 
then 
  echo "Notarization succeeded." 
else 
  echo
  echo "Error: Failed to submit for notarization." >&2 
  exit 1
fi

rm "${RELEASE_DIR}/${ZIP_FILE_UNSIGNED}"

echo
echo "Stapling files"
for file in "$FOLDER_TO_SIGN"/*; do
  if [[ -f "$file" || -d "$file" ]]; then
    echo "Stapling $file"
    xcrun stapler staple "$file"
  fi
done

# Make release
ZIP_FILE_SIGNED="${ZIP_NAME}.zip"
echo
echo "Making release at ${RELEASE_DIR}/${ZIP_FILE_SIGNED}"
zip -r -q "${RELEASE_DIR}/${ZIP_FILE_SIGNED}" "$FOLDER_TO_SIGN"

echo
echo "Done notarizing app!"
echo