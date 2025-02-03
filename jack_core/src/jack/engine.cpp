// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/engine.h>

#include <jack/builders/actionbuilder.h>      // for ActionBuilder
#include <jack/builders/resourcebuilder.h>    // for ResourceBuilder
#include <jack/builders/coroutinebuilder.h>   // for CoroutineBuilder
#include <jack/builders/rolebuilder.h>        // for RoleBuilder

#include <jack/agentexecutor.h>               // for AgentExecutor, AgentExecutor...
#include <jack/events/event.h>                // for Event
#include <jack/events/actionevent.h>          // for ActionEvent
#include <jack/events/actioncompleteevent.h>  // for ActionCompleteEvent
#include <jack/events/actionevent.h>          // for ActionEvent
#include <jack/events/auctionevent.h>         // for AuctionEvent
#include <jack/events/delegationevent.h>      // for DelegationEvent
#include <jack/events/event.h>                // for Event, Event::SUCCESS, Event...
#include <jack/events/messageevent.h>         // for MessageEvent
#include <jack/events/perceptevent.h>         // for PerceptEventBase, PerceptEvent
#include <jack/events/pursueevent.h>          // for PursueEvent
#include <jack/events/sharebeliefset.h>       // for ShareBeliefSetEvent
#include <jack/events/tacticevent.h>          // for TacticEvent
#include <jack/events/timerevent.h>           // for TimerEvent
#include <jack/goal.h>                        // for Goal
#include <jack/messageschema.h>               // for MessageSchema
#include <jack/proxyagent.h>                  // for ProxyAgent
#include <jack/service.h>                     // for Service
#include <jack/resource.h>                    // for Resource
#include <jack/role.h>                        // for Role
#include <jack/jack_version.h>                 // for VERSION
#include <jack/event-protocol/protocol.h>
#include <jack/utils.h>                       // for readableDurationStringFromMs

/// Third Party
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <string>                        // for char_traits, allocator
#include <unordered_set>
#include <stdint.h>                      // for sized types
#include <assert.h>                      // for assert
#include <string.h>                      // for memcpy

namespace aos::jack
{
constexpr std::chrono::milliseconds Engine::POLL_AUTO_DELTA_TIME;

std::string Action::toString() const
{
    std::string result = JACK_FMT("Action{{name='{}', request='{}', reply='{}', feedback='{}'}}",
                                  name,
                                  request,
                                  reply,
                                  feedback);
    return result;
}

/******************************************************************************
 * Constructor & Destructor
 ******************************************************************************/
Engine::Engine(const std::string& name)
    : m_name(name)
#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    , m_shared(true)
#endif
{
    reset();

    /// \note Grab the instance to initialise the default values if it hasn't
    /// been called yet before starting the engine.
    FieldRegistry::getInstance();

    m_busAddress = protocol::BusAddress(protocol::NodeType_NODE, m_id.toString(), name);
    JACK_INFO("Initialising engine [version={}]", versionDetail());

    static const uint16_t eventSizes[] = {
        sizeof(ActionCompleteEvent),
        sizeof(ActionEvent),
        sizeof(AuctionEvent),
        sizeof(protocol::Control),
        sizeof(DelegationEvent),
        sizeof(protocol::Drop),
        sizeof(Event),
        sizeof(MessageEvent),
        sizeof(PerceptEvent),
        sizeof(PursueEvent),
        sizeof(ShareBeliefSetEvent),
        sizeof(TacticEvent),
        sizeof(TimerEvent),
    };

    static uint16_t maxEventSize = 0;
    if (maxEventSize == 0) {
        for (uint16_t size : eventSizes) {
            maxEventSize = std::max(maxEventSize, size);
        }
    }

    assert(maxEventSize > 0);
    m_eventAllocator.m_chunkSize            = maxEventSize;
    m_eventAllocator.m_arena.m_minBlockSize = JACK_KILOBYTES(4);
}

Engine::~Engine()
{
    // shutdown theads and execution first
    // we might(probably should) even have a friendly shutdown where we inform agents first
    JACK_INFO_MSG("Shutting down JACK core...");

    exit();
    join();

    /// \todo wait for the main engine thread to finish

    // free all agents
    for (Agent* agent : m_agents) {
        JACK_DELETE(agent);
    }
    m_agents.clear();

    // free all services
    for (Service* service : m_services) {
        JACK_DELETE(service);
    }
    m_services.clear();

    for (auto const& p : m_plans) {
        JACK_DELETE(p.second);
    }
    m_plans.clear();

    for (auto const& g : m_goals) {
        JACK_DELETE(g.second);
    }
    m_goals.clear();

    // clear up the agent templates
    for (auto const& v : m_agentTemplates) {
        JACK_DELETE(v.second);
    }
    m_agentTemplates.clear();

    // clear up the service templates
    for (auto const& v : m_serviceTemplates) {
        JACK_DELETE(v.second);
    }
    m_serviceTemplates.clear();

    size_t eventsDequeued = 0;
    do {
        Event* events[32];
        eventsDequeued = m_eventQueue.dequeueBulk(events, arrayCountUSize(events));
        for (size_t eventIndex = 0; eventIndex < eventsDequeued; eventIndex++) {
            JACK_CHUNK_ALLOCATOR_GIVE(&m_eventAllocator, Event, events[eventIndex], JACK_ALLOCATOR_CLEAR_MEMORY);
        }
    } while(eventsDequeued > 0);

    JACK_DEBUG("Dumping leaked allocations to console {}", globalHeapAllocator.m_allocationTracker.stats(getUpTime().count()));

    FixedString<512> heapMetrics  = allocatorMetricsLog(globalHeapAllocator.metrics());
    FixedString<512> eventMetrics = chunkAllocatorMetricsLog(m_eventAllocator);
    JACK_INFO("Heap Alloc [name=Global, info={:s}]", heapMetrics.view());
    JACK_INFO("Event Alloc [name={}, info={}]", name(), eventMetrics.view());

    m_eventAllocator.freeAllocator(true /*clearMemory*/);
}

/******************************************************************************
 * Functions
 ******************************************************************************/
void Engine::setName(const std::string& name)
{
    /// \todo This name is duplicated between the name and the bus address
    /// which means we have this synchronizing problem in updating the engine's
    /// address and name uniformly.
    m_name       = name;
    m_busAddress = protocol::BusAddress(protocol::NodeType_NODE, m_id.toString(), name);
}

/**************************************************************************
 * Execute Functions
 **************************************************************************/
std::chrono::milliseconds Engine::getUpTime() const
{
    using namespace std::chrono;
    auto msSinceEpochStart = duration_cast<milliseconds>(m_startTime.time_since_epoch());
    auto msSinceEpochNow   = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    auto zero              = milliseconds(0);
    return (msSinceEpochStart == zero) ? zero : (msSinceEpochNow - msSinceEpochStart);
}

void Engine::start()
{
    std::thread t1(&Engine::execute, this);
    m_mainEngineThread.swap(t1);
}

void Engine::join()
{
    if (m_mainEngineThread.joinable()) {
        try {
            m_mainEngineThread.join();
        } catch (const std::exception& e) {
            JACK_WARNING("Failed to join the main engine thread: {}", e.what());
        }
    }
}

void Engine::execute()
{
    /*! *******************************************************************************************
     * Run the agent system using the calling thread
     * *******************************************************************************************/
    // check that there are no critical errors
    if (m_criticalErrors.size() > 0) {
        JACK_CRITICAL_MSG("Unable to start engine due to critical errors:");
        for(const std::string& error : m_criticalErrors) {
            JACK_CRITICAL_MSG(error);
        }
        return;
    }

    using Ms = std::chrono::milliseconds;
    for (m_running = true; m_running; ) {
        /// By default, assume the default engine on idle sleep duration. This
        /// is required until we can get notified of changes in the condition
        /// tests i.e. belief to be more event driven.
        auto       sleepDuration = m_onIdleSleepDuration;
        PollResult pollResult    = poll();

        if (pollResult.agentsExecuting) {
            sleepDuration = Ms(0);
        } else {
            /// \todo Possible race condition here inbetween the update and
            /// checking the agents events?
            std::lock_guard<decltype(m_guardAPI)> guardScope(m_guardAPI);
            for (const Agent* agent : m_agents) {
                /// Check for timed events that may expire earlier, we will
                /// adjust the sleep duration to hit that event.
                if (!agent->hasTimerEvents()) {
                    continue;
                }

                Ms nextEventTime = agent->getNextEventTimePoint();
                Ms msToNextEvent = std::max(Ms(0), nextEventTime - std::chrono::duration_cast<std::chrono::milliseconds>(m_internalClock));
                sleepDuration    = std::min(sleepDuration, msToNextEvent);
            }
        }

        if (m_exitWhenNoActiveAgents && pollResult.agentsRunning <= 0 && pollResult.agentsExecuting <= 0) {
            m_running = false;
            continue;
        }

        if (sleepDuration > Ms(0)) {
            std::unique_lock<std::mutex> lck(m_onIdleSleepMutex);
            m_condition.wait_for(lck, sleepDuration);
        }
    }
}

static constexpr inline size_t QUEUE_BUS_EVENTS_PRIOR_TO_THIS_POLL_COUNT = 1;
Engine::PollResult Engine::poll(std::chrono::milliseconds deltaTime)
{
    ZoneScoped;
    gJACKInstance = this;

    /*! *******************************************************************************************
     * Poll the BDI process on the agents in the engine
     * *******************************************************************************************/
    std::chrono::microseconds deltaTimeAdjusted = deltaTime;
    if (deltaTimeAdjusted == POLL_AUTO_DELTA_TIME) {
        auto now = std::chrono::high_resolution_clock::now();
        deltaTimeAdjusted = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastPollTimePoint);
        m_lastPollTimePoint = now;
    }

    if (m_pollCount > QUEUE_BUS_EVENTS_PRIOR_TO_THIS_POLL_COUNT) {
        /// \note Flush the backlogged events created by the sim, for more info
        /// see the comment on `m_busEventOnSimStartupBacklog`
        for (protocol::Event* event : m_busEventOnSimStartupBacklog) {
            sendBusEvent(event);
            JACK_DELETE(event);
        }

        /// \note This vector will no longer be populated until the next reset
        m_busEventOnSimStartupBacklog.clear();
        m_busEventOnSimStartupBacklog.shrink_to_fit();
    }

    /**************************************************************************
     * Process protocol bus events
     **************************************************************************/
    /// Heartbeat our node information onto the bus
    TracyCZoneN(debugTracyHandleBusEvents, "Handle bus events", true);
    if (haveBusAdapter()) {
        m_heartbeatTimer -= deltaTimeAdjusted;
        if (m_heartbeatTimer.count() <= 0) {
            m_heartbeatTimer            = heartbeatTimerPeriod(); /*heartbeat frequency*/
            protocol::Register busEvent = makeProtocolEvent<protocol::Register>();
            busEvent.senderNode         = m_busAddress;
            busEvent.address            = m_busAddress;
            sendBusEvent(&busEvent);
        }
    }

    for (BusAdapter* adapter : m_busAdapters) {

        /// Retrieve events from the bus
        uint32_t count = 0;
        do {
            protocol::Event* events[32];
            count = adapter->poll(events, static_cast<uint32_t>(jack::arrayCountUSize(events)));
            for (size_t index = 0; index < count; index++) {
                protocol::Event *baseEvent = events[index];
                protocolEventHandler(baseEvent);
            }
        } while (count);
    }
    TracyCZoneEnd(debugTracyHandleBusEvents);

    /**************************************************************************
     * Process internal engine events & run agents
     **************************************************************************/
    if (std::chrono::duration_cast<std::chrono::milliseconds>(m_startTime.time_since_epoch()).count() == 0) {
        m_startTime = std::chrono::system_clock::now();
    }

    m_internalClock += deltaTimeAdjusted;

    /// \todo The lock here is too all-encompassing. We have a thread-safe
    /// atomic queue, you don't need a mutex for that ... see new comments on
    /// the m_guardAPI member in the engine.
    m_guardAPI.lock();
    TracyCZoneN(debugTracyProcessEngineEventQueue, "Process engine event queue", true);
    if (haveEvents()) {
        processEvents();
    }
    TracyCZoneEnd(debugTracyProcessEngineEventQueue);

    TracyCZoneN(debugTracyRunAndUpdateAgents, "Run and update agents", true);
    PollResult result = {};
    for (Agent* agent : m_agents) {
        ZoneNamedN(debugTracyAgentUpdate, "Agent update", true);
        ZoneNameV(debugTracyAgentUpdate, agent->name().data(), agent->name().size());
        agent->processEvents();
        agent->run();
        result.agentsRunning   += static_cast<int>(agent->running());
        result.agentsExecuting += static_cast<int>(agent->runningState() == AgentExecutor::RunningState::EXECUTING);
    }
    TracyCZoneEnd(debugTracyRunAndUpdateAgents);

    // process the service events
    TracyCZoneN(debugTracyRunAndUpdateServices, "Run and update services", true);
    for (Service* service : m_services) {
        ZoneNamedN(debugTracyServiceUpdate, "Service update", true);
        ZoneNameV(debugTracyServiceUpdate, service->name().data(), service->name().size());
        service->processEvents();
        service->run();
    }
    TracyCZoneEnd(debugTracyRunAndUpdateServices);

    m_guardAPI.unlock();

    m_pollCount++;
    gJACKInstance = nullptr;
    return result;
}

void Engine::reset()
{
    for (Agent *agent : m_agents) {
        JACK_DELETE(agent);
    }

    for (Service *service : m_services) {
        JACK_DELETE(service);
    }

    m_agents.clear();
    m_services.clear();
    m_eventQueue.clear();
    m_criticalErrors.clear(); /// \todo Not sure if we need this
    m_busDirectory.clear();

    m_running                 = false;
    m_exitWhenNoActiveAgents  = false;
    m_internalClock           = {};
    m_heartbeatTimer          = {};
    m_pollCount               = 0;
    m_lastPollTimePoint       = {};
    m_startTime               = {};
    m_lastProtocolTimestampUs = {};

    // the startup events need flushing incase we never started
    m_busEventOnSimStartupBacklog.clear();
    m_busEventOnSimStartupBacklog.shrink_to_fit();

}

bool Engine::destroyAgent(const AgentHandle& handle)
{
    /// \todo destroy an agent
    /// \todo initially we will just remove it. However we might need to consider a slightly more friendly approach in
    /// \todo the future like informing the agent to shutdown so it can prepare for shutdown
    /// \todo return a promise for an asynchronous event
    /// \todo this should use an event instead
    /// just loop through the whole list for now
    JACK_INFO("Removing agent [agent={}]", handle.toString());
    for (auto it = m_agents.begin(); it != m_agents.end(); it++) {
        if ((*it)->handle() == handle) {
            JACK_DELETE(*it);
            m_agents.erase(it);
            return true;
        }
    }

    return false;
}

/******************************************************************************
 * Builders
 ******************************************************************************/
PlanBuilder Engine::plan(std::string_view name)
{
    /// \todo : If the template already exists we could pull that into the builder first
    PlanBuilder builder(*this, name);
    return builder;
}


GoalBuilder Engine::goal(std::string_view name)
{
    /// \todo : If the template already exists we could pull that into the builder first
    GoalBuilder builder(*this, name);
    return builder;
}

AgentBuilder Engine::agent(std::string_view name)
{
    auto it = m_agentTemplates.find(name);
    if (it == m_agentTemplates.end()) {
        AgentBuilder builder(*this, name);
        return builder;
    } else {
        AgentBuilder builder(*this, *(it->second));
        return builder;
    }
}

ServiceBuilder Engine::service(std::string_view name)
{
    auto it = m_serviceTemplates.find(name);
    if (it == m_serviceTemplates.end()) {
        ServiceBuilder builder(*this, name);
        return builder;
    } else {
        ServiceBuilder builder(*this, *(it->second));
        return builder;
    }
}

TacticBuilder Engine::tactic(std::string_view name)
{
    auto it = m_tacticTemplates.find(name);
    if (it == m_tacticTemplates.end()) {
        TacticBuilder builder(*this, name);
        return builder;
    } else {
        TacticBuilder builder(*this, it->second);
        return builder;
    }
}

CoroutineBuilder Engine::coroutine()
{
    CoroutineBuilder builder(*this);
    return builder;
}

ActionBuilder Engine::action(std::string_view name)
{
    ActionBuilder builder(*this, name);
    return builder;
}

ResourceBuilder Engine::resource(std::string_view name)
{
    ResourceBuilder builder(*this, name);
    return builder;
}

RoleBuilder Engine::role(std::string_view name)
{
    RoleBuilder builder(*this, name);
    return builder;
}

/******************************************************************************
 * Commit Functions
 ******************************************************************************/
Goal* Engine::commitGoal(const Goal* goal)
{
    Goal* result = nullptr;
    if (!goal) {
        return result;
    }

    const std::string& name = goal->name();
    auto it = m_goals.find(name);
    if (it != m_goals.end()) {
        JACK_DELETE(it->second); /// Overwrite existing one
    }

    /// Create the goal
    auto new_it = m_goals.insert_or_assign(name, JACK_CLONE_BDI_OBJECT(goal));
    result      = new_it.first->second;

    /// Create the builtin tactics for the goal and store the handles in the
    /// engine to allow agents to attain a default policy for their goal easily.
    constexpr static std::string_view BUILTIN_TACTIC_SUFFIX = " Builtin Tactic";

    std::string tacticName;
    tacticName.reserve(goal->name().size() + BUILTIN_TACTIC_SUFFIX.size());
    tacticName.append(goal->name());
    tacticName.append(BUILTIN_TACTIC_SUFFIX);
    m_builtinTacticTemplates[goal->name()] = tactic(tacticName)
                                             .goal(goal->name())
                                             .commit();

    // set the back pointer
    result->setEngine(*this);

    return result;
}

Plan* Engine::commitPlan(const Plan* plan)
{
    Plan* result = nullptr;
    if (!plan) {
        return result;
    }

    const std::string& name = plan->name();
    auto it = m_plans.find(name);
    if (it != m_plans.end()) {
        JACK_DELETE(it->second); /// Cleanup existing one for overwriting
    }

    auto new_it = m_plans.insert_or_assign(name, JACK_CLONE_BDI_OBJECT(plan));
    result      = new_it.first->second;

    // set the back pointer
    result->setEngine(*this);

    return result;
}

Agent* Engine::commitAgent(const Agent* agent)
{
    Agent* result = nullptr;
    if (!agent) {
        return result;
    }

    const std::string& name = agent->name();
    auto it = m_agentTemplates.find(name);
    if (it != m_agentTemplates.end()) {
        JACK_DELETE(it->second); /// Cleanup existing one for overwriting
    }

    result                 = agent->clone(name);
    result->m_templateName = name;
    m_agentTemplates[name] = result;
    return result;
}

Service* Engine::commitService(const Service* service)
{
    Service* result = nullptr;
    if (!service) {
        return result;
    }

    const std::string& name = service->name();
    auto it = m_serviceTemplates.find(name);
    if (it != m_serviceTemplates.end()) {
        JACK_DELETE(it->second); /// Cleanup existing one for overwriting
    }

    result                   = service->clone(name);
    result->m_templateName   = name;
    m_serviceTemplates[name] = result;
    return result;
}

Action* Engine::commitAction(const Action* action)
{
    Action* result = nullptr;
    if (!action) {
        return result;
    }

    if (action->name.empty()) {
        JACK_WARNING("Action does not have a name, commit failed [action={}]", action->toString());
        return result;
    }

    auto it = m_actions.insert_or_assign(action->name, *action);
    result  = &it.first->second;
    return result;
}

Resource* Engine::commitResource(const Resource *resource)
{
    Resource* result = nullptr;
    if (!resource) {
        return result;
    }

    const std::string& name = resource->name();
    auto it                 = m_resourceTemplates.insert_or_assign(name, *resource);
    result                  = &it.first->second;
    return result;
}

MessageSchema* Engine::commitMessageSchema(const MessageSchema* schema)
{
    MessageSchema* result = nullptr;
    if (!schema) {
        return result;
    }

    if (!schema->valid()) {
        JACK_WARNING("Message schema is invalid, commit failed [schema={}]", schema->toString());
        return result;
    }

    auto it = m_messageSchemas.insert_or_assign(schema->m_name, *schema);
    result  = &it.first->second;
    return result;
}

Role* Engine::commitRole(const Role *role)
{
    Role *result = nullptr;
    if (!role) {
        return result;
    }

    if (!role->valid()) {
        JACK_WARNING_MSG("Role is invalid, no name specified, commit failed");
        return result;
    }

    const std::string& name = role->name();
    auto it = m_roleTemplates.insert_or_assign(name, *role);
    result  = &it.first->second;
    return result;
}

Tactic* Engine::commitTactic(const Tactic* tactic)
{
    /// \note Validate the tactic
    Tactic *result = nullptr;
    if (!tactic) {
        return result;
    }

    if (tactic->goal().empty()) {
        JACK_WARNING("Tactic does not specify a goal, commit failed [tactic={}]", tactic->handle().toString());
        return result;
    }

    std::string_view goal = tactic->goal();
    if (!getGoal(goal)) {
        JACK_WARNING("Tactic specifies a goal that doesn't exist, commit failed [tactic={}, goal={}]", tactic->handle().toString(), goal);
        return result;
    }

    std::string_view plan = {};
    for (const std::string& nextPlanName : tactic->plans()) {

        /// \note Check if the plan exists
        const Plan *nextPlan = getPlan(nextPlanName);
        if (!nextPlan) {
            JACK_WARNING("Tactic uses non-existent plan, commit failed [tactic={}]", tactic->handle().toString());
            return result;
        }

        /// \note Grab the purported supported goal
        std::string_view nextPlanGoal = nextPlan->goal();
        if (goal != nextPlanGoal) {
            JACK_WARNING("Tactic uses a plan that handles a different goal than the one it supports, commit failed [tactic={}, goal={}, plan/goal={}/{}]",
                         tactic->handle().toString(), goal, plan, nextPlanGoal);
            return result;
        }
    }

    /// \note Add the tactic
    const std::string& name = tactic->handle().m_name;
    auto it                 = m_tacticTemplates.insert_or_assign(name, *tactic);
    result                  = &it.first->second;

    /// \note Enforce that the list of plans is a set, if the tactic
    /// policy is to choose the best plan from the list of plans (no need for
    /// duplicates).
    if (result->planOrder() == Tactic::PlanOrder::ChooseBestPlan) {
        const std::vector<std::string>& plans = result->plans();
        std::unordered_set<std::string> planSet(plans.begin(), plans.end());
        result->plans() = std::vector<std::string>(planSet.begin(), planSet.end());
    }

    return result;
}

/******************************************************************************
 * Create Functions
 ******************************************************************************/
Service *Engine::createServiceInstance(std::string_view templateName, std::string_view serviceName, bool proxy, const UniqueId& uuid)
{
    auto it = m_serviceTemplates.find(templateName);
    if (it == m_serviceTemplates.end()) {
        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        StringBuilder builder          = StringBuilder(scratch.arena);
        builder.append(FMT_STRING("The requested service template '{}' does not exist, service '{}' cannot be instantiated. There are {} template(s) available"),
                       templateName,
                       serviceName,
                       m_serviceTemplates.size());
        if (m_serviceTemplates.size()) {
            builder.appendRef("\n\n");
            bool once = true;
            for (auto templateIt : m_serviceTemplates) {
                if (once) {
                    once = false;
                    builder.appendRef("\n");
                }
                builder.append(FMT_STRING("  {}"), templateIt.first);
            }
        }
        m_criticalErrors.push_back(builder.toString());
        return nullptr;
    }

    // create a new service from the existing template and
    // add to the list of active services
    const Service* templateService = it->second;
    Service*       result          = templateService->clone(serviceName);
    result->setProxy(proxy);
    result->setUUID(uuid);

    JACK_INFO("Creating service [handle={}, proxy={}]", result->handle().toString(), proxy);
    m_services.push_back(result);

    /// \todo Protocol events creation and submission to the bus should be
    /// centralised into the event queue. Currently creating an agent is not an
    /// event, it probably should be. Even if it never turned out to be an
    /// event, there should be some BDI logging event for explainability
    /// submitted to the event queue where we can construct the protocol event.
    ///
    /// For early prototyping, I'm constructing them inline to reduce the code
    /// changes in this first draft.
    if (!proxy && haveBusAdapter()) {
        protocol::Register busEvent = makeProtocolEvent<protocol::Register>();
        busEvent.senderNode         = m_busAddress;
        busEvent.proxy              = proxy;
        busEvent.address            = result->busAddress();
        busEvent.templateType       = templateName;
        sendBusEvent(&busEvent);
    }
    return result;
}

ServiceHandle Engine::createService(std::string_view templateName, std::string_view serviceName, bool proxy, const UniqueId& uuid)
{
    Service *result = createServiceInstance(templateName, serviceName, proxy, uuid);
    return result ? result->handle() : ServiceHandle{};
}

Agent* Engine::createAgentInstance(std::string_view templateName, std::string_view agentName, const UniqueId& uuid)
{
    auto it = m_agentTemplates.find(templateName);
    if (it == m_agentTemplates.end()) {
        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        StringBuilder          builder = StringBuilder(scratch.arena);
        builder.append(FMT_STRING("The requested agent template '{}' does not exist, agent '{} cannot be instantiated. There are {} template(s) available"),
                       templateName,
                       agentName,
                       m_agentTemplates.size());
        if (m_agentTemplates.size()) {
            builder.append(FMT_STRING("\n\n"));
            bool once = true;
            for (auto templateIt : m_agentTemplates) {
                if (once) {
                    once = false;
                    builder.appendRef("\n");
                }
                builder.append(FMT_STRING("  {}"), templateIt.first);
            }
        }
        JACK_WARNING("{}", builder.toStringArena(scratch.arena));
        return nullptr;
    }

    // create a new agent from the existing template and
    // add to the list of active agents
    const Agent* templateAgent = it->second;
    Agent*       result        = templateAgent->clone(agentName);
    for (const std::string &desire : result->m_initialDesires) {
        const Goal* realGoal = getGoal(desire);
        std::string_view messageSchema = realGoal->messageSchema();
        auto msg = createMessage(messageSchema);
        result->pursue(desire, GoalPersistent_Yes, std::move(msg));
    }

    finishCreateAgent(templateName, result, &uuid);
    return result;
}

Team* Engine::createTeamInstance(std::string_view templateName, std::string_view agentName, const UniqueId& uuid)
{
    Agent *result = createAgentInstance(templateName, agentName, uuid);
    if (result == nullptr || !result->isTeam()) {
        JACK_WARNING(
            "The requested agent template can not be instantiated as a team as "
            "the template does not specify a team agent. The template must be "
            "inherit from the JACK Team class [template={}, name={}]",
            templateName, agentName);
        JACK_INVALID_CODE_PATH;
    }
    return dynamic_cast<Team*>(result);
}

AgentHandle Engine::createAgent(std::string_view templateName, std::string_view agentName, const UniqueId &uuid)
{
    Agent *result = createAgentInstance(templateName, agentName, uuid);
    return result ? result->handle() : AgentHandle{};
}

/// \todo We can enqueue a create agent command, what about services and is
/// there a way to reuse this code in the _REGISTER handler from the bus in
/// protocolEventHandler(..) ?
AgentHandle Engine::queueCreateAgent(std::string_view templateName, std::string_view agentName, const UniqueId &uuid, bool start, const AgentHandle *team)
{
    AgentHandle result = {};
    if (templateName.empty() ||
        agentName.empty() ||
        m_agentTemplates.find(templateName) == m_agentTemplates.end())
    {
        return result;
    }

    /// \note Reuse the protocol event to locally enqueue a creation of an
    /// agent. We don't need to fill in sender information in the event
    /// because of this.
    auto* jackEvent                   = JACK_ALLOCATOR_NEW(&m_eventAllocator, Event, Event::REGISTER);
    protocol::Register& registerEvent = jackEvent->registerEvent;
    registerEvent.proxy               = false;
    registerEvent.templateType        = templateName;
    registerEvent.address.name        = agentName;
    registerEvent.address.id          = uuid.toString();
    registerEvent.address.type        = protocol::NodeType_AGENT;
    registerEvent.start               = start;
    if (team) {
        registerEvent.team.name  = team->m_name;
        registerEvent.team.id    = team->m_id.toString();
        registerEvent.team.type  = protocol::NodeType_TEAM;
    }
    routeEvent(jackEvent);

    /// \note Return a fake handle that will eventually become valid after the
    /// event is processed.
    result.m_name      = agentName;
    result.m_id        = uuid;
    return result;
}

ProxyAgent* Engine::createProxyAgent(std::string_view templateName, std::string_view agentName, const UniqueId& uuid)
{
    auto it = m_agentTemplates.find(templateName);
    if (it == m_agentTemplates.end()) {
        JACK_WARNING("Creating unknown proxy agent type: '{}'", templateName);
        return nullptr;
    }

    const Agent* templateAgent = it->second;
    ProxyAgent*  newAgent      = JACK_NEW(ProxyAgent, templateAgent, agentName);
    finishCreateAgent(templateName, newAgent, &uuid);
    return newAgent;
}

ProxyAgent* Engine::createProxyAgent(std::string_view templateName, std::string_view agentName)
{
    return createProxyAgent(templateName, agentName, UniqueId::random());
}

std::unique_ptr<Message> Engine::createMessage(std::string_view templateName)
{
    MessageSchema* schema = getMessageSchema(templateName);
    return schema ? schema->createMessage() : std::unique_ptr<Message>(nullptr);
}

Resource* Engine::createResource(std::string_view templateName)
{
    auto      it       = m_resourceTemplates.find(templateName);
    Resource* resource = nullptr;
    if (it == m_resourceTemplates.end()) {
        // the resource template does not exist
        // just create an empty one for now
        resource = JACK_NEW(Resource, templateName);
        JACK_WARNING("Resource template not found: {}. Did you forget to commit?", templateName);
    } else {
        // create a new agent from the existing template and
        resource = JACK_NEW(Resource, it->second);
    }

    return resource;
}

/******************************************************************************
 * Get Functions
 ******************************************************************************/
Service* Engine::getService(const ServiceHandle& handle) const
{
    Service *result = getServiceByUUID(handle.m_id);
    return result;
}

Service* Engine::getServiceByUUID(const UniqueId& uuid) const
{
    // just loop through the whole list for now
    if (uuid.valid()) {
        for (auto it = m_services.begin(); it != m_services.end(); it++) {
            if ((*it)->UUID() == uuid) {
                return (*it);
            }
        }
    }
    return nullptr;
}

Agent* Engine::getAgent(const AgentHandle &handle) const
{
    Agent *result = getAgentByUUID(handle.m_id);
    return result;
}

Agent* Engine::getAgentByUUID(const UniqueId& uuid) const
{
    // just loop through the whole list for now
    if (uuid.valid()) {
        for (auto it = m_agents.begin(); it != m_agents.end(); it++) {
            if ((*it)->UUID() == uuid) {
                return (*it);
            }
        }
    }
    return nullptr;
}

const Goal *Engine::getGoal(std::string_view name) const
{
    auto it = m_goals.find(name);
    if (it == m_goals.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

const Plan *Engine::getPlan(std::string_view name) const
{
    auto it = m_plans.find(name);
    if (it == m_plans.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

const Role *Engine::getRole(std::string_view name) const
{
    auto it = m_roleTemplates.find(name);
    if (it == m_roleTemplates.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

const Tactic *Engine::getTactic(std::string_view name) const
{
    auto it = m_tacticTemplates.find(name);
    if (name.empty() || it == m_tacticTemplates.end()) {
        return nullptr;
    }
    return &it->second;
}

TacticHandle Engine::getBuiltinTactic(std::string_view goal) const
{
    TacticHandle result = {};
    if (auto it = m_builtinTacticTemplates.find(goal); it != m_builtinTacticTemplates.end()) {
        result = it->second;
        JACK_ASSERT(result.valid());
    } else {
        JACK_ASSERT(!result.valid());
    }
    return result;
}

const Action* Engine::getAction(std::string_view name) const
{
    auto it = m_actions.find(name);
    if (name.empty() || it == m_actions.end()) {
        return nullptr;
    }
    return &it->second;
}


ActionMessageSchemas Engine::getActionMessageSchema(const Action* action)
{
    ActionMessageSchemas result  = {};
    if (!action) {
        return result;
    }

    MessageSchema* request        = getMessageSchema(action->request);
    MessageSchema* reply          = getMessageSchema(action->reply);
    bool           missingRequest = action->request.size() && !request;
    bool           missingReply   = action->reply.size() && !reply;

    if (missingRequest) {
        result.invalidFlags |= ActionMessageSchemasFlag_MISSING_REQUEST;
    }

    if (missingReply) {
        result.invalidFlags |= ActionMessageSchemasFlag_MISSING_REPLY;
    }

    result.request = request;
    result.reply   = reply;
    return result;
}

const MessageSchema* Engine::getMessageSchema(std::string_view name) const
{
    auto it = m_messageSchemas.find(name);
    if (it == m_messageSchemas.end()) {
        return nullptr;
    }
    return &it->second;
}

MessageSchema* Engine::getMessageSchema(std::string_view name)
{
    const MessageSchema* result = const_cast<const Engine*>(this)->getMessageSchema(name);
    return const_cast<MessageSchema*>(result);
}

std::vector<AgentHandle> Engine::agentList() const
{
    std::vector<AgentHandle> result;
    result.reserve(m_agents.size());
    for (auto *agent : m_agents) {
        result.push_back(agent->handle());
    }
    return result;
}

std::vector<ServiceHandle> Engine::serviceList() const
{
    std::vector<ServiceHandle> result;
    result.reserve(m_services.size());
    for (auto *service : m_services) {
        result.push_back(service->handle());
    }
    return result;
}


/// return a list of the tactics names
std::vector<std::string> Engine::tacticList() const
{
    std::vector<std::string> result;
    result.reserve(m_tacticTemplates.size());
    for (const auto &kv : m_tacticTemplates) {
        result.push_back(kv.first);
    }
    return result;
}

/******************************************************************************
 * Bus Functions
 ******************************************************************************/
void Engine::addBusAdapter(BusAdapter* adapter)
{
    if (!adapter) {
        return;
    }
    if (std::find(m_busAdapters.begin(), m_busAdapters.end(), adapter) == m_busAdapters.end()) {
        m_busAdapters.push_back(adapter);
    }
}

void Engine::removeBusAdapter(const BusAdapter *adapter)
{
    if (!adapter) {
        return;
    }

    auto it = std::find(m_busAdapters.begin(), m_busAdapters.end(), adapter);
    if (it != m_busAdapters.end()) {
        m_busAdapters.erase(it);
    }
}

void Engine::sendBusEvent(protocol::Event* event)
{
    ZoneScoped;
    if (!event) {
        return;
    }

    /// \todo Workaround from the simulator pushing events on scenario startup,
    /// see `m_busEventOnSimStartupBacklog` comments for more information.
    if (m_pollCount <= QUEUE_BUS_EVENTS_PRIOR_TO_THIS_POLL_COUNT) {
        protocol::Event *copy = nullptr;
        switch (event->type) {
            /// JACK
            case jack::protocol::EventType_CONTROL:          copy = JACK_NEW(jack::protocol::Control, *static_cast<const jack::protocol::Control*>(event)); break;
            case jack::protocol::EventType_PERCEPT:          copy = JACK_NEW(jack::protocol::Percept, *static_cast<const jack::protocol::Percept*>(event)); break;
            case jack::protocol::EventType_PURSUE:           copy = JACK_NEW(jack::protocol::Pursue, *static_cast<const jack::protocol::Pursue*>(event)); break;
            case jack::protocol::EventType_DROP:             copy = JACK_NEW(jack::protocol::Drop, *static_cast<const jack::protocol::Drop*>(event)); break;
            case jack::protocol::EventType_DELEGATION:       copy = JACK_NEW(jack::protocol::Delegation, *static_cast<const jack::protocol::Delegation*>(event)); break;
            case jack::protocol::EventType_MESSAGE:          copy = JACK_NEW(jack::protocol::Message, *static_cast<const jack::protocol::Message*>(event)); break;
            /// Protocol
            case jack::protocol::EventType_REGISTER:         copy = JACK_NEW(jack::protocol::Register, *static_cast<const jack::protocol::Register*>(event)); break;
            case jack::protocol::EventType_DEREGISTER:       copy = JACK_NEW(jack::protocol::Deregister, *static_cast<const jack::protocol::Deregister*>(event)); break;
            case jack::protocol::EventType_AGENT_JOIN_TEAM:  copy = JACK_NEW(jack::protocol::AgentJoinTeam, *static_cast<const jack::protocol::AgentJoinTeam*>(event)); break;
            case jack::protocol::EventType_AGENT_LEAVE_TEAM: copy = JACK_NEW(jack::protocol::AgentLeaveTeam, *static_cast<const jack::protocol::AgentLeaveTeam*>(event)); break;
            case jack::protocol::EventType_ACTION_BEGIN:     copy = JACK_NEW(jack::protocol::ActionBegin, *static_cast<const jack::protocol::ActionBegin*>(event)); break;
            case jack::protocol::EventType_ACTION_UPDATE:    copy = JACK_NEW(jack::protocol::ActionUpdate, *static_cast<const jack::protocol::ActionUpdate*>(event)); break;
            case jack::protocol::EventType_BDI_LOG:          copy = JACK_NEW(jack::protocol::BDILog, *static_cast<const jack::protocol::BDILog*>(event)); break;
            case jack::protocol::EventType_NONE:             break;
            case jack::protocol::EventType_COUNT:            break;
        }

        JACK_ASSERT_MSG(copy, "Invalid event received or unhandled event type was passed in");
        m_busEventOnSimStartupBacklog.push_back(copy);
        return;
    }

    JACK_ASSERT(event->senderNode.valid());
    event->timestampUs = std::chrono::duration_cast<std::chrono::microseconds>(internalClock()).count();

    /// \note Check for time-travelling engine. We guarantee that the events
    /// are pushed in order of their timestamp.
    JACK_ASSERT(static_cast<uint64_t>(m_lastProtocolTimestampUs.count()) <= event->timestampUs);
    m_lastProtocolTimestampUs = std::chrono::microseconds(event->timestampUs);

    if (event->type == protocol::EventType_BDI_LOG) {
        const auto* result = static_cast<protocol::BDILog*>(event);
        if (result->logType == protocol::BDILogType_ACTION_FINISHED) {
            JACK_ASSERT_MSG(result->sender.type != protocol::NodeType_SERVICE, "Currently action finish must originate from the agent not the service");
        }
    }

    TracyCZoneN(debugTracySendEvents, "Broadcast events on all bus adapters", true);
    for (auto adaptor : m_busAdapters) {
        adaptor->sendEvent(event);
    }
    TracyCZoneEnd(debugTracySendEvents);
}

static void assertHeaderValid(const protocol::BDILogHeader& header)
{
    JACK_ASSERT(header.sender.valid());
}

protocol::BDILog Engine::makeBDILogGoalStarted(const protocol::BDILogHeader& header,
                                               std::string_view              goal,
                                               std::string_view              goalId)
{
    assertHeaderValid(header);
    protocol::BDILog result = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType          = protocol::BDILogType_GOAL_STARTED;
    result.level            = header.level;
    result.goal.goal        = goal;
    result.goal.goalId      = goalId;
    return result;
}

protocol::BDILog Engine::makeBDILogGoalFinished(const protocol::BDILogHeader&       header,
                                                std::string_view          goal,
                                                std::string_view          goalId,
                                                std::string_view          dropReason,
                                                protocol::BDILogGoalIntentionResult intentionResult)
{
    assertHeaderValid(header);
    protocol::BDILog result = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType          = protocol::BDILogType_GOAL_FINISHED;
    result.level            = header.level;
    result.goal.goal        = goal;
    result.goal.goalId      = goalId;
    result.goal.dropReason  = dropReason;
    result.goal.result      = intentionResult;
    return result;
}

protocol::BDILog Engine::makeBDILogSubGoalStarted(const protocol::BDILogHeader& header,
                                                  std::string_view goal,
                                                  std::string_view goalId,
                                                  std::string_view intentionId,
                                                  std::string_view taskId)
{
    assertHeaderValid(header);
    protocol::BDILog result = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType          = protocol::BDILogType_SUB_GOAL_STARTED;
    result.level            = header.level;
    result.goal.goal        = goal;
    result.goal.goalId      = goalId;
    result.goal.intentionId = intentionId;
    result.goal.taskId      = taskId;
    return result;
}

protocol::BDILog Engine::makeBDILogSubGoalFinished(const protocol::BDILogHeader&       header,
                                                   std::string_view                    goal,
                                                   std::string_view                    goalId,
                                                   std::string_view                    intentionId,
                                                   std::string_view                    taskId,
                                                   std::string_view                    dropReason,
                                                   protocol::BDILogGoalIntentionResult intentionResult)
{
    assertHeaderValid(header);
    protocol::BDILog result = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType          = protocol::BDILogType_SUB_GOAL_FINISHED;
    result.level            = header.level;
    result.goal.goal        = goal;
    result.goal.goalId      = goalId;
    result.goal.intentionId = intentionId;
    result.goal.taskId      = taskId;
    result.goal.dropReason  = dropReason;
    result.goal.result      = intentionResult;
    return result;
}

protocol::BDILog Engine::makeBDILogIntentionStarted(const protocol::BDILogHeader& header,
                                  std::string_view    goal,
                                  std::string_view    goalId,
                                  std::string_view    intentionId,
                                  std::string_view    plan)
{
    assertHeaderValid(header);
    protocol::BDILog result                = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType               = protocol::BDILogType_INTENTION_STARTED;
    result.level                 = header.level;
    result.intention.goal        = goal;
    result.intention.goalId      = goalId;
    result.intention.intentionId = intentionId;
    result.intention.plan        = plan;
    return result;
}

protocol::BDILog Engine::makeBDILogIntentionFinished(const protocol::BDILogHeader&       header,
                                   std::string_view          goal,
                                   std::string_view          goalId,
                                   std::string_view          intentionId,
                                   std::string_view          plan,
                                   protocol::BDILogGoalIntentionResult intentionResult)
{
    assertHeaderValid(header);
    protocol::BDILog result      = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType               = protocol::BDILogType_INTENTION_FINISHED;
    result.level                 = header.level;
    result.intention.goal        = goal;
    result.intention.goalId      = goalId;
    result.intention.intentionId = intentionId;
    result.intention.plan        = plan;
    result.intention.result      = intentionResult;
    return result;
}

protocol::BDILog Engine::makeBDILogActionStarted(const protocol::BDILogHeader& header,
                                                 std::string_view    goal,
                                                 std::string_view    goalId,
                                                 std::string_view    intentionId,
                                                 std::string_view    plan,
                                                 std::string_view    taskId,
                                                 std::string_view    action)
{
    assertHeaderValid(header);
    protocol::BDILog result   = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType            = protocol::BDILogType_ACTION_STARTED;
    result.level              = header.level;
    result.action.goal        = goal;
    result.action.goalId      = goalId;
    result.action.intentionId = intentionId;
    result.action.plan        = plan;
    result.action.taskId      = taskId;
    result.action.action      = action;
    return result;
}

protocol::BDILog Engine::makeBDILogActionFinished(const protocol::BDILogHeader& header,
                                                  const ActionEvent*            event,
                                                  std::string_view              reasoning)
{
    assertHeaderValid(header);
    protocol::BDILog result   = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType            = protocol::BDILogType_ACTION_FINISHED;
    result.level              = header.level;
    result.action.goal        = event->m_goal.m_name;
    result.action.goalId      = event->m_goal.m_id.toString();
    result.action.intentionId = event->m_intentionId.toString();
    result.action.plan        = event->m_plan;
    result.action.taskId      = event->m_taskId.toString();
    result.action.action      = event->name();
    result.action.reasoning   = reasoning;
    result.action.success     = event->status == Event::SUCCESS;
    return result;
}

protocol::BDILog Engine::makeBDILogSleepStarted(const protocol::BDILogHeader& header,
                                                std::string_view    goal,
                                                std::string_view    goalId,
                                                std::string_view    intentionId,
                                                std::string_view    plan,
                                                std::string_view    taskId,
                                                uint64_t            sleepMs)
{
    assertHeaderValid(header);
    protocol::BDILog result  = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType           = protocol::BDILogType_SLEEP_STARTED;
    result.level             = header.level;
    result.sleep.goal        = goal;
    result.sleep.goalId      = goalId;
    result.sleep.intentionId = intentionId;
    result.sleep.plan        = plan;
    result.sleep.taskId      = taskId;
    result.sleep.sleepMs     = sleepMs;
    return result;
}

protocol::BDILog Engine::makeBDILogSleepFinished(const protocol::BDILogHeader& header,
                                                 std::string_view    goal,
                                                 std::string_view    goalId,
                                                 std::string_view    intentionId,
                                                 std::string_view    plan,
                                                 std::string_view    taskId)
{
    assertHeaderValid(header);
    protocol::BDILog result  = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType           = protocol::BDILogType_SLEEP_FINISHED;
    result.level             = header.level;
    result.sleep.goal        = goal;
    result.sleep.goalId      = goalId;
    result.sleep.intentionId = intentionId;
    result.sleep.plan        = plan;
    result.sleep.taskId      = taskId;
    return result;
}

protocol::BDILog Engine::makeBDILogConditionResult(const protocol::BDILogHeader& header,
                                                   std::string_view    goal,
                                                   std::string_view    goalId,
                                                   std::string_view    intentionId,
                                                   std::string_view    plan,
                                                   std::string_view    taskId,
                                                   std::string_view    condition,
                                                   bool                success)
{
    assertHeaderValid(header);
    protocol::BDILog result      = makeProtocolEvent<protocol::BDILog>(header.sender, {} /*recipient*/, header.eventId);
    result.logType               = protocol::BDILogType_CONDITION;
    result.level                 = header.level;
    result.condition.goal        = goal;
    result.condition.goalId      = goalId;
    result.condition.intentionId = intentionId;
    result.condition.plan        = plan;
    result.condition.taskId      = taskId;
    result.condition.condition   = condition;
    result.condition.success     = success;
    return result;
}

std::string Engine::dumpBusDirectory() const
{
    const std::string_view TYPE    = "Type";
    const std::string_view ADDRESS = "Address";
    const std::string_view ID      = "ID";
    const std::string_view NAME    = "Name";

    /// Evaluate the longest string for each of the bus entries for pretty
    /// printing
    size_t longestName       = sizeof(NAME) - 1;
    size_t longestId         = sizeof(ID) - 1;
    size_t longestNodeType   = 0;
    size_t longestBusAddress = sizeof(ADDRESS) - 1;

    for (int enumInt = 0; enumInt < protocol::NodeType_COUNT; enumInt++) {
        std::string_view node = protocol::nodeTypeString(static_cast<protocol::NodeType>(enumInt));
        longestNodeType = std::max(longestNodeType, node.size());
    }

    for (auto it : m_busDirectory) {
        const protocol::BusAddress &address = it.first;
        longestName       = std::max(longestName, address.name.size());
        longestId         = std::max(longestId, address.id.size());
        longestBusAddress = std::max(longestBusAddress, protocol::makeBusAddressString(address, false /*compact*/).size());
    }

    /// Dump the bus directory, formatting it into a nice whitespace aligned
    /// table
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    auto                   builder = StringBuilder(scratch.arena);

    std::string addressString = protocol::makeBusAddressString(m_busAddress, true /*compact*/);
    builder.append(FMT_STRING("Dumping bus directory [node={}, size={}]"), addressString, m_busDirectory.size());
    if (m_busDirectory.size()) {
        builder.append(FMT_STRING("\n | {}"), TYPE);
        for (size_t index = TYPE.size(); index < longestNodeType; index++) {
            builder.appendRef(" ");
        }

        builder.append(FMT_STRING(" | {}"), NAME);
        for (size_t index = NAME.size(); index < longestName; index++) {
            builder.appendRef(" ");
        }

        builder.append(FMT_STRING(" | {}"), ID);
        for (size_t index = ID.size(); index < longestId; index++) {
            builder.appendRef(" ");
        }

        builder.append(FMT_STRING(" | {}"), ADDRESS);
        for (size_t index = ADDRESS.size(); index < longestBusAddress; index++) {
            builder.appendRef(" ");
        }

        builder.appendRef(" | Heartbeat\n");
        for (auto it : m_busDirectory) {
            const protocol::BusAddress &address  = it.first;
            std::string_view typeString = nodeTypeString(address.type);
            builder.append(FMT_STRING(" | {}"), typeString);
            for (size_t index = typeString.size(); index < longestNodeType; index++) {
                builder.appendRef(" ");
            }

            builder.append(FMT_STRING(" | {}"), address.name);
            for (size_t index = address.name.size(); index < longestName; index++) {
                builder.appendRef(" ");
            }

            builder.append(FMT_STRING(" | {}"), address.id);
            for (size_t index = address.id.size(); index < longestId; index++) {
                builder.appendRef(" ");
            }

            std::string busAddressString = protocol::makeBusAddressString(address, false /*compact*/);
            builder.append(FMT_STRING(" | {}"), busAddressString);
            for (size_t index = busAddressString.size(); index < longestBusAddress; index++) {
                builder.appendRef(" ");
            }

            auto timeSinceLastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(m_internalClock) - it.second.m_lastMessageClockTime;
            FixedString<64> duration = readableDurationStringFromMs(timeSinceLastHeartbeat.count(), ReadableDurationFlags_ALL);
            builder.append(FMT_STRING(" | {} ago\n"), duration);
        }
    }

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Private Functions
 ******************************************************************************/
void Engine::eventDispatch(Event* event)
{
    ZoneScoped;
    bool returnEventToAllocator = false;
    Service *recipient = event->recipient;
    if (recipient) {
        recipient->routeEvent(event);
    } else {
        switch(event->type) {
            case Event::REGISTER: {
                ZoneNamedN(debugTracyEngineRegister, "Engine register event", true);
                returnEventToAllocator                  = true;
                const protocol::Register* registerEvent = &event->registerEvent;
                UniqueId                             id = UniqueId::initFromString(registerEvent->address.id);
                if (!id.valid()) {
                    JACK_DEBUG("Event received with invalid UUID [event={}]", *registerEvent);
                    break;
                }

                if (registerEvent->address.type == protocol::NodeType_AGENT || registerEvent->address.type == protocol::NodeType_TEAM) {
                    Agent* agent = getAgentByUUID(id);
                    if (agent) {
                        JACK_DEBUG("Event registered a proxy agent that already exists in the node [event={}]", *registerEvent);
                        if (registerEvent->proxy != static_cast<bool>(dynamic_cast<ProxyAgent*>(agent))) {
                            if (registerEvent->proxy) {
                                JACK_DEBUG("Event registered a proxy agent but this node already has a concrete agent [event={}]", *registerEvent);
                            } else {
                                JACK_DEBUG("Event registered a concrete agent but this node already has a proxy agent [event={}]", *registerEvent);
                            }
                        }
                    } else {
                        if (registerEvent->proxy) {
                            agent = createProxyAgent(registerEvent->templateType, registerEvent->address.name, id);
                        } else {
                            agent = createAgentInstance(registerEvent->templateType, registerEvent->address.name, id);
                        }

                        if (registerEvent->start) {
                            agent->start();
                        }

                        if (registerEvent->team.isSet()) {
                            UniqueId teamId = UniqueId::initFromString(registerEvent->team.id);
                            Agent* teamAgent = getAgentByUUID(teamId);

                            if (teamAgent && teamAgent->isTeam()) {
                                auto *team = dynamic_cast<Team*>(teamAgent);
                                team->addMemberAgent(agent);
                            } else {
                                /// \note Only log a warning if the sender node is not set, i.e.
                                /// it's a local message. Remote nodes can be malicious and spam our
                                /// logs.
                                if (!registerEvent->senderNode.isSet()) {
                                    if (!teamAgent) {
                                        JACK_WARNING("Newly registered agent references non-existent team [event={}]", registerEvent->toString());
                                    } else {
                                        JACK_WARNING("Newly registered agent references membership to an agent that is not a team [event={}]", registerEvent->toString());
                                    }
                                }
                            }
                        }
                    }
                } else if (registerEvent->address.type == protocol::NodeType_SERVICE) {
                    Service* service = getServiceByUUID(id);
                    assert(!registerEvent->team.isSet() && "A service can't join a team");

                    if (service) {
                        JACK_DEBUG("Event registered a proxy service that already exists in the node [event={}]", registerEvent->toString());
                        if (registerEvent->proxy != service->isProxy()) {
                            if (registerEvent->proxy) {
                                JACK_DEBUG("Event registered a proxy service but this node already has a concrete service [event={}]", registerEvent->toString());
                            } else {
                                JACK_DEBUG("Event registered a concrete service but this node already has a proxy service [event={}]", registerEvent->toString());
                            }
                        }
                    } else {
                        service = createServiceInstance(registerEvent->templateType, registerEvent->address.name, registerEvent->proxy, id);
                        if (registerEvent->start) {
                            service->start();
                        }
                    }
                } else {
                    assert(!registerEvent->team.isSet() && "Generic/Node entities can't join a team");
                    assert(!registerEvent->start && "Generic/Node entities can't be started");
                    assert(registerEvent->address.type == protocol::NodeType_NODE || registerEvent->address.type == protocol::NodeType_GENERIC);
                }

                /// Store a record of the registered entity
                BusAddressableEntity& entity  = m_busDirectory[registerEvent->address];
                entity.m_lastMessageClockTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_internalClock);
            } break;

            case Event::MESSAGE: {
                ZoneNamedN(debugTracyEngineRegister, "Engine message event", true);

                auto* msgEvent = static_cast<MessageEvent*>(event);
                ZoneNameV(debugTracyEngineRegister, msgEvent->msg.m_schemaName.c_str(), msgEvent->msg.m_schemaName.size());

                JACK_ASSERT_MSG(!msgEvent->recipient, "Event with recipient should be handled earlier by direct routing");
                for (Agent* agent : m_agents) {
                    /// \note By default broadcast to all agents
                    bool sendPercept = true;

                    /// \note If the sender was a Service- instead we will only
                    /// route the message to agents that are attached to the
                    /// service that sent this message.
                    if (auto *service = dynamic_cast<Service*>(msgEvent->caller)) {
                        sendPercept = false;
                        for (const ServiceHandle& attachedService : agent->attachedServices()) {
                            if (attachedService == service->handle()) {
                                sendPercept = true;
                                break;
                            }
                        }
                    }

                    if (sendPercept) {
                        agent->routeEvent(msgEvent);
                        returnEventToAllocator = false;
                    }
                }
            } break;

            case Event::PERCEPT: {
                ZoneNamedN(debugTracyEnginePercept, "Engine message event", true);

                PerceptEvent* perceptEvent = static_cast<PerceptEvent*>(event);
                ZoneNameV(debugTracyEnginePercept, perceptEvent->name.c_str(), perceptEvent->name.size());

                for (Agent* agent : m_agents) {
                    bool sendPercept = true;

                    /// \note If the sender of the percept was a Service, route the
                    /// percept only to agents that are attached to the service.
                    if (auto *service = dynamic_cast<Service*>(perceptEvent->caller)) {
                        sendPercept = false;
                        for (const ServiceHandle& attachedService : agent->attachedServices()) {
                            if (attachedService == service->handle()) {
                                sendPercept = true;
                                break;
                            }
                        }
                    }

                    /// \todo In addition to filtering by Services, only send
                    /// this to the agents that are subscribed? (if they don't
                    /// have a satisfying service).
                    if (sendPercept) {
                        agent->routeEvent(perceptEvent->clone(&m_eventAllocator));
                    }
                }

                returnEventToAllocator = true;
            } break;

            case Event::ACTIONCOMPLETE: {
                assert(!"This path should never hit anymore");
            } break;

            case Event::ACTION: {
                /// \note Find a service to handle this action
                /// An event from an agent might be a remote agent eventually
                assert(event->caller);
                assert(!event->recipient);

                ZoneNamedN(debugTracyEngineAction, "Engine action event", true);
                auto* actionEvent = static_cast<ActionEvent*>(event);
                ZoneNameV(debugTracyEngineAction, actionEvent->name().data(), actionEvent->name().size());

                const Agent* agent       = dynamic_cast<Agent*>(event->caller);
                assert(agent &&
                       "We expect the caller to be an agent for now, if this hits reconsider our "
                       "approach here. We should probably move the action forwarding code into the "
                       "agent dispatch. I've chosen to handle it here so that all non-agent "
                       "handle-able actions are handled in one centralized code path");

                Service* service = nullptr;
                for (const ServiceHandle& attachedServiceHandle : agent->attachedServices()) {
                    Service* attachedService = getService(attachedServiceHandle);
                    if (!attachedService) {
                        JACK_WARNING("Attached service on agent no longer exists  [agent={}, service={}]",
                                     agent->handle().toString(),
                                     attachedServiceHandle.toString());
                        continue;
                    }

                    if (attachedService->handlesAction(actionEvent->name())) {
                        service = attachedService;
                        break;
                    }
                }

                /// \note Fallback path, if the agent does not have an explicit
                /// service to handle it, we will fallback to the first suitable
                /// service that can handle it.
                ///
                /// Right now we use this behaviour so that old code behaves as
                /// we expect.
                ///
                /// \todo The policy for falling back to services should be
                /// configurable potentially by the goal plan selection policy.
                ///
                /// i.e. Allow discovering of services or strictly only the
                /// attached service list.
                if (settings.unhandledActionsForwardedToFirstApplicableService) {
                    for (auto it = m_services.begin(); !service && it != m_services.end(); it++) {
                        service = (*it)->handlesAction(actionEvent->name()) ? *it : nullptr;
                    }
                }

                if (service) {
                    actionEvent->recipient = service;
                    service->routeEvent(actionEvent);
                } else {
                    JACK_ERROR("No suitable service to handle action [action={}, caller={}]",
                               actionEvent->name(),
                               actionEvent->caller->handle().toString());
                    returnEventToAllocator = true;
                }
            } break;

            default: {
                JACK_ERROR_MSG("Unhandle Event in Engine");
            } break;
        }
    }

    if (returnEventToAllocator) {
        JACK_CHUNK_ALLOCATOR_GIVE(&m_eventAllocator,
                                  Event,
                                  event,
                                  JACK_ALLOCATOR_CLEAR_MEMORY);
    }
}

/// \note Protocol Event Validation Helpers
/// Values for enforcing if a field should be set or not in a protocol event.
enum class CheckPresence
{
    EMPTY,    /// The field must not be set
    OPTIONAL, /// The field is optional, it can be set or not set
    REQUIRED, /// The field must be set
};

/// Bit flags for enforcing that a bus address must be a particular node type
/// in a protocol event.
enum CheckNodeType
{
    CheckNodeType_NO_CHECK    = 0,
    CheckNodeType_GENERIC     = 1 << 0,
    CheckNodeType_NODE        = 1 << 1,
    CheckNodeType_SERVICE     = 1 << 2,
    CheckNodeType_AGENT       = 1 << 3,
    CheckNodeType_TEAM        = 1 << 4,
    CheckNodeType_AGENT_TYPES = CheckNodeType_AGENT       | CheckNodeType_TEAM,
    CheckNodeType_BDI         = CheckNodeType_AGENT_TYPES | CheckNodeType_SERVICE,
};

/// The set of rules to check for validating the base protocol event
struct CheckRule
{
    CheckPresence recipient;     /// Enforce whether or not the recipient should be set
    CheckPresence sender;        /// Enforce whether or not the sender should be set
    CheckNodeType recipientType; /// Enforce the type of recipient if the recipient is set
    CheckNodeType senderType;    /// Enforce the type of sender if the sender is set
};

/// Validates the base protocol event is valid by verifying it against a set of
/// base event requirements for each protocol event type.
static bool baseProtocolEventCheck(const protocol::Event*      event,
                                   const protocol::BusAddress& engineAddress,
                                   std::vector<Agent*>&        agents,
                                   std::vector<Service*>&      services,
                                   Service**                   recipient,
                                   Service**                   sender)
{
    /// \todo Verify sender/recipient cryptographically maybe at the
    /// adapter level.

    /// \todo Its kind of wasteful that we don't bake in not-sending to
    /// ourselves in the protocol?

    /// \note Do basic instant failure checks
    if (!event->senderNode.isSet()) {
        JACK_BUS("Sender node information is missing [event={}]", event->toString());
        return false;
    }

    if (event->type == protocol::EventType_COUNT || event->type == protocol::EventType_NONE) {
        JACK_BUS("Invalid out-of-bounds event type received [event={}]", event->toString());
        return false;
    }

    /// \note Generate the rules for each event type
    if (event->senderNode == engineAddress) {
        /// \note Debug log the bus message we received to ourselves, useful but
        /// very noisy when working in multi-node scenarios.
        #if 0
        if (const auto* bdiLog = dynamic_cast<const protocol::BDILog*>(event)) {
            JACK_BUS("(Own) event ["sv << *bdiLog << "]"sv);
        } else if (const auto* control = dynamic_cast<const protocol::Control*>(event)) {
            JACK_BUS("(Own) event ["sv << *control << "]"sv);
        } else if (const auto* reg = dynamic_cast<const protocol::Register*>(event)) {
            JACK_BUS("(Own) event ["sv << *reg << "]"sv);
        } else if (const auto* dereg = dynamic_cast<const protocol::Delegation*>(event)) {
            JACK_BUS("(Own) event ["sv << *dereg << "]"sv);
        } else if (const auto* agentJoinTeam = dynamic_cast<const protocol::AgentJoinTeam*>(event)) {
            JACK_BUS("(Own) event ["sv << *agentJoinTeam << "]"sv);
        } else {
            JACK_BUS("(Own) event ["sv << *event << "]"sv);
        }
        #endif
        return false;
    }

    CheckRule rule = {};
    switch (event->type) {
        case protocol::EventType_COUNT: /*FALLTHRU*/
        case protocol::EventType_NONE: { assert(!"Internal error: Invalid enum case should be handled earlier"); } break;

        case protocol::EventType_CONTROL: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_BDI;
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_PERCEPT: /*FALLTHRU*/
        case protocol::EventType_MESSAGE: {
            rule.recipient = CheckPresence::OPTIONAL; rule.recipientType = CheckNodeType_BDI;
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_PURSUE: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_AGENT_TYPES;
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_DROP: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_BDI;
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_DELEGATION: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_AGENT_TYPES;
            rule.sender    = CheckPresence::REQUIRED; rule.senderType    = CheckNodeType_AGENT_TYPES;
        } break;

        case protocol::EventType_DEREGISTER: /*FALLTHRU*/
        case protocol::EventType_REGISTER: {
            rule.recipient = CheckPresence::OPTIONAL; /// \note We have dedicated team/agent fields in the event itself
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_AGENT_LEAVE_TEAM: /*FALLTHRU*/
        case protocol::EventType_AGENT_JOIN_TEAM: {
            rule.recipient = CheckPresence::OPTIONAL; /// \note We have dedicated team/agent fields in the event itself
            rule.sender    = CheckPresence::OPTIONAL;
        } break;

        case protocol::EventType_ACTION_BEGIN: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_BDI;

            /// \todo We may allow non-BDI entities to call actions in the
            /// future, right now we snap a pointer to an instance for the sender
            /// which is not possible for entites we only loosely know about,
            /// i.e. a node/generic application that does not have a proxy
            /// instance but only an address.
            rule.sender    = CheckPresence::REQUIRED; rule.senderType    = CheckNodeType_BDI;
        } break;

        case protocol::EventType_ACTION_UPDATE: {
            rule.recipient = CheckPresence::REQUIRED; rule.recipientType = CheckNodeType_AGENT_TYPES;
            rule.sender    = CheckPresence::REQUIRED; rule.senderType    = CheckNodeType_BDI;
        } break;

        case protocol::EventType_BDI_LOG: {
            rule.recipient = CheckPresence::OPTIONAL;
            rule.sender    = CheckPresence::OPTIONAL;
        } break;
    }

    /**************************************************************************
     * Validate rules
     **************************************************************************/
    /// \note Use a table driven approach to verify the base event
    struct BusAddressRuleCheck {
        const char *                errorLabel; /// The label to prefix errors with if there's an error
        const protocol::BusAddress& address;    /// The address in the event to verify
        CheckPresence               presence;   /// Enforce whether or not the address should be set
        CheckNodeType               nodeType;   /// Enforce the type of address it should be if it is set
        Service**                   instance;   /// Retrieves the BDI instance of the entity specified by the address
    } const busAddressRuleChecks[] = {
        BusAddressRuleCheck{"Recipient", event->recipient, rule.recipient, rule.recipientType, recipient},
        BusAddressRuleCheck{"Sender",    event->sender,    rule.sender,    rule.senderType,    sender},
    };

    for (const BusAddressRuleCheck &check : busAddressRuleChecks) {
        /// \note Check that the address type is in a valid state
        if (!check.address.valid()) {
            JACK_BUS("Invalid event [reason={} uses invalid values, event={}]", check.errorLabel, event->toString());
            return false;
        }

        /// \note Check whether or not the event sender/recipient field should
        /// be set or not for that event.
        {
            FixedString<128> error = {};
            switch (check.presence) {
                case CheckPresence::EMPTY:    if (check.address.isSet())  { error.append(FMT_STRING("{} field must be empty"), check.errorLabel); } break;
                case CheckPresence::OPTIONAL: break;
                case CheckPresence::REQUIRED: if (!check.address.isSet()) { error.append(FMT_STRING("{} field is required"), check.errorLabel); } break;
            }

            if (error.m_size) {
                JACK_BUS("Invalid event [reason={}, event={}]", error.view(), event->toString());
                return false;
            }
        }

        /// \note If the sender/recipient is set, check that it's one of the
        /// allowed types for that event.
        {
            FixedString<128> error = {};
            switch (check.presence) {
                case CheckPresence::EMPTY: assert(check.nodeType == CheckNodeType_NO_CHECK && "Invalid rule combination: No recipient means no check"); break;
                case CheckPresence::OPTIONAL: /*FALLTHRU*/
                case CheckPresence::REQUIRED: {
                    if (check.address.isSet() && (check.nodeType != CheckNodeType_NO_CHECK)) {
                        switch (check.address.type) {
                            case protocol::NodeType_GENERIC: if ((check.nodeType & CheckNodeType_GENERIC) == 0) { error.append("%s is not allowed to be GENERIC", check.errorLabel); } break;
                            case protocol::NodeType_NODE:    if ((check.nodeType & CheckNodeType_NODE)    == 0) { error.append("%s is not allowed to be NODE", check.errorLabel); } break;
                            case protocol::NodeType_AGENT:   if ((check.nodeType & CheckNodeType_AGENT)   == 0) { error.append("%s is not allowed to be AGENT", check.errorLabel); } break;
                            case protocol::NodeType_TEAM:    if ((check.nodeType & CheckNodeType_TEAM)    == 0) { error.append("%s is not allowed to be TEAM", check.errorLabel); } break;
                            case protocol::NodeType_SERVICE: if ((check.nodeType & CheckNodeType_SERVICE) == 0) { error.append("%s is not allowed to be SERVICE", check.errorLabel); } break;
                            case protocol::NodeType_COUNT:   { assert(!"Internal error: COUNT enum case should be handled in the valid() check"); } break;
                        }
                    }
                } break;
            }

            if (error.m_size) {
                JACK_BUS("Invalid event [reason={}, event={}]", error.view(), event->toString());
                return false;
            }
        }

        /// \note Retrieve the concrete instance of the sender/recipient
        /// specified in the event.
        if (check.address.isSet()) {
            FixedString<128> error = {};
            switch (check.address.type) {
                case protocol::NodeType_COUNT:   { assert(!"Internal error: COUNT enum case should be handled in the valid() check"); } break;
                case protocol::NodeType_GENERIC: { assert(!"We don't know how to handle GENERIC nodes yet"); } break;
                case protocol::NodeType_NODE: break;
                case protocol::NodeType_SERVICE: /*FALLTHRU*/
                case protocol::NodeType_AGENT: /*FALLTHRU*/
                case protocol::NodeType_TEAM: {
                    if (check.address.type == protocol::NodeType_SERVICE) {
                        for (auto serviceIt = services.begin(); serviceIt != services.end(); serviceIt++) {
                            if (check.address == (*serviceIt)->busAddress()) {
                                *check.instance = *serviceIt;
                            }
                        }
                    } else {
                        for (auto agentIt = agents.begin(); agentIt != agents.end(); agentIt++) {
                            if (check.address == (*agentIt)->busAddress()) {
                                *check.instance = *agentIt;
                            }
                        }
                    }

                    if ((*check.instance) == nullptr) {
                        char const* entityType =
                              check.address.type == protocol::NodeType_SERVICE ? "Service"
                            : check.address.type == protocol::NodeType_AGENT   ? "Agent"
                            : check.address.type == protocol::NodeType_TEAM    ? "Team"
                                                                               : "INTERNAL ADDRESS TYPE ERROR";

                        /// \note The event specified a sender/recipient that
                        /// the engine does not have an instance of.
                        error.append("%s specifies non-existent %s", check.errorLabel, entityType);
                    }
                } break;
            }

            if (error.m_size) {
                JACK_BUS("Invalid event [reason={}, event={}]", error.view(), event->toString());
                return false;
            }
        }
    }

    return true;
}

/******************************************************************************
 * Protocol Event Conversion Helpers: Protocol Event Data <-> Engine Event Data
 ******************************************************************************/
/// Given an event with a `parameters` field, get the concrete instance of
/// a goal, retrieve the message and verify that the event's parameters match
/// the goal message.
///
/// If the messages match, convert the `parameters` from the protocol
/// representation to its internal JACK message representation.
///
/// @return True if the event parameters were matched against the goal's
/// message. False otherwise.
template <typename T>
static bool verifyBusEventMessage(const Engine& engine, const T* event, std::string_view goalName)
{
    const Goal* realGoal = engine.getGoal(goalName);
    if (!realGoal) {
        JACK_BUS("Bus event specified goal that does not exist [goal={}]", event->toString());
        return false;
    }

    const MessageSchema* schema = engine.getMessageSchema(realGoal->messageSchema());
    if (!schema && realGoal->messageSchema().size()) {
        JACK_BUS("Bus event specified goal message schema that does not exist [goal={}]", event->toString());
        return false;
    }

/// @todo not sure what this is supposed to be doing
/// redo the checks in the function
/*    if (!schema && event->message.fields().size()) {
        JACK_BUS("Bus event specified goal message fields that do not match the schema [goal={}]", event->toString());
        return false;
    }*/

    if (schema) {
        MessageSchema::VerifyMessageResult verifyResult = schema->verifyMessage(*event->message.get());
        if (!verifyResult.success) {
            JACK_BUS("Bus event goal message has parameters that do not match the schema [goal={}, reason={}]", event->toString() ,verifyResult.msg);
            return false;
        }
    }

    return true;
}

bool Engine::protocolEventHandler(protocol::Event *baseEvent)
{
    ZoneScoped;
    /******************************************************************
     * Verify Base Event Payload
     ******************************************************************/
    std::lock_guard<decltype(m_guardAPI)> guardScope(m_guardAPI);
    Service *recipient = nullptr;
    Service *sender    = nullptr;
    if (!baseProtocolEventCheck(baseEvent, m_busAddress, m_agents, m_services, &recipient, &sender)) {
        return false;
    }

    #define BUS_REQUIRE_FIELD(event, container, reason)                              \
        if (container.empty()) {                                                     \
            JACK_BUS("Invalid event [reason={}, event={}]", reason, event->toString()); \
            return false;                                                            \
        }

    #define BUS_REQUIRE_RANGE(event, value, min, max, reason)                          \
        if ((value) < (min) || (value) > (max)) {                                      \
            JACK_BUS("Invalid event [reason={}, value={}, min/max={}/, event={}]", (reason), (value), (min), (max)); \
            return false;                                                              \
        }

    /// \todo Align the protocol events with internal JACK events.
    /// Right now we have to convert between the formats.
    switch (baseEvent->type) {
        case protocol::EventType_NONE: { assert(!"Internal error: NONE event should be handled in basic event checks"); } break;

        /**************************************************************
         * JACK
         **************************************************************/
        case protocol::EventType_CONTROL: {
            auto* event = dynamic_cast<protocol::Control*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_RANGE(event, event->command, 0, protocol::ControlCommand_COUNT - 1, "Control command value is out of bounds");

            /**********************************************************
             * Process Payload
             **********************************************************/
            auto *jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator, Event, Event::CONTROL);
            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set by basic event checks");
            jackEvent->controlEvent = std::move(*event);
            jackEvent->recipient    = recipient;
            jackEvent->caller       = sender;
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_PERCEPT: {
            auto* event = dynamic_cast<protocol::Percept*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_FIELD(event, event->beliefSet, "Percept beliefset must be non-empty");
            BUS_REQUIRE_FIELD(event, event->field.m_name, "Percept name must be non-empty");
            BUS_REQUIRE_FIELD(event, event->field.m_type, "Percept type must be non-empty");

            if (auto *agentRecipient = dynamic_cast<Agent*>(recipient)) {
                if (event->beliefSet.size() && !agentRecipient->message(event->beliefSet)) {
                    JACK_BUS("Event queried non-existing beliefset in agent [event={}]", event->toString());
                    return false;
                }
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            PerceptEvent *jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                         PerceptEvent,
                                                         /*message*/        event->beliefSet,
                                                         /*isMessage*/      true,
                                                         /*field*/          event->field,
                                                         /*broadcastToBus*/ false);
            if (jackEvent) {
                /// \todo We have C++ constructors that force us to
                /// choose one constructor over the other. What we really
                /// want is to always give it the beliefset name. If it's
                /// empty then it's not a belief percept automatically
                /// ...
                jackEvent->recipient      = recipient;
                jackEvent->caller         = sender;
                jackEvent->broadcastToBus = false;
                recipient->routeEvent(jackEvent);
            }
        } break;

        case protocol::EventType_MESSAGE: {
            auto* event = dynamic_cast<protocol::Message*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_FIELD(event, event->data->schema(), "Schema name must be non-empty");

            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set, this is enforced by basic event checks");
            [[ maybe_unused ]] auto *agentRecipient = dynamic_cast<Agent*>(recipient);
            JACK_ASSERT_MSG(agentRecipient, "Internal error: Recipient should be an agent, this is enforced by basic event checks");

            /**********************************************************
             * Process Payload
             **********************************************************/
            /// \todo We only receive new-style (beliefset/percept) messages
            /// deprecated messages relate to the old message handlers that we
            /// will remove shortly.
            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                 MessageEvent,
                                                 event->data,
                                                 false /*deprecatedMsg*/,
                                                 false /*broadcastToBus*/);
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_PURSUE: {
            auto* event = dynamic_cast<protocol::Pursue*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_FIELD(event, event->goal, "Goal value must be non-empty");

            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set, this is enforced by basic event checks");
            if (dynamic_cast<ProxyAgent*>(recipient)) {
                /// \todo We were listening on the topic but we don't
                /// actually hold the concrete agent. This
                /// should be allowed because proxy agents can toggle
                /// between real and proxy agents in the future.
                ///
                /// Any mapping from concrete agent to node will become
                /// stale over time/outdated or even invalidated and the
                /// message may be lost into the ether if the message
                /// arrives inbetween the mapping being
                /// updated/invalidated.
                ///
                /// Additionally this is a round-trip everytime it
                /// needs to be forwarded on to the "correct" node.
                ///
                /// In the future when ProxyAgents also hold a fully
                /// simulated model of the real agent they also need to
                /// become aware of this command so that they can
                /// advance their internal state.
                break;
            }

            if (!verifyBusEventMessage(*this, event, event->goal)) {
                break;
            }

            if (!event->eventId.valid()) {
                JACK_BUS("Event has invalid UUID [event={}]", event->toString());
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            /// \todo Pass the policy over the bus
            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                 PursueEvent,
                                                 event->goal,
                                                 0 /*NULL_ID*/ /*parent intention*/,
                                                 std::move(event->message),
                                                 event->persistent);
            jackEvent->recipient = recipient;
            jackEvent->caller    = sender;
            jackEvent->eventId   = std::move(event->eventId);
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_DROP: {
            auto* event = dynamic_cast<protocol::Drop*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            #if 0 /// \todo JACK currently doesn't set the goal name, we probably want to set the goal name though.
            if (event->goal.empty() || event->goalId.empty()) {
                JACK_BUS("Bus-event with invalid goal [" << *event << "]");
                break;
            }
            #else
            BUS_REQUIRE_FIELD(event, event->goalId, "Goal value must be non-empty");
            #endif

            UniqueId goalId = UniqueId::initFromString(event->goalId);
            if (!goalId.valid()) {
                JACK_WARNING("Bus-event with invalid UUID for goal [event={}]", event->toString());
                break;
            }

            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set, this is enforced by basic event checks");
            if (dynamic_cast<ProxyAgent*>(recipient)) {
                /// \todo We were listening on the topic but we don't
                /// actually hold the concrete agent. We broadcast to
                /// everyone, so the real agent should get it, we don't
                /// forward it to avoid duplicate events being pushed to
                /// the real agent.
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator, Event, Event::DROP);
            jackEvent->dropEvent = std::move(*event);
            jackEvent->caller    = sender;
            jackEvent->recipient = recipient;
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_DELEGATION: {
            auto* event = dynamic_cast<protocol::Delegation*>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set, this is enforced by basic event checks");
            if (dynamic_cast<ProxyAgent*>(recipient)) {
                /// \todo We were listening on the topic but we
                /// don't actually hold the concrete agent.
                ///
                /// Unlike PURSUE, I don't think we care about this
                /// for DELEGATIONS. Just silently ignore it for
                /// now.
                break;
            }

            Agent *senderAgent = dynamic_cast<Agent*>(sender);
            (void)senderAgent;
            JACK_ASSERT_MSG(sender, "Internal error: Sender should be set, this is enforced by basic event checks");
            JACK_ASSERT_MSG(senderAgent, "Internal error: Sender should be an agent, this is enforced by basic event checks");

            BUS_REQUIRE_FIELD(event, event->goalId, "Goal must be specified");
            UniqueId goalId = UniqueId::initFromString(event->goalId);
            if (!goalId.valid()) {
                JACK_BUS("Event has invalid UUID for goal [event={}]", event->toString());
                break;
            }

            UniqueId teamId = UniqueId::initFromString(event->teamId);
            if (!teamId.valid()) {
                JACK_BUS("Event has invalid UUID for teamId [event={}]", event->toString());
                break;
            }

            Agent *teamAgent = getAgentByUUID(teamId);
            if (!teamAgent) {
                JACK_BUS("Delegation event specifies non-existent team [event={}]", event->toString());
                break;
            }

            if (!verifyBusEventMessage(*this, event, event->goal)) {
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            BUS_REQUIRE_RANGE(event, event->status, 0, protocol::DelegationStatus_SUCCESS, "Delegation status value is out of bounds");
            Event::Status status = {};
            switch (event->status) {
                case protocol::DelegationStatus_PENDING: status = Event::PENDING; break;
                case protocol::DelegationStatus_FAILED:  status = Event::FAIL; break;
                case protocol::DelegationStatus_SUCCESS: status = Event::SUCCESS; break;
                default: assert(!"Internal error: Delegation status enum value out of bounds should be handled earlier"); break;
            }

            auto* jackEvent      = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                      DelegationEvent,
                                                      GoalHandle{event->goal, goalId},
                                                      event->message // copy the shared pointer
            );
                                                      //JACK_INIT_SHARED_PTR(Message, event->message->m_schemaName.size() ? JACK_NEW(Message, event->message) : nullptr));
            jackEvent->status    = status;
            jackEvent->caller    = sender;
            jackEvent->recipient = recipient;
            jackEvent->analyse   = event->analyse;
            jackEvent->score     = event->score;
            jackEvent->team      = teamAgent->handle();
            recipient->routeEvent(jackEvent);
        } break;

        /**************************************************************
         * Protocol
         **************************************************************/
        case protocol::EventType_REGISTER: {
            auto *event = dynamic_cast<protocol::Register *>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            bool requiresTemplate = event->address.type == protocol::NodeType_AGENT ||
                                    event->address.type == protocol::NodeType_TEAM ||
                                    event->address.type == protocol::NodeType_SERVICE;
            if (!event->address.isSet() || (requiresTemplate && event->templateType.empty())) {
                JACK_BUS("Event with invalid property(s) [event={}]", event->toString());
                break;
            }

            /// Verify the ID can be constructed
            UniqueId id = UniqueId::initFromString(event->address.id);
            if (!id.valid()) {
                JACK_BUS("Register has invalid UUID [event={}]", event->toString());
                break;
            }

            if (event->team.isSet()) {
                if (event->team.type != protocol::NodeType_TEAM) {
                    JACK_BUS("Register specifies team bus address [event={}]", event->toString());
                }

                if (event->address.type != protocol::NodeType_AGENT &&
                    event->address.type != protocol::NodeType_TEAM)
                {
                    JACK_BUS(
                        "Register specifies team membership but entity to register is "
                        "incapable of joining a team [event={}]", event->toString());
                    break;
                }

                UniqueId teamId = UniqueId::initFromString(event->team.id);
                if (!teamId.valid()) {
                    JACK_BUS("Register has invalid team UUID [event={}]", event->toString());
                    break;
                }

                Agent *teamAgent = getAgentByUUID(teamId);
                if (!teamAgent) {
                    JACK_BUS("Register specifies non-existent team [event={}]", event->toString());
                    break;
                }

                if (!teamAgent->isTeam()) {
                    JACK_BUS("Register specifies agent that is not a team [event={}]", event->toString());
                    break;
                }
            }

            if (requiresTemplate) {
                if (event->address.type == protocol::NodeType_SERVICE) {
                    auto serviceIt = m_serviceTemplates.find(event->templateType);
                    if (serviceIt == m_serviceTemplates.end()) {
                        JACK_BUS("Register specifies a template that does not exist in the engine [event={}]", event->toString());
                        break;
                    }
                } else {
                    assert(event->address.type == protocol::NodeType_AGENT || event->address.type == protocol::NodeType_TEAM);

                    auto agentIt = m_agentTemplates.find(event->templateType);
                    if (agentIt == m_agentTemplates.end()) {
                        JACK_BUS("Register specifies a template that does not exist in the engine [event={}]", event->toString());
                        break;
                    }

                    if (event->address.type == protocol::NodeType_TEAM) {
                        if (!agentIt->second->isTeam()) {
                            JACK_BUS("Register is requesting to create a proxy team, but the template is not a team [event={}]", event->toString());
                            break;
                        }
                    } else {
                        assert(event->address.type == protocol::NodeType_AGENT);
                        if (agentIt->second->isTeam()) {
                            JACK_BUS("Register is requesting to create a proxy agent, but the template is not an agent [event={}]", event->toString());
                            break;
                        }
                    }
                }
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator, Event, Event::REGISTER);
            jackEvent->registerEvent = std::move(*event);
            jackEvent->caller        = sender;
            routeEvent(jackEvent);
        } break;

        case protocol::EventType_AGENT_JOIN_TEAM: {
            auto *event = dynamic_cast<protocol::AgentJoinTeam *>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            if (!event->team.isSet() || !event->agent.isSet()) {
                JACK_BUS("Event is missing agent/team pair [event={}]", event->toString());
                break;
            }

            UniqueId teamId = UniqueId::initFromString(event->team.id);
            if (!teamId.valid()) {
                JACK_BUS("Event has invalid UUID for team [event={}]", event->toString());
                break;
            }

            UniqueId agentId = UniqueId::initFromString(event->agent.id);
            if (!agentId.valid()) {
                JACK_BUS("Event with invalid UUID for agent [event={}]", event->toString());
                break;
            }

            Agent* agent = getAgentByUUID(agentId);
            Agent* team  = getAgentByUUID(teamId);
            if (!agent) {
                JACK_BUS("Bus event queried non-existing agent [event={}]", event->toString());
                break;
            }

            if (!team) {
                JACK_BUS("Bus event queried non-existing team [event={}]", event->toString());
                break;
            }

            if (!team->isTeam()) {
                JACK_BUS("Bus event queried a team but returned agent is not a team [event={}]", event->toString());
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            dynamic_cast<Team*>(team)->addMemberAgent(agent);
        } break;

        case protocol::EventType_ACTION_BEGIN: {
            auto *event = dynamic_cast<protocol::ActionBegin *>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_FIELD(event, event->name, "Action name must be non-empty");

            const Action *action = getAction(event->name);
            if (!action) {
                JACK_BUS("Bus event queried invalid action [event={}]", event->toString());
                break;
            }

            ActionMessageSchemas actionSchema = getActionMessageSchema(action);
            if (actionSchema.invalidFlags) {
                JACK_BUS("Bus action complete specifies action with invalid request or reply schema [event={}]", event->toString());
                break;
            }

            UniqueId taskId = UniqueId::initFromString(event->taskId);
            if (!taskId.valid()) {
                JACK_BUS("Bus event action task ID is invalid [event={}]", event->toString());
                break;
            }

            UniqueId intentionId = UniqueId::initFromString(event->intentionId);
            if (!intentionId.valid()) {
                JACK_BUS("Bus event action intention ID is invalid [event={}]", event->toString());
                break;
            }

            UniqueId goalId = UniqueId::initFromString(event->goalId);
            if (!goalId.valid()) {
                JACK_BUS("Bus event specified invalid goal ID [event={}]", event->toString());
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                 ActionEvent,
                                                 *this,
                                                 *action,
                                                 GoalHandle{event->goal, goalId},
                                                 intentionId,
                                                 event->plan,
                                                 taskId);
            if (!jackEvent->valid()) {
                JACK_CHUNK_ALLOCATOR_GIVE(&m_eventAllocator, ActionEvent, jackEvent, JACK_ALLOCATOR_CLEAR_MEMORY);
                break;
            }

            jackEvent->request() = std::move(event->message);
            jackEvent->recipient = recipient;
            jackEvent->caller    = sender;
            jackEvent->m_taskId  = std::move(taskId);
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_ACTION_UPDATE: {
            auto *event = dynamic_cast<protocol::ActionUpdate *>(baseEvent);
            JACK_BUS("Event received [event={}]", event->toString());

            /**********************************************************
             * Verify Payload
             **********************************************************/
            BUS_REQUIRE_FIELD(event, event->name, "Action name must be non-empty");

            UniqueId taskId = UniqueId::initFromString(event->taskId);
            if (!taskId.valid()) {
                JACK_BUS("Bus event action task ID is invalid [event={}]", event->toString());
                break;
            }

            if (event->status != protocol::ActionStatus_SUCCESS &&
                event->status != protocol::ActionStatus_FAILED) {
                JACK_BUS("Bus event action status is not success or failure enum [event={}]", event->toString());
                break;
            }

            const Action* action = getAction(event->name);
            if (!action) {
                JACK_BUS("Bus event queried invalid action [event={}]", event->toString());
                break;
            }

            /**********************************************************
             * Process Payload
             **********************************************************/
            Event::Status status = {};
            switch (event->status) {
                case protocol::ActionStatus_SUCCESS: status = Event::SUCCESS; break;
                case protocol::ActionStatus_FAILED:  status = Event::FAIL; break;
                default: { assert(!"Invalid code path: Should be handled in verify check"); } break;
            }

            JACK_ASSERT_MSG(recipient, "Internal error: Recipient should be set, this is enforced by basic event checks");
            assert(sender    && "Internal error: Sender should be set, this is enforced by basic event checks");

            /// Create a reply message
            ActionMessageSchemas actionSchema = getActionMessageSchema(action);
            if (actionSchema.invalidFlags) {
                JACK_BUS("Bus action complete specifies action with invalid request or reply schema [event={}]", event->toString());
                break;
            }

            if (actionSchema.reply) {
                MessageSchema::VerifyMessageResult verifyResult = actionSchema.reply->verifyMessage(*event->reply.get());
                if (!verifyResult.success) {
                    JACK_BUS("Bus event goal message has parameters that do not match the schema [goal={}, reason={}]", event->toString(), verifyResult.msg);
                    break;
                }
            }

            /// @todo populate the reply message from the incoming protocol message
            /// @note for now the message is empty and we don't support action reply message over the network
            auto goalId = jack::UniqueId::initFromString(event->goalId);
            if (!goalId.valid()) {
                JACK_BUS("Bus action complete specifies invalid goal ID [event={}]", event->toString());
                break;
            }

            auto intentionId = jack::UniqueId::initFromString(event->intentionId);
            if (!intentionId.valid()) {
                JACK_BUS("Bus action complete specifies invalid intention ID [event={}]", event->toString());
                break;
            }

            auto* jackEvent = JACK_ALLOCATOR_NEW(&m_eventAllocator,
                                                 ActionCompleteEvent,
                                                 event->name,
                                                 taskId,
                                                 status,
                                                 GoalHandle{event->goal, goalId},
                                                 intentionId,
                                                 event->plan,
                                                 event->reply);

            jackEvent->recipient = recipient;
            jackEvent->caller    = sender;
            recipient->routeEvent(jackEvent);
        } break;

        case protocol::EventType_BDI_LOG: {
            auto* event = dynamic_cast<protocol::BDILog*>(baseEvent);
            if (!event) {
                assert(event);
                break;
            }
            JACK_BUS("Event received [event={}]", event->toString());
        } break;

        default: {
            JACK_BUS("Unhandled event [event={}]", baseEvent->toString());
            return false;
        } break;
    }

    #undef BUS_REQUIRE_FIELD
    #undef BUS_REQUIRE_RANGE
    return true;
}

void Engine::returnEvent(Event *event)
{
    if (!event) {
        JACK_WARNING_MSG("Null event trying to be returned");
        return;
    }

    if (!event->caller) {
        JACK_WARNING_MSG("Trying to return an event with no caller");
        return;
    }

    Service* t       = event->caller;
    event->caller    = event->recipient;
    event->recipient = t;
    routeEvent(event);
}

void Engine::finishCreateAgent(std::string_view templateName, Agent* agent, const UniqueId* uuid)
{
    if (uuid) {
        agent->setUUID(*uuid);
    }

    std::string_view teamLabel  = "Team";
    std::string_view agentLabel = "Agent";
    std::string_view proxyLabel = "Proxy";
    std::string_view type       = agentLabel;

    if (dynamic_cast<ProxyAgent*>(agent)) {
        type = proxyLabel;
    } else if (dynamic_cast<Team*>(agent)) {
        type = teamLabel;
    }

    agent->m_debugState.m_instantiatedFromTemplate = true;

    JACK_INFO("Agent created [type={}, handle={}]", type, agent->handle().toString());
    {
        std::lock_guard<decltype(m_guardAPI)> lock(m_guardAPI);
        for (const Agent *otherAgent : m_agents) {
            if (agent->name() == otherAgent->name()) {
                JACK_WARNING("Newly created agent has the same name as an existing agent [name={}]", agent->name());
                break;
            }
        }
        m_agents.push_back(agent);
    }

    /// \todo Protocol events creation and submission to the bus should be
    /// centralised into the event queue. Currently creating an agent is not an
    /// event, it probably should be. Even if it never turned out to be an
    /// event, there should be some BDI logging event for explainability
    /// submitted to the event queue where we can construct the protocol event.
    ///
    /// For early prototyping, I'm constructing them inline to reduce the code
    /// changes in this first draft.
    if (type != proxyLabel && haveBusAdapter()) {
        protocol::Register busEvent = makeProtocolEvent<protocol::Register>();
        busEvent.proxy              = true;
        busEvent.address            = agent->busAddress();
        busEvent.templateType       = templateName;
        sendBusEvent(&busEvent);
    }
}
} /// namespace aos::jack
