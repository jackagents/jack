# \note RTIConnextDDS package requires CONNEXTDDS_DIR to be specified for CMake to work
# \note RTICodeGenerator package requires RTICODEGEN_DIR to be specified for CMake to work
#       It must point to the bin directory where rticodegen.(bat/sh) is present.

if (NOT DEFINED RTICONNEXTDDS_REQUIRED_VERSION)
    message(STATUS "RTICONNEXTDDS_REQUIRED_VERSION not set, using default 5.3.1")
    set(RTICONNEXTDDS_REQUIRED_VERSION "5.3.1" CACHE STRING "RTI Connext DDS verison required")
endif()

# Allow per-user to set directory according to setup.
if (DEFINED CONNEXTDDS_DIR)
    if (NOT DEFINED RTICODEGEN_DIR)
        set(RTICODEGEN_DIR ${CONNEXTDDS_DIR}/bin CACHE INTERNAL "")
    endif()
else()
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        if (EXISTS "/opt/rti/rti_connext_dds-${RTICONNEXTDDS_REQUIRED_VERSION}")
            set(CONNEXTDDS_DIR "/opt/rti/rti_connext_dds-${RTICONNEXTDDS_REQUIRED_VERSION}"
                CACHE INTERNAL "")
            set(RTICODEGEN_DIR "/opt/rti/rti_connext_dds-${RTICONNEXTDDS_REQUIRED_VERSION}/bin"
                CACHE INTERNAL "")
        else()
            message(FATAL_ERROR "WTF")
        endif()
    endif()
endif()

if (NOT DEFINED CONNEXTDDS_DIR)
    message(FATAL_ERROR "CMake variable CONNEXTDDS_DIR must be set to the root of the RTI installation")
endif()
list(PREPEND CMAKE_MODULE_PATH "${CONNEXTDDS_DIR}/resource/cmake")

find_package(RTIConnextDDS "${RTICONNEXTDDS_REQUIRED_VERSION}" EXACT REQUIRED MODULE)

# \todo WARNING! RTI 6.1.0 does not work yet, the source code needs massaging
# to work because the API has changed between the two versions that break our
# code.
if (${RTICONNEXTDDS_VERSION} VERSION_LESS "6.1.0")
    # \todo We only have 2 RTI installations. We assume before 6.1.0 is when
    # they decided to use one CMake file for find_package to use ...
    find_package(RTICodeGenerator "2.5.2" EXACT REQUIRED MODULE)
endif()
