set(CMAKE_SYSTEM_PROCESSOR aarch64)     # Only 64-bit support
set(GCC_COMPILER_VERSION "" CACHE STRING "GCC Compiler version")
set(GNU_MACHINE "aarch64-linux-gnu" CACHE STRING "GNU compiler triple")
set(FLOAT_ABI_SUFFIX "")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

if (NOT "x${GCC_COMPILER_VERSION}" STREQUAL "x")
    set(__GCC_VER_SUFFIX "-${GCC_COMPILER_VERSION}")
endif()

# Specify the cross compiler
find_program(CMAKE_C_COMPILER NAMES ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-gcc${__GCC_VER_SUFFIX})
if (NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "C Compiler ${CMAKE_C_COMPILER} not found")
endif()

find_program(CMAKE_CXX_COMPILER NAMES ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-g++${__GCC_VER_SUFFIX})
if (NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "CXX Compiler ${CMAKE_CXX_COMPILER} not found")
endif()

find_program(CMAKE_LINKER NAMES ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-ld${__GCC_VER_SUFFIX} ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-ld)
find_program(CMAKE_AR NAMES ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-ar${__GCC_VER_SUFFIX} ${GNU_MACHINE}${FLOAT_ABI_SUFFIX}-ar)

if (NOT DEFINED ARM_LINUX_SYSROOT AND DEFINED GNU_MACHINE)
  set(ARM_LINUX_SYSROOT /usr/${GNU_MACHINE}${FLOAT_ABI_SUFFIX})
endif()

if (NOT DEFINED CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS           "" CACHE INTERNAL "")
    set(CMAKE_C_FLAGS             "" CACHE INTERNAL "")
    set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "")
    set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "")
    set(CMAKE_EXE_LINKER_FLAGS    "" CACHE INTERNAL "")

    set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -fdata-sections -Wa,--noexecstack -fsigned-char -Wno-psabi")
    set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -fdata-sections -Wa,--noexecstack -fsigned-char -Wno-psabi")

    # Note: These linking flags aren't used because we run into linking issues for the RPi, Resorting to
    # default compiler linker flags for now. These are here only as a reference
    set(ARM_LINKER_FLAGS    "-Wl,--no-undefined -Wl,--gc-sections -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now")
endif()

set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} ${ARM_LINUX_SYSROOT})

# Search for programs only in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
