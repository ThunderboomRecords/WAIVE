# Build Instructions (for developers)

If you want to build WAIVE-Plugins from source instead of using the provided installers/binaries, follow the instructions below.

## Pre-requisites

Clone this repo

```sh
git clone --recursive https://github.com/ThunderboomRecords/WAIVE.git
cd WAIVE/
```

Requires statically built `onnxruntime` for your platform. You can download pre-built libraries from [csukuangfj/onnxruntime-libs](https://huggingface.co/csukuangfj/onnxruntime-libs/tree/main), or build them yourself (such as with [ort-builder](https://github.com/olilarkin/ort-builder/tree/bfbd362c9660fce9600a43732e3f8b53d5fb243a)).
Tested with 1.17.3.

- Mac: [onnxruntime-osx-universal2-static_lib-1.17.3.zip](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-osx-universal2-static_lib-1.17.3.zip)
- Linux: [onnxruntime-linux-x64-static_lib-1.17.3.zip](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-linux-x64-static_lib-1.17.0.zip)
- Windows: [onnxruntime-win-x64-static_lib-1.17.3.tar.bz2](https://huggingface.co/csukuangfj/onnxruntime-libs/blob/main/onnxruntime-win-x64-static_lib-1.17.1.tar.bz2)

Download and extract the folders `lib/` and `include/` from the above link for your platform into this repo's `external/onnxruntime` directory.

Mac and Linux: requires `cmake` and `ninja`:

- on Mac, with [homebrew](https://brew.sh/): `$ brew install cmake ninja`
- on Linux: use your distributions package manager

Windows: requires [Visual Studio 2022](https://visualstudio.microsoft.com/):

- install "Desktop development with C++" for CMake support

Requires `vcpkg`:

- on Mac:

  ```sh
  git clone https://github.com/microsoft/vcpkg "$HOME/vcpkg"
  export VCPKG_ROOT="$HOME/vcpkg"
  ```

  (you may wish to add the last line to your .bashrc or .zshrc to make it permanent)

- on Windows/Linux: [vcpkg installation instructions](https://learn.microsoft.com/en-gb/vcpkg/get_started/get-started?pivots=shell-cmd)

To update `vcpkg`:

```sh
cd $VCPKG_ROOT
git pull
./bootstrap-vcpkg.sh -disableMetrics
```

## Build

### macOS

This creates a universal build that targets both Apple silicon and older Intel CPUs.

```sh
cmake --preset default -B build-arm64 -DCMAKE_BUILD_TYPE=Release
cmake --preset default -B build-x86_64 -DCMAKE_BUILD_TYPE=Release -DTARGET_MAC_X86=1
cmake --build build-arm64 -j8 && cmake --build build-x86_64 -j8

cd tools/
./macOS_make_universal.sh ../build-arm64/bin ../build-x86_64/bin ../build-universal/bin
```

### Linux

```sh
cmake --preset="default" -DCMAKE_BUILD_TYPE="Release"
cmake --build ./build -j8 --config Release
```

### Windows

Run the build commands in Developer Command Prompt (not Powershell!).

```sh
C:\path\to\WAIVE>cmake --preset="windows" -A x64
C:\path\to\WAIVE>cmake --build build -j8 --config Release
```

## Post build

The plugin builds are found in `build/bin/` folder.
Move your preferred format binary to your plugins folder for manual installation.

To create installers:

### macOS

Create a file `notarization_secrets.env` file in `tools/` with your Apple Developer details (this file is ignored by git):

```
export APPLE_ID="your@email.com"
export DEVELOPER_NAME="Your Name"
export TEAM_ID="1AB2CD34EF"
export APP_SPECIFIC_PASSWORD="abcd-efgh-ijkl-mnop"
export BUNDLE_ID="com.example.plugin"
```

The run:

```bash
cd tools
./macOS_sign_and_notarize.sh ../build-universal/bin/ WAIVE-Plugins
./macOS_make_installer.sh ../build-universal/bin WAIVE-Plugins
```

The installer `.dmg` is found in the `release/vX.Y.Z/macOS_ARCH/` folder.

### Windows

Download and install [INNO Setup](https://jrsoftware.org/isinfo.php).
Launch it, open `tools\windows_installer.iss` and click `Compile`.
The `.exe` installer can be found in `release\vX.Y.Z\win64\`.

### Linux

Create an archive with all the compiled binaries:

```bash
cd tools
./linux_package_plugins.sh ../build/bin WAIVE-Plugins
```
