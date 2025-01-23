
add_library(jack-json-adapter STATIC) #SHARED)

target_sources(jack-json-adapter PRIVATE
    jack/json-adapter/jsonadapter.cpp
    PUBLIC FILE_SET HEADERS BASE_DIRS . FILES
        jack/json-adapter/jsonadapter.h
)

target_include_directories(jack-json-adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(jack-json-adapter PUBLIC
    -DJACK_WITH_JSON_ADAPTER)

target_link_libraries(jack-json-adapter
    PUBLIC
        jack-event-protocol
    PRIVATE
        fmt::fmt-header-only
)

install(TARGETS jack-json-adapter
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack-json-adapter ALIAS jack-json-adapter)

