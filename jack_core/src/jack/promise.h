// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_PROMISE_H
#define JACK_PROMISE_H

#include <jack/engine/uuid.h>

#include <functional>
#include <memory>

namespace aos { namespace jack {

class Future;
struct Event;

/*! ***********************************************************************************************
 * @class   Promise
 *
 * @brief A Promise allows a caller to listen for a future asynchronous event
 * e.g. the result of a pursue(Goal)
 *
 * NOTE: This is not thread safe. The then callbacks are called from the engine/agent threads currently
 *
 * ************************************************************************************************/
class Promise
{
public:
    /// Construct a Promise from a UniqueId
    Promise(UniqueId id)
        : m_eventId(id)
    {}

    /// Specify a callback for when the event is finished
    void then(std::function<void()> func)
    {
        m_then = func;
    }

    /// Specify seperate callbacks for success and fail for when the event is finished
    void then(std::function<void()> func, std::function<void()> failfunc)
    {
        m_then = func;
        m_fail = failfunc;
    }

    /// Return the UniqueId of the Event that is linked to this Promise
    UniqueId eventId() const
    {
        return m_eventId;
    }

protected:

    /// Called by the Future when the associated Event is successful
    void resolve()
    {
        if (m_then) m_then();
    }

    /// Called by the Future when the associated Event fails
    void fail()
    {
        if (m_fail) {
            m_fail();
        } else if (m_then) {
            m_then();
        }
    }

    std::function<void()> m_then;
    std::function<void()> m_fail;

    friend Future;
    friend Event;

    UniqueId m_eventId;
};

typedef std::shared_ptr<Promise> PromisePtr;

}} // namespace aos::jack

#endif
