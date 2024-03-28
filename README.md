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
Built with [DPF](https://github.com/DISTRHO/DPF) and [PyTorch](https://pytorch.org/).


### Build Instructions
To build WAIVE-Plugins from source.


#### Pre-requisite: build static libtorch libraries
Requires cmake.
- On macOS (with [homebrew](https://brew.sh/)): ```$ brew install cmake libomp python```

Requires python packages ```pyyaml```, ```typing-extensions```:
- on Linux: ```$ pip install pyyaml```
- on macOS: ```$ brew install pyyaml python-typing-extensions```

```shell
$ git clone --recursive https://github.com/pytorch/pytorch.git
$ mkdir libtorch-build
$ cd libtorch-build/
$ cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_PYTHON:BOOL=OFF \
    -DCMAKE_BUILD_TYPE:STRING=Release -DUSE_CUDA:BOOL=OFF \
    -DCMAKE_CXX_FLAGS:STRING=-fPIC \
    -DCMAKE_INSTALL_PREFIX:PATH=../libtorch ../pytorch
$ cmake --build . --target install -j8
```


#### Linux/macOS
```shell
$ git clone --recursive https://github.com/ThunderboomRecords/WAIVE.git
$ cd WAIVE/
$ mkdir build
$ cd build

$ cmake -DCMAKE_PREFIX_PATH=/absolute/path/to/static/libtorch ..

$ cmake --build . --config Release
```

The plugins are found in ```build/bin``` folder. *TODO:* installation instructions.

#### Windows

*TODO*

#### MacOS (not complete)



```bash
$ mkdir build
$ cd build

# download and extract libtorch from https://pytorch.org/get-started/locally/
# (download arm64 for M1/M2 chips)
$ cmake -DCMAKE_PREFIX_PATH=/absolute/path/to/libtorch ..

$ cmake --build . --config Release

$ sudo cp /opt/homebrew/Cellar/libomp/10.1.2/lib/libomp.dylib /usr/local/lib
```

