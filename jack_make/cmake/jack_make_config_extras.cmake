#Hand crafted assistance
# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

set(_path_to_look_in "${_IMPORT_PREFIX}/lib/jack-make")
find_program(JACK_MAKE_EXE
    NAMES "jack-make"
    PATHS "${_path_to_look_in}"
    NO_DEFAULT_PATH
)

if(NOT JACK_MAKE_EXE)
    message(FATAL_ERROR "Could not find program 'jack-make' using suggestion ${_path_to_look_in} ")
else()
    message(STATUS "Found JACK_MAKE_EXE=${JACK_MAKE_EXE}")
endif()

