# NOTE: idlc -o option does not work for idlc and cxx genration in our current version.
# So use WORKING_DIRECTORY when running
# https://github.com/eclipse-cyclonedds/cyclonedds-cxx/issues/383
set(IDLC_COMMAND
    $<TARGET_FILE:idlc> -W no-implicit-extensibility
     -f case-sensitive -l cxx ${CMAKE_CURRENT_LIST_DIR}/jack/dds-adapter/events.idl)
If (UNIX)
    #set(IDLC_COMMAND "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib ${IDLC_COMMAND}")
    #message(STATUS "${IDLC_COMMAND}")
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/cyclone/events.cpp
    COMMAND ${IDLC_COMMAND}
    DEPENDS jack/dds-adapter/events.idl idlcxx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/cyclone
    VERBATIM)

add_library(jack-cyclone-dds-adapter STATIC) #SHARED

set(JACK_CYCLONE_DDS_PUBLIC_HEADERS
    jack/dds-adapter/cyclone/cycddsadapter.h
)

target_sources(jack-cyclone-dds-adapter PRIVATE
    jack/dds-adapter/cyclone/cycddsadapter.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/cyclone/events.cpp
    PUBLIC FILE_SET HEADERS BASE_DIRS . FILES ${JACK_CYCLONE_DDS_PUBLIC_HEADERS}
)

target_include_directories(jack-cyclone-dds-adapter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(jack-cyclone-dds-adapter PUBLIC
    jack-event-protocol
    ddscxx
)

target_compile_definitions(jack-cyclone-dds-adapter PUBLIC
        -DJACK_WITH_CYCLONE_DDS
        # \todo Cyclone has problems compiling with GTEST RTTI printing functions, disabled for now
        -DGTEST_HAS_RTTI=0)

install(TARGETS jack-cyclone-dds-adapter
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_cyclone
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack-cyclone-dds-adapter ALIAS jack-cyclone-dds-adapter)

