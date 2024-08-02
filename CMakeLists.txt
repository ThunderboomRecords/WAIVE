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
    external/Gist/libs/kiss_fft130
)

set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
set(OPENSSL_LIBRARIES /usr/local/opt/openssl/lib)



set(BUILD_SHARED_LIBS OFF)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_compile_options(-DUSE_KISS_FFT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
set(LIBSAMPLERATE_EXAMPLES OFF CACHE BOOL "Build libsamplerate examples")
set(BUILD_TESTING OFF CACHE BOOL "Enable libsamplerate tests")
message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")


set(LIBSAMPLERATE_STATIC ON CACHE BOOL "Build libsamplerate as static library")

# Poco configuration to disable unwanted components
set(ENABLE_ACTIVERECORD OFF CACHE BOOL "Disable Poco ActiveRecord")
set(ENABLE_ACTIVERECORD_COMPILER OFF CACHE BOOL "Disable Poco ActiveRecord Compiler")
set(ENABLE_PROMETHEUS OFF CACHE BOOL "Disable Prometheus")
set(ENABLE_XML OFF CACHE BOOL "Disable Poco XML")
set(ENABLE_REDIS OFF CACHE BOOL "Disable Poco Redis")
set(ENABLE_PDF OFF CACHE BOOL "Disable Poco PDF")
set(ENABLE_UTIL OFF CACHE BOOL "Disable Poco Util")
set(ENABLE_ZIP OFF CACHE BOOL "Disable Poco Zip")
set(ENABLE_PAGECOMPILER OFF CACHE BOOL "Disable Poco PageCompiler")
set(ENABLE_PAGECOMPILER_FILE2PAGE OFF CACHE BOOL "Disable Poco File2Page")
set(ENABLE_NETSSL_WIN OFF CACHE BOOL "Disable Poco NetSSL Win")
set(ENABLE_MONGODB OFF CACHE BOOL "Disable Poco MongoDB")
set(ENABLE_DATA_ODBC OFF CACHE BOOL "Disable Poco Data ODBC")
set(ENABLE_DATA_MYSQL OFF CACHE BOOL "Disable Poco Data MySQL")
set(ENABLE_DATA_POSTGRESQL OFF CACHE BOOL "Disable Poco Data PostgreSQL")


add_subdirectory(external/poco)
add_subdirectory(external/libsamplerate)
add_subdirectory(external/Gist)
add_subdirectory(dpf)

message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")

# add_subdirectory(plugins/WAIVE_Sampler)
add_subdirectory(plugins/WAIVE_Midi)
