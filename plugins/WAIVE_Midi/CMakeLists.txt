dpf_add_plugin(
  WAIVE_MIDI
  TARGETS 
      # jack
      lv2
      vst3
      vst2
      clap
  FILES_DSP
      WAIVE_MIDI.cpp
  FILES_UI
      WAIVE_MIDIUI.cpp
)

target_include_directories(
  WAIVE_MIDI 
  PUBLIC 
    .
    widgets
    models
)