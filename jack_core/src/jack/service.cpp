#include <jack/service.h>

#include <jack/event-protocol/protocol.h>
#include <jack/engine.h>                      // for Engine
#include <jack/events/actioncompleteevent.h>  // for ActionCompleteEvent
#include <jack/events/actionevent.h>          // for ActionEvent
#include <jack/events/messageevent.h>         // for MessageEvent
#include <jack/events/pursueevent.h>
#include <jack/corelib.h>                     // for JACK_DEBUG, JACK_ERROR, JACK...

#include <unordered_set>                 // for unordered_set, _Uset_traits<...
#include <memory.h>

namespace aos::jack
{
/* ************************************************************************************************
 * Public Ctor & Dtor
 * ************************************************************************************************/

Service::Service(Engine& engine, std::string_view name)
        : m_engine(engine)
        , m_handle({std::string(name), UniqueId::random()}) /// \todo Duplicated between the handle and the address
        , m_state(Service::STOPPED)
        , m_busAddress(protocol::BusAddress{protocol::NodeType_SERVICE, m_handle.m_id.toString(), name})
        , m_isAvailable(true)
{
}

Service::Service(const Service* other, std::string_view newName)
    : Service(other->m_engine, newName)
{
    m_templateName   = other->m_templateName;
    m_actionHandlers = other->m_actionHandlers;
    m_beliefsetIds   = other->m_beliefsetIds;
}

Service::~Service()
{
    size_t eventsDequeued = 0;
    do {
        Event* events[32];
        eventsDequeued = m_eventQueue.dequeueBulk(events, arrayCountUSize(events));
        for (size_t eventIndex = 0; eventIndex < eventsDequeued; eventIndex++) {
            /// \note Trigger any handlers and cleanup on the event before
            /// returning it to the allocator.
            /// \todo WARNING If the promise generates more data to pipe into
            /// the engine that data may be leaked as we're in the destructor of
            /// this agent and generating new data that we're not handling.
            Event *event = events[eventIndex];
            if (dynamic_cast<PursueEvent*>(event)) {
              /// \todo This is a dodgy work-around! The promise callback on a
              /// pursue event for a subgoal (of which a subgoal that is
              /// triggered by a coroutine) may no longer be valid at this point
              /// in time.
              ///
              /// It's possible that the executor that owns the plan (which owns
              /// the coroutine, which owns the pursue task) has been
              /// deinitialised.
              ///
              /// The promise associated with this event is a lambda that takes
              /// a pointer to the task (that is has been freed!) to trigger the
              /// task's success or fail method which would advance the
              /// coroutine.
              ///
              /// Since the coroutine does not exist anymore because it's been
              /// deinitialised, this causes a use-after-free.
              ///
              /// This instance of the bug is non-critical because we are
              /// shutting down and we typically hit this in unit-tests on
              /// cleaning up. We may hit this bug if we destruct agents during
              /// the run-time of the engine whilst they're executing, but we do
              /// not do this so it's unlikely we'll hit it again for now.
            } else {
                event->fail();
            }
            JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, Event, event, JACK_ALLOCATOR_CLEAR_MEMORY);
        }
    } while(eventsDequeued > 0);

    for (auto it : m_currentActions) {
        /// \todo WARNING If the promise generates more data to pipe into
        /// the engine that data may be leaked as we're in the destructor of
        /// this agent and generating new data that we're not handling.
        it->fail();
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, ActionEvent, it, JACK_ALLOCATOR_CLEAR_MEMORY);
    }
}

/* ************************************************************************************************
 * Public Functions
 * ************************************************************************************************/

std::chrono::milliseconds Service::getUpTime() const
{
    using namespace std::chrono;
    auto msSinceEpochStart = duration_cast<milliseconds>(m_startTime.time_since_epoch());
    auto msSinceEpochNow   = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    auto zero              = milliseconds(0);
    return (msSinceEpochStart == zero) ? zero : (msSinceEpochNow - msSinceEpochStart);
}

void Service::start()
{
    auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, Event, Event::CONTROL);
    event->controlEvent         = m_engine.makeProtocolEvent<protocol::Control>(busAddress() /*sender*/,
                                                                                busAddress() /*recipient*/,
                                                                                UniqueId::random());
    event->controlEvent.command = protocol::ControlCommand_START;
    addEvent(event);
}

void Service::stop()
{
    auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, Event, Event::CONTROL);
    event->controlEvent         = m_engine.makeProtocolEvent<protocol::Control>(busAddress() /*sender*/,
                                                                                busAddress() /*recipient*/,
                                                                                UniqueId::random());
    event->controlEvent.command = protocol::ControlCommand_STOP;
    addEvent(event);
}

void Service::pause()
{
    auto *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, Event, Event::CONTROL);
    event->controlEvent         = m_engine.makeProtocolEvent<protocol::Control>(busAddress() /*sender*/,
                                                                                busAddress() /*recipient*/,
                                                                                UniqueId::random());
    event->controlEvent.command = protocol::ControlCommand_PAUSE;
    addEvent(event);
}

bool Service::setState(State state)
{
    std::string_view entityLabel = "Service"sv;
    if (const auto* agent = dynamic_cast<Agent*>(this)) {
        entityLabel = agent->isTeam() ? "team"sv : "agent"sv;
    }

    if (m_state == state) {
        std::string_view label = "BAD_AGENT_STATUS_ENUM"sv;
        switch (state) {
            case Service::STOPPED:  label = "stopped"sv;  break;
            case Service::STOPPING: label = "stopping"sv; break;
            case Service::RUNNING:  label = "running"sv;  break;
            case Service::PAUSED:   label = "paused"sv;   break;
        }
        JACK_INFO("{} {} request ignored, already {} [handle={}]", entityLabel, label, label, handle().toString());
        return false;
    }

    if (state == RUNNING) {
        JACK_INFO("Starting {} [handle={}]", entityLabel, handle().toString());
        m_startTime = std::chrono::system_clock::now();  // record the start time of the agent
        onStart();
    } else if (state == STOPPED || state == PAUSED) {
        if (state == STOPPED) {
            JACK_INFO("Stopped {} [handle={}]", entityLabel, handle().toString());
        } else {
            JACK_INFO("Pausing {} [handle={}]", entityLabel, handle().toString());
        }
    } else if (state == STOPPING) {
        JACK_INFO("Stopping {} [handle={}]", entityLabel, handle().toString());
    } else {
        assert(!"Invalid code path, unhandled enum");
    }

    m_state = state;
    return true;

}

void Service::sendMessage(std::shared_ptr<Message> msg, bool broadcastToBus, Service *recipient)
{
    MessageEvent *event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                             MessageEvent,
                                             msg,
                                             false /*deprecatedMsg*/,
                                             broadcastToBus);
    event->caller    = this;
    event->recipient = recipient;
    routeEvent(event);
}

void Service::setUUID(const UniqueId &id)
{
    /// \todo This ID is duplicated between the handle and the bus address which
    /// means we have this synchronizing problem in updating the agent's address
    /// and ID uniformly.
    m_handle.m_id   = id;
    m_busAddress.id = id.toString();
}

bool Service::finishActionHandle(const ActionHandle &handle, bool success, const jack::Message* reply)
{
    ZoneScoped;
    /// Map the action handle to the concrete action object, and mark the
    /// concrete action object as finished.
    ActionEvent *action = nullptr;
    for (ActionEvent *check : m_currentActions) {
        if (check->handle() == handle) {
            action = check;
            break;
        }
    }

    bool result = false;
    if (action && action->status == Event::PENDING) {

        /// \note Apply the reply message to the action
        if (reply) {
            /// \note If the schema is missing we have an inconsistent engine
            /// state that's gone out of sync with the model.
            const MessageSchema* replySchema = m_engine.getMessageSchema(action->reply()->schema());
            if (JACK_CHECK(replySchema)) {
                MessageSchema::VerifyMessageResult verify = replySchema->verifyMessage(*reply);
                if (verify.success) {
                    /// They are the same message type so we can copy over
                    *action->reply() = *reply;
                } else {
                    JACK_WARNING("Action was completed but the reply message did not match the action's reply schema [service={}, action={}, reason={}]",
                                 this->handle().toString(),
                                 action->name(),
                                 verify.msg);
                }
            }
        }

        /// \note Update the action status
        if (success) {
            action->status = Event::SUCCESS;
        } else {
            action->status = Event::FAIL;
        }
        result = true;
    }

    return result;
}

ChunkAllocator *Service::eventAllocator()
{
    ChunkAllocator *result = &m_engine.m_eventAllocator;
    return result;
}

void Service::processCurrentActions()
{
    ZoneScoped;
    /**************************************************************************
     * Actions that are complete are finalized by generating the
     * ActionCompleteEvent when the action is marked completed.
     **************************************************************************/
    for (auto it = m_currentActions.begin(); it != m_currentActions.end(); ) {
        ActionEvent *action = *it;
        if (processCompletedAction(action)) {
            it = m_currentActions.erase(it);
        } else {
            it++;
        }
    }

}

bool Service::processCompletedAction(ActionEvent* action)
{
    ZoneScoped;
    bool result = false;
    if (action && (action->status == Event::SUCCESS || action->status == Event::FAIL)) {
        ActionCompleteEvent* event = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                                        ActionCompleteEvent,
                                                        action->name(),
                                                        action->m_taskId,
                                                        action->status,
                                                        action->m_goal,
                                                        action->m_intentionId,
                                                        action->m_plan,
                                                        action->reply());
        event->caller              = action->recipient;
        event->recipient           = action->caller;
        event->resourceLocks       = std::move(action->m_resourceLocks);

        if (!action->recipient) {
            /// \note If the action has no recipient, then this is a local
            /// action we sent to ourselves, the caller is also ourselves.
            ///
            /// \todo This is quite confusing the routing. I put this re-routing
            /// information into this function because I think services and
            /// actions are affected by this.
            ///
            /// We should consider only having 1 queue, since we have
            /// a lock-free queue, it doesn't matter that much to allow
            /// agents/services to have their own queues since even if we
            /// multithread them it will still be thread safe.
            assert(action->caller);
            event->caller = action->caller;
        }

        if (m_engine.haveBusAdapter()) {
            /// \note Pull out the explainability messaging from the action
            auto reply = action->reply();

                /// \note Emit messages on the bus
            protocol::BDILog logEvent =
                m_engine.makeBDILogActionFinished(
                    bdiLogHeader(reply ? static_cast<protocol::BDILogLevel>(reply->reasoningLevel()) : protocol::BDILogLevel::BDILogLevel_NORMAL),
                    action,
                    reply ? reply->reasoningText() : "");

            /// \todo If we are a pure service, we currently don't want to set
            /// the sender as the service itself. The agent executed an action
            /// which got routed to a service. Currently the JS clients don't
            /// have a way of matching an action started from an agent and an 
            /// action finished from a service.
            logEvent.sender = event->recipient->busAddress();
            m_engine.sendBusEvent(&logEvent);
        }

        addEvent(event);
        result = true;
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, ActionEvent, action, JACK_ALLOCATOR_CLEAR_MEMORY);
    }

    return result;
}

void Service::eventDispatch(Event *event)
{
    ZoneScoped;
    /**************************************************************************
     * Process an outgoing event
     **************************************************************************/
    if (event->recipient) {
        if (event->recipient->UUID() != UUID()) {
            // It's not for this service route to the engine
            event->caller = this;
            m_engine.routeEvent(event);
            return;
        }

        assert(event->recipient->UUID() == UUID() && "Recipient should be us");
    }

    /**************************************************************************
     * Process an incoming event
     **************************************************************************/
    /// \note Proxy services intercept events and route it onto the bus
    /// \todo We don't have a thing such as a proxy service. We do have a proxy
    /// agent, but we've had offline discussions and agreed that the step
    /// forward here is that proxy-iness is just a flag on the agent/service.
    ///
    /// The idea is that agents/services can be upgrade and demoted from a proxy
    /// version very easily but they should contain all the information that the
    /// real version has so it can substitute in easily.
    ///
    /// Instead of introducing a ProxyService class to follow suite with
    /// ProxyAgents since it doesn't exist yet we use the proxy flag approach
    /// here.
    ///
    /// This does mean that we copy and paste some code from the ProxyAgent,
    /// namely the bus event setup code. Eventually in the future we can convert
    /// the ProxyAgent to using a flag and helpers methods to go from JACK
    /// internal events to the protocol events exposed in a nice helper file
    /// instead of copying and pasting them as we do for now.
    bool returnEventToAllocator = true;
    switch(event->type) {
        case (Event::CONTROL): {
            ZoneNamedN(debugTracyEngineControl, "Service control event", true);
            protocol::Control* controlEvent = &event->controlEvent;
            [[maybe_unused]] std::string_view label = protocol::commandTypeString(controlEvent->command);
            ZoneNameV(debugTracyEngineControl, label.data(), label.size());

            bool               stateTransition = false;
            if (controlEvent->command == protocol::ControlCommand_START) {
                stateTransition = setState(RUNNING);
            } else if (controlEvent->command == protocol::ControlCommand_STOP) {
                stateTransition = setState(STOPPING);
            } else if (controlEvent->command == protocol::ControlCommand_PAUSE) {
                stateTransition = setState(PAUSED);
            }

            /// \todo I think there's a feedback loop here, the events should
            /// stop at the proxy?
            if (stateTransition && engine().haveBusAdapter()) {
                controlEvent->senderNode = engine().busAddress();
                controlEvent->recipient  = busAddress();

                /// \todo We create the event earlier and submit it into the
                /// event queue. Since the event queue might be processed on the
                /// next tick, the timestamp is no longer accurate and causes
                /// a sanity assert.
                controlEvent->timestampUs = std::chrono::duration_cast<std::chrono::microseconds>(m_engine.internalClock()).count();

                engine().sendBusEvent(controlEvent);
            }
        } break;

        case Event::ACTION: {
            ZoneNamedN(debugTracyServiceAction, "Service action event", true);
            ActionEvent* actionEvent = static_cast<ActionEvent*>(event);
            ZoneNameV(debugTracyServiceAction, actionEvent->name().data(), actionEvent->name().size());

            assert(actionEvent);
            assert(actionEvent->caller    != this && "We are expecting actions to be requested by the Agent and we use this information to return the event back to them");
            assert(actionEvent->recipient == this);
            assert(returnEventToAllocator);

            if (0/*isProxy()*/) {   /// @todo removed util the distributed service are working again on the bus
                /// \note Forward event onto the bus so it arrives at the real
                /// service instance.
                if (engine().haveBusAdapter()) {
                  auto busEvent = m_engine.makeProtocolEvent<protocol::ActionBegin>(event->caller->busAddress(),
                                                                                    event->recipient->busAddress(),
                                                                                    event->eventId);
                  busEvent.name          = actionEvent->name();
                  busEvent.taskId        = actionEvent->m_taskId.toString();
                  busEvent.goalId        = actionEvent->m_goal.m_id.toString();
                  busEvent.goal          = actionEvent->m_goal.m_name;
                  busEvent.intentionId   = actionEvent->m_intentionId.toString();
                  busEvent.plan          = actionEvent->m_plan;
                  busEvent.message       = actionEvent->request();
                  busEvent.resourceLocks = actionEvent->m_resourceLocks;
                  engine().sendBusEvent(&busEvent);
                }
            } else {
                auto actionHandlerIt = m_actionHandlers.find(actionEvent->name());
                if (actionHandlerIt != m_actionHandlers.end()) {

                    /// \todo We log actions executed by an agent via a service
                    /// through the agent because the UI is currently unable to
                    /// link the action to the agent.
                    ///
                    /// For accuracy, we should be logging from the service, bus
                    /// , but, there's no information currently emitted that
                    /// indicates who requested the action to be executed
                    /// through the service.
                    #if 0
                    BDILogHeader header = {};
                    header.node         = engine().busAddress();
                    header.sender       = busAddress();
                    header.level        = protocol::BDILogLevel_NORMAL;
                    header.eventId      = jack::UniqueId::random().toString();

                    for (BusAdapter* adapter : engine().busAdapters()) {
                        adapter->bdiLogActionStarted(header,
                                                     actionEvent->m_goal.m_name,            /// goal
                                                     actionEvent->m_goal.m_id.toString(),   /// goalId
                                                     actionEvent->m_intentionId.toString(), /// intentionId
                                                     actionEvent->m_plan,                   /// plan
                                                     actionEvent->name());
                    }
                    #endif

                    const auto& actionFunction = actionHandlerIt->second;
                    actionEvent->status        = actionFunction(*this, *actionEvent->request(), *actionEvent->reply(), actionEvent->handle());

                    /// \todo Test resource locks being sent over the bus and
                    /// locking them here
                    /// Try and fast track the action, no-op if not possible
                    returnEventToAllocator = false;
                    if (!processCompletedAction(actionEvent)) {
                        m_currentActions.push_back(actionEvent);
                    }
                } else {
                    // just return back to the engine failed
                    JACK_ERROR("Action is not handled by service [name={}, action={}]", handle().toString(), actionEvent->name());
                    m_engine.routeEvent(JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator,
                                                           ActionCompleteEvent,
                                                           actionEvent->name(),
                                                           actionEvent->m_taskId,
                                                           Event::FAIL,
                                                           actionEvent->m_goal,        /// goal
                                                           actionEvent->m_intentionId, /// intentionId
                                                           actionEvent->m_plan,        /// plan
                                                           actionEvent->reply()));
                }
            }
        } break;

        case Event::MESSAGE: {
            ZoneNamedN(debugTracyServiceMessage, "Service message event", true);
            [[maybe_unused]] const MessageEvent* msgEvent = static_cast<MessageEvent*>(event);
            ZoneNameV(debugTracyServiceMessage, msgEvent->msg.m_schemaName.data(), msgEvent->msg.m_schemaName.size());

            m_engine.routeEvent(event);
            returnEventToAllocator = false;
        } break;

        default: {
            JACK_ERROR("Unhandle Event in service '{}'", handle().toHumanString());
        } break;
    }

    if (returnEventToAllocator) {
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, Event, event, JACK_ALLOCATOR_CLEAR_MEMORY);
    }
}

void Service::run()
{
    ZoneScoped;
    [[maybe_unused]] const std::string_view serviceName = name();
    ZoneName(serviceName.data(), serviceName.size());

    /*! ***************************************************************************************
    * Execute the process for a service
    * ****************************************************************************************/
    if (m_state == STOPPING) {
        /// \todo Shutdown any active actions, transition to stopped
        m_state = STOPPED;
    }

    if (m_state == STOPPED) {
        return;
    }

    processCurrentActions();
}

PromisePtr Service::addEvent(Event* event)
{
    PromisePtr p = Dispatch::addEvent(event);
    notifyScheduler();
    return p;
}

void Service::setAvailability(bool available)
{
    ZoneScoped;
    /// @todo: @robust This should be an event so that the protocol can
    /// observe it but also it needs to trigger a dirty on all agents that rely
    /// on it. But this needs some thought, does it go all the way up the agent
    /// hierarchy? Is this a BDI log or an event, because wouldn't other remote
    /// nodes determine if a service is available by their own heartbeating
    /// mechanism?
    ///
    /// Can a remote application sitting on the bus willingly set a service to
    /// be unavailable? Probably yes you can imagine a GUI that is a dashboard
    /// that lets you override agents and services in JACK.
    ///
    /// In the meantime this can be considered a proof-of-concept of supporting
    /// services that can go offline and that agents will plan around it.
    m_isAvailable = available;

    /// \note Trigger dirty on all agents reliant on this service
    for (Agent* agent : m_engine.agents()) {
        for (const ServiceHandle& check : agent->attachedServices()) {
            if (handle() == check) {
                agent->forceReschedule();
                for (Team* team : agent->teamMemberships()) {
                    team->forceReschedule();
                }
            }
        }
    }
}

void Service::percept(std::string_view message, std::string_view key, const std::any& value)
{
    ZoneScoped;

    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    [[maybe_unused]] std::string_view label = JACK_FMT_ALLOC(scratch.arena, "Send key-value percept {}, {}", message, key);
    ZoneName(label.data(), label.size());

    const MessageSchema* schema = engine().getMessageSchema(message);
    if (schema) {
        Field field = {};
        for (const auto& check: schema->fields()) {
            if (check.m_name == key) {
                field = check;
            }
        }

        if (field.m_name.empty()) {
            JACK_DEBUG("Percept to send via service specifies an invalid field for the message it is updating, percept rejected [service={}, message={}, key={}]",
                       handle().toString(),
                       message,
                       key);
        } else {
            field.m_value = value; /// @todo: We must validate that the m_type field matches

            /// \todo We don't cache percepts in the service? We should only trigger
            /// a percept event if the value has changed.
            auto* event = JACK_ALLOCATOR_NEW(eventAllocator(), PerceptEvent, message, /*isMessage*/ true, field, /*broadcastToBus*/ true);
            if (!JACK_CHECK(event)) {
                JACK_ERROR("Memory allocation failure, event could not be allocated [service={}, message={}]",
                           handle().toString(),
                           message);
                return;
            }

            event->caller = this;
            m_engine.routeEvent(event);
        }
    } else {
        JACK_ERROR("Percept to send via service {} specifies an unknown message "
                   "because no matching message schema was found, percept "
                   "rejected. [message={}, key={}]",
                   m_handle.toHumanString(),
                   message,
                   key);
    }

}

void Service::notifyScheduler()
{
    /*! ***************************************************************************************
     * Let this service's scheduler know that we have work to do
     * Wake it's thread up if it's hybernating
     * ****************************************************************************************/
    m_engine.notify();
}

protocol::BDILogHeader Service::bdiLogHeader(protocol::BDILogLevel level, const UniqueId& id) const
{
    protocol::BDILogHeader result = {};
    result.sender                 = busAddress();
    result.level                  = level;
    result.eventId                = id;
    return result;
}
}  // namespace aos::jack
