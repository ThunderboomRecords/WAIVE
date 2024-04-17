cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

project(
    WAIVE_Midi
)

add_subdirectory(dpf)
add_subdirectory(plugins/WAIVE_Midi)
