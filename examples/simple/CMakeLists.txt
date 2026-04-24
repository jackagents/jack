cmake_minimum_required(VERSION 3.22.1)
project(simple)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find jack-make using our custom module
include(../../cmake/FindJackMake.cmake)
find_jack_make()

if(NOT TARGET jack_core)
    message(FATAL_ERROR "jack_core target not found - build jack_core first")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (EXISTS ${JACK_MAKE_EXE})
    set(MODEL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simple.mod.jack.json)
    set(JACK_MAKE_ARGS --skip-validation --output-dir ${CMAKE_CURRENT_SOURCE_DIR}/.. ${MODEL_FILE})
    execute_process(COMMAND ${JACK_MAKE_EXE} ${JACK_MAKE_ARGS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE JACK_MAKE_RESULT
	)

    if (JACK_MAKE_RESULT)
        message(FATAL_ERROR "jack-make failed to generate meta files for ${MODEL_FILE}, result - ${JACK_MAKE_RESULT}")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MODEL_FILE})
else()
    message(FATAL_ERROR "jack-make not found!")
endif()

include (${CMAKE_CURRENT_SOURCE_DIR}/meta/simple.cmake RESULT_VARIABLE includeResult)

if (NOT includeResult)
    message(FATAL_ERROR "meta/simple.cmake not found, jack-make gen may have failed")
endif()

# Original monolithic example
add_executable(simpleapp
    ${JACK_MAKE_GENERATED_HEADERS_SIMPLE}
    ${JACK_MAKE_GENERATED_SOURCES_SIMPLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(simpleapp PUBLIC
    ${JACK_MAKE_GENERATED_INCLUDE_DIRS_SIMPLE}
)

target_link_libraries(simpleapp PRIVATE
    jack_core::jack_core
)

# Distributed service node (FanService only)
add_executable(simple_service
    ${JACK_MAKE_GENERATED_HEADERS_SIMPLE}
    ${JACK_MAKE_GENERATED_SOURCES_SIMPLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main_service.cpp
)

target_include_directories(simple_service PUBLIC
    ${JACK_MAKE_GENERATED_INCLUDE_DIRS_SIMPLE}
)

target_link_libraries(simple_service PRIVATE
    jack_core::jack_core
)

# Distributed agent node (ThermostatAgent with proxy FanService)
add_executable(simple_agent
    ${JACK_MAKE_GENERATED_HEADERS_SIMPLE}
    ${JACK_MAKE_GENERATED_SOURCES_SIMPLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/main_agent.cpp
)

target_include_directories(simple_agent PUBLIC
    ${JACK_MAKE_GENERATED_INCLUDE_DIRS_SIMPLE}
)

target_link_libraries(simple_agent PRIVATE
    jack_core::jack_core
)

install(TARGETS
    simpleapp
    simple_service
    simple_agent
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
