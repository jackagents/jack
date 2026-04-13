
if(DEFINED ENV{CI_JOB_TOKEN})
    list(APPEND git_config url.$ENV{CI_SERVER_PROTOCOL}://gitlab-ci-token:$ENV{CI_JOB_TOKEN}@$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}/.insteadOf=git@$ENV{CI_SERVER_HOST}:)
endif()

if(DEFINED ENV{CI_SERVER_TLS_CA_FILE})
    list(APPEND git_config http.$ENV{CI_SERVER_PROTOCOL}://$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}.sslCAInfo=$ENV{CI_SERVER_TLS_CA_FILE})
    list(APPEND git_config http.$ENV{CI_SERVER_PROTOCOL}://$ENV{CI_SERVER_HOST}:$ENV{CI_SERVER_PORT}.version=HTTP/1.1)
endif()

include(FetchContent)
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(fmt)
find_package(concurrentqueue)

option(TRACY_ENABLE "Enable profiling with Tracy" ${JACK_WITH_TRACY})
option(TRACY_ONLY_LOCALHOST "Enable discovery of Tracy clients exclusively via localhost" ${JACK_WITH_TRACY})
find_package(Tracy CONFIG REQUIRED)

if (JACK_WITH_RTI_DDS)
    #Using include for scoping reasons
    include(deps/rti/rti.cmake)
endif()

if (JACK_WITH_WEBSOCKETS)
    find_package(unofficial-uwebsockets CONFIG REQUIRED)
endif()
