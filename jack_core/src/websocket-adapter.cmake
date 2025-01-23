if (NOT WIN32)
    # \note Using these sets of lines in deps/zlib/CMakeLists.txt doesn't seem
    # to create globally usable variables. The workaround is to use find_package
    # here directly.
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
    set(ZLIB_USE_STATIC_LIBS ON)
    find_package(ZLIB REQUIRED)
endif()

add_library(jack-websocket-adapter STATIC) #SHARED)

# include(${FlatBuffers_SOURCE_DIR}/CMake/BuildFlatBuffers.cmake)
# flatbuffers_generate_headers(
#     TARGET jack-msgs
#     SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/jack/websocket-adapter/events.fbs
#     FLAGS --cpp --strict-json
# )

# target_sources(jack-msgs
#     PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
#     FILES ${CMAKE_CURRENT_BINARY_DIR}/jack-msgs/events_generated.h
# )

# Write the IDL file into a C++ string for including it at compile time.
# file(READ jack/websocket-adapter/events.fbs
#     FLAT_BUFFER_EVENTS_IDL_FROM_CMAKE NEWLINE_CONSUME)

# configure_file(jack/websocket-adapter/events_idl.h.in
#                jack/websocket-adapter/events_idl.h)

# Write the test website files into a C++ strings for including it at compile time.
# We're not building the site fresh here - do this manually via yarn build in webpage
# file(READ jack/websocket-adapter/www/dist/index.js JS_TEST_FROM_CMAKE NEWLINE_CONSUME)
# file(READ jack/websocket-adapter/www/dist/index.html HTML_TEST_FROM_CMAKE NEWLINE_CONSUME)
# file(READ jack/websocket-adapter/www/dist/index.css CSS_TEST_FROM_CMAKE NEWLINE_CONSUME)
# configure_file(jack/websocket-adapter/testsite.h.in
#                jack/websocket-adapter/testsite.h)

set(WEBSOCKET_ADAPTER_PUBLIC_HEADERS
    jack/websocket-adapter/websocketadapter.h
)

target_sources(jack-websocket-adapter PRIVATE
    jack/websocket-adapter/websocketadapter.cpp
    PUBLIC FILE_SET HEADERS BASE_DIRS . FILES ${WEBSOCKET_ADAPTER_PUBLIC_HEADERS}
)

target_include_directories(jack-websocket-adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(jack-websocket-adapter PUBLIC
    -DJACK_WITH_WEBSOCKETS)

target_link_libraries(jack-websocket-adapter
    PRIVATE
        # jack-msgs
        # flatbuffers
        uv_a
        uWebSockets::uWebSockets
        fmt::fmt-header-only
    PUBLIC
        jack-event-protocol
        ZLIB::ZLIB
)

install(TARGETS jack-websocket-adapter # jack-msgs
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack-websocket-adapter ALIAS jack-websocket-adapter)

