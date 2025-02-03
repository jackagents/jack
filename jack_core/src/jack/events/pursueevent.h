// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_PURSUEEVENT_H
#define JACK_PURSUEEVENT_H

#include <jack/events/event.h>
#include <jack/message.h>

namespace aos { namespace jack {

class Task;

/*! ***********************************************************************************************
 * \struct  PursueEvent
 *
 * An event to pursue a new desire
 * ************************************************************************************************/

struct PursueEvent : public Event
{
    PursueEvent(const std::string&       goal,
                IntentionExecutor::Id    parentId,
                std::shared_ptr<Message> parameters,
                bool                     persistent)
        : Event(Event::PURSUE),
          goal(goal),
          parentId(parentId),
          parameters(parameters),
          persistent(persistent)
    {
    }

    // the goal identifier
    std::string goal;

    // the intention that wants to pursue this goal (sub - goals)
    IntentionExecutor::Id parentId;

    UniqueId parentTaskId;

    // The promise to notify
    /// \todo this is shared unlike the base which is weak. This is quite
    /// confusing. Need a better ownership paradigm here.
    std::shared_ptr<Promise> strongPromise;

    /// The starting parameters for the goal to be pursued
    std::shared_ptr<Message> parameters;

    /// The goal is permanently desired and reattempted even if the goal
    /// succeeds or fails.
    bool persistent;
};

}} // namespace aos::jack

#endif // JACK_PURSUEEVENT_H
