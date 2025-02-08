// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/agent.h>

#include <jack/agentexecutor.h>               // for AgentExecutor, AgentExecutor...
#include <jack/engine.h>                      // for Engine
#include <jack/engine/intentionexecutor.h>    // for IntentionExecutor, Intention...
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
#include <jack/tasks/actiontask.h>            // for ActionTask
#include <jack/goal.h>                        // for Goal
#include <jack/message.h>                     // for Message
#include <jack/plan.h>                        // for Plan
#include <jack/proxyagent.h>                  //
#include <jack/resource.h>                    // for Resource
#include <jack/role.h>                        // for Role, RoleBeliefSet
#include <jack/schedule.h>                    // for Schedule, JACK_SCHEDULE_COST...
#include <jack/tasks/task.h>                  // for Task
#include <jack/team.h>                        // for Team
#include <jack/utils.h>                       // for bytesSizeString
#include <jack/fieldregistry.h>
#include <jack/event-protocol/protocol.h>

/// Third Party
#include <tracy/Tracy.hpp>
#include <cassert>       // for assert
#include <cstddef>       // for size_t
#include <cstring>       // for memcmp
#include <iterator>      // for back_insert_iterator, back_i...
#include <stack>         // for stack
#include <type_traits>   // for forward, move
#include <unordered_set> // for unordered_set, _Uset_traits<...
#include <utility>       // for min, exchange, max


namespace aos::jack
{
bool CompareTimerEvent::operator()(const TimerEvent *a, const TimerEvent *b)
{
    auto aEndTime = a->internalClock + a->duration;
    auto bEndTime = b->internalClock + b->duration;
    return aEndTime > bEndTime;
}

/* ************************************************************************************************
 * Public Ctor & Dtor
 * ************************************************************************************************/
void Agent::initConstants()
{
    m_busAddress.type   = protocol::NodeType_AGENT;
    m_schedule          = nullptr;
    m_scheduleIdCounter = 0;
    m_debugState        = {};
}

Agent::Agent(Engine& engine, std::string_view name)
        : Service(engine, name)
        , m_executor(this)
{
    initConstants();
}

Agent::Agent(const Agent* other, std::string_view newName)
    : Service(other->m_engine, newName)
    , m_plans(other->m_plans)
    , m_roles(other->m_roles)
    , m_initialDesires(other->m_initialDesires)
    , m_resourcesToGoals(other->m_resourcesToGoals)
    , m_actionHandlers(other->m_actionHandlers)
    , m_messageHandlers(other->m_messageHandlers)
    , m_beliefsetIds(other->m_beliefsetIds)
    , m_resourceIds(other->m_resourceIds)
    , m_executor(this)
    , m_currentTactics(other->m_currentTactics)
{
    initConstants();

    // avoid duplicate beliefsets for now
    std::unordered_set<const std::string*> requiredBeliefSets;

    // ensure construction of the belief sets
    for(const std::string &bsName : m_beliefsetIds) {
        requiredBeliefSets.insert(&bsName);
    }

    // extract the beliefset required by the roles
    for (const std::string &roleName : m_roles) {
        const Role* role = m_engine.getRole(roleName);
        if (role != nullptr) {
            const std::vector<RoleBeliefSet> &bs = role->beliefsets();
            for (const RoleBeliefSet &roleBeliefSet : bs) {
                requiredBeliefSets.insert(&roleBeliefSet.m_name);
            }
        } else {
            JACK_ERROR("Unknown role used by agent [role={}]", roleName);
        }
    }

    /// todo What do we do if a role specifies a beliefset but we don't have
    /// a beliefset in our beliefsetIds? This code takes a union of the two so
    /// the / agent always instantiates the correct beliefsets it needs.
    /// And how do we portray this nicely to the user?
    ///
    /// This means there's 2 ways of adding a beliefset to the agent where it
    /// might be worth making it just one way. An idea may be that builders take
    /// a beliefset, by default that appends to a "default" agent role that is
    /// unique to the template of the agent we're building.
    ///
    /// By default this beliefset will not be read/writeable to the team. In
    /// this sense, all agents have a default role, and the UX would appear
    /// nicer in a GUI tool, where we'd only have 1 way of adding a beliefset to
    /// an agent.

    for (const std::string *bsName : requiredBeliefSets) {
        const MessageSchema* schema = m_engine.getMessageSchema(*bsName);
        if (!schema) {
            JACK_ERROR("Message schema is missing, message was not constructed [agent={}, msg={}]", handle(), *bsName);
            continue;
        }

        std::unique_ptr<Message> msg = schema->createMessage();

        if (msg) {
            m_context.addMessage(std::move(msg));
        } else {
            assert(msg && "Memory allocation failure");
        }
    }

    // ensure construction of the resources
    for(const std::string &rsName : m_resourceIds) {
        Resource *ptr = m_engine.createResource(rsName);
        ptr->setAgent(this);
        //ptr->setContext(&m_context);
        m_context.addResource(ptr);
    }
}

Agent::~Agent()
{
    if (!m_debugState.m_startedAtLeastOnce && m_debugState.m_instantiatedFromTemplate) {
        JACK_WARNING("Agent was created but not started at any point [name={}]", handle().toString());
    }

    if (isLogged(log::Severity::Debug)) {
        size_t openHWM    = m_debugState.m_searchNodeOpenHighWaterMark;
        size_t closedHWM  = m_debugState.m_searchNodeClosedHighWaterMark;
        size_t pendingHWM = m_debugState.m_searchNodePendingHighWaterMark;
        size_t totalHWM   = openHWM + closedHWM + pendingHWM;

        char openHWMByteString[128];
        char pendingHWMByteString[128];
        char closedHWMByteString[128];
        char totalHWMByteString[128];
        readableByteSizeString(openHWMByteString,    sizeof(openHWMByteString),    openHWM    * sizeof(SearchNode));
        readableByteSizeString(pendingHWMByteString, sizeof(pendingHWMByteString), pendingHWM * sizeof(SearchNode));
        readableByteSizeString(closedHWMByteString,  sizeof(closedHWMByteString),  closedHWM  * sizeof(SearchNode));
        readableByteSizeString(totalHWMByteString,   sizeof(totalHWMByteString),   totalHWM   * sizeof(SearchNode));

        JACK_DEBUG("SearchNode stats [agent={}, total={} ({}), open={} ({}), pending={} ({}), closed={} ({})]",
                   name(),
                   totalHWM, totalHWMByteString,
                   openHWM, openHWMByteString,
                   pendingHWM, pendingHWMByteString,
                   closedHWM, closedHWMByteString);

    }

    for (Goal *goal : m_desires) {
        /// \todo This is problematic because we have tests with lambdas that
        /// poke this same agent that we're destructing which is causing nasty
        /// behaviour.
        /// goal->fail();
        JACK_DELETE(goal);
    }
    m_desires.clear();

    for (auto &it : m_delegationEventBackLog) {
        /// \todo WARNING If the promise generates more data to pipe into the
        /// engine that data may be leaked as we're in the destructor of this
        /// agent and generating new data that we're not handling.
        DelegationEvent *event = it.delegationEvent;
        event->fail();
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, DelegationEvent, event, JACK_ALLOCATOR_CLEAR_MEMORY);
    }
    m_delegationEventBackLog.clear();

    while (hasTimerEvents()) {
        TimerEvent *timerEvent = m_timerEvents.top();
        m_timerEvents.pop();
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, TimerEvent, timerEvent, JACK_ALLOCATOR_CLEAR_MEMORY);
    }

    JACK_ALLOCATOR_DELETE(&m_heapAllocator, m_schedule);
    /// \note Current pending actions are freed in the Service destructor

    /// Dump allocator information to the logger
    /// \todo Only dump the data if the agent is not a template agent- we shold
    /// stop instantiating template agents but we need to do this to do the
    /// 'prototype' pattern and clone runtime agents from the template.
    static const std::string templateSuffix = "Template";
    if (name().size() < templateSuffix.size() ||
        memcmp(name().data() + (name().size() - templateSuffix.size()), templateSuffix.data(), templateSuffix.size()) != 0) {
        FixedString<512> heapMetrics  = allocatorMetricsLog(m_heapAllocator.metrics());
        FixedString<512> arenaMetrics = arenaMetricsLog(m_tickAllocator.metrics());
        JACK_INFO("Heap Alloc [owner={}, info={}]", m_handle.toString(), heapMetrics.view());
        JACK_INFO("Tick Alloc [owner={}, info={}]", m_handle.toString(), arenaMetrics.view());
    }
    m_tickAllocator.freeAllocator(true /*clearMemory*/);
}

/******************************************************************************
 * Public Functions
 ******************************************************************************/
std::vector<std::string> Agent::goals() const
{
    std::vector<std::string> result;
    for (const std::string& planName : m_plans) {
        bool found       = false;
        if (const Plan *plan = m_engine.getPlan(planName)) {
            JACK_ASSERT(plan->goal().size());
            for (size_t resultIndex = 0; !found && resultIndex < result.size(); resultIndex++) {
                const std::string& goal = result[resultIndex];
                found = plan->goal() == goal;
            }

            if (!found) {
                result.push_back(std::string(plan->goal()));
            }
        } else {
            static std::set<std::string> errorMap;
            if (errorMap.count(planName) == 0) {
                errorMap.insert(planName);
                JACK_ERROR("Agent references non-existent plan [agent={}, plan={}]", handle().toString(), planName);
            }
        }
    }

    return result;
}

std::vector<std::string> Agent::desires() const
{
     /*! *******************************************************************************************
     * Return a list of the current goals this agent desires
     * *******************************************************************************************/
    std::vector<std::string> result(m_desires.size());
    for (size_t desireIndex = 0; desireIndex < m_desires.size(); desireIndex++)
        result[desireIndex] = m_desires[desireIndex]->name();
    return result;
}

const Goal* Agent::getDesire(const UniqueId &id) const
{
    for (auto it = m_desires.cbegin(); it != m_desires.cend(); ++it) {
        const Goal *goal = *it;
        if (goal->id() == id) {
            return goal;
        }
    }
    return nullptr;
}

Goal* Agent::getDesire(const UniqueId &id)
{
    Goal* result = const_cast<Goal*>(const_cast<const Agent *>(this)->getDesire(id));
    return result;
}

const Goal* Agent::getDesire(std::string_view name) const
{
    for (auto it = m_desires.cbegin(); it != m_desires.cend(); ++it) {
        const Goal *goal = *it;
        if (goal->name() == name) {
            return goal;
        }
    }
    return nullptr;
}

Goal* Agent::getDesire(std::string_view name)
{
    Goal* result = const_cast<Goal*>(const_cast<const Agent *>(this)->getDesire(name));
    return result;
}

const Tactic* Agent::currentTactic(std::string_view goal) const
{
    const Tactic* result = nullptr;
    auto it = m_currentTactics.find(goal);
    if (it != m_currentTactics.end()) {
        result = it->second;
    }

    /// \note If the tactic does not exist, we will by default use a pursue
    /// policy
    if (!result) {
        TacticHandle builtin = engine().getBuiltinTactic(goal);
        if (builtin.valid()) {
            result = engine().getTactic(builtin.m_name);
        } else {
            /// \note Degenerate case, the inbulit tactic was deleted and
            /// doesn't exist or the goal is straight out non-existent. In this
            /// case we return the nil tactic, e.g. all default values.
            ///
            /// The engine asserts if this is the case, in release mode the nil
            /// tactic will be returned.
            static const Tactic nil = {};
            result                  = &nil;

            JACK_ASSERT_MSG(
                engine().getGoal(goal) != nullptr,
                "Goal exists but the builtin tactic does not exist. All "
                "tactics are created on goal commit, at some point the tactic "
                "was invalidly deleted internally in the engine which is not "
                "expected behaviour");
        }
    }
    return result;
}

Span<const IntentionExecutor*> Agent::intentions() const
{
     /*! *******************************************************************************************
     * Return a list of the current intentions for this agent
     * *******************************************************************************************/
    return m_executor.intentions();
}

const std::vector<const Plan*>& Agent::getGoalPlans(const Goal* goal)
{
    ZoneScoped;
    /*! ***************************************************************************************
     * Get the list of plans supporting a goal
     * 1. Check the cache for a prior generated list for the goal and early return if possible
     * 2. Generate a new list in the cache and enumerate the plans that support the goal
     * ****************************************************************************************/

    /// Step 1
    std::string const &goalName = goal->name();
    auto it = m_goalPlans.find(goalName);
    if (it != m_goalPlans.end()) {
        return it->second;
    }

    /// Step 2
    std::vector<const Plan*>& plans = m_goalPlans[goalName];
    for (const std::string &planName : m_plans) {
        const Plan *plan = m_engine.getPlan(planName);
        if (plan != nullptr && plan->goal() == goalName) {
            plans.push_back(plan);
        }
    }

    return plans;
}

/// @todo: Use a tactic handle
/// @todo: Add globally static handles when code generating JACK model data
bool Agent::selectTactic(std::string_view tactic)
{
    const Tactic *tacticPtr = m_engine.getTactic(tactic);
    if(!tacticPtr) {
        JACK_WARNING("Requested tactic does not exist [tactic={}, agent={}]", tactic, handle().toString());
        return false;
    }

    routeEvent(JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, TacticEvent, tacticPtr->handle()));
    return true;
}

bool Agent::setTactic(const TacticHandle& handle)
{
    const Tactic *tactic = m_engine.getTactic(handle.m_name);
    if (!tactic) {
        JACK_WARNING("Tactic to set does not exist [agent={}, tactic={}]", this->handle().toString(), handle.toString());
        return false;
    }
    JACK_ASSERT(tactic->goal().size());

    JACK_INFO("Tactic set [agent={}, tactic={}]", this->handle().toString(), handle.toString());
    const std::string &goalName = tactic->goal();
    m_currentTactics[goalName]  = tactic;

    const Goal* goal = m_engine.getGoal(goalName);
    m_goalTacticPlans.erase(goalName);

    /// Precalculate the list of concrete plans for the goal
    getGoalTacticPlans(goal);

    /// \note Tactic has changed, reset the plan selection history for the goal
    /// if it already exists
    for (Goal *desire : m_desires) {
        if (desire->name() == tactic->goal()) {
            desire->m_planSelection          = {};
            desire->m_planSelection.m_tactic = tactic->handle().m_name;
        }
    }
    return true;
}

const std::vector<const Plan*>& Agent::getGoalTacticPlans(const Goal* goal)
{
    ZoneScoped;
    const std::string& goalName           = goal->name();
    const std::vector<const Plan*>& plans = getGoalPlans(goal);

    /// \note No tactic set for goal
    auto tacticIt = m_currentTactics.find(goalName);
    if (tacticIt == m_currentTactics.end()) {
        return plans;
    }

    /// \note Try and return plan list from cache
    const Tactic* tactic = tacticIt->second;
    auto it = m_goalTacticPlans.find(goalName);
    if (it != m_goalTacticPlans.end()) {
        return it->second;
    }

    /// \note Get the plans supported by the tactic
    std::vector<const Plan*>& goalTacticPlans = m_goalTacticPlans[goalName];
    for (const Plan* plan : plans) {
        if (tactic->planAllowed(plan->name())) {
            goalTacticPlans.push_back(plan);
        }
    }

    return goalTacticPlans;
}

Agent::HandlesActionResult Agent::handlesAction(std::string_view action, HandlesActionSearch search) const
{
    JACK_ASSERT(search >= 0 && search <= HandlesActionSearch_ALL);
    HandlesActionResult result = {};
    if (search & HandlesActionSearch_AGENT) {
        result.agent = m_actionHandlers.count(action);
    }

    if (search & HandlesActionSearch_SERVICE) {
        if (m_engine.settings.unhandledActionsForwardedToFirstApplicableService) {
            for (const ServiceHandle& handle : m_engine.serviceList()) {
                const Service* service = m_engine.getService(handle);
                if (service && service->isAvailable() && service->handlesAction(action)) {
                    result.service = service->handle();
                    break;
                }
            }
        } else {
            for (const ServiceHandle& handle : attachedServices()) {
                const Service* service = m_engine.getService(handle);
                if (service && service->isAvailable() && service->handlesAction(action)) {
                    result.service = service->handle();
                    break;
                }
            }
        }
    }

    return result;
}

Agent::HandlesPlanResult Agent::handlesPlan(Plan const *plan, HandlesActionSearch search)
{
    HandlesPlanResult result = {};
    if (!plan) {
        return result;
    }

    if (m_handlesPlanCachedOnTick != m_engine.pollCount()) {
        m_handlesPlanCache.clear();
        m_handlesPlanCachedOnTick = m_engine.pollCount();
    }

    auto it = m_handlesPlanCache.find(plan);
    if (it == m_handlesPlanCache.end()) {
        result.success = true;
        for (const auto* rawTask : plan->body()->tasks()) {
            const auto* task = dynamic_cast<const ActionTask*>(rawTask);
            if (!task) {
                continue;
            }

            Agent::HandlesActionResult handler = handlesAction(task->action(), search);
            if (!handler.agent && !handler.service.valid()) {
                result.success = false;
                result.action = task->action();
                break;
            }
        }

        m_handlesPlanCache[plan] = result;
    } else {
        result = it->second;
    }

    return result;
}


/// \todo This is one of the bottle necks of our hot loop once team/agent
/// scenarios are doing a lot of work and it's a static/side-effect-less
/// function which means this is trivially calculatable out of
/// band for the main thread!

/// Expand the schedule's search tree and form nodes (goals and plans) to
/// execute in the A* tree.
/// @param schedule The schedule to process. This parameter can be nullptr
/// and will do a no-op and cause the function to return false.
/// @return True if the schedule has finished planning, false if otherwise
static bool processSchedule(Schedule *schedule)
{
    ZoneScoped;
    if (!schedule) {
        return false;
    }

    // finish the planning or stop if the schedule is pending (e.g. waiting for a delegation auction)
    int const MAX_ITERATIONS = 64;
    for (int iterations = 0;
         (!schedule->isFinished() && !schedule->isWaitingForAuctions()) && (iterations < MAX_ITERATIONS);
         iterations++)
    {
        /*! ***************************************************************************************
        * 2. A.2.3.2 Expand Schedule
        * Expand a list of candidate plans that could form intentions
        * ****************************************************************************************/
        schedule->expand();

        /*! ***************************************************************************************
        * 3. A.2.3.3 Cost Schedule
        * ****************************************************************************************/
        schedule->cost();

        /*! ***************************************************************************************
        * 4. A.2.3.4 Optimise Schedule
        * ****************************************************************************************/
        schedule->deconflict();

        /*! ***************************************************************************************
        * 5. A.2.3.5 Bind Schedule
        * ****************************************************************************************/
        schedule->bind();

        /// \note Assert that we can never transition into waiting for an
        /// auction. We currently only support this when we create the schedule
        /// not any point after that.
        assert(!schedule->isWaitingForAuctions());
    }

    bool result = schedule->isFinished();
    if (result) {
        schedule->deconflict();
    }

    return result;
}

void Agent::eventDispatch(Event *rawEvent)
{
    ZoneScoped;
    // JACK_DEBUG("Processing events");
    if (rawEvent->recipient) {
        if (rawEvent->recipient->UUID() != UUID()) {
            rawEvent->caller = this; // It's not for this agent route to the engine
            m_engine.routeEvent(rawEvent);
            return;
        }

        JACK_ASSERT_MSG(rawEvent->recipient->UUID() == UUID(), "Recipient should be us");
    }

    if (rawEvent->type != Event::CONTROL) {
        if (!running()) {
            m_debugState.m_eventBackLog++;
        }
    }

    /// \note Unconditionally transmit events that are only transmitted if this
    /// agent was a proxy (this ensures that the messages that were routed
    /// locally- always be routed on the bus. The engine ignores messages that
    /// comes from itself- but this allows listening applications to see what is
    /// happening).
    ///
    /// Before this change, we'd only send messages onto the bus if the required
    /// agent was a proxy (e.g. the real instance is on another node). But if
    /// all agents are local, then JACK will onyl route messages locally, they
    /// never show up on the bus so listening applications cannot reason about
    /// what is going on in the system.
    ///
    /// \todo This is done badly because I merely copied the code out from the
    /// ProxyAgent, to reuse the code from a ProxyAgent for a real Agent (e.g.
    /// to ensure all messages that can be routed locally always appear on the
    /// bus). Shortcut taken because of time constraints and stability of the
    /// engine for a demo.
    if (rawEvent->type != Event::ACTION) {
        ProxyAgent::ensureThatEventsThatMightOnlyBeRoutedLocallyGetRoutedOntoTheBus(*this, rawEvent, false /*proxy*/);
    } else {
        /// \todo Actions are special. We may forward this event onto a service
        /// if we're unable to handle it. If the service is a proxy, it may
        /// forward it onto the real agent.
        ///
        /// In that instance, a bus event is created with the service set as the
        /// recipient.
        ///
        /// We can't blanket send action events onto the bus here. We need to
        /// check if we locally handle it, or, otherwise, the event will get
        /// routed to the service and sent on the bus from there.
        ///
        /// These are all work-arounds that are compounding on the various
        /// work-arounds to get this working whilst preserving v4 stability.
    }

    bool returnEventToAllocator = true;
    JACK_DEFER {
        if (returnEventToAllocator) {
            JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, Event, rawEvent, JACK_ALLOCATOR_CLEAR_MEMORY);
        }
    };

    switch (rawEvent->type) {
        case Event::TIMER: {
            ZoneNamedN(debugTracyEngineSleep, "Agent sleep event", true);
            TimerEvent* event = static_cast<TimerEvent*>(rawEvent);
            ZoneValueV(debugTracyEngineSleep, event->duration.count());

            m_timerEvents.push(event);

            if (m_engine.haveBusAdapter()) {
                protocol::BDILog logEvent = m_engine.makeBDILogSleepStarted(
                    bdiLogHeader(protocol::BDILogLevel_NORMAL),
                    event->goal.m_name,
                    event->goal.m_id.toString(),
                    event->intentionId.toString(),
                    event->plan,
                    event->taskId.toString(),
                    event->duration.count());

                m_engine.sendBusEvent(&logEvent);
            }

            /// \note Timer events live until the timer is triggered and deleted
            /// there.
            returnEventToAllocator = false;
        } break;

        case Event::CONTROL: {
            ZoneNamedN(debugTracyEngineControl, "Agent control event", true);

            State prevState = m_state;
            Service::eventDispatch(rawEvent); /// Forward event to common implementation
            bool stateTransition = prevState != m_state;

            /// \note This return event to allocator stuff sucks, the problem is
            /// that event lifetime is not trivial (but it can be made trivial).
            /// Some event things persist the event for other purposes, we
            /// should try to avoid this and make it always returnable after
            /// completing event dispatch.
            returnEventToAllocator = false; /// Event returned when forwarded to Service.

            if (stateTransition) {
                if (m_state == RUNNING) {
                    m_scheduleDirty                   |= ScheduleDirty_AGENT_STARTED; // a new schedule
                    m_debugState.m_startedAtLeastOnce  = true;
                } else if (m_state == STOPPED || m_state == PAUSED) {
                    /// \note Reset the warning flag, this will allow the agent to
                    /// re-log a warning every time it is stopped and events start
                    /// backing up in the agent.
                    m_debugState.m_eventBackLogWarning = false;
                    m_debugState.m_eventBackLog        = 0;
                } else if (m_state == STOPPING) {
                    m_executor.stop();
                }
            }
        } break;

        case Event::MESSAGE: {
            ZoneNamedN(debugTracyAgentMessage, "Agent message event", true);
            MessageEvent*      event   = static_cast<MessageEvent*>(rawEvent);
            const std::string& msgName = event->msg->schema();
            ZoneNameV(debugTracyAgentMessage, msgName.c_str(), msgName.size());

            if (event->deprecatedMsg) {
                // handle the message event
                if (m_messageHandlers.find(msgName) != m_messageHandlers.end()) {
                    // call the agents message handler directly
                    m_messageHandlers[msgName](*this, *event->msg);
                } else {
                    JACK_WARNING("Message is not handled by the agent [name={}]", msgName);
                }
            } else {
                m_scheduleDirty               |= ScheduleDirty_MESSAGE;
                m_lastTimeBeliefSetsDirtied   = std::chrono::duration_cast<std::chrono::milliseconds>(engine().internalClock());
                std::shared_ptr<Message> bSet = message(msgName);
                if (bSet) {
                    std::shared_ptr<Message> msg = std::move(event->msg);
                    context().addMessage(msg);
                    if (event->broadcastToBus && engine().haveBusAdapter()) {
                        if (!event->recipient) {
                            event->recipient = this;
                        }

                        auto busEvent = m_engine.makeProtocolEvent<protocol::Message>(
                            /*sender*/     event->caller ? rawEvent->caller->busAddress() : jack::protocol::BusAddress(),
                            /*recipient*/  event->recipient->busAddress(),
                            /*eventId*/    event->eventId);

                        busEvent.data = msg;
                        engine().sendBusEvent(&busEvent);
                    }
                }

            }
        } break;

        case Event::TACTIC: {
            ZoneNamedN(debugTracyAgentTactic, "Agent tactic event", true);
            TacticEvent *event = static_cast<TacticEvent*>(rawEvent);
            if (setTactic(event->handle)) {
                ZoneNameV(debugTracyAgentTactic, event->handle.m_name.c_str(), event->handle.m_name.size());
                m_scheduleDirty |= ScheduleDirty_TACTICS_CHANGED;
            }
        } break;

        case Event::ACTIONCOMPLETE: {
            ZoneNamedN(debugTracyAgentActionComplete, "Agent action complete event", true);
            ActionCompleteEvent* event = static_cast<ActionCompleteEvent*>(rawEvent);
            ZoneNameV(debugTracyAgentActionComplete, event->name.data(), event->name.size());
            assert(event->actionStatus == Event::SUCCESS ||
                   event->actionStatus == Event::FAIL);

            /// Return the action message back to the Task
            /// \todo Currently actions are local and piped directly from
            /// a coroutine to the responsible agent. This allows us to make an
            /// assertion that the ID corresponds to a task ID, but this is not
            /// an assertion we can make in the future once we have to pipe
            /// actions across a completely different agent/service/process
            /// boundary.
            ///
            /// We would have to do a look-up and determine what this ID is and
            /// where it needs to go (is it local? or does it need to go onto
            /// the bus and be re-routed?)

            if (!m_executor.onActionTaskComplete(event->intentionId, event->taskId, event->actionStatus == Event::SUCCESS, event->reply())) {
                JACK_WARNING("Action completed but associated task no longer exists [action={}, taskId={}]",
                             event->name,
                             event->taskId.toString());
            }
        } break;

        case Event::ACTION: {
            /// Retrieve the action
            ZoneNamedN(debugTracyAgentAction, "Agent action event", true);
            ActionEvent *event = static_cast<ActionEvent *>(rawEvent);
            ZoneNameV(debugTracyAgentAction, event->name().data(), event->name().size());

            if (!event->valid()) {
                JACK_ERROR_MSG("Action was not constructed properly");
                break;
            }
            std::string_view actionName = event->name();
            if (m_engine.haveBusAdapter()) {
                protocol::BDILog logEvent = m_engine.makeBDILogActionStarted(
                    bdiLogHeader(protocol::BDILogLevel_NORMAL),
                    event->m_goal.m_name,
                    event->m_goal.m_id.toString(),
                    event->m_intentionId.toString(),
                    event->m_plan,
                    event->m_taskId.toString(),
                    actionName);

                m_engine.sendBusEvent(&logEvent);
            }

            auto actionHandlerIt = m_actionHandlers.find(actionName);
            if (actionHandlerIt != m_actionHandlers.end()) {
                /// Lock the resources
                BeliefContext &beliefContext = context();
                beliefContext.lockResources(event->m_resourceLocks);

                assert(beliefContext.hasResourceViolation(nullptr) == false &&
                       "Resources were not locked and unlocked in tandem correctly there's a lock "
                       "leakage in the codebase.");

                /// \todo We should re-evaluate re-fast-tracking the action
                /// status here if it finished after the initial handler call
                /// here instead of waiting on the main loop that evaluates all
                /// the current actions and sends off the ActionCompleteEvent.
                /// It just simplifies the code here greatly for now to not
                /// fast-track it.
                const auto &actionFunction = actionHandlerIt->second;

                event->status = actionFunction(*this, *event->request(), *event->reply(), event->handle());

                /// \todo Workaround, see the top of this function where we call
                /// this function
                ProxyAgent::ensureThatEventsThatMightOnlyBeRoutedLocallyGetRoutedOntoTheBus(*this, rawEvent, false /*proxy*/);

                /// Try and fast track the action, no-op if not possible
                if (!processCompletedAction(event)) {
                    m_currentActions.push_back(event);
                }
            } else {
                // This agent doesn't directly handle the action, route to the
                // engine for dispatch to the service.
                JACK_DEBUG("Action routed to agent but it does not directly handle it [agent={}, action={}]", handle().toString(), actionName);
                assert(!event->recipient);
                m_engine.routeEvent(event);
            }
            returnEventToAllocator = false;
        } break;

        case Event::PERCEPT: {
            ZoneNamedN(debugTracyAgentPercept, "Agent percept event", true);
            PerceptEvent* event = static_cast<PerceptEvent*>(rawEvent);
            ZoneNameV(debugTracyAgentPercept, event->name.data(), event->name.size());

            /// \todo A.1 Reason about this incoming percept, e.g. reason(perceptEvent)

            /// \note: Apply the percept to the belief set
            bool handled = true;
            if (event->isMessage) {
                std::shared_ptr<Message> bSet = message(event->name);
                if (bSet) {
                    ZoneNamedN(debugTracyApplyPercept, "Apply percept value", true);
                    handled = bSet->setField(event->field.m_name, event->field.m_value);
                } else {
                    JACK_DEBUG("Percept received but references non-existent message [agent={}, message={}]", handle().toString(), event->name);
                }

            } /// else assume resource percept is handled since it is currently applied immediately.

            if (handled) {
                ZoneNamedN(debugTracyApplyPercept, "Broadcast percept to bus", true);
                m_scheduleDirty |= ScheduleDirty_PERCEPT;
                m_lastTimeBeliefSetsDirtied = std::chrono::duration_cast<std::chrono::milliseconds>(engine().internalClock());
                if (event->broadcastToBus && engine().haveBusAdapter()) {
                    if (!event->recipient) {
                        event->recipient = this;
                    }

                    auto busEvent = m_engine.makeProtocolEvent<protocol::Percept>(/*sender*/    event->caller ? event->caller->busAddress() : jack::protocol::BusAddress(),
                                                                                  /*recipient*/ event->recipient->busAddress(),
                                                                                  /*eventId*/   event->eventId);
                    busEvent.beliefSet = event->name;
                    busEvent.field     = std::move(event->field);
                    engine().sendBusEvent(&busEvent);
                }
            }
        } break;

        case Event::PURSUE: {
            ZoneNamedN(debugTracyAgentPursue, "Agent pursue event", true);
            auto* event = static_cast<const PursueEvent*>(rawEvent);
            ZoneNameV(debugTracyAgentPursue, event->goal.data(), event->goal.size());

            // merge goals with the same id
            bool merged = false;
            for (Goal *g : m_desires) {
                if (g->id() == event->eventId) {
                    JACK_DEBUG("Goal pursue with same ID, merging parameters [agent={}, goal={}, params={}]",
                               handle(),
                               g->handle(),
                               event->parameters->toString());
                    /// Wtodo handle if the parameters or the type of the goal has changed...

                    /// @todo: There's a bug here where we don't trigger
                    /// the goal promise.
                    // take on the new promise instead
                    g->setPromise(event->strongPromise);

                    // take the new parameters
                    BeliefContext &context = g->context();
                    context.setGoalContext(event->parameters);

                    merged = true;
                    break;
                }
            }

            if (merged) {
                break;
            }

            // attempt to merge similar persistamt root goals
            if(event->persistent) {

                // merge the repeated desires
                bool found = false;
                for (Goal *g : m_desires) {
                    if (g->name() == event->goal) {

                        // if there are no parameter for this goal then it is already a match
                        if (!g->context().goal()) {

                            found = true;

                            // take the new promise
                            g->setPromise(event->strongPromise);

                            break;
                        }

                        /// Compare the two goal messages
                        if (g->context().goal() == event->parameters) {

                            found = true;

                            // take the new promise
                            g->setPromise(event->strongPromise);

                            break;
                        }
                    }
                }

                // already have this goal
                if (found) {
                    JACK_DEBUG("Dropping duplicate goal [agent={}, goal={}]", handle().toString(), event->goal);
                    break;
                }
            }

            const Goal *goal = m_engine.getGoal(event->goal);
            std::string_view label = event->parentId ? "Goal" : "Sub-goal";
            if (!goal) {
                JACK_ERROR("{} pursue rejected, unknown goal requested [agent={}, goal={}]", label, m_handle.toString(), event->goal);
                break;
            }

            /// \note Verify the goal message parameterisation is correct
            if (goal->messageSchema().size() && !event->parameters) {
                JACK_WARNING("{} pursue rejected, goal requires a message but the event did not specify the message [agent={}, goal={}, message={}]",
                             label,
                             m_handle,
                             event->goal,
                             goal->messageSchema());
                break;
            } else if (goal->messageSchema().empty() && event->parameters) {
                JACK_WARNING("{} pursue rejected, event has a message but the goal does not accept a message [agent={}, goal={}, eventMsg={}]",
                             label,
                             m_handle,
                             event->goal,
                             event->parameters->toString());
                break;
            }

            /// \note Verify the event message if available
            if (event->parameters) {

                auto msg = event->parameters;
                
                if (goal->messageSchema() != msg->schema()) {
                    JACK_WARNING("{} pursue rejected, goal schema does not match the schema specified in the event [agent={}, goal={}, goalMsg='{}', eventMsg={}]",
                                 label,
                                 m_handle,
                                 goal->messageSchema(),
                                 msg->toString(),
                                 event->parameters->toString());
                    break;
                }

                const MessageSchema *schema = engine().getMessageSchema(msg->schema());
                if (!schema) {
                    JACK_WARNING("{} pursue rejected, event schema specified does not exist [agent={}, goal={}]",
                                 label,
                                 m_handle,
                                 event->goal);
                    break;
                }

                MessageSchema::VerifyMessageResult verifyResult = schema->verifyMessage(*msg.get());
                if (!verifyResult.success) {
                    JACK_WARNING("{} pursue rejected, message does not match the schema [error={}]",
                                 label,
                                 verifyResult.msg);
                    break;
                }
            }

            // configure the desire from the goal template
            Goal* desiredGoal = JACK_CLONE_BDI_OBJECT(goal);
            desiredGoal->setId(event->eventId);
            desiredGoal->setDelegated(getGoalPlans(goal).empty());

            // Find the (intention) executor that triggered this pursue if there is any (i.e.
            // sub-goal) and create an association between the two.
            IntentionExecutor *parentIntention = m_executor.findMatchingIntentionByIntentionId(event->parentId);
            if (parentIntention) {
                parentIntention->addSubGoalDesireId(desiredGoal->id());
                Goal::Parent parent = {};
                parent.handle       = parentIntention->goalPtr()->handle();
                parent.planTaskId   = event->parentTaskId;
                desiredGoal->setParent(parent);
            }

            // the goal(desire) need to remember who what created it so it can inform them when complete
            desiredGoal->setPromise(event->strongPromise);
            desiredGoal->setPersistent(event->persistent);

            BeliefContext &context = desiredGoal->context();
            context.setAgentContext(m_context, handle());

            if (event->parameters) {
                context.setGoalContext(event->parameters);
            }

            JACK_DEBUG("{} pursue [agent={}, handle={}]", label, m_handle.toString(), desiredGoal->handle().toString());
            m_desires.push_back(desiredGoal);


            if (m_engine.haveBusAdapter()) {
                protocol::BDILogHeader header = bdiLogHeader(protocol::BDILogLevel_NORMAL);
                protocol::BDILog logEvent     = {};
                if (parentIntention) {
                    logEvent = m_engine.makeBDILogSubGoalStarted(header,
                                                                 desiredGoal->name(),                               /// goal
                                                                 desiredGoal->id().toString(),                         /// goalId
                                                                 parentIntention->goalPtr()->handle().m_id.toString(), /// intentionId
                                                                 event->parentTaskId.toString());
                } else {
                    logEvent = m_engine.makeBDILogGoalStarted(header,
                                                              desiredGoal->name(),
                                                              desiredGoal->id().toString());
                }

                m_engine.sendBusEvent(&logEvent);
            }

            /// \note New desire so we should trigger a replan
            m_scheduleDirty |= ScheduleDirty_GOAL_ADDED;
        } break;

        case Event::DROP: {
            /*! ***************************************************************************************
            * Handle DropEvent
            * 1. Find sub-goals, goals executed as a result of the main goal executing.
            * 2. Remove the goal and sub-goals
            * 3. Trigger re-plan (event?)
            * 4. Fast Track drop - directly to the executor
            * ****************************************************************************************/
            ZoneNamedN(debugTracyAgentDrop, "Agent drop event", true);
            const protocol::Drop* event = &rawEvent->dropEvent;
            ZoneNameV(debugTracyAgentDrop, event->goal.data(), event->goal.size());

            auto dropHandle = GoalHandle{event->goal, UniqueId::initFromString(event->goalId)};

            if (!dropHandle.m_id.valid()) {
                JACK_DEBUG("Drop goal received with invalid id [event={}]", *event);
                break;
            }

            // 1. Find sub-goals, goals executed as a result of the main goal
            // executing.

            // Recursively search from the initial goal to drop any sub goals
            // and sub-sub goals and etc that might currently be queued to
            // execute as an intention.
            std::stack<IntentionExecutor *> intentionExecutorStack;
            if (IntentionExecutor *executor = m_executor.findMatchingIntentionByGoalId(dropHandle.m_id)) {
                intentionExecutorStack.push(executor);
            }

            /// \todo If a goal was created but dropped imemdiately before an
            /// intention is formed, then the drop() call above will not fail
            /// the goal for us. We need to do it ourselves. This is a special
            /// case rule that should be avoided at all costs. We can improve
            /// this, see my comment below regarding eraseDesire().
            ///
            /// My initial attempt at trying to paper over this nicely was to
            /// include this logic in eraseDesire. However, we hit a few
            /// roadbumps, we get a double goal finish in the IntentionExecutor
            /// (because we clone the goal instance for running- which will
            /// finish and log completion, and then erase the desire, which was
            /// *not* finished, and it'd log failure).
            ///
            /// Since they are cloned, then the promises point to the same
            /// thing .. and trigger twice. Lame.
            ///
            /// Instead, when this macro is set to 1 we will only log the BDI
            /// event instead of actually finishing the goal. The goal will get
            /// deleted directly without callign the promise.
            #define GOAL_FINISH_WORKAROUND 1

            while (intentionExecutorStack.size()) {
                IntentionExecutor *intentionExecutor = intentionExecutorStack.top();
                intentionExecutorStack.pop();

                for (const UniqueId &desireId : intentionExecutor->subGoalDesireIds()) {
                    // Queue any sub goals we might currently be executing
                    // (intentions) for recursive dropping
                    IntentionExecutor *child = m_executor.findMatchingIntentionByGoalId(desireId);
                    if (!child) {
                        continue;
                    }
                    intentionExecutorStack.push(child);

                    // Drop the sub goal;
                    /// \todo Use hashed names instead of string compares everywhere
                    Goal* subGoal = getDesire(child->desireHandle().m_id);
                    if (!subGoal) {
                        continue;
                    }

                    /// \note Drop the goal via the executor, if we don't
                    /// have an intention to drop for this goal, the
                    /// function returns fals and we will manually remove
                    /// the goal from the agent.
                    JACK_DEBUG("Dropping sub goal [agent={}, subGoal={}]", handle().toString(), subGoal->handle().toString());
                    if (!m_executor.internalDrop(subGoal->handle(), event->mode, rawEvent->reason)) {
                        if (m_engine.haveBusAdapter()) {
                            JACK_ASSERT_MSG(subGoal->parent().handle.valid(), "Sub-goal drop, hence, we must always have a parent");
                            protocol::BDILogHeader header = bdiLogHeader(protocol::BDILogLevel_NORMAL);
                            protocol::BDILog logEvent = m_engine.makeBDILogSubGoalFinished(header,
                                                                                           subGoal->handle().m_name,              /// subGoal
                                                                                           subGoal->handle().m_id.toString(),        /// subGoalId
                                                                                           subGoal->parent().handle.m_id.toString(), /// intentionId
                                                                                           subGoal->parent().planTaskId.toString(),  /// taskId
                                                                                           rawEvent->reason,                      /// dropReason
                                                                                           protocol::BDILogGoalIntentionResult_DROPPED);
                            m_engine.sendBusEvent(&logEvent);
                        }
                        subGoal->finish(FinishState::DROPPED);
                        eraseDesire(dropHandle);
                    }
                }
            }

            // 2. Remove the desire (goal)
            if (Goal *goal = getDesire(dropHandle.m_id)) {
                /// \note Drop the goal via the executor, if we don't have an
                /// intention to drop for this goal, the function returns fals
                /// and we will manually remove the goal from the agent.
                JACK_DEBUG("Dropping goal [agent={}, goal={}]", handle().toString(), dropHandle.toString());
                if (!m_executor.internalDrop(dropHandle, event->mode, rawEvent->reason)) {
                    if (m_engine.haveBusAdapter()) {
                        protocol::BDILogHeader header = bdiLogHeader(protocol::BDILogLevel_NORMAL);
                        protocol::BDILog logEvent     = {};
                        if (goal->parent().handle.valid()) {
                            logEvent = m_engine.makeBDILogSubGoalFinished(header,
                                                                          goal->handle().m_name,              /// goal
                                                                          goal->handle().m_id.toString(),        /// goalId
                                                                          goal->parent().handle.m_id.toString(), /// intentionId
                                                                          goal->parent().planTaskId.toString(),  /// taskId
                                                                          rawEvent->reason,                   /// dropReason
                                                                          protocol::BDILogGoalIntentionResult_DROPPED);
                        } else {
                            logEvent = m_engine.makeBDILogGoalFinished(header,
                                                                       goal->handle().m_name,
                                                                       goal->handle().m_id.toString(),
                                                                       rawEvent->reason,                         /// dropReason
                                                                       protocol::BDILogGoalIntentionResult_DROPPED);
                        }
                        m_engine.sendBusEvent(&logEvent);
                    }
                    goal->finish(FinishState::DROPPED);
                    eraseDesire(dropHandle);
                }
            }

            /// 3. Trigger re-plan (event?)
            /// \todo
            break;
        }

        case Event::SCHEDULE: {
            JACK_WARNING_MSG("Re-schedule event not handled by the agent");
        } break;

        case Event::AUCTION: {
            /// \todo Cleanup the schedule juggling code we have in the
            /// update loop- we now check against the auction 'generation'
            /// meaning- we don't need to wait around for a schedule to finish
            /// anymore- we can immediately discard.
            ZoneNamedN(debugTracyAgentAuction, "Agent drop event", true);
            auto* event = static_cast<AuctionEvent*>(rawEvent);
            ZoneNameV(debugTracyAgentAuction, event->m_goal.m_name.c_str(), event->m_goal.m_name.size());
            JACK_DEBUG("Processing auction event [agent={}, goal={}]", handle().toString(), event->m_goal);

            /// Apply the auction to the schedule if there is one
            if (m_schedule && m_schedule->id() == event->m_scheduleId) {
                /// \todo A schedule is not allowed to be used if the goals
                /// changed or we requested a force reschedule. If the goals
                /// changed, then the schedule may be holding onto invalid goal
                /// pointers.
                /// If we force a reschedule, we must respect that request.
                ///
                /// Schdule's should not hold pointers to goals in the agent,
                /// this is doubly important when we try to run schedule
                /// concurrently. We can pass in the goal query functions
                /// required and the runtime context for that goal we don't need
                /// the original goal from the agent, or, clone the goal in so
                /// it has its own unique copy.
                uint16_t blockAuctionFlags = ScheduleDirty_GOAL_REMOVED | ScheduleDirty_MEMBER_REMOVED | ScheduleDirty_FORCE;
                bool auctionBlocked        = m_scheduleDirty & blockAuctionFlags;
                if (!auctionBlocked) {
                    m_schedule->processAuction(event);
                }
            } else {
                for (DelegationEventBackLogEntry &entry : m_delegationEventBackLog) {
                    if (!entry.schedule) {
                        continue;
                    }

                    if (entry.schedule->id() == event->m_scheduleId) {
                        entry.schedule->processAuction(event);
                        break;
                    }
                }
            }

            event->success();
        } break;

        case Event::DELEGATION: {
            JACK_ASSERT(rawEvent->recipient->UUID() == UUID()); // it's for this agent

            ZoneNamedN(debugTracyAgentDelegation, "Agent delegation event", true);
            auto*             event      = static_cast<DelegationEvent*>(rawEvent);
            const GoalHandle& goalHandle = event->goalHandle;
            ZoneNameV(debugTracyAgentDelegation, goalHandle.m_name.c_str(), goalHandle.m_name.size());

            if (event->message) {
                JACK_ASSERT(event->message->schema().size());
            }
            JACK_ASSERT(event->team.valid());

            /// \note Delegations are not returned to the allocator until they
            /// get returned back to the team that initiated the delegations.
            returnEventToAllocator            = false;

            // Store the delegation and generate a pursue goal event
            JACK_DEBUG("Delegated goal [agent={}, goal={}, msg={}, analyse={}]",
                       handle().toString(),
                       goalHandle.toString(),
                       event->message ? event->message->toString() : "",
                       static_cast<int>(event->analyse));

            if (event->status != Event::PENDING) {
                // \note The delegation is done if the status is *not* 'PENDING'.
                JACK_ASSERT(isTeam() && "Returned events should be routed back to the team that sent the delegation");
                returnEventToAllocator = true;

                if (event->status == Event::Status::SUCCESS) {
                    event->success();
                } else {
                    event->fail();
                }

                if (event->analyse) {

                    /// The agent we delegated to finished simulating the goal
                    /// and returned the score to us. Collate this event's
                    /// result into our auction state for later evaluation.
                    for (CurrentAuction& auction : m_currentAuctions) {
                        if (auction.goal != event->goalHandle || auction.scheduleId != event->delegatorScheduleID) {
                            continue;
                        }

                        /// Check if the bid already exists
                        ///
                        /// \todo This is potentially malicious remote input, we
                        /// should have more verification steps
                        ///
                        /// \todo Check that the bidder is supposed to be a part
                        /// of the auction, i.e. was part of the original
                        /// delegates when the auction started.
                        {
                            bool exists = false;
                            for (const AuctionEventBid &bid : auction.bids) {
                                if (bid.bidder == event->caller->handle()) {
                                    exists = true;
                                    break;
                                }
                            }

                            if (exists) {
                                JACK_DEBUG("Duplicate auction bid was received from the same bidder [bidder={}, goal={}, team={}]",
                                           event->caller->handle().toString(),
                                           event->goalHandle.toString(),
                                           handle().toString());
                                break;
                            }
                        }

                        auto clockMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_engine.internalClock());
                        if (!auction.finished(clockMs)) {
                            AuctionEventBid bid = {};
                            bid.bidder          = event->caller->handle();
                            bid.score           = event->score;
                            auction.bids.push_back(bid);
                        }
                    }
                } else {
                    // Inform the team that the delegation hsa finished
                    m_executor.handleDelegationEvent(event);
                }
                break;
            }

            // Delegation was sent to team member for auctioning. The team
            // member will simulate the cost of running the goal and return it
            // back to the team for deciding on the best member.
            if (event->analyse) {

                /// \todo Re-evaluate this todo below. We re-wrote a good chunk
                /// of this function.
                /// \todo this should use role information to figure out which
                /// to filter - but for now we only allow a team member to
                /// perform a single team goal
                DelegationEventBackLogEntry entry = {};
                entry.delegationEvent             = event;

                for (const Goal *desire : m_desires) {
                    if (desire->id() == goalHandle.m_id) {
                        entry.delegationGoalAlreadyBeingExecuted = true;
                        break;
                    }
                }

                if (entry.delegationGoalAlreadyBeingExecuted) {
                    event->score = 0;
                } else {
                    /// Set up the back log entry for goal simulation.
                    /// \todo It would be better if the delegation event already
                    /// contained the goal template Clone the goal for
                    /// simulation
                    const Goal* realGoal      = m_engine.getGoal(goalHandle.m_name);
                    Goal*       simulatedGoal = JACK_CLONE_BDI_OBJECT(realGoal);
                    {
                        /// \todo This event is untrusted! We can not trust the
                        /// input we need almost verbatim the same exact
                        /// validation steps we do in the pursue goal.

                        simulatedGoal->setPersistent(false);
                        simulatedGoal->setId(goalHandle.m_id);
                        simulatedGoal->setDelegated(getGoalPlans(realGoal).empty());

                        BeliefContext &context = simulatedGoal->context();
                        context.setAgentContext(m_context, handle());
                        context.setGoalContext(event->message);
                    }

                    /// \todo This schedule can potentially out-live the desires
                    /// of the agent. This is ok and is a implementation
                    /// compromise we're taking to get things over the line.
                    /// Since simulated schedules are completely separate from
                    /// the main schedule- these schedules have the "ability" to
                    /// take an unbounded amount of time depending on how
                    /// complex and deep the team of teams hierarchy is.
                    ///
                    /// By the time a simulated schedule may be finished, the
                    /// original root-desire may not exist- so this schedule
                    /// could reference and use an invalid pointer.
                    ///
                    /// For now, we clone all the desires and wrap them in
                    /// a unique_ptr to handle their lifetime separate from the
                    /// agent's own desires. If by the time the agent finishes
                    /// this simulated schedule it's out of date, that's fine,
                    /// that's up to the team to handle.
                    ///
                    /// I've started work on addressing this by introducing
                    /// a GoalHandle and isolating the scope of the codebase
                    /// that holds onto Goal pointers when they don't need to.
                    ///
                    /// My suggested approach is to use handles everywhere and
                    /// consult the engine when we need the concrete goal. The
                    /// mutable bits of the goal like the context that we want
                    /// to carry forward should be stored separately.
                    ///
                    /// So far it turns out that there's a lot of places that
                    /// don't actually need a Goal pointer but just the handle
                    /// which is a good sign for person who finishes this task.

                    for (Goal* desire : m_desires) {
                        Goal* desireClone = JACK_CLONE_BDI_OBJECT(desire);
                        entry.desires_HACK.push_back(JACK_INIT_UNIQUE_PTR(Goal, desireClone));

                        BeliefContext &context = desireClone->context();
                        context.setAgentContext(m_context, handle());

                        std::shared_ptr<Message> goalContext = desire->context().goal();
                        if (goalContext) {
                            context.setGoalContext(goalContext);
                        }
                    }

                    entry.desires_HACK.push_back(JACK_INIT_UNIQUE_PTR(Goal, simulatedGoal));
                    std::vector<Goal *> desires;
                    for (auto &desire : entry.desires_HACK) {
                        desires.push_back(desire.get());
                    }

                    /// \todo This API shouldn't take a vector .. pointer and length.
                    entry.schedule = JACK_INIT_UNIQUE_PTR(Schedule, JACK_NEW(Schedule, this, desires));
                    entry.schedule->m_delegator           = event->caller->handle();
                    entry.schedule->m_delegatorScheduleID = event->delegatorScheduleID;
                }

                /// Defer processing of delegation event until the agent update
                /// step.
                m_delegationEventBackLog.push_back(std::move(entry));
                break;
            }

            /// pursue a goal for this delegated goal
        
            GoalPursue goalPursue = pursue(goalHandle.m_name,
                                           GoalPersistent_No,
                                           event->message,
                                           goalHandle.m_id);
            goalPursue.promise->then(
                // success
                [event, this]() {
                    // send the delegation back
                    event->status = Event::SUCCESS;

                    JACK_DEBUG("Delegated goal complete [agent={}, goal={}]", handle().toString(), event->goalHandle.toString());
                    /// \note See the comment on the field in delegationevent.h
                    if (event->team != event->recipient->handle()) {
                        m_engine.returnEvent(event);
                    } else {
                        m_engine.routeEvent(event);
                    }
                    JACK_DEBUG("Delegated goal complete [goal={}]", event->goalHandle.toString());
                },
                // fail
                [event, this]() {

                    /// Send the delegation back
                    event->status = Event::FAIL;

                    /// \note See the comment on the field in delegationevent.h
                    if (event->team != event->recipient->handle()) {
                        m_engine.returnEvent(event);
                    } else {
                        m_engine.routeEvent(event);
                    }
                    JACK_DEBUG("Delegated goal failed [goal={}]", event->goalHandle.toString());
                });
        } break;

        case Event::SHAREBELIEFSET: {
            ZoneNamedN(debugTracyAgentShareBeliefset, "Agent share beliefset event", true);
            JACK_ASSERT(rawEvent->recipient->UUID() == UUID());

            const auto *event = static_cast<ShareBeliefSetEvent *>(rawEvent);
            if (!event->caller) {
                JACK_ASSERT(event->caller && "Invalid Code Path: Caller should be set");
                break;
            }

            if (dynamic_cast<Team *>(this)) {
                // Agent is writing to the team
                JACK_ASSERT(dynamic_cast<Agent *>(event->caller));
                JACK_ASSERT(dynamic_cast<Team *> (event->recipient));
            } else {
                // Team is writing to agent
                JACK_ASSERT(dynamic_cast<Team *>(event->caller));
                JACK_ASSERT(dynamic_cast<Agent *>(event->recipient));
            }

            std::vector<SharedBeliefSet> &list = m_sharedBeliefSets[event->beliefSet->schema()];
            auto it = std::find(list.begin(), list.end(), event->beliefSetOwnerId);
            if (it == list.end()) {
                list.push_back({});
                it               = list.end() - 1;
                it->m_memberId   = std::move(event->beliefSetOwnerId);
                it->m_memberName = std::move(event->beliefSetOwnerName);
            }

            it->m_beliefSet   = std::move(event->beliefSet);
            it->m_lastUpdated = std::chrono::duration_cast<std::chrono::microseconds>(m_engine.internalClock());

            /// \note Dirty the schedule to force a replan. This agent may be
            /// dependent on the the beliefset we just received, i.e. using it
            /// in a goal they're executing- and since we've just received it-
            /// this may influence the outcome of the goal, so we need to replan
            /// taking this into account.
            m_scheduleDirty |= ScheduleDirty_PERCEPT;
        } break;

        case Event::REGISTER: {
            ZoneNamedN(debugTracyAgentRegister, "Agent register event", true);
            JACK_DEBUG("Agent requested a registration of a new BDI entity [agent={}, register={}]", handle().toString(), rawEvent->registerEvent);
            assert(!"It is currently unknown if agents should be able to dispatch this message, it's possibly a bug if this happens");

            returnEventToAllocator = false;
            rawEvent->recipient = nullptr;
            m_engine.routeEvent(rawEvent);
        } break;
    }
}

// handle a goal delegation
/// \todo could be delegate event?
void Agent::delegateGoal(const GoalHandle &goalHandle, Agent* /*delegate*/, const std::shared_ptr<Message>& /*parameters*/)
{
    // the agent can't delegate a goal by default
    JACK_ERROR("{} is trying to delegate a goal '{}'", m_handle.toHumanString(), goalHandle.m_name);
}

bool Agent::analyseDelegation(Goal *goal, size_t)
{
    // this is agent id not a team
    JACK_ERROR("{} is trying to analyse a delegation goal '{}'", m_handle.toHumanString(), goal->name());
    return false; // no auction
}

void Agent::dropDelegation(const GoalHandle &goalHandle, Agent * /*delegate*/)
{
    JACK_ERROR("{} is trying to drop a delegation goal {}", m_handle.toHumanString(), goalHandle.m_name);
}

Agent::GetDelegatesResult Agent::getDelegates([[maybe_unused]] const GoalHandle& goal)
{
    static std::vector<Agent*> delegates;
    static GetDelegatesResult result{delegates, "The agent is not a team and hence cannot delegate goals"};
    return result;
}

std::vector<Goal*> Agent::activateGoals(BeliefContext * /*context*/)
{
    /**************************************************************************
     * Process A.2.2
     * Filter the current desires of this agent using the pre-conditions to
     * determine the applicable set of active goals to form intentions
     **************************************************************************/
    std::vector<Goal*> result;
    for (Goal* desire : m_desires) {
        const GoalHandle& desireHandle = desire->handle();

        #define GOAL_CANNOT_BE_ACHIEVED_BUILD_MSG(builder)                                  \
            builder.appendCopy(handle().toHumanString("Agent"));                               \
            builder.appendRef(" is planning a schedule for its desires however the goal "); \
            builder.appendCopy(desireHandle.toHumanString());                                  \
            builder.appendRef(" can not be achieved and will be dropped");

        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        auto builder                   = StringBuilder(scratch.arena);
        if (desire->shouldDrop() || (!(desire->isPersistent() || desire->delegated()) && !desire->isValid())) {
            GOAL_CANNOT_BE_ACHIEVED_BUILD_MSG(builder);
            if (desire->shouldDrop()) {
                builder.appendRef(" because the drop precondition is active");
            } else {
                builder.appendRef(" because the goal precondition is no longer valid");
            }

            std::string_view dropReason = builder.toStringArena(scratch.arena);
            JACK_DEBUG("{}", dropReason);
            dropWithMode(desireHandle, protocol::DropMode_NORMAL, dropReason);
            continue;
        }

        const std::vector<const Plan*>& plans = getGoalPlans(desire);
        if (plans.empty()) {
            /// \todo We cannot reject a goal on the basis of not having a team
            /// member with a role for it, because, it may eventually become
            /// valid if it's persistent (e.g. unavailable team member becomes
            /// available, or, dynamic team formations).
            GetDelegatesResult getDelegatesResult = getDelegates(desireHandle);
            if (getDelegatesResult.delegates.empty()) {
                GOAL_CANNOT_BE_ACHIEVED_BUILD_MSG(builder);
                builder.append(FMT_STRING(". This agent has no plans to achieve the goal. {}"), JACK_STRING_FMT(getDelegatesResult.delegationOutcome));

                std::string_view dropReason = builder.toStringArena(scratch.arena);
                JACK_DEBUG("{}", dropReason);
                dropWithMode(desireHandle, protocol::DropMode_NORMAL, dropReason);
                continue;
            }
        }
        #undef GOAL_CANNOT_BE_ACHIEVED_OSTREAM_APPEND

        /// \note Check if intention is being force-dropped, the intent is to
        /// delete the intention and desire so we do not activate the goal
        /// for replanning.
        bool intentionIsBeingForceDropped = false;
        for (const IntentionExecutor* executor : m_executor.intentions()) {
            if (executor->desireHandle() != desire->handle()) {
                continue;
            }

            if (executor->state() == IntentionExecutor::ExecutorState::FORCE_DROPPING) {
                intentionIsBeingForceDropped = true;
            }
        }

        if (intentionIsBeingForceDropped) {
            continue;
        }

        result.push_back(desire);
    }
    return result;
}

Schedule *Agent::generateSchedule(const std::vector<Goal*> &activeGoals)
{
    /*! ***************************************************************************************
     * Process A.2.3
     * Generate a new schedule of intention to be executed by this agent
     * Sub-Processes:
     * 1. Start Schedule (A.2.3.1)
     * 2. Expand Schedule (A.2.3.2)
     * 3. Cost Schedule (A.2.3.3)
     * 4. Optimised Schedule(A.2.3.4)
     * 5. Bind Schedule(A.2.3.5)
     *
     * Inputs:
     * activeGoals
     *
     * Outputs:
     * The new Schedule
     *
     * Resources:
     *
     * ****************************************************************************************/

    /*! ***************************************************************************************
     * 1. A.2.3.1 Start Schedule
     * ****************************************************************************************/

    /// A.2.3.1.1 Create Empty Schedule
    /// A.2.3.1.2 Add Root Goals
    ///

    Schedule *schedule = JACK_ALLOCATOR_NEW(&m_heapAllocator, Schedule, this, activeGoals);

    // * 2. Expand Schedule (A.2.3.2)
    // * 3. Cost Schedule (A.2.3.3)
    // * 4. Optimised Schedule(A.2.3.4)
    // * 5. Bind Schedule(A.2.3.5)
    if (!activeGoals.empty()) {
        processSchedule(schedule);
    }

    return schedule;
}

Schedule * Agent::plan()
{
    /*! ***************************************************************************************
     * Process A.2 - Plan
     * Responsible for formulating a new Schedule to be considered for the generation of
     * Intentions using the current context and desires
     *
     * Sub-Processes:
     * 1. Setup Context (A.2.1)
     * 2. Activate Goals (A.2.2)
     * 3. Generate Schedule (A.2.3)
     *
     * Inputs: Beliefs, Desires, Current Intentions
     *
     * Outputs: New Schedule
     *
     * Tools: Resources, Plan Library, Heuristic
     *
     * ****************************************************************************************/

    /// 1. Setup Context (A.2.1)
    BeliefContext *context = nullptr;

    /// 2. Activate Goals (A.2.2)
    std::vector<Goal*> activeGoals = activateGoals(context);

    /// 3. Generate Schedule (A.2.3)
    Schedule *schedule = generateSchedule(activeGoals);

    /// 4. Dispatch New Schedule Event (A.2.4?)
    // addEvent(new ScheduleEvent(schedule));

    // return the new schedule
    return schedule;
}

static void debugCollateSearchNodeStats(Agent::DebugState &state, const Schedule *schedule)
{
    if (!schedule) {
        return;
    }

    // schedule->printSchedule();

    state.m_searchNodeOpenHighWaterMark    = std::max(state.m_searchNodeOpenHighWaterMark, schedule->openNodes().size());
    state.m_searchNodeClosedHighWaterMark  = std::max(state.m_searchNodeClosedHighWaterMark, schedule->closedNodes().size());
    state.m_searchNodePendingHighWaterMark = std::max(state.m_searchNodePendingHighWaterMark, schedule->pendingNodes().size());
}

void Agent::run()
{
    ZoneScoped;
    /*! ***************************************************************************************
    * Execute the BDI process for an agent by one iteration
    * 1. Process any pending events/auctions/actions/shared beliefs
    * 2. Generate a new schedule if we're ready for a new one.
    * 3. Process the schedule into an A* tree, when ready, hand it to the
    *    executor
    * 4. Process the timer queue
    * 5. Tick the executor (execute the schedule prior assigned/generated)
    * ****************************************************************************************/
    if (!running()) {
        /// \note Check if more events have been queued to this agent whilst we
        /// are halted.
        if (m_debugState.m_eventBackLog && !m_debugState.m_eventBackLogWarning) {
            m_debugState.m_eventBackLogWarning = true;
            JACK_WARNING("Stopped agent received events [agent={}, events={}]", m_handle.toString(), m_debugState.m_eventBackLog);
        }
        return;
    }

    if (m_state != Agent::STOPPING) {
        /// Step 1. Process any pending events/auctions/actions/shared beliefs
        processCurrentDelegationEvents();
        processCurrentAuctions();
        processCurrentActions(); /// Complete actions are finalized by generating a ActionCompleteEvent
        processSharedBeliefs();

        /// Step 2. Generate a new schedule if we're ready for a new one.
        if (m_scheduleDirty || (!m_schedule && m_executor.done())) {
            bool planNewSchedule = true;

            if (m_schedule && !m_schedule->isFinished()) {
                planNewSchedule = false;

                /// \note Whilst the schedule is dirty, planning will only
                /// proceed in certain circumstances if the schedule is not
                /// complete!
                ///
                /// This stops the agent getting into a schedule dirty
                /// replanning loop, e.g. when percepts are sent incessantly
                /// causing the agent to re-enter planning and live-lock.
                ///
                /// - Members changed; schedule holds invalid references to agents to auction to
                ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
                StringBuilder          builder = StringBuilder(scratch.arena);
                if (m_scheduleDirty & ScheduleDirty_GOAL_REMOVED) {
                    builder.appendRef("goal removed;");
                }
                if (m_scheduleDirty & ScheduleDirty_MEMBER_REMOVED) {
                    builder.appendRef("member removed;");
                }
                if (m_scheduleDirty & ScheduleDirty_FORCE) {
                    builder.appendRef("force;");
                }

                std::string_view reason = builder.toStringArena(scratch.arena);
                if (reason.size()) {
                    planNewSchedule = true;
                    JACK_DEBUG("Allowing replan of dirty schedule [agent={}, reason={}]", handle(), reason);
                }
            }

            /// \todo We should double buffer the schedule, we only ever
            /// juggle at most 2.
            if (planNewSchedule) {
                debugCollateSearchNodeStats(m_debugState, m_schedule);
                JACK_ALLOCATOR_DELETE(&m_heapAllocator, m_schedule);
                m_schedule      = plan();
                m_scheduleDirty = ScheduleDirty_NONE;
            }
        }

        /// Step 3. Process the schedule into an A* tree, when ready, hand it to
        ///         the executor
        if (processSchedule(m_schedule)) {
            debugCollateSearchNodeStats(m_debugState, m_schedule);
            m_executor.setSchedule(m_schedule);
            m_schedule = nullptr;
        }

        /// Step 4.
        /// Pull off each timer that has expired and inform the owner
        std::chrono::milliseconds internalClock = std::chrono::duration_cast<std::chrono::milliseconds>(m_engine.internalClock());
        while (hasTimerEvents() && getNextEventTimePoint() <= internalClock) {
            /// Grab the top timer event and inform the owner of compleation
            TimerEvent *timerEvent = m_timerEvents.top();
            timerEvent->success();

            if (m_engine.haveBusAdapter()) {
                protocol::BDILog logEvent =
                    m_engine.makeBDILogSleepFinished(bdiLogHeader(protocol::BDILogLevel_NORMAL),
                                                     timerEvent->goal.m_name,
                                                     timerEvent->goal.m_id.toString(),
                                                     timerEvent->intentionId.toString(),
                                                     timerEvent->plan,
                                                     timerEvent->taskId.toString());
                m_engine.sendBusEvent(&logEvent);
            }

            /// TODO:
            /// things to consider:
            /// what if the agent is finished? do we flush all timer events?
            /// what if the agent is paused and then resumed? do we play catch up and trigger all timers
            /// support a simulation time. (pausing simulation, running faster/slower, debugging)
            m_state = Agent::RUNNING;

            /// clean up the timer events
            m_timerEvents.pop();
            JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, TimerEvent, timerEvent, JACK_ALLOCATOR_CLEAR_MEMORY);
        }
    } /// m_state != Agent::STOPPING

    /// Step 6. Tick the executor (execute the schedule prior assigned/generated)
    m_executor.execute();

    JACK_ASSERT(running() &&
                "State changes can not happen directly in the execution of a executor, they should "
                "get queued onto the event queue and enacted when the agent processes events.");

    if (m_state == Agent::STOPPING && m_executor.runningState() == AgentExecutor::RunningState::IDLE) {
        setState(STOPPED);
    }
}

bool Agent::attachService(const ServiceHandle& serviceHandle, bool force)
{
    Service* service = m_engine.getService(serviceHandle);
    if (!service) {
        JACK_WARNING("Service to attach to agent no longer exists [agent={}, service={}]", m_handle.toString(), serviceHandle.toString());
        return false;
    }

    /// \note In this loop we will ensure that we detach any service that we are
    /// attached to that conflicts with the one we want to attach as agents can
    /// only use one service of a particular "template type" at a time.
    ///
    /// This function is extra careful and makes no assumptions about the
    /// validity of current the services this agent is connected to, hence, this
    /// loop is exhaustive and checks every single service, even for duplicates
    /// (which is not normally allowed) and ensures to deattach them if they
    /// conflict.
    bool result = true;
    for (auto it = m_attachedServices.begin(); it != m_attachedServices.end(); ) {
        const ServiceHandle& attachedServiceHandle = *it;
        Service*             attachedService       = m_engine.getService(attachedServiceHandle);
        bool                 erased                = false;
        if (attachedService) {
            /// \note Check if the we have a service that is already attached that
            /// is the same type as the one we want to attach.
            if (attachedService->m_templateName == service->m_templateName) {
                if (attachedService->handle() == serviceHandle) {
                    JACK_WARNING("Service to attach to agent is already attached to the agent [agent={}, service={}]", m_handle.toString(), service->handle().toString());
                    result = false;
                } else {
                    /// \note Pre-existing service exists with the same type, but
                    /// it's a different instance (since handles are not equal).
                    if (force) {
                        it     = m_attachedServices.erase(it);
                        erased = true;
                    } else {
                        JACK_WARNING("Service to attach to agent can not be attached. Agent already has a "
                                     "different instance of the service attached [agent={}, service={}, serviceToAttach={}]",
                                     handle(),
                                     attachedServiceHandle,
                                     serviceHandle);
                        result = false;
                    }
                }
            }

            if (attachedService->name() == service->name() && attachedService->handle().m_id != service->handle().m_id) {
                JACK_WARNING("Duplicate service name detected that are different (template) types "
                             "[agent={}, service={}, serviceTemplate={}, serviceToAttach={}, serviceToAttachTemplate={}]",
                             m_handle.toString(),
                             attachedServiceHandle,
                             attachedService->m_templateName,
                             serviceHandle,
                             service->m_templateName);
            }

        } else {
            JACK_WARNING("Service attached to agent no longer exists [agent={}, service={}]",
                         m_handle.toString(),
                         attachedService->handle().toString());
        }

        if (!erased) {
            it++;
        }
    }

    if (result) {
        m_attachedServices.push_back(serviceHandle);
    }
    return result;
}

bool Agent::detachService(const ServiceHandle& handle)
{
    bool result = false;
    for (auto it = m_attachedServices.begin(); it != m_attachedServices.end(); it++) {
        if (*it == handle) {
            result = true;
            m_attachedServices.erase(it);
            break;
        }
    }
    return result;
}

GoalPursue Agent::pursue(const GoalBuilder& goal,
                         GoalPersistent     persistent,
                         std::shared_ptr<Message>     parameters,
                         const UniqueId&    id)
{
    return pursue(goal.name(), persistent, parameters, id);
}

GoalPursue Agent::pursue(std::string_view goal,
                         GoalPersistent   persistent,
                         std::shared_ptr<Message> parameters,
                         const UniqueId&  id)
{
    GoalPursue result = pursueSub(goal,
                                  IntentionExecutor::NULL_ID /*parent intention*/,
                                  persistent,
                                  parameters,
                                  &id,
                                  nullptr /*parentTaskId*/);
    return result;
}

/// shared_ptr version
GoalPursue Agent::pursueSub(std::string_view         goal,
                            IntentionExecutor::Id    parent,
                            GoalPersistent           persistent,
                            std::shared_ptr<Message> parameters,
                            const UniqueId*          id,
                            const UniqueId*          parentTaskId)
{
    /*! ***************************************************************************************
     * Queue the sub goal specified for execution and inform the given task when finished
     * ****************************************************************************************/
    /// \note We take ownership of the BeliefSet
    PursueEvent* pursueEvent = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                                  PursueEvent,
                                                  std::string(goal),
                                                  parent,
                                                  parameters,
                                                  persistent);
    if (id) {
        assert(id->valid());
        pursueEvent->eventId = *id;
    }

    if (parentTaskId) {
        pursueEvent->parentTaskId = *parentTaskId;
    } else {
        /// \note Invalidate it
        pursueEvent->parentTaskId = aos::jack::UniqueId(0, 0);
    }

    /// @todo Code must not access 'pursueEvent' after 'addEvent' is called
    /// in a multithreaded context as the engine will race to consume events
    /// So we lock the API whilst we modify the event after it has been
    /// dispatched.
    ///
    /// I attempted to fix this here
    ///
    /// https://gitlab.aosgrp.net/jack/core/jack-core/-/commit/e479d2ad9eeb285f692eef672efbb5db5cde56e3
    ///
    /// However, we have plan tasks that generate an event, uses the promise
    /// from the API and holds a pointer to the plan task itself. Then JACK
    /// may delete the plan leading to a dangling pointer. This coincidentally
    /// worked because all the promises by default are weak.
    ///
    /// This works "accidentally" but causes a spagetti of relationships in the
    /// codebase. Promises are great when you are attempting to JACK to
    /// interface with external codebases where JACK can't make assumptions
    /// about the outside domain and the code running.
    ///
    /// However, promises internally in the codebase where JACK itself is in
    /// control of the code it executes, it's not very beneficial to adopt a
    /// future/promise model which favours calling into black boxes by allowing
    /// the programmer to spuriously sprinkle callbacks and inadvertently create
    /// complex lifetime graphs.
    auto       lock            = std::unique_lock<std::mutex>(m_guardAPI);
    GoalPursue result          = {};
    result.promise             = addEvent(pursueEvent);
    result.handle              = GoalHandle{std::string(goal), pursueEvent->eventId};
    pursueEvent->strongPromise = result.promise;
    notifyScheduler();
    return result;
}

/*
GoalPursue Agent::pursueSub(std::string_view      goal,
                            IntentionExecutor::Id parent,
                            GoalPersistent        persistent,
                            std::shared_ptr<Message> parameters,
                            const UniqueId*       id,
                            const UniqueId*       parentTaskId)
{
    std::shared_ptr<Message> msg(parameters.clone());
    return pursueSub(goal, parent, persistent, parameters, msg, id, parentTaskId);
}

*/

void Agent::dropWithMode(const GoalHandle& handle, protocol::DropMode mode, std::string_view reason)
{
    if (!handle.valid()) {
        return;
    }

    auto* event                 = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, Event, Event::DROP);
    event->dropEvent.goal       = handle.m_name;
    event->dropEvent.goalId     = handle.m_id.toString();
    event->dropEvent.senderNode = engine().busAddress();
    event->dropEvent.sender     = busAddress();
    event->dropEvent.recipient  = busAddress();
    event->dropEvent.mode       = mode;

    if (reason.size()) {
        event->setReason(std::string(reason));
    } else {
        event->setReason(JACK_FMT("{} dropping {}", this->handle().toHumanString("Agent"), handle.toHumanString("goal")));
    }

    routeEvent(event);
    notifyScheduler();
}

void Agent::drop(const GoalHandle& handle, std::string_view reason)
{
    /*! ***************************************************************************************
     * Raise a drop goal event
     * ****************************************************************************************/
    if (!handle.valid()) {
        return;
    }
    dropWithMode(handle, protocol::DropMode_FORCE, reason);
}

bool Agent::eraseDesire(const GoalHandle &handle)
{
    /*! ***************************************************************************************
     * Find the matching desire and drop it
     * ****************************************************************************************/
    for (auto it = m_desires.begin(); it != m_desires.end(); ++it) {
        Goal *entry = *it;
        if (entry->handle() == handle) {
            JACK_DELETE(entry);
            m_desires.erase(it);
            m_scheduleDirty |= ScheduleDirty_GOAL_REMOVED;
            return true;
        }
    }

    return false;
}

PromisePtr Agent::addEvent(Event* event)
{
    /*! ***************************************************************************************
     * Post an event to the agent
     * ****************************************************************************************/
    PromisePtr p = Dispatch::addEvent(event);
    notifyScheduler();

    return p;
}

void Agent::addResourcePerceptEvent(const std::string &resourceName, int value)
{
    /// \todo This is done to avoid a MSVC compiler error, we currently have
    /// a cyclic include problem with perceptevent.h/beliefset.h. The desired
    /// implementation is to have Resources take the agent's dispatch event
    /// queue and in resource.cpp dispatch->addEvent(new PerceptEvent(...)).
    ///
    /// But adding "#include <events/perceptevent.h>" into resource.cpp causes
    /// MSVC to get confused and fail to compile with
    ///
    /// jack-core\src\beliefset.h|153| error C2760: syntax error: unexpected token 'identifier', expected 'type specifier'
    /// jack-core\src\beliefset.h|153| note: This diagnostic occurred in the compiler generated function 'void aos::jack::BeliefSet::set(const std::string &,T)'
    ///
    /// So we add a helper function here to work-around that for now that
    /// resource.cpp can use.
    Field field   = FieldRegistry::getInstance().queryType<int>()->create();
    field.m_value = value;

    auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, PerceptEvent, resourceName, /*isMessage*/ false, field, false /*broadcastToBus*/);
    Dispatch::routeEvent(event);
    notifyScheduler();
}

void Agent::notifyScheduler()
{
    /*! ***************************************************************************************
     * Let this agent's scheduler know that we have work to do
     * Wake it's thread up if it's hybernating
     * ****************************************************************************************/
    m_engine.notify();
}

std::chrono::milliseconds Agent::getNextEventTimePoint() const
{
    TimerEvent *              event         = m_timerEvents.top();
    std::chrono::milliseconds nextTimePoint = event->internalClock + event->duration;
    return nextTimePoint;
}

std::shared_ptr<Message> Agent::message(const std::string& name) const
{
    return m_context.message(name);
}

void Agent::sendMessageToHandler(std::unique_ptr<Message> msg)
{
    routeEvent(JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, MessageEvent, std::move(msg), true /*deprecatedMsg*/, false /*broadcastToBus*/));
}

bool Agent::addTeamMembership(Team *newTeam)
{
#if !defined(NDEBUG)
    bool found = false;
    for (const auto *member : newTeam->memberAgents()) {
        if (member->UUID() == UUID()) {
            found = true;
            break;
        }
    }
    assert(found);
#endif

    for (const Team *team : m_teamMemberships) {
        if (team->UUID() == newTeam->UUID()) {
            JACK_WARNING("Agent '{}' is already a member of the team '{}'", m_handle.toHumanString(), newTeam->handle().toHumanString());
            return false;  // Already exists
        }
    }

    m_teamMemberships.push_back(newTeam);
    return true;
}

void Agent::processSharedBeliefs()
{
    /// \note Rate limit the sharing of beliefsets otherwise we can
    /// saturate the Team's comm channels and cause it to constantly
    /// redirty the schedule causing the team to never be able to
    /// schedule a plan.
    ///
    /// \note If the executor is done and we haven't shared our
    /// beliefset, i.e. we finished and our last beliefset was dirtied
    /// 100ms ago- we flush out these remaining changes to everyone when
    /// we're idle- otherwise the other nodes may never receive this
    /// data because the time threshold to relay is greater than 100ms
    /// and since the agent is no longer doing anything, nothing may
    /// warrant a change in their beliefset to cause a relay, so we
    /// ensure that happens by triggering it in this case.
    bool shareBeliefs = false;
    std::chrono::milliseconds msSinceLastShare = m_lastTimeBeliefSetsDirtied - m_lastTimeBeliefSetsShared;

    if (m_roles.size() &&
        ((msSinceLastShare >= std::chrono::milliseconds(500)) ||
         (m_executor.done() && msSinceLastShare != std::chrono::milliseconds(0))))
    {
        shareBeliefs = true;
    }

    // Share my beliefsets to team. For each of the beliefsets we have,
    // check if we have a role that has write access to the team. If so,
    // we can share (write) it to the team.
    if (shareBeliefs) {
        m_lastTimeBeliefSetsShared = m_lastTimeBeliefSetsDirtied;
        std::shared_ptr<BeliefContext::MessageMap> messages = m_context.messages();
        for (const auto &pair : *messages) {
            for (const std::string &roleName : m_roles) {
                const Role *role = engine().getRole(roleName);
                if (!role) {
                    JACK_ERROR("Queried role '{}' does not exist in engine", roleName);
                    continue;
                }

                if (!role->canWriteBeliefSetToTeam(pair.first /*beliefSet name*/)) {
                    continue;
                }

                // Share the beliefset to every team member since write is
                // permitted then, exit the loop and check the next
                // beliefset.
                for (Team *team : teamMemberships()) {
                    const std::shared_ptr<Message> &beliefSet = pair.second;
                    auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, ShareBeliefSetEvent, this, team, UUID(), name(), beliefSet);
                    routeEvent(event);
                }
                break;
            }
        }
    }

    // Share our received shared beliefsets from the team to our members
    if (auto *team = dynamic_cast<Team *>(this)) {
        for (auto &it : m_sharedBeliefSets) {
            for (SharedBeliefSet &sharedBeliefSet : it.second /*vector<SharedBeliefSet>*/) {

                // Skip the beliefset if it has not been updated since the last time we checked
                if (sharedBeliefSet.m_lastUpdated == sharedBeliefSet.m_prevLastUpdated) {
                    continue;
                } else {
                    sharedBeliefSet.m_prevLastUpdated = sharedBeliefSet.m_lastUpdated;
                }

                // Relay the *updated* beliefset to our team members
                for (Agent *member : team->memberAgents()) {
                    if (member->UUID() == sharedBeliefSet.m_memberId) {
                        // Don't share the beliefset back to the member that it
                        // originated from.
                        continue;
                    }

                    const std::vector<std::string> &roleNames = member->roles();
                    for (const std::string &roleName : roleNames) {

                        const Role *role = engine().getRole(roleName);
                        if (!role) {
                            JACK_ERROR("Queried role '{}' does not exist in engine", roleName);
                            continue;
                        }

                        if (!role->canReadBeliefSetFromTeam(sharedBeliefSet.m_beliefSet->schema())) {
                            continue;
                        }

                        // The member has a role that can read the beliefset, we
                        // will share it to them then, exit the loop and check
                        // the next beliefset.
                        auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                                         ShareBeliefSetEvent,
                                                         this,
                                                         member,
                                                         sharedBeliefSet.m_memberId,
                                                         sharedBeliefSet.m_memberName,
                                                         sharedBeliefSet.m_beliefSet);
                        routeEvent(event);
                        break;
                    }
                }
            }
        }
    }
}

bool Agent::CurrentAuction::finished(std::chrono::milliseconds clock) const
{
    bool result = clock >= expiryTimePoint || bids.size() == totalDelegations;
    return result;
}

void Agent::processCurrentDelegationEvents()
{
    /**************************************************************************
     * Delegation requests received to this agent are processed out-of-band from
     * the event queue because analysing the cost of the event may take multiple
     * iterations which are executed in this function.
     *
     * 1. Analyse the delegation event in the schedule
     * 2. Handle the completed analysis of the delegation goal
     **************************************************************************/

    for (auto it = m_delegationEventBackLog.begin(); it != m_delegationEventBackLog.end(); ) {
        bool finished = true;

        /// 1. Analyse the delegation event in the schedule
        DelegationEvent* event = it->delegationEvent;
        JACK_ASSERT(event);

        if (it->delegationGoalAlreadyBeingExecuted) {
            /// \todo We are already executing the exact goal the team
            /// is auctioning to us at this point in time. Can cost be 0?
            /// Review this
            event->score = 0.f;
        } else {
            processSchedule(it->schedule.get());
            if (it->schedule->isFinished()) {
                event->score = it->schedule->getBestCost();
            } else {
                finished = false;
            }
        }

        /// 2. Handle the completed analysis of the delegation goal
        if (finished) {
            if (event->score == Schedule::FAILED_COST) {
                event->status = Event::FAIL;
            } else {
                event->status = Event::SUCCESS;
            }

            /// Simulation finished return to sender (i.e. the Team)
            event->simulatedSchedule.swap(it->schedule);
            m_engine.returnEvent(event);
            it = m_delegationEventBackLog.erase(it);
        } else {
            it++;
        }
    }
}

void Agent::processCurrentAuctions()
{
    /**************************************************************************
     * Process auctions that were initiated by this agent by evaluating the
     * auction state.
     **************************************************************************/
    for (auto it = m_currentAuctions.begin(); it != m_currentAuctions.end(); ) {
        CurrentAuction &auction = *it;
        auto clockMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_engine.internalClock());
        if (auction.finished(clockMs)) {
            AuctionEvent *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                                     AuctionEvent,
                                                     auction.goal,
                                                     auction.scheduleId);
            event->m_bids        = std::move(auction.bids);
            event->m_missingBids = static_cast<uint16_t>(auction.totalDelegations - event->m_bids.size());
            JACK_ASSERT(auction.totalDelegations >= event->m_bids.size());
            addEvent(event);
            it = m_currentAuctions.erase(it);
        } else {
            it++;
        }
    }
}
}  // namespace aos::jack
