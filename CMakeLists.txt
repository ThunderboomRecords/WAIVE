cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED OFF)

project(
    WAIVE_Sampler
)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(dpf)
add_subdirectory(external/LibrosaCpp)
add_subdirectory(external/nlohmann_json)

add_subdirectory(plugins/WAIVE_Sampler)
