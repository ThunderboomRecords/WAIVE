#!/bin/bash

source ./notarization_secrets.env

FOLDER_OF_SIGNED="$1"
APP_NAME="$2"
RELEASE_DIR="$3"

if [[ -z "$FOLDER_OF_SIGNED" || -z "$APP_NAME" ]]; then
  echo "Usage: $0 <folder_of_signed> <app_name> [<release_dir>]"
  exit 1
fi

if [ -f "$FOLDER_OF_SIGNED/../version.txt" ]; then
  APP_VERSION=$(cat "$FOLDER_OF_SIGNED/../version.txt" | tr -d '[:space:]')
  echo " - App version: ${APP_VERSION}"
else
  echo "Error: version.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -f "$FOLDER_OF_SIGNED/../build_info.txt" ]; then
    source "$FOLDER_OF_SIGNED/../build_info.txt"
    echo " - Built for OS: $OS, Architecture: $ARCH"
else
  echo "Error: build_info.txt not found. Ensure CMake generates this file."
  exit 1
fi

if [ -z "$RELEASE_DIR" ]; then
    RELEASE_DIR="../release/v${APP_VERSION}/${OS}_${ARCH}"
fi

DEVELOPER_ID_APPLICATION="Developer ID Application: ${DEVELOPER_NAME} (${TEAM_ID})"
DEVELOPER_ID_INSTALLER="Developer ID Installer: ${DEVELOPER_NAME} (${TEAM_ID})"
IMAGE_NAME="${RELEASE_DIR}/${APP_NAME}_${OS}_${ARCH}_v${APP_VERSION}.dmg"

echo "Building release package in ${RELEASE_DIR}"

mkdir -p ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/Components
mkdir -p ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/VST
mkdir -p ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/VST3 
cp -r "${FOLDER_OF_SIGNED}"/*.component ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/Components/ 
cp -r "${FOLDER_OF_SIGNED}"/*.vst ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/VST/ 
cp -r "${FOLDER_OF_SIGNED}"/*.vst3 ${RELEASE_DIR}/payload/Library/Audio/Plug-Ins/VST3/

echo
pkgbuild --root "${RELEASE_DIR}/payload" --identifier "${BUNDLE_ID}" --version "${APP_VERSION}" --install-location / "${RELEASE_DIR}/${APP_NAME}_Unsigned.pkg"

echo
productsign --sign "$DEVELOPER_ID_INSTALLER" "${RELEASE_DIR}/${APP_NAME}_Unsigned.pkg" "${RELEASE_DIR}/${APP_NAME}.pkg"
rm "${RELEASE_DIR}/${APP_NAME}_Unsigned.pkg"

echo
hdiutil create -volname "${APP_NAME}_Installer" -srcfolder "${RELEASE_DIR}/${APP_NAME}.pkg" -format UDZO "${IMAGE_NAME}"

echo
codesign --sign "${DEVELOPER_ID_APPLICATION}" --timestamp "${IMAGE_NAME}"
codesign --verify --verbose=2 "${IMAGE_NAME}"

echo
xcrun notarytool submit "${IMAGE_NAME}" --apple-id "${APPLE_ID}" --team-id "${TEAM_ID}" --password "${APP_SPECIFIC_PASSWORD}" --wait
xcrun stapler staple "${IMAGE_NAME}"


if [ $? -eq 0 ] 
then 
  echo "Installer made successfully!" 
else 
  echo "Error: Failed stapling." >&2 
  exit 1
fi