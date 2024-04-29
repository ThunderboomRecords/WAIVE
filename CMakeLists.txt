cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED OFF)

project(
    WAIVE_Sampler
)

add_subdirectory(dpf)
add_subdirectory(external/LibrosaCpp)

add_subdirectory(plugins/WAIVE_Sampler)
