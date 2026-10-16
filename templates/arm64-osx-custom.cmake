set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

# Oldest macOS the plugins run on. CMakeLists.txt reads this, so the plugins
# and their dependencies always target the same version. Keep it the same in
# arm64-osx-custom.cmake and x64-osx-custom.cmake, and no lower than the
# prebuilt onnxruntime library (12.7 for 1.17.3).
set(VCPKG_OSX_DEPLOYMENT_TARGET 12.7)
