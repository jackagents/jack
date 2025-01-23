#ifndef JACK_EVENT_H
#define JACK_EVENT_H

#include <jack/promise.h>
#include <jack/event-protocol/protocol.h>

/// Third Party
#include <memory>
#if defined(JACK_WITH_STACKTRACE)
#include <cpptrace/cpptrace.hpp>
#endif

namespace aos::jack
{
class Service;

/******************************************************************************
 * \class Event
 * The base event type
 ******************************************************************************/
struct Event
{
    /**************************************************************************
     * Definitions
     **************************************************************************/
    enum Type
    {
        TIMER,
        CONTROL,
        ACTION,
        ACTIONCOMPLETE,
        MESSAGE,
        PERCEPT,
        PURSUE,
        DROP,
        SCHEDULE,
        DELEGATION,
        AUCTION,
        SHAREBELIEFSET,
        TACTIC,
        REGISTER,
    };

    enum Status
    {
        FAIL,
        PENDING,
        SUCCESS
    };

    /**************************************************************************
     * Functions
     **************************************************************************/
    Event(Type type) : type(type), status(Event::PENDING) {}

    virtual ~Event() {}

    void success()
    {
        if (!promise.expired()) {
            promise.lock()->resolve();
        }
        status = Event::SUCCESS;
    }

    void fail()
    {
        if (!promise.expired()) {
            promise.lock()->fail();
        }
        status = Event::FAIL;
    }

    void setReason(std::string&& msg)
    {
        reason = std::move(msg);
    }

    /**************************************************************************
     * Fields
     **************************************************************************/
    Type                   type;
    Status                 status;
    std::weak_ptr<Promise> promise; ///< a weak future

    // the routing information
    // These are 'weak' pointers
    // and should only be used by the owner of the agents (i.e. the engine)
    // who can ensure that they are still valid while routing the event.
    // the agent can individually set these if required

    Service*    caller    = nullptr;            ///< the caller is the agent the event is coming from
    Service*    recipient = nullptr;            ///< the recipient is the agent the event should be handled by
    UniqueId    eventId   = UniqueId::random(); ///< The event id - used for tracking events
    std::string reason;                         ///< (Optional): The reason the event was generated.

    /// \todo Temporarily compose the Event data structure with the protocol
    /// events. We are moving away from having 2 copies of the same data
    /// structures, the engine's internal representation and the protocol's
    /// representation.
    ///
    /// They both have different base classes which makes integration
    /// non-trivial, so we do composition of the types here, replace all the
    /// internal event usage code with protocol compatible usage code and slowly
    /// transition over until we can replace the base Event class in the engine.
    protocol::Control  controlEvent;
    protocol::Drop     dropEvent;
    protocol::Register registerEvent;
};
} // namespace aos::jack
#endif /// JACK_EVENT_H
