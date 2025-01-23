#ifndef JACK_PERCEPTEVENT_H
#define JACK_PERCEPTEVENT_H

#include <jack/events/event.h>
#include <jack/corelib.h>
#include <jack/message.h>

namespace aos::jack
{
/******************************************************************************
 * \struct  PerceptEvent
 ******************************************************************************/
struct PerceptEvent : public Event
{
    PerceptEvent(std::string_view name, bool isMessage, const Field& field, bool broadcastToBus)
        : Event(Event::PERCEPT)
        , name(name)
        , isMessage(isMessage)
        , field(field)
        , broadcastToBus(broadcastToBus)
    {}

    PerceptEvent* clone(ChunkAllocator *eventAllocator) const
    {
        return JACK_ALLOCATOR_NEW(eventAllocator, PerceptEvent, name, isMessage, field, broadcastToBus);
    }

    /// Name of the beliefset (message) or resource this event modifies.
    std::string name;
    bool        isMessage; ///< The percept modifies a beliefset (message) otherwise modifies a resource
    Field       field;     ///< The percept value

    /// Forward this event onto the bus to be broadcast to all other instances
    /// of the recipient (proxy or concrete).
    bool broadcastToBus = false;
};
} // namespace aos::jack
#endif // JACK_PERCEPT_EVENT_H
