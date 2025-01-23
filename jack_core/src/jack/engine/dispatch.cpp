#include <jack/engine/dispatch.h>
#include <jack/events/event.h>       // for Event
#include <jack/promise.h>            // for Promise, PromisePtr

#include <jack/events/delegationevent.h>
#include <jack/service.h>

#include <tracy/Tracy.hpp>
#include <memory>               // for make_shared, weak_ptr

namespace aos::jack
{
void Dispatch::processEvents()
{
    ZoneScoped;
    Event *event;
    while(m_eventQueue.dequeue(event)) {
        eventDispatch(event);
    }
}

PromisePtr Dispatch::addEvent(Event *event)
{
    PromisePtr p = std::make_shared<Promise>(event->eventId);
    event->promise = p;
    m_eventQueue.enqueue(event);
    return p;
}

void Dispatch::routeEvent(Event *event)
{
    m_eventQueue.enqueue(event);
}
} // namespace aos::jack
