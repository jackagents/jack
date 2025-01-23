#ifndef JACK_TACTIC_EVENT_H
#define JACK_TACTIC_EVENT_H

#include <jack/events/event.h>
#include <jack/handles.h>

namespace aos::jack
{
/*! ***********************************************************************************************
 * @struct  TacticEvent
 * An event to set a tactic on an agent
 * ************************************************************************************************/
struct TacticEvent : public Event
{
    TacticEvent(TacticHandle handle)
    : Event(Event::TACTIC), handle(handle)
    {
    }

    TacticHandle handle;
};
} // namespace aos::jack
#endif // JACK_TACTIC_EVENT_H
