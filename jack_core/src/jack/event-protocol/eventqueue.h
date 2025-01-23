#ifndef JACK_EVENT_QUEUE_H
#define JACK_EVENT_QUEUE_H

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4127) // event-protocol\src\concurrentqueue.h|788| warning C4127: conditional expression is constant
#endif /// _MSC_VER

#include <concurrentqueue.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif /// _MSC_VER

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * @class EventQueue
 *
 * Implementa a lock free queue for JACK events
 *
 * @tparam T The queue type
 *
 * ************************************************************************************************/

template<typename T>
class EventQueue
{
public:

    /// Construct the queue
    EventQueue() : m_size(0) {}

    /// Enqueue an item into the queue
    /// @param The item to copy into the queue
    inline bool enqueue(T const& item)
    {
        if (m_queue.enqueue(item)) {
            m_size.fetch_add(1, std::memory_order_release);
            return true;
        } else {
            return false;
        }
    }

    /// Enqueue an item into the queue
    /// @param The item to move into the queue
    inline bool enqueue(T&& item)
    {
        if (m_queue.enqueue(item)) {
            m_size.fetch_add(1, std::memory_order_release);
            return true;
        } else {
            return false;
        }
    }

    /// Remove the next item from the queue
    /// @param item The item returned from the queue
    bool dequeue(T& item)
    {   bool result = m_queue.try_dequeue(item);
        if(result) {
            m_size.fetch_sub(1, std::memory_order_release);
        }
        return result;
    }

    /// Remove several items from the queue
    /// @param array The buffer for dequeing items into
    /// @param arraySize The buffer for dequeing items maximum capacity
    /// @return The number of items dequeued
    size_t dequeueBulk(T* array, size_t arraySize)
    {   size_t result = array ? m_queue.try_dequeue_bulk(array, arraySize) : 0;
        if (result) {
            m_size.fetch_sub(result, std::memory_order_release);
        }
        return result;
    }

    /// Clear the queue
    void clear() {
        T item;
        while(m_queue.try_dequeue(item)) {
            m_size.fetch_sub(1, std::memory_order_release);
        };
    }

    /// @return Is this queue empty
    bool empty() const {
        return size() == 0; // its a uint
    }

    // Size from our atomic
    uint64_t size() const {
        return m_size.load(std::memory_order_acquire);
    }

    // Size from what concurrentqueue thinks it MIGHT be
    uint64_t size_approx() const {
        return m_queue.size_approx();
    }

private:
    moodycamel::ConcurrentQueue<T> m_queue;

    // track the number of events in the queue
    std::atomic<uint64_t> m_size;
};

}} // namespace aos::jack

#endif
