set(CMAKE_CXX_STANDARD 17)

dpf_add_plugin(
  WAIVE_Midi
  TARGETS 
      jack
      vst3
      vst2
      clap
  FILES_DSP
      WAIVEMidi.cpp
      ${CMAKE_SOURCE_DIR}/common/src/model_utils.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Notes.cpp
      ./models/score_decoder.onnx.c
      ./models/score_encoder.onnx.c
      ./models/groove_decoder.onnx.c
      ./models/groove_encoder.onnx.c
      ./models/full_groove_model.onnx.c
  FILES_UI
      WAIVEMidiUI.cpp
      ${CMAKE_SOURCE_DIR}/common/fonts.h
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Notes.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Slider.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VSlider.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Playhead.cpp
      ${CMAKE_SOURCE_DIR}/common/src/ScoreGrid.cpp
      ${CMAKE_SOURCE_DIR}/common/src/GrooveGraph.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DrumPattern.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SimpleButton.cpp
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
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/external/onnxruntime/include
)
