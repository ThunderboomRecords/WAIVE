set(CMAKE_CXX_STANDARD 17)

dpf_add_plugin(
  WAIVE_Sampler
  TARGETS 
      jack
    #   lv2
      vst3
      vst2
      clap
  FILES_DSP
      WAIVESampler.cpp
      SampleDatabase.cpp
      FeatureExtractor.cpp
      Envelopes.cpp
      Filters.cpp
      OSCClient.cpp
      HTTPClient.cpp
      ${CMAKE_SOURCE_DIR}/common/WAIVEUtils.hpp
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ${CMAKE_SOURCE_DIR}/external/Gist/src/Gist.cpp
      ${CMAKE_SOURCE_DIR}/external/Gist/libs/kiss_fft130/kiss_fft.c
      ./models/tsne.c
  FILES_UI
      WAIVESamplerUI.cpp
      ./components/TagMap.cpp
      ./components/TagBrowser.cpp
      ./components/SampleSlot.cpp
      ./components/SourceList.cpp
      ./components/SampleMap.cpp
      ./components/SampleBrowser.cpp
      ./components/SampleBrowserRoot.cpp
      ${CMAKE_SOURCE_DIR}/external/libtinyfiledialogs/tinyfiledialogs.c
      ${CMAKE_SOURCE_DIR}/common/fonts/fonts.h
      ${CMAKE_SOURCE_DIR}/common/icons/download_icon.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/GridLayout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WidgetGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxList.cpp
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Link.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Menu.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Icon.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob3D.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Label.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Popup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Panel.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Spinner.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Checkbox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Waveform.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DropDown.cpp
      ${CMAKE_SOURCE_DIR}/common/src/TextInput.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEImage.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEWidget.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
      ${CMAKE_SOURCE_DIR}/common/src/ValueIndicator.cpp
)


# Externals:
if(APPLE)
  find_library(FOUNDATION Foundation)
  find_library(COREFOUNDATION CoreFoundation)
elseif(WIN32)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(WAIVE_Sampler PUBLIC LINUX)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
    ${CMAKE_SOURCE_DIR}/external/Gist/libs/kiss_fft130
)

set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
set(OPENSSL_LIBRARIES /usr/local/opt/openssl/lib)

set(BUILD_SHARED_LIBS OFF)
set(POCO_STATIC ON)
add_compile_options(-DUSE_KISS_FFT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
set(BUILD_TESTING OFF CACHE BOOL "Enable libsamplerate tests")

message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

set(LIBSAMPLERATE_EXAMPLES OFF CACHE BOOL "Build libsamplerate examples")
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

add_subdirectory(${CMAKE_SOURCE_DIR}/external/poco ${CMAKE_BINARY_DIR}/external/poco)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/libsamplerate ${CMAKE_BINARY_DIR}/external/libsamplerate)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/Gist ${CMAKE_BINARY_DIR}/external/Gist)

message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

link_directories(
  /usr/lib
)
find_library(SNDFILE_LIBRARY NAMES sndfile)
if(SNDFILE_LIBRARY)
message(STATUS "Sndfile library found: ${SNDFILE_LIBRARY}")
else()
message(FATAL_ERROR "Sndfile library not found")
endif()

find_package(fmt REQUIRED)

target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ./models
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/fonts
    ${CMAKE_SOURCE_DIR}/common/icons
    ${CMAKE_SOURCE_DIR}/external/libsamplerate/include
    ${CMAKE_SOURCE_DIR}/external/json/single_include
    ${CMAKE_SOURCE_DIR}/external/Gist/src
    ${CMAKE_SOURCE_DIR}/external/kd-tree
    ${CMAKE_SOURCE_DIR}/external/libtinyfiledialogs
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
    ${CMAKE_SOURCE_DIR}/external/cpp-httplib
)

target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
    ${SNDFILE_LIBRARY}
    fmt::fmt
    Poco::Foundation
    Poco::Data
    Poco::DataSQLite
    Poco::NetSSL
    Poco::Net
    Poco::Crypto
    samplerate
    Gist
    PRIVATE
    ${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a
)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(WAIVE_Sampler PROPERTIES LINK_FLAGS "--libs=sndfile")