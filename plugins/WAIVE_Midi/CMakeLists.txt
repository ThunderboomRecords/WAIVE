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
      model_utils.cpp
      ../../common/src/Notes.cpp
      ./models/score_decoder.onnx.c
      ./models/score_encoder.onnx.c
      ./models/groove_decoder.onnx.c
      ./models/groove_encoder.onnx.c
      ./models/full_groove_model.onnx.c
  FILES_UI
      WAIVEMidiUI.cpp
      ../../common/fonts.h
      ../../common/src/HBox.cpp
      ../../common/src/VBox.cpp
      ../../common/src/Notes.cpp
      ../../common/src/Slider.cpp
      ../../common/src/VSlider.cpp
      ../../common/src/Playhead.cpp
      ../../common/src/ScoreGrid.cpp
      ../../common/src/GrooveGraph.cpp
      ../../common/src/DrumPattern.cpp
      ../../common/src/SimpleButton.cpp
)

find_library(FOUNDATION Foundation)
find_library(COREFOUNDATION CoreFoundation)

target_link_libraries(
    WAIVE_Midi 
    PUBLIC 
    ${PROJECT_SOURCE_DIR}/plugins/WAIVE_Midi/lib/libonnxruntime.a
    ${CoreFoundation}
    ${Foundation}
)

target_include_directories(
  WAIVE_Midi
  PUBLIC 
    .
    ./models
    ./include
    ../../common
)
