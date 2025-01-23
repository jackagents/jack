#ifndef JACK_MESSAGEEVENT_H
#define JACK_MESSAGEEVENT_H

#include <jack/events/event.h>
#include <jack/message.h>

namespace aos { namespace jack {

/******************************************************************************
 * \struct  MessageEvent
 * The message event type
 ******************************************************************************/
struct MessageEvent : public Event
{
    MessageEvent(std::shared_ptr<Message> msg, bool deprecatedMsg, bool broadcastToBus)
        : Event(Event::MESSAGE)
        , msg(msg)
        , deprecatedMsg(deprecatedMsg)
        , broadcastToBus(broadcastToBus)
    {}

    std::shared_ptr<Message> msg;
    bool deprecatedMsg;
    bool broadcastToBus;
};

}} // namespace aos::jack

#endif // JACK_MESSAGEEVENT_H
