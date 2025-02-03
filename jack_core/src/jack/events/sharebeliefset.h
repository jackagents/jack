// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_SHARE_BELIEFSET_EVENT_H
#define JACK_SHARE_BELIEFSET_EVENT_H

#include <jack/events/event.h>

#include <jack/team.h>
#include <jack/message.h>

namespace aos { namespace jack {

struct ShareBeliefSetEvent : public Event
{
    ShareBeliefSetEvent(Agent*                   sender,
                        Agent*                   dest,
                        const UniqueId&          beliefSetOwnerId,
                        const std::string&       beliefSetOwnerName,
                        std::shared_ptr<Message> beliefSet)
        : Event(Event::SHAREBELIEFSET)
        , beliefSetOwnerId(beliefSetOwnerId)
        , beliefSetOwnerName(beliefSetOwnerName)
        , beliefSet(beliefSet)
    {
        caller    = sender;
        recipient = dest;
    }

    UniqueId                 beliefSetOwnerId;
    std::string              beliefSetOwnerName;
    std::shared_ptr<Message> beliefSet;
};
}} // namespace aos::jack
#endif // JACK_SHARE_BELIEFSET_EVENT_H
