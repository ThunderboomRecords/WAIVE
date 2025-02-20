dpf_add_plugin(
  WAIVE_Sequencer
  TARGETS 
      jack
      vst3
      vst2
      clap
      au
  FILES_DSP
      WAIVESequencer.cpp
      ./components/Notes.cpp
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ./models/score_decoder.onnx.c
      ./models/score_encoder.onnx.c
      ./models/groove_decoder.onnx.c
      ./models/groove_encoder.onnx.c
      ./models/full_groove_model.onnx.c
  FILES_UI
      WAIVESequencerUI.cpp
      ./components/Notes.cpp
      ./components/ScoreGrid.cpp
      ./components/GrooveGraph.cpp
      ./components/DrumPattern.cpp
      ${CMAKE_SOURCE_DIR}/common/fonts/fonts.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Box.cpp
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Icon.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Menu.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Label.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Panel.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Playhead.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DropDown.cpp
      ${CMAKE_SOURCE_DIR}/common/src/TextInput.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEImage.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEWidget.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WidgetGroup.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
)

# Externals:
set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a")

if(APPLE)
  find_library(FOUNDATION Foundation)
  find_library(COREFOUNDATION CoreFoundation)
elseif(WIN32)
  set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/onnxruntime.lib")
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(WAIVE_Sampler PUBLIC LINUX)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

link_directories(
  /usr/lib
)

find_package(fmt CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)

target_include_directories(
  WAIVE_Sequencer
  PUBLIC 
    .
    ./models
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/fonts
    ${CMAKE_SOURCE_DIR}/common/icons
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
)

target_link_libraries(
    WAIVE_Sequencer 
    PUBLIC 
    tinyfiledialogs::tinyfiledialogs
    fmt::fmt
    PRIVATE
    ${ONNX_LIB_PATH}
    ${CoreFoundation}
    ${Foundation}
)

