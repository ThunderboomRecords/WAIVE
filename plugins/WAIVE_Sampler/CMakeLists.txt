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
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ${CMAKE_SOURCE_DIR}/external/Gist/src/Gist.cpp
      ${CMAKE_SOURCE_DIR}/external/Gist/libs/kiss_fft130/kiss_fft.c
      ./models/tsne.c
  FILES_UI
      WAIVESamplerUI.cpp
      ./components/SourceList.cpp
      ./components/SourceBrowser.cpp
      ./components/SourceBrowserRoot.cpp
      ${CMAKE_SOURCE_DIR}/external/libtinyfiledialogs/tinyfiledialogs.c
      ${CMAKE_SOURCE_DIR}/common/fonts.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/GridLayout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WidgetGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/CheckboxList.cpp
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Menu.cpp
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
      ${CMAKE_SOURCE_DIR}/common/src/SampleMap.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SampleSlot.cpp
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

find_package(Poco REQUIRED COMPONENTS Net Foundation DataSQLite Data)

target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ./models
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/external/libsamplerate/include
    ${CMAKE_SOURCE_DIR}/external/json/single_include
    ${CMAKE_SOURCE_DIR}/external/Gist/src
    ${CMAKE_SOURCE_DIR}/external/kd-tree
    ${CMAKE_SOURCE_DIR}/external/libtinyfiledialogs
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
    ${CMAKE_SOURCE_DIR}/external/cpp-httplib
    ${Poco_INCLUDE_DIRS}
)

target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
    ${SNDFILE_LIBRARY}
    fmt::fmt
    ${Poco_LIBRARIES}
    samplerate
    Gist
    PRIVATE
    ${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a
)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(WAIVE_Sampler PROPERTIES LINK_FLAGS "--libs=sndfile")