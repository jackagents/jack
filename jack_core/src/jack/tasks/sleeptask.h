// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_SLEEPTASK_H
#define JACK_SLEEPTASK_H

#include <jack/tasks/task.h>

#include <jack/events/timerevent.h>
#include <jack/engine.h>
#include <jack/corelib.h>

#include <string>
#include <vector>
#include <chrono>

namespace aos::jack
{
/*! ***********************************************************************************************
 * \class   SleepTask
 *
 * This task forces a task to delay/sleep for a configurable amount of time
 * ************************************************************************************************/
class SleepTask : public ClonableTask<SleepTask>
{
public:
    SleepTask(const SleepTask &other) : m_duration(other.m_duration) { m_wait = other.m_wait; }
    SleepTask(int ms) : m_duration(ms) {}

    Task::State execute([[maybe_unused]] BeliefContext& context,
                        const GoalHandle&               goal,
                        const UniqueId&                 intentionId,
                        std::string_view                plan) override final
    {
        // Register a timer with the agent, on completion trigger success
        auto clockMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_agent->engine().internalClock());
        m_promise    = m_agent->addEvent(JACK_ALLOCATOR_NEW(&m_agent->engine().m_eventAllocator,
                                                            TimerEvent,
                                                            clockMs,
                                                            m_duration,
                                                            goal,
                                                            intentionId,
                                                            m_id /*taskId*/,
                                                            plan));
        m_promise->then([this]() { succeed(); });

        return Task::WAIT;
    }

protected:
    std::chrono::milliseconds m_duration;
    PromisePtr m_promise;
};
} // namespace aos::jack
#endif // JACK_SLEEPTASK_H
