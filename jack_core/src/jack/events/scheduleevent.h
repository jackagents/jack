// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_SCHEDULEEVENT_H
#define JACK_SCHEDULEEVENT_H

#include <jack/events/event.h>

#include <string>

namespace aos { namespace jack {

class Schedule;

/*! ***********************************************************************************************
 * \struct  ScheduleEvent
 *
 * An event to handle changes to the schedule
 * ************************************************************************************************/

struct ScheduleEvent : public Event
{
    ScheduleEvent(Schedule *schedule)
        : Event(Event::SCHEDULE)
        , schedule(schedule)
    {}

    Schedule *schedule;
};

}} // namespace aos::jack

#endif // JACK_SCHEDULEEVENT_H
