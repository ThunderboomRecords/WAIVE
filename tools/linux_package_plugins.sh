#!/bin/bash

FOLDER_TO_PKG="$1"
APP_NAME="$2"
RELEASE_DIR="$3"

if [[ -z "$FOLDER_TO_PKG" || -z "$APP_NAME" ]]; then
  echo "Usage: $0 <folder_to_package> <app_name> [<release_dir>]"
  exit 1
fi

if [ -f "$FOLDER_TO_PKG/../version.txt" ]; then
  APP_VERSION=$(cat "$FOLDER_TO_PKG/../version.txt" | tr -d '[:space:]')
else
  echo "Error: version.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -f "$FOLDER_TO_PKG/../build_info.txt" ]; then
    source "$FOLDER_TO_PKG/../build_info.txt"
    echo " - Built for OS: $OS, Architecture: $ARCH"
else
  echo "Error: build_info.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -z "$RELEASE_DIR" ]; then
    RELEASE_DIR="../release/v${APP_VERSION}/${OS}_${ARCH}"
fi

mkdir -p "$RELEASE_DIR"
ARCHIVE_NAME="${APP_NAME}_${OS}_${ARCH}_v${APP_VERSION}"

echo "Packaging folder: ${FOLDER_TO_PKG}"
echo " - APP_VERSION: ${APP_VERSION}"
echo " - APP_NAME: ${APP_NAME}"
echo " - RELEASE_DIR: ${RELEASE_DIR}"
echo " - ARCHIVE_NAME: ${ARCHIVE_NAME}"

echo
echo "Making archive..."

mkdir -p $RELEASE_DIR/$ARCHIVE_NAME
cp -r $FOLDER_TO_PKG/*.vst3 $RELEASE_DIR/$ARCHIVE_NAME
cp -r $FOLDER_TO_PKG/*.clap $RELEASE_DIR/$ARCHIVE_NAME
cp -r $FOLDER_TO_PKG/*vst2.so $RELEASE_DIR/$ARCHIVE_NAME

tar -czf "${RELEASE_DIR}/${ARCHIVE_NAME}.tar.gz" -C $RELEASE_DIR/$ARCHIVE_NAME .

rm -rf $RELEASE_DIR/$ARCHIVE_NAME

echo "${RELEASE_DIR}/${ARCHIVE_NAME}.tar.gz"
echo "Done"