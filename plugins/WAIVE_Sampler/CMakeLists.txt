set(CMAKE_CXX_STANDARD 17)

dpf_add_plugin(
  WAIVE_Sampler
  TARGETS 
    #   jack
      vst3
      vst2
      clap
  FILES_DSP
      WAIVESampler.cpp
  FILES_UI
      WAIVESamplerUI.cpp
      ../../common/fonts.h
      ../../common/src/HBox.cpp
      ../../common/src/VBox.cpp
      ../../common/src/Notes.cpp
      ../../common/src/Slider.cpp
      ../../common/src/VSlider.cpp
      ../../common/src/Playhead.cpp
      ../../common/src/SimpleButton.cpp
)

find_library(FOUNDATION Foundation)
find_library(COREFOUNDATION CoreFoundation)

target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
)

target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ../../common
)
