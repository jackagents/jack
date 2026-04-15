cmake_minimum_required(VERSION 3.22.1)

# use vcpkg package manager
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")

project(jack LANGUAGES C CXX)

enable_testing()

# Used for creating a ROS 2 package
option(USE_AMENT "Use ament build system" OFF)

add_subdirectory(jack_core)
add_subdirectory(examples)
