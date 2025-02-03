// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_FUTURE_H
#define JACK_FUTURE_H

#include <jack/promise.h>

#include <memory>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * @class   Future
 *
 * @brief A Future represents a future asynchronous event. A Future can fulfil a promise by informing it
 * when the Future event happens
 *
 * ************************************************************************************************/
class Future
{
public:
    Future(const std::shared_ptr<Promise> &promise)
        : m_promise(promise)
    {
    }

    void success()
    {
        /// \todo This should actually send an event to trigger the promise
        /// if the promise is still available
        if (m_promise) { m_promise->resolve(); };
    }

    void fail()
    {
        /// \todo This should actually send an event to trigger the promise
        /// if the promise is still available
        if (m_promise) { m_promise->fail(); }
    }

private:

    /// \todo - this should be a weak_ptr
    /// \todo - or just an id and dispatch an event to be thread safe
    /// \todo - store a pointer to the event queue or agent
    std::shared_ptr<Promise> m_promise;
};

}} // namespace aos::jack

#endif
