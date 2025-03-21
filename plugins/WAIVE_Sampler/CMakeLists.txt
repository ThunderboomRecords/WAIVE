# Check for Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build detected")

    # Check if LOCAL_SERVER is explicitly enabled
    if(DEFINED ENABLE_LOCAL_SERVER)
        message(STATUS "Enabling LOCAL_SERVER macro")
        add_compile_definitions(LOCAL_SERVER)
    endif()
endif()

if(APPLE)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_MacOS.mm)
elseif(WIN32)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_Windows.cpp)
elseif(UNIX AND NOT APPLE)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_Linux.cpp)
endif()

dpf_add_plugin(
  WAIVE_Sampler
  TARGETS 
      jack
      vst3
      vst2
      clap
      au
  FILES_DSP
      WAIVESampler.cpp
      Tasks.cpp
      SampleDatabase.cpp
      FeatureExtractor.cpp
      Envelopes.cpp
      Filters.cpp
      OSCClient.cpp
      SampleInfo.cpp
      HTTPClient.cpp
      ${CMAKE_SOURCE_DIR}/common/WAIVEUtils.hpp
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ${CMAKE_SOURCE_DIR}/external/Gist/src/Gist.cpp
      ./models/tsne.c
  FILES_UI
      WAIVESamplerUI.cpp
      SamplePlayer.cpp
      ./components/TagMap.cpp
      ./components/TagBrowser.cpp
      ./components/SampleSlot.cpp
      ./components/SourceList.cpp
      ./components/SampleMap.cpp
      ./components/SampleBrowser.cpp
      ${CMAKE_SOURCE_DIR}/common/fonts/fonts.h
      ${CMAKE_SOURCE_DIR}/common/icons/download_icon.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WidgetGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxList.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Box.cpp
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
      ${CMAKE_SOURCE_DIR}/common/src/DragDrop.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Checkbox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Waveform.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DropDown.cpp
      ${CMAKE_SOURCE_DIR}/common/src/TextInput.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEImage.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEWidget.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
      ${CMAKE_SOURCE_DIR}/common/src/RadioButtons.cpp
      ${DRAG_SOURCES}
)

# Externals:
set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a")

if(APPLE)
  target_link_libraries(WAIVE_Sampler PUBLIC "-framework Cocoa")
elseif(WIN32)
  set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/onnxruntime.lib")
  target_link_libraries(WAIVE_Sampler PUBLIC ole32 shell32)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(WAIVE_Sampler PUBLIC LINUX)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
set(OPENSSL_LIBRARIES /usr/local/opt/openssl/lib)

set(BUILD_SHARED_LIBS OFF)
add_compile_definitions(USE_KISS_FFT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
set(BUILD_TESTING OFF CACHE BOOL "Enable libsamplerate tests")

message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

set(LIBSAMPLERATE_EXAMPLES OFF CACHE BOOL "Build libsamplerate examples")
set(LIBSAMPLERATE_STATIC ON CACHE BOOL "Build libsamplerate as static library")

add_subdirectory(${CMAKE_SOURCE_DIR}/external/Gist ${CMAKE_BINARY_DIR}/external/Gist)

link_directories(
  /usr/lib
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(SampleRate CONFIG REQUIRED)
find_package(SndFile CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(kissfft CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)

if(WIN32)
  find_package(Poco CONFIG REQUIRED Foundation NetSSLWin Data DataSQLite)
else()
  find_package(Poco CONFIG REQUIRED Foundation NetSSL Data DataSQLite)
endif()


target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ./models
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/fonts
    ${CMAKE_SOURCE_DIR}/common/icons
    ${CMAKE_SOURCE_DIR}/external/Gist/src
    ${CMAKE_SOURCE_DIR}/external/kd-tree
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
)

target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    kissfft::kissfft-float
    nlohmann_json::nlohmann_json
    tinyfiledialogs::tinyfiledialogs
    PRIVATE
    SndFile::sndfile
    fmt::fmt
    Gist
    Poco::Foundation
    Poco::Data
    Poco::DataSQLite
    SampleRate::samplerate
    ${ONNX_LIB_PATH}
)

if(WIN32)
  target_link_libraries(WAIVE_Sampler PUBLIC Poco::NetSSLWin)
else()
  target_link_libraries(WAIVE_Sampler PUBLIC Poco::NetSSL)
endif()

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")