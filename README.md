# WAIVE-Plugins

<p align="center">
    <img src="assets/logo.png">
</p>

<p align="center">    
AI x Archive music tools
</p>

A plugin suite that combines music, sound and MIDI generation with European cultural archives. 
Aims to be an offline, modular version of [WAIVE-studio](https://www.waive.studio/) that can be integrated into your DAW.
Built with [DPF](https://github.com/DISTRHO/DPF) and [PyTorch](https://pytorch.org/).

> :construction: Currently in _very_ early development stage! :construction:

See [al165/DPF_Tests](https://github.com/al165/DPF_Tests) for a proof-of-concept test version called "AITests". 


### Build Instructions
To build WAIVE-Plugins from source.

```bash
$ git clone --recursive https://github.com/ThunderboomRecords/WAIVE.git
```

#### Linux
Requires cmake

```bash
$ mkdir build
$ cd build

# if installed PyTorch with pip:
$ cmake -DCMAKE_PREFIX_PATH=`python3 -c 'import torch;print(torch.utils.cmake_prefix_path)'` ..
# or, if causing issues, download and extract libtorch from https://pytorch.org/get-started/locally/:
$ cmake -DCMAKE_PREFIX_PATH=/absolute/path/to/libtorch ..

$ cmake --build . --config Release
```

The plugins are found in ```build/bin``` folder. *TODO:* installation instructions.

#### Windows

*TODO*

#### MacOS

*TODO*