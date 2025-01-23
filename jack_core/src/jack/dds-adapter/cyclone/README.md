# jack-cyc-dds-adapter

A sample implementation of an adapter for communicating on the JACK
distribution network protocol utilising the RTI implementation of DDS.

## Setup

The adapter uses the CycloneDDS C and CXX libraries that must be installed as
pre-requisite before building with this adapter.

### Windows

**Static builds are not supported on Windows yet**. Static builds require
investigating what the `security_core` means for CycloneDDS as we will need to
vendor OpenSSL for this.

### CycloneDDS Prerequisites

The following command builds and installs CycloneDDS C and CXX libraries. On
Windows, prefer to install to a specific directory like `build/install` as there
are no standard paths for libraries. On Linux and alike, there's a standard
installation path and this step can be omitted if desired. However, when
cross-building for `aarch64` use a suitable toolchain file and specify an
install directory for the ARM64 architecture (e.g. /usr/aarch64-linux-gnu).

Some examples of building various variants of CycloneDDS pre-requisites are
provided below.

**Build Cyclone & CycloneDDS-CXX and install to default directory**

```bash
cd CycloneDDS
cmake -Bbuild -GNinja -S.
cmake --build build --parallel --verbose --config RelWithDebInfo --target install

cd CycloneDDS-CXX
cmake -Bbuild -GNinja -S.
cmake --build build --parallel --verbose --config RelWithDebInfo --target install
```

**Build CycloneDDS & CycloneDDS-CXX and install to build/install**

```bash
cd CycloneDDS
cmake -D CMAKE_INSTALL_PREFIX=build/install -Bbuild -GNinja -S.
cmake --build build --parallel --verbose --config RelWithDebInfo --target install

cd CycloneDDS-CXX
cmake -D CMAKE_INSTALL_PREFIX=build/install -Bbuild -GNinja -S. -D CMAKE_PREFIX_PATH=<path/to/cyclonedds/install_dir>
cmake --build build --parallel --verbose --config RelWithDebInfo --target install
```

**Build CycloneDDS & CycloneDDS-CXX for ARM64 and install to architecture specific directory**

```bash
cd CycloneDDS
cmake -D CMAKE_INSTALL_PREFIX=/usr/aarch64-linux-gnu -Bbuild -GNinja -S. --toolchain <path/to/toolchain/toolchain-aarch64.cmake>
cmake --build build --parallel --verbose --config RelWithDebInfo --target install

cd CycloneDDS-CXX
cmake -D CMAKE_INSTALL_PREFIX=/usr/aarch64-linux-gnu -Bbuild -GNinja -S. --toolchain <path/to/toolchain/toolchain-aarch64.cmake>
cmake --build build --parallel --verbose --config RelWithDebInfo --target install
```

## Build

This repository exposes a CMake target `jack-cyc-dds-adapter` that is available
after an `add_subdirectory(cyc-dds-adapter)` command in CMake. The target can be
linked to using the
`target_link_libraries(<your library> PRIVATE jack-cyc-dds-adapter)` command.

If CycloneDDS and CycleonDDS-CXX were installed to a custom path, you must
make available the paths to the install location to CMake by setting
`CMAKE_PREFIX_PATH` in the configure step.

```bash
-D CMAKE_PREFIX_PATH=<path/to/cyclonedds-cxx/install_dir/lib/cmake>;<path/to/cyclonedds/install_dir/lib/cmake>
```

## Licensing

Cyclone DDS uses the Eclipse Public License v2.0. We will need to look into what
responsibilities we are required to give back by linking to this library.

## Compatibility with RTI DDS and merging Adapters

RTI DDS *should* be compatible at the protocol level.

As such they should be able to communicate when both enabled.

Merging the adapters could be possible, but would require A LOT of #ifdefs to
deal with the difference in the code generated types that are output by the RTI
and Cyclone DDS IDL compilers.


