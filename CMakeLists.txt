cmake_minimum_required(VERSION 3.18)
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED OFF)

project(
    WAIVE_Sampler
)

add_subdirectory(dpf)
add_subdirectory(external/LibrosaCpp)
# externals
# add_subdirectory(external/Gist)
# add_compile_options(-DUSE_KISS_FFT)


add_subdirectory(plugins/WAIVE_Sampler)
