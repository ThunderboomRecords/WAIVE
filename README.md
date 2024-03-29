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
Built with [DPF](https://github.com/DISTRHO/DPF) and [ONNX Runtime](https://github.com/microsoft/onnxruntime)


### Build Instructions
To build WAIVE-Plugins from source.

#### Pre-requisites
Requires statically built onnxruntime for your platform. You can download pre-built libraries from [csukuangfj/onnxruntime-libs](https://huggingface.co/csukuangfj/onnxruntime-libs/tree/main), or build them yourself (such as with [ort-builder](https://github.com/olilarkin/ort-builder/tree/bfbd362c9660fce9600a43732e3f8b53d5fb243a))

#### Linux/macOS
```shell
$ git clone --recursive https://github.com/ThunderboomRecords/WAIVE.git
$ cd WAIVE/

# copy lib/ and include/ from static built onnxruntime, then:

$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --config Release
```

The plugins are found in ```build/bin``` folder. *TODO:* installation instructions.

#### Windows

*TODO*


