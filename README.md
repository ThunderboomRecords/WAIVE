# WAIVE-Plugins

<p align="center">    
AI x Archive music tools
</p>

<p align="center">
    <img 
        src="assets/screenshot.png" 
        style="border-radius: 4px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);"
        width="500"
    >
</p>

A plugin suite that combines music, sound and MIDI generation with European cultural archives. 
Aims to be an offline, modular version of [WAIVE-studio](https://www.waive.studio/) that can be integrated into your DAW.
Built with [DISTRHO Plugin Framework](https://github.com/DISTRHO/DPF) and [ONNX Runtime](https://github.com/microsoft/onnxruntime). 

### Installation
1. Download and extract the zip archive containing the binaries for you system in the [Releases](https://github.com/ThunderboomRecords/WAIVE/releases) page, under the heading "Assets".
    - Current supported platforms: MacOS (Apple silicon) and Linux x86_64.
2. Choose which plugin format you prefer and place it in your plugins path of your DAW. 
    - Current formats: VST2, VST3, CLAP, and a Standalone (JACK) app.
    - Common (system-wide) plugin paths:
      
        |          |  VST2  |  VST3*  |  CLAP |
        |----------|--------|---------|-------|
        | macOS    | `Library/Audio/Plug-ins/VST3` | `Library/Audio/Plug-ins/VST3` | `Library/Audio/Plug-ins/CLAP` |
        | Linux    |  `/usr/lib/vst` | `/usr/lib/vst3` |  `/usr/lib/clap`     |
        | Windows  | `C:\Program Files\Common Files\VST2`  | `C:\Program Files\Common Files\VST3` | `C:\Program Files\Common Files\CLAP`  |
      
       \* for VST3, move the whole `WAIVE_Midi.vst/` folder here. 
   
4. In your DAW, rescan plugins if it does not automatically. 

### Build Instructions
To build WAIVE-Plugins from source.

#### Pre-requisites
Requires statically built onnxruntime for your platform. You can download pre-built libraries from [csukuangfj/onnxruntime-libs](https://huggingface.co/csukuangfj/onnxruntime-libs/tree/main), or build them yourself (such as with [ort-builder](https://github.com/olilarkin/ort-builder/tree/bfbd362c9660fce9600a43732e3f8b53d5fb243a)).
Tested with 1.17.1.

Requires cmake:
- on Mac, with [homebrew](https://brew.sh/): ```$ brew install cmake```
- on Linux: use your distributions package manager
- Windows: *TODO*

#### Linux/macOS
```shell
$ git clone --recursive https://github.com/ThunderboomRecords/WAIVE.git
$ cd WAIVE/

# copy lib/ and include/ from static built onnxruntime into plugins/WAIVE_Midi,
# then from project root:

$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --config Release
```

The plugins are found in ```build/bin``` folder. Move your prefered format binary to your plugins folder (see [instructions](#installation) above).

*TODO:* installation instructions.

#### Windows

*TODO*


