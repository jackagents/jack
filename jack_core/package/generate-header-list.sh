#!/bin/bash

# Generate a list of JACK headers used by a program when you include the <jack>
# super header and dump it to standard output.
#
# This script should not have any dependencies except the JACK repository itself
# having been cloned.
#
# ./jack-generate-header-list

script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")

arg_qt_logger=0
for arg in "$@"
do
    if [ "$arg" = "qt-logger" ]; then
        arg_qt_logger=1
    fi
done

# Make simple CPP program to include the JACK super header
mkdir --parents build/jack-core-header-usage
pushd build/jack-core-header-usage &> /dev/null
cat >main.cpp <<EOL
#include <jack>
int main(int, char **) { return 0; }
EOL

extra_flags=""

error_exit()
{
    echo "[Error] $1" 1>&2
    exit 1
}

# Generate a pre-processed file using the JACK super header
g++ \
    -std=c++11 \
    -E ${extra_flags} \
    -I ${script_dir}/src \
    -I ${script_dir}/3rd/flatbuffers/include \
    -I ${script_dir}/event-protocol/src \
    -I ${script_dir}/event-protocol/3rd/concurrentqueue \
    -I ${script_dir}/3rd/aos-log/src \
    -o main.i \
    main.cpp || error_exit "Failed to build JACK super header program"

# Analyse the super header to filter out all jack headers used
cat main.i | grep jack_core | cut -d'"' -f 2 | sort | uniq

popd &> /dev/null
