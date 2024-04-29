set(CMAKE_CXX_STANDARD 17)

dpf_add_plugin(
  WAIVE_Sampler
  TARGETS 
      jack
      # lv2
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


# Externals:
if(APPLE)
  find_library(FOUNDATION Foundation)
  find_library(COREFOUNDATION CoreFoundation)
elseif(WIN32)
elseif(UNIX AND NOT APPLE)
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


# add_subdirectory(../../external/LibrosaCpp)

    
target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
    ${SNDFILE_LIBRARY}
)
    
target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ../../common
    # ../../external/Gist/src
    ../../external/LibrosaCpp
)

set_target_properties(WAIVE_Sampler PROPERTIES LINK_FLAGS "--libs=sndfile")