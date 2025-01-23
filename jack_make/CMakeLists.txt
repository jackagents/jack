cmake_minimum_required(VERSION 3.24.2)

include(projectVersionDetails.cmake)
project(jack_make VERSION ${jack_make_VER})

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to RelWithDebInfo as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE
      STRING "Choose the type of build.")
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(ament_cmake REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml.in ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml @ONLY)

find_program(cargo cargo REQUIRED)
set(JACK_MAKE_BUILD_CMD
    cargo ament-build
    --install-base "${CMAKE_INSTALL_PREFIX}")

message(STATUS "JACK_MAKE_BUILD_CMD=${JACK_MAKE_BUILD_CMD}")
execute_process(COMMAND ${JACK_MAKE_BUILD_CMD}
                RESULT_VARIABLE JACK_MAKE_BUILD_RESULT
                ERROR_VARIABLE JACK_MAKE_BUILD_ERROR
                WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
if (NOT JACK_MAKE_BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to build jack-make, cargo failed: ${JACK_MAKE_BUILD_RESULT}:${JACK_MAKE_BUILD_ERROR}")
endif()

if (JACK_WITH_JACK_MAKE_WASM)
    find_program(cargo cargo REQUIRED)
    find_program(wasm_pack wasm-pack REQUIRED)
    if (NOT $<CONFIG:Release>)
        set(JACK_WASM_BUILD_TYPE --dev)
    endif()

    set(JACK_WASM_BUILD_CMD
        ${wasm_pack}
        build
        ${JACK_WASM_BUILD_TYPE}
        --out-dir ${CMAKE_CURRENT_BINARY_DIR}/jack-make-wasm
        --target web
        --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml)

    add_custom_target(jack_make_wasm ALL
                      COMMENT "Build jack-wasm from jack-make via wasm-pack"
                      COMMAND ${JACK_WASM_BUILD_CMD}
                      BYPRODUCTS
                        jack-make-wasm/jack-make_bg.wasm
                        jack-make-wasm/jack-make_bg.wasm.d.ts
                        jack-make-wasm/jack-make.d.ts
                        jack-make-wasm/jack-make.js
                        jack-make-wasm/package.json
                        jack-make-wasm/README.md
                      VERBATIM
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      USES_TERMINAL)

    install(DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/jack-make-wasm
        COMPONENT jack_make_RunTime
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

#ament_export_targets(jack_make)
ament_package(CONFIG_EXTRAS cmake/jack_make_config_extras.cmake)
