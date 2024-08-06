cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)
set(POSITION_INDEPENDENT_CODE ON)

project(
    WAIVE_Plugins VERSION 0.13.3
)

configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

add_subdirectory(dpf)

add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Midi)
