cmake_minimum_required(VERSION 3.24)

find_package(ament_cmake REQUIRED)
find_package(jack_make REQUIRED)

if (EXISTS ${JACK_MAKE_EXE})
    set(MODEL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tests.mod.jack.json)
    set(JACK_MAKE_ARGS --skip-validation --output-dir ${CMAKE_CURRENT_SOURCE_DIR}/.. ${MODEL_FILE})
    execute_process(COMMAND ${JACK_MAKE_EXE} ${JACK_MAKE_ARGS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE JACK_MAKE_RESULT
	)

    if (JACK_MAKE_RESULT)
        message(FATAL_ERROR "jack-make failed to generate meta files for ${MODEL_FILE}, result - ${JACK_MAKE_GENERATED_RESULT}")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MODEL_FILE})
else()
    message(FATAL_ERROR "jack-make not found!")
endif()

include (${CMAKE_CURRENT_SOURCE_DIR}/meta/tests.cmake RESULT_VARIABLE includeResult)

if (NOT includeResult)
    message(FATAL_ERROR "meta/tests.cmake not found, jack-make gen may have failed")
endif()

include(GoogleTest)

# Macro to add a unit test
macro(AddUnitTest unitTestName)
    add_executable(${unitTestName} ${ARGN}
        ${JACK_MAKE_GENERATED_HEADERS_TESTS}
        ${JACK_MAKE_GENERATED_SOURCES_TESTS})
  target_compile_definitions(${unitTestName} PUBLIC
      -DJACK_WITH_TESTS=1)
  target_link_libraries(${unitTestName} PUBLIC
      jack_core
      fmt::fmt-header-only
      gtest
      gtest_main)
    target_include_directories(${unitTestName} PUBLIC
        ${JACK_MAKE_GENERATED_INCLUDE_DIRS_TESTS}
    )

  if (CMAKE_CROSSCOMPILING)
      # NOTE: Avoid using gtest_discover_test(${unitTestName}) which causes GTest to
      # try and run the test to "discover" the test, but, when we cross-compile we
      # aren't able to run the binaries and this can cause tests to not completely
      # compile.
      add_test(${unitTestName} ${unitTestName})
  else()
      #message(STATUS "looking for tests in ${unitTestName}")
      gtest_discover_tests(${unitTestName})
  endif()
endmacro()

# Add the unit tests
AddUnitTest(agents agents.cpp)
#AddUnitTest(allocators allocator.cpp)
#AddUnitTest(resources resources.cpp)
#AddUnitTest(goals goals.cpp)
#AddUnitTest(intentions intentions.cpp)
#AddUnitTest(vs1 main.cpp)
#AddUnitTest(gt_example gt_main.cpp)
#AddUnitTest(bitmaskops bitmaskops.cpp)
#AddUnitTest(beliefs beliefs.cpp)
AddUnitTest(messages messages.cpp)
#AddUnitTest(node node.cpp)
#AddUnitTest(hanoi hanoi.cpp)
#AddUnitTest(tactics tactics.cpp)
#AddUnitTest(team team.cpp)
#AddUnitTest(service service.cpp)

# TODO: The 1k perform goal test is very slow. It certainly should *not*
# take 25s~ to solve. Our goal should be to bring the agent test timeout
# down to 6s and faster once we solve the degenerate goal planning case and
# fast-path non-plannable goals through to the executor.
set_tests_properties(${agents_TESTS} PROPERTIES TIMEOUT 25)

set_tests_properties(${vs1_TESTS} PROPERTIES TIMEOUT 60)
set_tests_properties(${gt_example_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${goals_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${intentions_TESTS} PROPERTIES TIMEOUT 20)
set_tests_properties(${resources_TESTS} PROPERTIES TIMEOUT 8)
set_tests_properties(${beliefs_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${messages_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${bitmaskops_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${node_TESTS} PROPERTIES TIMEOUT 20)
set_tests_properties(${hanoi_TESTS} PROPERTIES TIMEOUT 6)
set_tests_properties(${service_TESTS} PROPERTIES TIMEOUT 6)

# TODO: Investigate why this is creeping up on Windows, was originally 6, then 11 now 15 seconds
set_tests_properties(${team_TESTS} PROPERTIES TIMEOUT 15)

# Integration Tests
if (JACK_WITH_RTI_DDS)
    add_executable(busadapters_rtidds
        integration/busadapters.cpp)
    #Needs target_compile_options
    target_link_libraries(busadapters_rtidds
        PUBLIC
        jack-rti-dds-adapter
        jack-event-protocol
        jack_core
        gtest
        gtest_main)

    add_test(busadapters_rtidds busadapters_rtidds)
endif()

if (JACK_WITH_CYCLONE_DDS)
    add_executable(busadapters_cyclonedds
        integration/busadapters.cpp)

    target_link_libraries(busadapters_cyclonedds
        PUBLIC
        jack-cyclone-dds-adapter
        jack_core
        gtest
        gtest_main)

    add_test(busadapters_cyclonedds busadapters_cyclonedds)
endif()

