################################################################################
# Core Library
################################################################################
set(JACK_SOURCES
    jack/engine.cpp
    jack/agent.cpp
    jack/agentexecutor.cpp
    jack/beliefcontext.cpp
    jack/beliefquery.cpp
    jack/coroutine.cpp
    jack/corelib.cpp
    jack/dag.cpp
    jack/fieldregistry.cpp
    jack/message.cpp
    jack/messageschema.cpp
    jack/plan.cpp
    jack/proxyagent.cpp
    jack/goal.cpp
    jack/lex.yy.c
    jack/builders/builder.cpp
    jack/builders/actionbuilder.cpp
    jack/builders/planbuilder.cpp
    jack/builders/goalbuilder.cpp
    jack/builders/agentbuilder.cpp
    jack/builders/tacticbuilder.cpp
    jack/builders/coroutinebuilder.cpp
    jack/builders/resourcebuilder.cpp
    jack/builders/rolebuilder.cpp
    jack/builders/servicebuilder.cpp
    #See jack-cyc-dds-adapter
    #jack/dds-adapter/cyclone/cycddsadapter.cpp
    #jack/dds-adapter/cyclone/cycddsadapter.h
    #See jack-rti-dds-adapter
    #jack/dds-adapter/rti/rtiddsadapter.cpp
    #jack/dds-adapter/rti/rtiddsadapter.h
    jack/engine/intentionexecutor.cpp
    jack/engine/uuid.cpp
    jack/engine/shared.cpp
    jack/engine/dispatch.cpp
    jack/events/actionevent.cpp
    jack/tactic.cpp
    jack/tactic.h
    jack/tasks/actiontask.cpp
    jack/tasks/pursuetask.cpp
    jack/tasks/conditionaltask.cpp
    jack/tasks/task.cpp
    jack/resource.cpp
    jack/role.cpp
    jack/schedule.cpp
    jack/service.cpp
    jack/team.cpp
    jack/utils.cpp
)

set(JACK_HEADERS
    jack/agent.h
    jack/agentexecutor.h
    jack/beliefcontext.h
    jack/beliefquery.h
    jack/bqltokens.h
    jack/builders/actionbuilder.h
    jack/builders/agentbuilder.h
    jack/builders/builder.h
    jack/builders/coroutinebuilder.h
    jack/builders/goalbuilder.h
    jack/builders/planbuilder.h
    jack/builders/resourcebuilder.h
    jack/builders/rolebuilder.h
    jack/builders/servicebuilder.h
    jack/builders/tacticbuilder.h
    jack/jack.h
    jack/coroutine.h
    jack/corelib.h
    jack/dag.h
    jack/engine.h
    jack/engine/dispatch.h
    jack/engine/intentionexecutor.h
    jack/engine/shared.h
    jack/engine/uuid.h
    jack/events/actioncompleteevent.h
    jack/events/actionevent.h
    jack/events/auctionevent.h
    jack/events/delegationevent.h
    jack/events/event.h
    jack/events/messageevent.h
    jack/events/perceptevent.h
    jack/events/pursueevent.h
    jack/events/scheduleevent.h
    jack/events/sharebeliefset.h
    jack/events/tacticevent.h
    jack/events/timerevent.h
    jack/future.h
    jack/fieldregistry.h
    jack/goal.h
    jack/handles.h
    jack/message.h
    jack/messageschema.h
    jack/plan.h
    jack/promise.h
    jack/proxyagent.h
    jack/resource.h
    jack/role.h
    jack/schedule.h
    jack/service.h
    jack/tactic.h
    jack/tasks/actiontask.h
    jack/tasks/conditionaltask.h
    jack/tasks/droptask.h
    jack/tasks/printtask.h
    jack/tasks/pursuetask.h
    jack/tasks/sleeptask.h
    jack/tasks/task.h
    jack/tasks/yielduntiltask.h
    jack/team.h
    jack/utils.h
)

add_library(jack_core STATIC)

target_sources(jack_core PRIVATE
    ${JACK_SOURCES}
    PUBLIC FILE_SET HEADERS BASE_DIRS . FILES ${JACK_HEADERS}
)

if(JACK_WITH_TESTS)
    #FIXME: Mikew does not like this mechanism to throw exceptions or not
    #this will lead to packages that will not assert.
    # Search JACK_INVALID_CODE_PATH
    target_compile_definitions(jack_core PUBLIC
        -DJACK_WITH_TESTS=1
)
endif()

target_include_directories(jack_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
install(TARGETS jack_core
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack_core ALIAS jack_core)

target_link_libraries(jack_core
    PUBLIC
        jack_core-version
        jack-event-protocol
        TracyClient
        fmt::fmt-header-only
)

if (NOT MSVC)
    target_link_libraries(jack_core PRIVATE Threads::Threads)
endif()

################################################################################
# Other Libraries
################################################################################
# \todo Version the jack-event-protocol
add_library(jack-event-protocol STATIC) #SHARED)
set(JACK_EVENT_PROTOCOL_HEADERS
    jack/event-protocol/busadapter.h
    jack/event-protocol/eventqueue.h
    jack/event-protocol/protocolhelpers.h
    jack/event-protocol/protocol.h
)
target_sources(jack-event-protocol PRIVATE
    jack/event-protocol/protocol.cpp
    jack/event-protocol/protocolhelpers.cpp
    PUBLIC FILE_SET HEADERS BASE_DIRS . FILES ${JACK_EVENT_PROTOCOL_HEADERS}
)

target_include_directories(jack-event-protocol PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(jack-event-protocol PUBLIC
    concurrentqueue
    TracyClient
    PRIVATE
        fmt::fmt-header-only
)

install(TARGETS jack-event-protocol
    EXPORT ${PROJECT_NAME}_Targets
    COMPONENT jack_core_RunTime
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
add_library(jack_core::jack-event-protocol ALIAS jack-event-protocol)

if (JACK_WITH_RTI_DDS)
    include(dds-adapter-rti.cmake)
endif()

if (JACK_WITH_CYCLONE_DDS)
    # TODO: Doesn't build correctly
    # include(dds-adapter-cyclone.cmake)
endif()

if (JACK_WITH_JSON_ADAPTER)
    include(json-adapter.cmake)
    target_link_libraries(jack_core PUBLIC jack-json-adapter)
endif()

if (JACK_WITH_WEBSOCKETS)
    include(websocket-adapter.cmake)
    target_link_libraries(jack_core PUBLIC jack-websocket-adapter)
endif()

if (JACK_WITH_TESTS)
    add_subdirectory(tests)
endif()

################################################################################
# Compile Options
################################################################################
option(JACK_SHARED_MEMORY_DEBUGGING "Enable JACK dumping engine data to shared memory" OFF)
option(JACK_ALLOCATION_TRACKING "Enable call-site tracking of allocations" OFF)

if(MSVC)
    set(JACK_SHARED_MEMORY_DEBUGGING OFF) # TODO(aos): Shared memory is not supported in MSVC yet.
    target_compile_options(jack_core PRIVATE /W4 /WX)
    target_compile_definitions(jack_core PRIVATE YY_NO_UNISTD_H=1)
    set_source_files_properties(jack/lex.yy.c PROPERTIES COMPILE_FLAGS /w) # switch off all warning for the lex file
else()
    target_compile_options(jack_core PRIVATE -Wall -Wextra -pedantic -Werror)
    set_source_files_properties(jack/lex.yy.c PROPERTIES COMPILE_FLAGS -Wno-all) # switch off all warning for the lex file
endif()

if (JACK_SHARED_MEMORY_DEBUGGING)
    target_compile_definitions(jack_core PRIVATE -D JACK_SHARED_MEMORY_DEBUGGING)
endif()

if (JACK_SLOW_DEBUG_CHECKS)
    target_compile_definitions(jack_core PRIVATE -D JACK_SLOW_DEBUG_CHECK_BUILD_FLAG)
endif()

if (JACK_WITH_GCC_CXX03_ABI)
    target_compile_definitions(jack_core INTERFACE -D _GLIBCXX_USE_CXX11_ABI=0)
endif()

if (JACK_ALLOCATION_TRACKING)
    target_compile_definitions(jack_core PUBLIC -D JACK_ALLOCATION_TRACKING)

    # NOTE: These targets use the allocator interface but also don't link to
    # jack_core hence they also need these definitions which change the function
    # signatures that the application links to.
    target_compile_definitions(jack-event-protocol PUBLIC -D JACK_ALLOCATION_TRACKING)
    if (JACK_WITH_JSON_ADAPTER)
        target_compile_definitions(jack-json-adapter PUBLIC -D JACK_ALLOCATION_TRACKING)
    endif()
    if (JACK_WITH_WEBSOCKETS)
        target_compile_definitions(jack-websocket-adapter PUBLIC -D JACK_ALLOCATION_TRACKING)
    endif()
endif()

# TODO: Only enable stack trace in debug builds, or, we switch to dumping raw
# frame traces in release that customers can send to us and we resolve in our
# internal debug builds.
if (JACK_WITH_STACKTRACE)
    target_compile_definitions(jack_core PUBLIC JACK_WITH_STACKTRACE)
    target_link_libraries(jack_core PUBLIC cpptrace)
endif()

