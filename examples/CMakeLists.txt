cmake_minimum_required(VERSION 3.22)
project(jack_examples)

if(NOT TARGET jack_core)
    message(FATAL_ERROR "jack_core target not found - build jack_core first")
endif()

add_subdirectory(gol)
add_subdirectory(simple)

