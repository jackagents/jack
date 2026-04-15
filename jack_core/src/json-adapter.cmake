
add_library(jack-json-adapter STATIC) #SHARED)

target_sources(jack-json-adapter PRIVATE
    jack/json-adapter/jsonadapter.cpp
)
target_include_directories(jack-json-adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(jack-json-adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(jack-json-adapter PUBLIC
    -DJACK_WITH_JSON_ADAPTER)

target_link_libraries(jack-json-adapter
    PUBLIC
        nlohmann_json::nlohmann_json
        jack-event-protocol
    PRIVATE
        fmt::fmt-header-only
)

install(TARGETS jack-json-adapter
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/jack/json-adapter
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jack
    COMPONENT jack_core_Development
    FILES_MATCHING PATTERN "*.h"
)
add_library(jack_core::jack-json-adapter ALIAS jack-json-adapter)
