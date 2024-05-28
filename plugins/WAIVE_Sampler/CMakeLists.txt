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
      SampleDatabase.cpp
      Envelopes.cpp
      SimpleUDP.cpp
      Filters.cpp
      ${CMAKE_SOURCE_DIR}/external/tinyosc/tinyosc.c
  FILES_UI
      WAIVESamplerUI.cpp
      ${CMAKE_SOURCE_DIR}/external/tinyfiledialogs-code/tinyfiledialogs.c
      ${CMAKE_SOURCE_DIR}/common/fonts.h
      ${CMAKE_SOURCE_DIR}/common/src/Layout.cpp
      ${CMAKE_SOURCE_DIR}/common/src/WAIVEWidget.cpp
      ${CMAKE_SOURCE_DIR}/common/src/HBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/VBox.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Menu.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Knob3D.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Label.cpp
      ${CMAKE_SOURCE_DIR}/common/src/Waveform.cpp
      ${CMAKE_SOURCE_DIR}/common/src/DropDown.cpp
      ${CMAKE_SOURCE_DIR}/common/src/TextInput.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SampleMap.cpp
      ${CMAKE_SOURCE_DIR}/common/src/SampleSlot.cpp
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

target_include_directories(
  WAIVE_Sampler
  PUBLIC 
    .
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/external/LibrosaCpp/librosa
    ${CMAKE_SOURCE_DIR}/external/libsamplerate/include
    ${CMAKE_SOURCE_DIR}/external/nlohmann_json/single_include
    ${CMAKE_SOURCE_DIR}/external/Gist/src
    ${CMAKE_SOURCE_DIR}/external/kissfft
    ${CMAKE_SOURCE_DIR}/external/kd-tree
    ${CMAKE_SOURCE_DIR}/external/tinyfiledialogs-code
    ${CMAKE_SOURCE_DIR}/external/tinyosc
)

target_link_libraries(
    WAIVE_Sampler
    PUBLIC
    ${CoreFoundation}
    ${Foundation}
    ${SNDFILE_LIBRARY}
    fmt::fmt
    samplerate
    kissfft
    Gist
    PRIVATE
    nlohmann_json::nlohmann_json
    
)

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(WAIVE_Sampler PROPERTIES LINK_FLAGS "--libs=sndfile")