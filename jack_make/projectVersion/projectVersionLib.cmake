function(projectVersionSetup productName)

find_package(Git REQUIRED)
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    RESULT_VARIABLE result
    OUTPUT_VARIABLE GIT_HASH_FULL
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
string(SUBSTRING ${GIT_HASH_FULL} 0 7 GIT_HASH) # Trim git hash to 7 characters
if(result)
    message(FATAL_ERROR "Failed to get git hash for
        ${PROJECT_NAME} - ${PROJECT_SOURCE_DIR}: ${result}")
endif()

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-list -1 HEAD --
        projectVersionDetails.cmake
    RESULT_VARIABLE result
    OUTPUT_VARIABLE lastChangeHash
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
if(result)
    message(FATAL_ERROR "Failed to get hash of last change of
        ${PROJECT_NAME} - ${PROJECT_SOURCE_DIR} projectVersionDetails.cmake: ${result}")
endif()

execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-list ${lastChangeHash}..HEAD
    RESULT_VARIABLE result
    OUTPUT_VARIABLE hashList
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
if(result)
    message(FATAL_ERROR "Failed to get list of git hashes for
        ${PROJECT_NAME} - ${PROJECT_SOURCE_DIR} : ${result}")
endif()

string(REGEX REPLACE "[\n\r]+" ";" hashList "${hashList}")
list(LENGTH hashList COMMITS_SINCE_VERSION_CHANGE)

execute_process(
    COMMAND ${GIT_EXECUTABLE} diff --quiet
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
if(${result} GREATER 1)
    message(FATAL_ERROR "Failed to get dirty status of
        ${PROJECT_NAME} - ${PROJECT_SOURCE_DIR}: ${result}")
elseif(${result} EQUAL 1)
   set(GIT_DIRTY "true")
elseif(${result} EQUAL 0)
   set(GIT_DIRTY "false")
endif()

configure_file(projectVersion/projectVersion.h ${productName}/${productName}_version.h COPYONLY)
configure_file(projectVersion/projectVersion.cpp.in ${productName}/${productName}_version.cpp @ONLY)

add_library(${PROJECT_NAME}-version STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/${productName}/${productName}_version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/${productName}/${productName}_version.h
)

add_library(${PROJECT_NAME}::${PROJECT_NAME}-version ALIAS ${PROJECT_NAME}-version)

set_target_properties(${PROJECT_NAME}-version PROPERTIES
    PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${productName}/${productName}_version.h")

target_include_directories(${PROJECT_NAME}-version PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS ${PROJECT_NAME}-version
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT ${PROJECT_NAME}_RunTime
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${productName}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(PROJECT_COMMITS_SINCE_VERSION_CHANGE
    ${COMMITS_SINCE_VERSION_CHANGE} PARENT_SCOPE)
set(PROJECT_GIT_HASH
    ${GIT_HASH} PARENT_SCOPE)
set(PROJECT_GIT_DIRTY
    ${GIT_DIRTY} PARENT_SCOPE)

endfunction()
