#if(PROJECT_IS_TOP_LEVEL)
    include(baseline.cmake)
    include(overrides.cmake)
#endif()

set(GIT_HOST "git@gitlab.aosgrp.net:"
    CACHE STRING "Git host prefix")

if(DEFINED ENV{CI_JOB_TOKEN})
    list(APPEND git_config url.$ENV{CI_SERVER_PROTOCOL}://gitlab-ci-token:$ENV{CI_JOB_TOKEN}@$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}/.insteadOf=git@$ENV{CI_SERVER_HOST}:)
endif()

if(DEFINED ENV{CI_SERVER_TLS_CA_FILE})
    list(APPEND git_config http.$ENV{CI_SERVER_PROTOCOL}://$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}.sslCAInfo=$ENV{CI_SERVER_TLS_CA_FILE})
    list(APPEND git_config http.$ENV{CI_SERVER_PROTOCOL}://$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}.version=HTTP/1.1)
endif()

include(FetchContent)
add_subdirectory(deps/googletest)
add_subdirectory(deps/concurrentqueue)
add_subdirectory(deps/fmt)
add_subdirectory(deps/date)
add_subdirectory(deps/nlohmann)
install(TARGETS nlohmann_json
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)


if (JACK_WITH_STACKTRACE)
    add_subdirectory(deps/cpptrace)
endif()

if (JACK_WITH_CYCLONE_DDS)
    add_subdirectory(deps/cyclonedds)
endif()

if (JACK_WITH_RTI_DDS)
    #Using include for scoping reasons
    include(deps/rti/rti.cmake)
endif()

if (JACK_WITH_WEBSOCKETS)

    # configure uwebsockets
    OPTION(LSQUIC_BIN "Compile example binaries that use the library" OFF)
    OPTION(LSQUIC_TESTS "Compile library unit tests" OFF)
    OPTION(LSQUIC_SHARED_LIB "Compile as shared librarry" OFF)
    OPTION(LSQUIC_DEVEL "Compile in development mode" OFF)

    # add_subdirectory(deps/zlib)
    # add_subdirectory(deps/libuv)
    add_subdirectory(deps/uwebsockets)
endif()

add_subdirectory(deps/tracy)
