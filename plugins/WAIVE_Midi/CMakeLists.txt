set(CMAKE_CXX_STANDARD 17)

dpf_add_plugin(
  WAIVE_Midi
  TARGETS 
      jack
      vst3
      vst2
      clap
      au
  FILES_DSP
      WAIVEMidi.cpp
      ./components/Notes.cpp
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ./models/score_decoder.onnx.c
      ./models/score_encoder.onnx.c
      ./models/groove_decoder.onnx.c
      ./models/groove_encoder.onnx.c
      ./models/full_groove_model.onnx.c
  FILES_UI
      WAIVEMidiUI.cpp
      ./components/Notes.cpp
      ./components/ScoreGrid.cpp
      ./components/GrooveGraph.cpp
      ./components/DrumPattern.cpp
      ${CMAKE_SOURCE_DIR}/common/fonts/fonts.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Menu.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Label.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Panel.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Playhead.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DropDown.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEWidget.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WidgetGroup.cpp
)

find_library(FOUNDATION Foundation)
find_library(COREFOUNDATION CoreFoundation)

target_link_libraries(
    WAIVE_Midi 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/external/onnxruntime/lib/libonnxruntime.a
    ${CoreFoundation}
    ${Foundation}
)

target_include_directories(
  WAIVE_Midi
  PUBLIC 
    .
    ./models
    ./components
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/fonts
    ${CMAKE_SOURCE_DIR}/common/icons
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
)
