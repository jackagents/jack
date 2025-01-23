set(CPACK_VERBATIM_VARIABLES TRUE)
set(CPACK_PACKAGE_VENDOR "Agent Oriented Sofware")
set(CPACK_PACKAGE_MAINTAINER "AOS Support")
set(CPACK_PACKAGE_CONTACT "aos.support@aosgrp.net")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AOS ${PROJECT_NAME} Package")
string(CONCAT CPACK_PACKAGE_DESCRIPTION
    "${PROJECT_NAME}\n"
    "GitSHA1:${PROJECT_GIT_HASH}")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://aosgroup.com")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
#set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Decision point, CPACK_PACKAGE_NAME is including the package version in its name.
# This allow installation of multiple version while getting the latest of that version.
# NOTE This compromise of this flexibilty is in complication in deployment and cleanup.
set(CPACK_PACKAGE_NAME "aos-${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

# TODO: I have no idea how to get the build configuration into this damn
# variable. CMAKE_BUILD_TYPE only works on single configuration generated
# builds. Multi configuration generators do not set this variable ...
if (DEFINED CMAKE_BUILD_TYPE)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-${CMAKE_BUILD_TYPE_LOWER}")
endif()

if(BUILD_SHARED_LIBS)
    string(CONCAT CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-shared")
else()
    string(CONCAT CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-static")
endif()

set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_BINARY_DIR}/_packages")

if(WIN32)
    set(CPACK_GENERATOR ZIP)
    #NSIS
    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
elseif(APPLE)
    set(CPACK_GENERATOR TGZ productbuild)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/aos/${CPACK_PACKAGE_NAME}/")
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER
        "${CPACK_PACKAGE_MAINTAINER} <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    string(CONCAT CPACK_DEBIAN_PACKAGE_VERSION
        "${CPACK_PACKAGE_VERSION_MAJOR}"
        ".${CPACK_PACKAGE_VERSION_MINOR}"
        ".${CPACK_PACKAGE_VERSION_PATCH}"
        ".${PROJECT_COMMITS_SINCE_VERSION_CHANGE}"
    )
else()
    set(CPACK_GENERATOR TGZ)
endif()

set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /dist
    /.*build.*
    /\\\\.DS_Store
)

#Be selective about what components to package
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_DEB_COMPONENT_INSTALL YES)

get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
message(STATUS "Components we could pack: ${CPACK_COMPONENTS_ALL}")
list(REMOVE_ITEM CPACK_COMPONENTS_ALL
    MyProj_Samples
    MyProj_ApiDocs
)

set(CPACK_COMPONENTS_ALL
    jack_core_Development
    jack_core_RunTime
    jack_core_cyclone
    jack_core_rti
    concurrentqueue_Devel
    uWebSockets
    uSockets_boringssl
    libuv
    Flatbuffers
    cpptoml
    cyclonedds
    iceoryx
    devel
    Tracy
    fmt
)

#Use this to find installations that do not specify a component and
#elimintate them, either by adding a COMPONENT option to the relevant
#install command, or which carefully scoped use of
#set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME blah)
set(DEBUG_COMPONENTS 0)
if (DEBUG_COMPONENTS)
    set(CPACK_COMPONENTS_ALL
        Unspecified
    )
endif()
message(STATUS "Components we will  pack: ${CPACK_COMPONENTS_ALL}")

#after everything
include(CPack)
