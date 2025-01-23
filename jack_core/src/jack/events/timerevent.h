#ifndef JACK_TIMEREVENT_H
#define JACK_TIMEREVENT_H

/// Local
#include <jack/events/event.h>

/// Project
#include <jack/engine/uuid.h>
#include <jack/handles.h>

/// Third Party
#include <chrono>
#include <string_view>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * \struct  TimerEvent
 *
 * An event to request a timer to be notified at some point in the future
 * ************************************************************************************************/

struct TimerEvent : public Event
{
    TimerEvent(std::chrono::milliseconds internalClock,
               std::chrono::milliseconds duration,
               const GoalHandle& goal,
               const UniqueId& intentionId,
               const UniqueId& taskId,
               std::string_view plan)
    : Event(Event::TIMER)
    , internalClock(internalClock)
    , duration(duration)
    , goal(goal)
    , intentionId(intentionId)
    , taskId(taskId)
    , plan(plan)
    {
    }

    std::chrono::milliseconds internalClock; /// The time the event was scheduled based on the engine's internal clock
    std::chrono::milliseconds duration;      ///< The duration the timer event will be waiting for
    GoalHandle                goal;          ///< Goal that triggered this task
    UniqueId                  intentionId;   ///< ID of the intention that triggered this task
    UniqueId                  taskId;        ///< ID of the task in intention's plan that triggered this task
    std::string               plan;          ///< Plan that triggered this task
};

}} // namespace aos::jack

#endif // JACK_TIMEREVENT_H
