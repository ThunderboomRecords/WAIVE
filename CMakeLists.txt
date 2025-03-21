cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(POSITION_INDEPENDENT_CODE ON)
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
  message(STATUS "Building on Mac")
  set(BUILD_OS "macOS")
  set(CMAKE_GENERATOR "Ninja")
  set(CMAKE_MAKE_PROGRAM /opt/homebrew/bin/ninja CACHE FILEPATH "Path to Ninja build system")
  if(TARGET_MAC_X86)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE INTERNAL "" FORCE)
    set(CMAKE_SYSTEM_PROCESSOR "x86" CACHE INTERNAL "" FORCE)
    set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE INTERNAL "" FORCE)
  else()
    set(CMAKE_SYSTEM_PROCESSOR "arm64" CACHE INTERNAL "" FORCE)
  endif()
  message(STATUS "CMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")
elseif(WIN32)
  message(STATUS "Building on Windows")
  set(CMAKE_C_COMPILER "cl")
  set(CMAKE_CXX_COMPILER "cl")
  set(BUILD_OS "win")
  set(CMAKE_SYSTEM_PROCESSOR "x64" CACHE INTERNAL "" FORCE)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  set(POCO_MT ON)
  set(ENABLE_NETSSL_WIN ON CACHE INTERNAL "" FORCE)
  set(CMAKE_GENERATOR "Visual Studio 17 2022")
  set(VCPKG_TARGET_TRIPLET "x64-windows-static-custom")
elseif(UNIX AND NOT APPLE)
  message(STATUS "Building on Linux")
  set(CMAKE_GENERATOR "Ninja")
  set(CMAKE_SYSTEM_PROCESSOR "x64" CACHE INTERNAL "" FORCE)
  set(BUILD_OS "linux")
endif()

set(POCO_STATIC ON)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

set(BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "Detected OS: ${BUILD_OS}")
message(STATUS "Detected Architecture: ${BUILD_ARCH}")
file(WRITE "${CMAKE_BINARY_DIR}/build_info.txt" "OS=${BUILD_OS}\nARCH=${BUILD_ARCH}\n")

project(
    WAIVE_Plugins VERSION 1.5.6
    LANGUAGES CXX C
)

configure_file(templates/version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
configure_file(templates/version_template.txt ${CMAKE_BINARY_DIR}/version.txt)
if(WIN32)
  configure_file(templates/version_template_inno.txt ${CMAKE_BINARY_DIR}/inno_build_info.txt)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

add_subdirectory(DPF)

add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Sequencer)
