cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(POSITION_INDEPENDENT_CODE ON)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_GENERATOR "Ninja")

if(APPLE)
  message(STATUS "Building on Mac") 
  set(CMAKE_MAKE_PROGRAM /opt/homebrew/bin/ninja CACHE FILEPATH "Path to Ninja build system")
elseif(WIN32)
  message(STATUS "Building on Windows")
elseif(UNIX AND NOT APPLE)
  message(STATUS "Building on Linux") 
endif()

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

project(
    WAIVE_Plugins VERSION 1.2.1
)

configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

add_subdirectory(dpf)

add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Midi)
