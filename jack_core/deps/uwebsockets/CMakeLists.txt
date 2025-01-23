cmake_minimum_required(VERSION 3.24)
project(uWebSockets)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "..." FORCE)
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT FALSE)
endif()

enable_language(C)
enable_language(CXX)
include(GNUInstallDirs)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(uSockets/boringssl)

add_library(uWebSockets STATIC)
add_library(uWebSockets::uWebSockets ALIAS uWebSockets)
set_property(TARGET uWebSockets PROPERTY CXX_STANDARD 17)


set(uwebsockets_HEADERS
    src/App.h
    src/AsyncSocketData.h
    src/AsyncSocket.h
    src/BloomFilter.h
    src/ChunkedEncoding.h
    src/ClientApp.h
    src/Http3App.h
    src/Http3ContextData.h
    src/Http3Context.h
    src/Http3Request.h
    src/Http3ResponseData.h
    src/Http3Response.h
    src/HttpContextData.h
    src/HttpContext.h
    src/HttpParser.h
    src/HttpResponseData.h
    src/HttpResponse.h
    src/HttpRouter.h
    src/LoopData.h
    src/Loop.h
    src/MessageParser.h
    src/MoveOnlyFunction.h
    src/Multipart.h
    src/PerMessageDeflate.h
    src/ProxyParser.h
    src/QueryParser.h
    src/TopicTree.h
    src/Utilities.h
    src/WebSocketContextData.h
    src/WebSocketContext.h
    src/WebSocketData.h
    src/WebSocketExtensions.h
    src/WebSocket.h
    src/WebSocketHandshake.h
    src/WebSocketProtocol.h

    uSockets/src/libusockets.h
)

target_sources(uWebSockets
    PRIVATE
        uSockets/src/bsd.c
        uSockets/src/context.c
        uSockets/src/loop.c
        uSockets/src/quic.c
        uSockets/src/socket.c
        uSockets/src/udp.c
        uSockets/src/eventing/epoll_kqueue.c
        uSockets/src/eventing/gcd.c
        uSockets/src/eventing/libuv.c
        uSockets/src/crypto/openssl.c

        uSockets/src/crypto/sni_tree.cpp
        ${uwebsockets_HEADERS}
)
set_target_properties(uWebSockets PROPERTIES PUBLIC_HEADER "${uwebsockets_HEADERS}")

target_compile_definitions(uWebSockets PUBLIC
    LIBUS_NO_SSL
    WITH_BORINGSSL=1
    #WITH_UV=1
)

target_include_directories(uWebSockets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/uSockets/src>
        $<INSTALL_INTERFACE:include/uWebSockets>
    PRIVATE
)

if (NOT WIN32)
    target_link_libraries(uWebSockets PUBLIC z)
endif()

find_package(Threads)
option(UWEBSOCKETS_BUILD_EXAMPLES "Build uwebsocket examples, c++20 required" OFF)
if(UWEBSOCKETS_BUILD_EXAMPLES)
    add_executable(uws_helloworld_threaded
        examples/HelloWorldThreaded.cpp
    )

    set_property(TARGET uws_helloworld_threaded PROPERTY CXX_STANDARD 20)

    target_link_libraries(uws_helloworld_threaded PRIVATE
        uWebSockets
        Threads::Threads
    )
endif()

install(TARGETS uWebSockets
    EXPORT uWebSocketsTargets
    COMPONENT uWebSockets
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/uWebSockets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/uWebSockets
)

install(EXPORT uWebSocketsTargets
    FILE uWebSocketsTargets.cmake
    NAMESPACE uWebSockets::
    COMPONENT uWebSockets
    DESTINATION lib/cmake/uWebSockets
)

install(FILES uWebSocketsConfig.cmake
    COMPONENT uWebSockets
    DESTINATION lib/cmake/uWebSockets
)

