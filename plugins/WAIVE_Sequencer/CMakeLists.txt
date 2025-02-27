if(APPLE)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_MacOS.mm)
elseif(WIN32)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_Windows.cpp)
elseif(UNIX AND NOT APPLE)
  set(DRAG_SOURCES ${CMAKE_SOURCE_DIR}/common/src/DragSource_Linux.cpp)
endif()

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
      ${DRAG_SOURCES}
)

# Externals:
set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a")

if(APPLE)
  target_link_libraries(WAIVE_Sequencer PUBLIC "-framework Cocoa")
elseif(WIN32)
  set(ONNX_LIB_PATH "${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/onnxruntime.lib")
  target_link_libraries(WAIVE_Sequencer PUBLIC ole32 shell32)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(WAIVE_Sequencer PUBLIC LINUX)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}/generated/
)

find_package(fmt CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation)

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
    Poco::Foundation
    ${ONNX_LIB_PATH}
    # ${CoreFoundation}
    # ${Foundation}
)

