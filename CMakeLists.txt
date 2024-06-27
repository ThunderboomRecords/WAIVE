cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED OFF)

project(
    WAIVE_Plugins VERSION 0.12.0
)

configure_file(version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h)
include_directories(
    ${CMAKE_BINARY_DIR}/generated/
    external/Gist/libs/kiss_fft130
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_compile_options(-DUSE_KISS_FFT)
set(CMAKE_CXX_FLAGS -fdiagnostics-color=always)
set(BUILD_TESTING OFF)
set(LIBSAMPLERATE_EXAMPLES OFF)

add_subdirectory(dpf)
add_subdirectory(external/Gist)

add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Midi)
