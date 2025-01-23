#ifndef JACK_DISPATCH_H
#define JACK_DISPATCH_H

#include <jack/promise.h>     // for PromisePtr
#include <jack/event-protocol/eventqueue.h>  // for EventQueue

namespace aos { namespace jack {

struct Event;

/*! ***********************************************************************************************
 * @class   Dispatch
 *
 * A dispatch for managing an event queue and handling the events
 * ************************************************************************************************/

class Dispatch
{
public:
    Dispatch() = default;

    /// Process all events in the queue and dispatch
    void processEvents();

    virtual void eventDispatch(Event *event) = 0;

    /// Add the event and return a promise
    PromisePtr addEvent(Event *event);

    // route event - same as add but with no promise
    void routeEvent(Event *event);

    /// @return Are there any event current in the queue
    bool haveEvents() const { return !m_eventQueue.empty(); }

protected:
    /* ****************************************************************************************
    * Attributes
    * ****************************************************************************************/
    EventQueue<Event*> m_eventQueue;
};

}} // namespace aos::jack

#endif
