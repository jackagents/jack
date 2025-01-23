# NOTE: You must define the following CMake variables to use this file, for
# example, to cross-compile to Windows. See zig -targets under "libc" to see
# more potential targets.
#
# -D JACK_COMPILER_TARGET=x86_64-windows-gnu
# -D JACK_ZIG_COMPILER="path/to/zig.exe"
# -D CMAKE_SYSTEM_NAME=Windows
# -D CMAKE_SYSTEM_PROCESSOR=x86_64
# -D CMAKE_TOOLCHAIN_FILE="<path/to/ZigCrossCompileToolchain.cmake>"

# TODO: The CXX/CC toolchain doesn't like it when the compiler is invoked like
#
# 'zig c++' i.e. that space breaks a bunch of things.
#
# And so forth for 'zig [cc|ar|ranlib]' etc. We make some wrapper scripts to
# avoid this problem in the build directory
#
# zig-ar.[sh|bat]
# zig-cc.[sh|bat]
# zig-c++.[sh|bat]
#
# This will eventually be resolved by zig-toolchain which will generate the
# scripts for us, see:
#
# https://github.com/ziglang/zig/issues/8973

if (WIN32)
    set(CMAKE_AR ${CMAKE_CURRENT_BINARY_DIR}/zig-ar.bat CACHE FILEPATH "Archiver")
    set(CMAKE_C_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/zig-cc.bat CACHE FILEPATH "C Compiler")
    set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/zig-c++.bat CACHE FILEPATH "CXX Compiler")

    file(WRITE ${CMAKE_AR} "@echo off\n\"${JACK_ZIG_COMPILER}\" ar %*")
    file(WRITE ${CMAKE_C_COMPILER} "@echo off\n\"${JACK_ZIG_COMPILER}\" cc %*")
    file(WRITE ${CMAKE_CXX_COMPILER} "@echo off\n\"${JACK_ZIG_COMPILER}\" c++ %*")
else()
    # When we create scripts on the fly we need to set execute permissions for
    # Linux. We do this by writing the script temporarily to a temp directory
    # then use CMake's OS agnostic copy function that allows copying and setting
    # the execute bit on the script.
    #
    # Also note that CMake's copy command only takes a directory, so we can't
    # just copy the script onto-itself with the execute bit set, so that's why
    # we have to dump to a temp directory before writing it to the proper
    # directory.

    # Setup variables for the temporary script locations
    set(TMP__AR ${CMAKE_CURRENT_BINARY_DIR}/tmp_scripts/zig-ar.sh)
    set(TMP__C_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/tmp_scripts/zig-cc.sh)
    set(TMP__CXX_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/tmp_scripts/zig-c++.sh)

    # Generate the temporary scripts
    file(WRITE ${TMP__AR} "#!/bin/bash\n\"${JACK_ZIG_COMPILER}\" ar $@")
    file(WRITE ${TMP__C_COMPILER} "#!/bin/bash\n\"${JACK_ZIG_COMPILER}\" cc $@")
    file(WRITE ${TMP__CXX_COMPILER} "#!/bin/bash\n\"${JACK_ZIG_COMPILER}\" c++ $@")

    # Copy the scripts now with the execute bit set
    file(COPY ${TMP__AR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    file(COPY ${TMP__C_COMPILER} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
    file(COPY ${TMP__CXX_COMPILER} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

    # Setup CMake variables
    set(CMAKE_AR ${CMAKE_CURRENT_BINARY_DIR}/zig-ar.sh CACHE FILEPATH "Archiver")
    set(CMAKE_C_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/zig-cc.sh CACHE FILEPATH "C Compiler")
    set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_BINARY_DIR}/zig-c++.sh CACHE FILEPATH "CXX Compiler")

endif()

set(CMAKE_C_COMPILER_TARGET ${JACK_COMPILER_TARGET})
set(CMAKE_CXX_COMPILER_TARGET ${JACK_COMPILER_TARGET})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
