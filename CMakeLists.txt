cmake_minimum_required(VERSION 3.18)

project(
    WAIVE_Midi
)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_subdirectory(dpf)
add_subdirectory(plugins/WAIVE_Midi)