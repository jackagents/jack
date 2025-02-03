// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/proxyagent.h>

#include <jack/events/delegationevent.h>     // for DelegationEvent
#include <jack/events/pursueevent.h>         // for PursueEvent
#include <jack/events/actioncompleteevent.h> // for PursueEvent
#include <jack/events/event.h>               // for Event, Event::DELEGATION, Event:...
#include <jack/engine.h>                     //
#include <jack/event-protocol/protocol.h>
#include <jack/corelib.h>

#include <utility>                  // for swap, pair
#include <vector>                   // for vector
#include <string.h>                 // for memcpy

namespace aos::jack
{
void ProxyAgent::ensureThatEventsThatMightOnlyBeRoutedLocallyGetRoutedOntoTheBus(Agent& agent, const Event* event, bool proxy)
{
    ZoneScoped;
    Engine& engine = agent.engine();

    /// \todo Unify the JACK events with the protocol events, then we have
    /// a trivial forwarding of events from the engine to the bus. This
    /// might not be possible sometimes because the transport format may not
    /// be optimal/compatible with the internal format but we should strive
    /// towards this.
    if (!engine.haveBusAdapter()) {
        /// Event was either requested directly on the ProxyAgent or
        /// forwarded from the Engine, but we don't have a bus so we can't
        /// forward this any further, we have to throw away this message as
        /// we can't do anything with it.
        return;
    }

    protocol::BusAddress sender    = {};
    protocol::BusAddress recipient = {};
    if (event->caller) {
        sender = event->caller->busAddress();
    } else {
        sender = agent.busAddress();
    }

    if (event->recipient) {
        recipient = event->recipient->busAddress();
    } else {
        recipient = agent.busAddress();
    }

    if (event->type == Event::PURSUE) {
        ZoneNamedN(debugTracyAgentPursue, "Proxy agent pursue event", true);
        if (const auto *pursue = dynamic_cast<const PursueEvent *>(event)) {
            ZoneNameV(debugTracyAgentPursue, pursue->goal.data(), pursue->goal.size());

            if (pursue->parentId != IntentionExecutor::NULL_ID /*triggered from subgoal*/) {
                if (proxy) {
                    /// \todo We don't handle sub-goals yet across nodes.
                    /// There needs to be a request, reply scenario because on
                    /// task completion we need to advance the coroutine
                    /// from the original sender.
                    assert(!"It's time to implement this");
                }
            } else {
                auto busEvent       = engine.makeProtocolEvent<jack::protocol::Pursue>(sender, recipient, event->eventId);
                busEvent.goal       = pursue->goal;
                busEvent.persistent = pursue->persistent;
                busEvent.message    = pursue->parameters;
                engine.sendBusEvent(&busEvent);
            }
        } else {
            JACK_ERROR_MSG("Event has pursue type, but event is not a pursue");
            JACK_ASSERT(pursue);
        }
    } else if (event->type == Event::DELEGATION) {
        ZoneNamedN(debugTracyAgentDelegation, "Proxy agent delegation event", true);
        if (const auto* delegation = dynamic_cast<const DelegationEvent*>(event)) {
            ZoneNameV(debugTracyAgentDelegation, delegation->goalHandle.m_name.c_str(), delegation->goalHandle.m_name.size());
            bool  converted       = true;
            jack::protocol::DelegationStatus status = {};
            switch (event->status) {
                case jack::Event::FAIL:    status = jack::protocol::DelegationStatus_FAILED; break;
                case jack::Event::PENDING: status = jack::protocol::DelegationStatus_PENDING; break;
                case jack::Event::SUCCESS: status = jack::protocol::DelegationStatus_SUCCESS; break;
                default: {
                    converted = false;
                    JACK_WARNING_MSG("Failed to convert protocol delegation status, ignoring event");
                } break;
            }

            if (converted) {
                auto busEvent     = engine.makeProtocolEvent<jack::protocol::Delegation>(sender, recipient, event->eventId);
                busEvent.status   = status;
                busEvent.goal     = delegation->goalHandle.m_name;
                busEvent.goalId   = delegation->goalHandle.m_id.toString();
                busEvent.analyse  = delegation->analyse;
                busEvent.score    = delegation->score;
                busEvent.team     = delegation->team.m_name;
                busEvent.teamId   = delegation->team.m_id.toString();

                /// @todo is making a copy/cone here the correct thing?
                // busEvent.message  = *delegation->message;
                busEvent.message  = delegation->message->clone();

                engine.sendBusEvent(&busEvent);
            }
        } else {
            JACK_ERROR_MSG("Event has delegation type, but event is not a delegation");
            JACK_ASSERT(delegation);
        }
    } else if (event->type == Event::DROP) {
        ZoneNamedN(debugTracyAgentDrop, "Proxy agent drop event", true);
        protocol::Drop drop = event->dropEvent;
        ZoneNameV(debugTracyAgentDrop, drop.goal.data(), drop.goal.size());

        /// \todo We create the event earlier and submit it into the
        /// event queue. Since the event queue might be processed on the
        /// next tick, the timestamp is no longer accurate and causes
        /// a sanity assert.
        drop.timestampUs = std::chrono::duration_cast<std::chrono::microseconds>(engine.internalClock()).count();
        engine.sendBusEvent(&drop);
    } else if (event->type == Event::ACTIONCOMPLETE) {
        ZoneNamedN(debugTracyAgentActionComplete, "Proxy agent action complete event", true);
        if (const auto *action = dynamic_cast<const ActionCompleteEvent *>(event)) {
            ZoneNameV(debugTracyAgentActionComplete, action->name.data(), action->name.size());
            if (JACK_CHECK(action->actionStatus != Event::PENDING) && engine.haveBusAdapter()) {
                protocol::ActionStatus status = {};
                bool converted = true;
                switch (action->actionStatus) {
                    case Event::FAIL:    status = protocol::ActionStatus_FAILED; break;
                    case Event::SUCCESS: status = protocol::ActionStatus_SUCCESS; break;
                    default: { converted = false; JACK_INVALID_CODE_PATH; } break;
                }

                if (converted) {
                    auto busEvent        = engine.makeProtocolEvent<protocol::ActionUpdate>(sender, recipient, event->eventId);
                    busEvent.name        = action->name;
                    busEvent.taskId      = action->taskId.toString();
                    busEvent.goalId      = action->goal.m_id.toString();
                    busEvent.goal        = action->goal.m_name;
                    busEvent.intentionId = action->intentionId.toString();
                    busEvent.plan        = action->plan;
                    busEvent.status      = status;
                    busEvent.reply       = action->reply();
                    engine.sendBusEvent(&busEvent);
                }

            }
        } else {
            JACK_ERROR_MSG("Event has action type, but event is not a action");
            JACK_ASSERT(action);
        }
    } else {
        /// \note We do not propagate this type of event yet.
    }
}

void ProxyAgent::eventDispatch(Event *event)
{
    ZoneScoped;
    /****************************************************************************************
     * Forward an agent event to the real agent
     * 1. Create a tracking ticket for the event
     * 2. Create a timer event to track timeouts
     * 3. Serialise the event
     * 4. Track the event
     * 5. Call the handler(s)
     * ****************************************************************************************/
    // Forward to the request event handler to dispatch to the real agent
    if (event->type == Event::CONTROL || event->type == Event::PERCEPT || event->type == Event::MESSAGE) {
        Agent::eventDispatch(event);
    } else if (event->type == Event::PURSUE     ||
               event->type == Event::DELEGATION ||
               event->type == Event::DROP       ||
               event->type == Event::ACTIONCOMPLETE)
    {
        if (event->type == Event::DELEGATION) {
            /// Delegations already have their caller and recipient setup as we
            /// are "delegating" to someone.
            assert(event->caller);
            assert(event->recipient);
        }

        if (!event->caller) {
            /// \note If there's no caller set, this event originates from
            /// ourselves, the proxy version.
            event->caller = this;
        }

        if (!event->recipient) {
            event->recipient = this; /// Forward the event to the concrete version of us.
        }
        JACK_ASSERT_MSG(event->recipient == this, "Dispatching an event to this proxy agent by definition means we're the recipient");
        ProxyAgent::ensureThatEventsThatMightOnlyBeRoutedLocallyGetRoutedOntoTheBus(*this, event, true /*proxy*/);
        JACK_CHUNK_ALLOCATOR_GIVE(&m_engine.m_eventAllocator, Event, event, JACK_ALLOCATOR_CLEAR_MEMORY);
    } else if (event->type == Event::CONTROL) {
        Agent::eventDispatch(event); /// forward the control events
    } else {
        /// Forward to engine and let it route it because we have a concrete
        /// version somewhere on the bus
        JACK_ASSERT_MSG(!event->recipient, "TODO: I'm not sure in what situations this will be set for a proxy agent yet");
        event->caller    = this;  /// The caller is us
        event->recipient = this;  /// The recipient is us (but the real agent version of us)
        engine().routeEvent(event);
    }
}
} // namespace aos::jack
