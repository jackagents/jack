#ifndef JACK_DELEGATIONEVENT_H
#define JACK_DELEGATIONEVENT_H

#include <jack/events/event.h>

#include <jack/handles.h>
#include <jack/goal.h>
#include <jack/schedule.h>

namespace aos::jack
{
class Message;

/******************************************************************************
 * \struct  DelegationEvent
 *
 * An event to delegation the execution of a goal to another agent
 ******************************************************************************/

struct DelegationEvent : public Event
{
    DelegationEvent(const GoalHandle& goalHandle, const std::shared_ptr<Message>& message)
        : Event(Event::DELEGATION),
          goalHandle(goalHandle),
          message(message)
    {}

    GoalHandle               goalHandle;                  ///< A lightweight reference to the goal this delegation event is for
    std::shared_ptr<Message> message;                     ///< The configuration of this goal
    bool                     analyse             = false; ///< True if the goal cost should be simulated
    float                    score               = 0.0f;  ///< The cost to complete the delegation
    size_t                   delegatorScheduleID = 0;     ///< The schedule ID of the team that is delegating this event

    /// (Internal) Record the team that initially requested the delegation. This
    /// is used internally to ensure that the delegation is returned to the
    /// original team, no matter when it is interrupted during the lifecycle of
    /// the delegation.
    /// \todo This is not robust. We may want a sender stack here so that we can
    /// bounce these back through all the intermediate proxies for
    /// explainability.
    /// \todo This is a work-around because you can trigger delegation failure
    /// whilst the event is in flight- or in the middle of being returned (in
    /// which case the sender and recipient have been swapped) and since we call
    /// into a callback (the promise) we don't know if we need to swap the
    /// sender or recipient or not.
    ServiceHandle team = {};

    /// (Internal) When analyse is true, the event stores the team member's
    /// schedule that they used to calculate their auction bid here. This is
    /// used for metrics to visualise the schedule when they are recorded to
    /// disk.
    JACK_UNIQUE_PTR(Schedule) simulatedSchedule = JACK_INIT_UNIQUE_PTR(Schedule, nullptr);
};
} // namespace aos::jack
#endif // JACK_DELEGATIONEVENT_H
