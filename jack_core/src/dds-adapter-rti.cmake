
#Could make this a macro/function
#See https://github.com/rticommunity/ros-data-types/blob/master/resources/cmake/ConnextDdsCodegen.cmake
set(IDL_PATH ${CMAKE_CURRENT_LIST_DIR}/jack/dds-adapter/events.idl)
set(idl_generated_sources
    ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti/events.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti/eventsPlugin.cxx
    #eventsSupport.cxx
)
set(idl_generated_headers
    ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti/events.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti/eventsPlugin.hpp
    #eventsSupport.hpp
)

# NOTE: With Makefile the build fails because the folder does not exist
# ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti 
# Which means that the build cannot continue. Ninja seems to be more intelligent
# and realises that the building of the project will generate those folders and
# proceed.
#
# Manually create the folder to appease Makefiles.
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti)

# NOTE: RTI generates some C++ files that GCC 11 does not accept
# See: https://community.rti.com/forum-topic/after-upgrade-fedora-linux-34-rtiddsgen-no-longer-works
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
    set(RTI_DDS_GEN_FLAGS -ppDisable)
endif()

add_custom_command(
    OUTPUT
        ${idl_generated_sources}
        ${idl_generated_headers}
    VERBATIM
    COMMAND
    ${RTICODEGEN} -language C++11 -stl -unboundedSupport ${RTI_DDS_GEN_FLAGS}
                  -d ${CMAKE_CURRENT_BINARY_DIR}/jack/dds-adapter/rti
                  -replace ${IDL_PATH}
    DEPENDS ${IDL_PATH}
)

add_library(jack-rti-dds-adapter STATIC #SHARED
    ${idl_generated_sources}
    ${idl_generated_headers}
    jack/dds-adapter/rti/rtiddsadapter.h
    jack/dds-adapter/rti/rtiddsadapter.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # NOTE: On Windows the Visual Studio generates a build target to build the .idl
    # file with Microsoft's MIDL compiler. By setting the file has a header file it
    # will prevent CMake for trying to compile it.
    set_source_files_properties(${IDL_PATH} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

if (UNIX)
    target_compile_options(jack-rti-dds-adapter PRIVATE
        "-Wall;-Wno-class-memaccess;-Wno-strict-aliasing")
    set_target_properties(jack-rti-dds-adapter PROPERTIES LINK_FLAGS "--no-as-needed")

    if ("${RTICONNEXTDDS_VERSION}" VERSION_LESS "5.3.2")
        # Our CentOS 8 target links to RTI DDS v5.3.1 built with GCC 4.8.2
        # which contains a std::string that predates a standard compliant
        # implementation in GCC.
        #
        # To link against such libraries using the old non-compliant std::string
        # we have to ensure the programs we build also links against the old ABI
        # so that our programs use the compatiblek std::string implementation,
        # i.e. making it possible to pass strings to the library's without
        # crashing.
        #NOTE: from mikew
        #   I don't think messing with the abi of just his library is enough.
        #   The whole build needs to use the same ABI
        #   probably needs project level  add_definitions(PUBLIC -D_GLIBCXX_USE_CXX11_ABI=0)
        target_compile_definitions(jack-rti-dds-adapter PUBLIC -D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

# \note CONNEXTDDS_DEFINITIONS defines compiler definitions but also compile
# flags so we can't use target_compile_definitions.
#
# Additionally, CONNEXTDDS_DEFINITIONS returns a quoted string i.e. "-m64
# -DRTI_UNIX", when passed to target_compile_options CMAKE preserves the quotes
# causing the compiler to fail to parse the arguments
#
# So we separate the arguments into a comma seperated list and then pass that
# into target_compile_options
separate_arguments(COMMA_SEP_CONNEXTDDS_DEFINITIONS NATIVE_COMMAND ${CONNEXTDDS_DEFINITIONS})
target_compile_options(jack-rti-dds-adapter
    PUBLIC ${COMMA_SEP_CONNEXTDDS_DEFINITIONS} -DRTI_STATIC)

# Apply the user compile time configuration header/options
set(DDS_ADAPTER_USER_INCLUDE_DIRS "" CACHE PATH
    "Set additional include paths intended for making structures available to use in the adapter's user config header")
set(DDS_ADAPTER_USER_CONFIG_H     "" CACHE PATH
    "Set the path to the compile time user config header for the adapter")
if (NOT DDS_ADAPTER_USER_CONFIG_H STREQUAL "")
    target_compile_definitions(jack-rti-dds-adapter
        PUBLIC -DAOS_DDS_ADAPTER_USER_CONFIG_H="${DDS_ADAPTER_USER_CONFIG_H}")
endif()

target_link_libraries(jack-rti-dds-adapter
    PUBLIC
        jack-event-protocol
    PRIVATE
        "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>,${CONNEXTDDS_CPP2_API_LIBRARIES_RELEASE_STATIC},${CONNEXTDDS_CPP2_API_LIBRARIES_DEBUG_STATIC}>"
        ${CONNEXTDDS_EXTERNAL_LIBS})

target_include_directories(jack-rti-dds-adapter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${CONNEXTDDS_INCLUDE_DIRS}
        ${DDS_ADAPTER_USER_INCLUDE_DIRS}
)

target_compile_definitions(jack-rti-dds-adapter
    PUBLIC -DJACK_WITH_RTI_DDS
)

install(TARGETS jack-rti-dds-adapter
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_rti
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack-rti-dds-adapter ALIAS jack-rti-dds-adapter)

