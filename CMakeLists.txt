cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED OFF)

project(
    WAIVE_Sampler
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_compile_options(-DUSE_KISS_FFT)

add_subdirectory(dpf)
add_subdirectory(external/LibrosaCpp)
add_subdirectory(external/nlohmann_json)
add_subdirectory(external/kissfft)
add_subdirectory(external/Gist)

add_subdirectory(plugins/WAIVE_Sampler)
