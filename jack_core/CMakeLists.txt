################################################################################
# jack_core
################################################################################
cmake_minimum_required(VERSION 3.24.2)

option(BUILD_SHARED_LIBS "Prefer build shared libraries (DLLs)." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to RelWithDebInfo as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE
      STRING "Choose the type of build.")
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(base $ORIGIN)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(base @loader_path)
endif()

include(projectVersionDetails.cmake)
project(jack_core VERSION ${jack_core_VER})

# ccache stuff
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CUDA_COMPILER_LAUNCHER "${CCACHE_PROGRAM}") # CMake 3.9+
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(jack_make REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

include(GNUInstallDirs)
file(RELATIVE_PATH relDir
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
)
set(CMAKE_INSTALL_RPATH ${base} ${base}/${relDir})

projectVersionSetup(jack)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install"
        CACHE PATH "default local install directory" FORCE)
endif()

if (MSVC)
    # Disable MSVC wanting *_s "secure" versions of CRT API (i.e. buffer length
    # checks) making cross-platform usage of CRT messy.
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

find_package(jack_make REQUIRED)

################################################################################
# Options
################################################################################

# \todo Not sure how we handle multi-config builds here
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(JACK_IS_DEBUG_BUILD ON)
else()
    set(JACK_IS_DEBUG_BUILD OFF)
endif()

option(JACK_ALLOCATION_TRACKING "Enable call-site tracking of allocations" OFF)
option(JACK_SHARED_MEMORY_DEBUGGING "(Linux only) Enable JACK dumping engine data to shared memory" OFF)
option(JACK_SLOW_DEBUG_CHECKS "Enable slow verification checks of the engine state throughout the code base. Use this to vet the engine in applications" ${JACK_IS_DEBUG_BUILD})
option(JACK_WITH_ASAN "Enable address sanitizer, this must be used with debug build" OFF)
option(JACK_WITH_CYCLONE_DDS "Enable CycloneDDS (bus distribution) support in the engine" OFF)
set   (JACK_WITH_GCC_CXX03_ABI OFF CACHE BOOL "Build jack_core using the CXX03 ABI flag")
option(JACK_WITH_RTI_DDS "Enable DDS (bus distribution) support in the engine" OFF)
option(JACK_WITH_TESTS "Build with unit/integration test suites" OFF)
set   (JACK_RUST_VERSION "" CACHE STRING "Rust version to use to build jack-make")
option(JACK_WITH_WEBSOCKETS "Enable websocket server in the engine" ON)
option(JACK_WITH_JSON_ADAPTER "Enable JSON (bus distribution) support in the engine" ON)
option(CTEST_OUTPUT_ON_FAILURE "ctest output on failure" 1)

if (UNIX)
    if (JACK_WITH_GCC_CXX03_ABI OR JACK_WITH_RTI_DDS)
        # \note Poison the build with the CXX03 ABI because everything must be
        # built with this flag for correct interop between JACK and any other
        # dependency.
        #
        # Additionally this will set the CMake target of the core to
        # transitively apply this flag to projects that link JACK to ensure
        # compatibility.
        add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
    endif()
endif()

if (JACK_WITH_ASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    if (NOT MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
    endif()
endif()

################################################################################
# Building
################################################################################
include(deps.cmake)

if(JACK_WITH_TESTS)
    enable_testing()
endif()
add_subdirectory(src)

include(CMakePackageConfigHelpers)
# This will be used to replace @PACKAGE_cmakeModulesDir@
set(cmakeModulesDir cmake)
configure_package_config_file(
    Config.cmake.in ${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    PATH_VARS cmakeModulesDir
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    COMPONENT jack_core_RunTime
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(EXPORT ${PROJECT_NAME}_Targets
     NAMESPACE ${PROJECT_NAME}::
     FILE ${PROJECT_NAME}_Targets.cmake
     COMPONENT ${PROJECT_NAME}_Development
     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

#This must be last.
if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(package)
endif()
