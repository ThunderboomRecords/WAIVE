cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(POSITION_INDEPENDENT_CODE ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_GENERATOR "Ninja")

if(APPLE)
  message(STATUS "Building on Mac")
  set(BUILD_OS "macOS")
  set(CMAKE_MAKE_PROGRAM /opt/homebrew/bin/ninja CACHE FILEPATH "Path to Ninja build system")
  if(DEFINED TARGET_MAC_X86)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "" FORCE)
    set(CMAKE_SYSTEM_PROCESSOR "x86" CACHE INTERNAL "" FORCE)
    set(VCPKG_TARGET_TRIPLET "x64-osx")
  else()
  set(CMAKE_SYSTEM_PROCESSOR "arm64" CACHE INTERNAL "" FORCE)
  endif()
  message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
elseif(WIN32)
  message(STATUS "Building on Windows")
  set(BUILD_OS "win")
elseif(UNIX AND NOT APPLE)
  message(STATUS "Building on Linux")
  set(BUILD_OS "linux")
endif()

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set(BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "Detected OS: ${BUILD_OS}")
message(STATUS "Detected Architecture: ${BUILD_ARCH}")
file(WRITE "${CMAKE_BINARY_DIR}/build_info.txt" "OS=${BUILD_OS}\nARCH=${BUILD_ARCH}\n")

project(
    WAIVE_Plugins VERSION 1.5.2
)

configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
configure_file(version_template.txt ${CMAKE_BINARY_DIR}/version.txt)

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

add_subdirectory(DPF)

add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Sequencer)
