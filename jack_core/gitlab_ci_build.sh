#!/bin/bash

buildDir="build"
if [ $# -eq 1 ];
then
    buildDir=$1
fi

CMAKE_PATH="/opt/cmake/3.24.2/bin"
NINJA_PATH="/opt/ninja/1.11.1/bin"
TMUX_PATH="/opt/tmux/latest/bin"
RTI_CONNEXT_DDS_PATH="/opt/rti/rti_connext_dds-5.3.1/bin"
export NDDSHOME="/opt/rti/rti_connext_dds-5.3.1"
export RTI_LICENSE_FILE="/opt/rti/rti_connext_dds-5.3.1/rti_license.dat"
QT_PATH="/opt/Qt/5.15.2/gcc_64/bin"

export GOROOT="/opt/go/1.18.3"
export GOPATH=$HOME/go
GOLANG_PATH=$GOPATH/bin:$GOROOT/bin

#Select cuda path or just use
CUDA_PATH="/usr/local/cuda-12.0/bin"
CUDA_LD_LIBRARY_PATH="/usr/local/cuda-12.0/lib64"

RUST_CARGO_PATH="$HOME/.cargo/bin"

CGDB_PATH="/opt/cgdb/bin"

export PATH=${CMAKE_PATH}:${NINJA_PATH}:${QT_PATH}:${GOLANG_PATH}:${RUST_CARGO_PATH}:${TMUX_PATH}:${CGDB_PATH}:${PATH}

uname -a
cmake --version
ninja --version
cargo --version
go version


if ! command -v wasm-pack &> /dev/null
then
    echo "attempting to install wasm-pack"
    cargo install wasm-pack
fi

# ASAN is off because we are very leaky and
# they're non-trivial fixes ATM
# and breaks the testing framework.
echo "~~~ cmake configure ~~~"
cmake \
    -S . \
    -B ${buildDir} \
    -G Ninja \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo \
    -D BUILD_SHARED_LIBS=ON \
    -D CMAKE_INSTALL_PREFIX=install \
    -D JACK_ALLOCATION_TRACKING=OFF \
    -D JACK_SHARED_MEMORY_DEBUGGING=OFF \
    -D JACK_SLOW_DEBUG_CHECKS=ON \
    -D JACK_WITH_ASAN=OFF \
    -D JACK_WITH_CYCLONE_DDS=OFF \
    -D JACK_WITH_GCC_CXX03_ABI=OFF \
    -D JACK_WITH_RTI_DDS=OFF \
    -D JACK_WITH_TESTS=ON \
    -D JACK_WITH_WEBSOCKETS=ON \
    -D JACK_WITH_TRACY=ON \
    -D JACK_WITH_JSON_ADAPTER=ON

if [[ $? != 0 ]]; then
    exit 1;
fi

echo "~~~ cmake build ~~~"
cmake --build ${buildDir}
if [[ $? != 0 ]]; then
    exit 2;
fi

echo "~~~ cmake pack ~~~"
cd ${buildDir}
cpack -G DEB
if [[ $? != 0 ]]; then
    exit 3;
fi

echo "~~~ cmake test ~~~"
cd ${buildDir}
ctest -R . --output-on-failure -j2 --timeout 60
if [[ $? != 0 ]]; then
    exit 4;
fi

