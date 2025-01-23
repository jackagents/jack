cmake_minimum_required(VERSION 3.22)
project(gol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(jack_make REQUIRED)
find_package(jack_core REQUIRED)

# SFML fetch based on https://github.com/SFML/cmake-sfml-project

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    SYSTEM)
FetchContent_MakeAvailable(SFML)

if (EXISTS ${JACK_MAKE_EXE})
    set(MODEL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/gol.mod.jack.json)
    set(JACK_MAKE_ARGS --skip-validation --output-dir ${CMAKE_CURRENT_SOURCE_DIR}/.. ${MODEL_FILE})
    execute_process(COMMAND ${JACK_MAKE_EXE} ${JACK_MAKE_ARGS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE JACK_MAKE_RESULT
	)

    if (JACK_MAKE_RESULT)
        message(FATAL_ERROR "jack-make failed to generate meta files for ${MODEL_FILE}, result - ${JACK_MAKE_GENERATED_RESULT}")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MODEL_FILE})
else()
    message(FATAL_ERROR "jack-make not found!")
endif()

include (${CMAKE_CURRENT_SOURCE_DIR}/meta/gol.cmake RESULT_VARIABLE includeResult)

if (NOT includeResult)
    message(FATAL_ERROR "meta/gol.cmake not found, jack-make gen may have failed")
endif()

add_executable(golapp
    ${JACK_MAKE_GENERATED_HEADERS_GOL}
    ${JACK_MAKE_GENERATED_SOURCES_GOL}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gridworld.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gameoflife.h
)

target_include_directories(golapp PUBLIC
    ${JACK_MAKE_GENERATED_INCLUDE_DIRS_GOL}
)

target_link_libraries(golapp PRIVATE
    jack_core::jack_core
    sfml-graphics
)

install(TARGETS
    golapp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
