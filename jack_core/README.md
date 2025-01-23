# JACK core library

## Requirements

| Dependency  | Optional | License               | Distribute License        | Install Package             | Notes                                                      |
| ----------  | -------- | --------------        | ------------------------- | --------------------------- | ---------------------------------------------------------- |
| cmake       | No       | BSD-3                 | No (we don't distribute)  | `cmake`                     | Build script generator                                     |
| doxygen     | Yes      | GPL v2                | No (we distribute docs)   | `doxygen`                   | Generate JACK docs                                         |
| graphviz    | Yes      | Common Public License | No (we distribute docs)   | `graphviz`                  | Generate graphs for JACK docs                              |
| pdflatex    | Yes      |                       | No (we distribute docs)   | `texlive-latex-recommended` | Tools for building JACK docs PDF from latex                |
|             | Yes      |                       | No (we distribute docs)   | `texlive-latex-extra`       | Tools for building JACK docs PDF from latex                |
|             | Yes      |                       | No (we distribute docs)   | `texlive-fonts-recommended` | Fonts for building JACK docs PDF from latex                |
|             | Yes      |                       | No (we distribute docs)   | `texlive-fonts-recommended` | Fonts for building JACK docs PDF from latex                |
| rust        | Yes      |                       | No (we distribute docs)   |                             | For building jack-make                                     |
| wasm-pack   | Yes      |                       | No (we distribute docs)   | `cargo install wasm-pack`   | For building jack-make WASM module                         |
| golang      | Yes      |                       | No (we distribute docs)   | `golang`                    | Websockets via BoringSSL requires Go for code generation   |
| RTI DDS     | Yes      | Commercial License    | ???                       |                             | For DDS distribution, tested against v5.3.1                |
| libacl      | Yes      |                       |                           | `libacl1-dev`               | Access control for CycloneDDS via iceoryx                  |
| perl        | Yes      |                       |                           |                             | uWebsockets requires perl for code generation via lsquic   |
| OpenGL      |          |                       | No                        | `libgl-dev`                 | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| flac        |          |                       | No                        | `libflac-dev`               | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| freetype    |          |                       | No                        | `libfreetype6-dev`          | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| openal      |          |                       | No                        | `libopenal-dev`             | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| udev        |          |                       | No                        | `libudev-dev`               | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| vorbis      |          |                       | No                        | `libvorbis-dev`             | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| x11         |          |                       | No                        | `libx11-dev`                | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| xrandr      |          |                       | No                        | `libxrandr-dev`             | Required for UNIX systems to build SFML, see [SFML dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) |
| Capstone    | No       |                       |                           |                             | Optional for Linux profiling with Tracy                    |
| Wayland     | No       |                       |                           |                             | Optional for Linux profiling with Tracy                    |
| GLFW        | No       |                       |                           |                             | Optional for Linux profiling with Tracy                    |


**One-liner Dependency Installation**

The following lines of code will install all the dependencies required to build JACK.

```
sudo apt install build-essential cmake golang libacl1-dev libgl-dev libflac-dev libfreetype6-dev libopenal-dev libudev-dev libvorbis-dev libx11-dev libxrandr-dev`

# Install Rust via Rustup
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
cargo install wasm-pack
```

## Build

### Updating JACK Version

Update the version number specified in `projectVersionDetails.cmake`. By
default there is an auto-incrementing number that tracks changes from the last
version.

### Build Options

Some useful options for building JACK are listed below.

- `JACK_ALLOCATION_TRACKING (default OFF, in debug ON)`

    Enable call-site tracking of allocations.

    Upon exit, JACK will dump all of the active allocations, and their locations, that have not been released.

- `JACK_SHARED_MEMORY_DEBUGGING (default OFF)`

    (Linux only) Enable JACK dumping engine data to shared memory.

- `JACK_SLOW_DEBUG_CHECKS (default OFF, in debug ON)`

    Enable slow verification checks of the engine state throughout the code base.
    Use this to vet the engine in applications.

- `JACK_WITH_ASAN (default OFF, in debug ON)`

    Enable address sanitizer, this must be used with a debugged build.

- `JACK_WITH_CYCLONE_DDS (default OFF)`

    Enable CycloneDDS (bus distribution) support in the engine

- `JACK_WITH_GCC_CXX03_ABI (default OFF)`

    Build jack_core using the CXX03 ABI flag (necessary for RTI version <=5.3.1)

- `JACK_WITH_RTI_DDS (default OFF)`

    Enable DDS (bus distribution) support in the engine.
    Note that `CONNEXTDDS_DIR` must be set to the root installation directory of RTI.
    On Linux we check `/opt/rti/rti_connext_dds_5.3.1` if `CONNEXTDDS_DIR` is not set.

- `JACK_WITH_TESTS (default ON)`

    Build with unit/integration test suites.

- `JACK_WITH_WEBSOCKETS (default ON)`

    Enable websocket server in the engine.

- `JACK_WITH_GCC_CXX03_ABI (default OFF)`

    (Linux only) Build with the compile flag `-D_GLIBCXX_USE_CXX11_ABI=0`,
    which compiles the project with C++'s CXX03 ABI.
    Required to be compatible with rti_connext_dds v5.3.1.

- `JACK_RUST_VERSION`

    Use Rust version to build jack-make.

### Build/Package Examples

JACK can be built using CMake presets from `CMakePresets.json`, of which
several default are provided.

```
cmake --list-presets                         # See all the available presets

# Debug build w/ Ninja
cmake --preset debugNinja                    # Configure debug build using Ninja
cmake --build --preset debugNinja --parallel # Debug build using Ninja

# Install ninja build
cmake --install build/debugNinja             # Install to build/debugNinja/install

# Package ninja build into an OS suitable package
cmake --build --preset debugNinjaPackage
```

### Tests

Following the steps above, build the repository in the folder where the build
was generated. Run `ctest` or run the individual tests under the
`build/tests/<test_executable>` folder.

Tests can optionally be run with Valgrind given `ctest -T memcheck`, this is
only available on Linux.

### Documentation

JACK is annotated with doxygen docs and can be generated by executing the
command at the root of the repository.

```bash
doxygen ./doxygen/Doxyfile
```

Graphviz's dot can generate 'call' and other misc graphs by setting the
`HAVE_DOC` variable in the Doxyfile. The generated files will be output to
`./docs`. You may optionally generate a PDF file from latex to
`docs/latex/refman.pdf` by using `make` in the latex directory.

```bash
pushd docs/latex
make
popd
```

### Cross Compilation

We provide cross-compiling from any platform that is able to run the Zig
compiler provided by the Zig CMake toolchain file. To set up a cross-compiled
build, we need to set some CMake parameters.

First define the following CMake variables.

```bash
-D JACK_COMPILER_TARGET=<target...>
-D JACK_ZIG_COMPILER="path/to/zig"
-D CMAKE_SYSTEM_NAME=<system name...>
-D CMAKE_SYSTEM_PROCESSOR=<processor...>
-D CMAKE_TOOLCHAIN_FILE="<path/to/cmake/ZigCrossCompileToolchain.cmake>"
```

The available compiler targets, `JACK_COMPILER_TARGET` can be enumerated by
running `zig targets` under the `libc` section. The common ones you may want to
cross-compile are listed below.

```
x86_64-linux-gnu
x86_64-linux-musl
x86_64-windows-gnu
```

The `CMAKE_SYSTEM_NAME` should be set to the platform you are targeting, e.g.,

```
Windows
Linux
```

The `CMAKE_SYSTEM_PROCESSOR` should be set to the platform's processor architecture you are targeting, e.g.,

```
x86_64
arm64
aarch64
```

The `CMAKE_TOOLCHAIN_FILE` should be set to this repository's 'cmake/ZigCrossCompileToolchain.cmake` file.


#### ARM64 Targets

You can cross compile to a 64-bit ARM architecture from a desktop Linux host
(provided by the aarch64 CMake toolchain file). To set up a cross-compiled
build, we first need to install the following packages on the host PC.

```
gcc-aarch64-linux-gnu
binutils-aarch64-linux-gnu
```

Here is an example of an executed cross-compilation command:

```bash
cmake -S. -Bbuild/aarch64 -DCMAKE_TOOLCHAIN_FILE=toolchains/toolchain-aarch64.cmake
cmake --build build/aarch64 --parallel --verbose --config Debug
```

#### Example

Examples of cross-compilations, executed from the root of this repository, are
provided below. The same commands can be used to cross-compile from Windows by
changing the end-of-line delimiter from `\` to `^` if run in a script', or
alternatively, turn it into a one-liner if run directly on the terminal.

Zig must be available on the path, or otherwise set the absolute path to the compiler in the build command.

```bash
# Cross compile to Windows x64 MinGW
cmake -GNinja -S. -Bbuild/win64-mingw \
      -D CMAKE_TOOLCHAIN_FILE=./cmake/ZigCrossCompileToolchain.cmake -D JACK_ZIG_COMPILER=zig \
      -D JACK_COMPILER_TARGET=x86_64-windows-gnu \
      -D CMAKE_SYSTEM_NAME=Windows \
      -D CMAKE_SYSTEM_PROCESSOR=x86_64
cmake --build build/win64-mingw --parallel --verbose --config Debug

# Cross compile to Linux x64 musl libc
cmake -GNinja -S. -Bbuild/linux64-musl \
      -D CMAKE_TOOLCHAIN_FILE=./cmake/ZigCrossCompileToolchain.cmake -D JACK_ZIG_COMPILER=zig \
      -D JACK_COMPILER_TARGET=x86_64-linux-musl \
      -D CMAKE_SYSTEM_NAME=Linux \
      -D CMAKE_SYSTEM_PROCESSOR=x86_64
cmake --build build/linux64-musl --parallel --verbose --config Debug

# Cross compile to ARM x64 GNU
cmake -GNinja -S. -Bbuild/arm64-gnu \
      -D CMAKE_TOOLCHAIN_FILE=./cmake/ZigCrossCompileToolchain.cmake -D JACK_ZIG_COMPILER=zig \
      -D JACK_COMPILER_TARGET=aarch64-linux-gnu \
      -D CMAKE_SYSTEM_NAME=Linux \
      -D CMAKE_SYSTEM_PROCESSOR=aarch64
cmake --build build/arm64-gnu --parallel --verbose --config Debug
```

#### DDS

Integration tests are available for JACK utilising a DDS bus, which can be
enabled by setting `JACK_WITH_RTI_DDS=ON` or `JACK_WITH_CYCLONE_DDS=ON`. These
tests currently work with both Windows and Linux. DDS is an alternative protocol
and backend for distribution and we support multiple implementations, such as
RTI and Cyclone.

RTI-DDS requires a license file to be set up in the environment variable
`RTI_LICENSE_FILE`. For more options, please refer to the dds-adapter readme
file.

When the RTI-DDS test has been run, it should look like:

```
RTI_LICENSE_FILE=<path/to/rti_license.dat> <path/to/build/tests/busadapters> --gtest_filter=BusAdapters.*DDS
i.e.  RTI_LICENSE_FILE=/home/doyle/rti_license.dat /home/doyle/jack_core/build/tests/busadapters --gtest_filter=BusAdapters.*DDS
```

NOTE: RTI DDS Version 5.3.1 for Linux has been compiled with GCC 4.8.2. This
comes with a CXX03 ABI that requires the `-DJACK_WITH_GCC_CXX03_ABI` to be set
on the CMake build command, for the jack_core to compile and execute
successfully.

#### Bus Testing

JACK supports the testing of both buses simultaneously, provided the bus tests
are invoked correctly. The `busadapter` can run both tests dynamically by
ensuring:

1. JACK is built with DDS support

2. A RTI-DDS license file is provided in one of the previously mentioned
environment variables or config files, if using RTI.

For example

```
RTI_LICENSE_FILE=<path/to/rti_license.dat> <path/to/build/tests/busadapters>
i.e RTI_LICENSE_FILE=/home/doyle/rti_license.dat /home/doyle/jack_core/build/tests/busadapters
```

## Release

Releases can be generated by using CMake's `cpack`. Currently, only Linux is
supported and can be created by running the following command in the root of the
build directory of the configuration you want to create an installer for.

```bash
# Create a debian installer and install using `dpkg` which installs into
# `/opt/aos/`

> jack_core/build/debugBusNinja
cpack -G DEB
sudo dpkg -i _packages/aos_jack_core_0.5.3-debug-static_0.5.3.445_amd64.deb
```

## Profiling via Tracy

On Linux it's recommended to build Tracy from source due to build complications
and various distributions missing libraries or shipping libraries without the
necessary features for Tracy to run.

On Windows Tracy provides a standalone `vcpkg` script at
`vcpkg\install_vcpkg_dependencies.bat` which can build the entire project
portably without modifying the the machine's environment.

**One-liner Dependency Installation**

The following lines of code will install all the dependencies required to build
the Tracy client for profiling on Linux.

```bash
git clone git@gitlab.aosgrp.net:aos/3rd/tracy.git && \
git clone https://github.com/capstone-engine/capstone.git && \
cd capstone && ./make.sh && sudo ./make.sh install && cd .. && \
sudo apt install -y gnome-session-wayland && \
sudo apt update && \
sudo apt install -y libglfw3 libglfw3-dev libdbus-glib-1-dev && \
cd tracy/profiler/build/unix && \
make -j8 LEGACY=1
```

Any further build issues can be clarified in Tracy's official PDF documentation
available at [Tracy Github Releases](https://github.com/wolfpld/tracy/releases).

### Using the profiler

Compile JACK with `-D JACK_WITH_TRACY=ON` set followed by launching the
profiler. The profiler will connect to the JACK instance and transmit profiling
metrics to the UI.

Tracy by default is configured as a sampling profiler. In order to receive
sampling metrics, JACK must be launched with admin permissions in order for
kernel-level performance metrics to be accessible by Tracy.
