# ONNX Runtime Libs

Requires statically built onnxruntime for your platform to be downloaded and placed here. You can download pre-built libraries from [csukuangfj/onnxruntime-libs](https://huggingface.co/csukuangfj/onnxruntime-libs/tree/main), or build them yourself (such as with [ort-builder](https://github.com/olilarkin/ort-builder/tree/bfbd362c9660fce9600a43732e3f8b53d5fb243a)).
Tested with 1.17.1.
- Mac: [onnxruntime-osx-universal2-1.17.3.tgz](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-osx-universal2-1.17.3.tgz)
- Linux: [onnxruntime-linux-x64-static_lib-1.17.0.zip](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-linux-x64-static_lib-1.17.0.zip)
- Windows: [onnxruntime-win-x64-static_lib-1.17.1.tar.bz2](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-win-x64-static_lib-1.17.1.tar.bz2)

This directory should have this folder structure:
```
...
external/
    ...
    onnxruntime/
        include/
            ...
        lib/
            ...
        README.md
...
```