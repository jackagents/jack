#ifndef JACK_YIELDUNTIL_H
#define JACK_YIELDUNTIL_H

#include <jack/tasks/task.h>

#include <jack/beliefquery.h>

#include <string>
#include <vector>
#include <chrono>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * @class   YieldUntilTask
 *
 * @brief This task will yield the coroutine until the given condition is satisfied.
 * ************************************************************************************************/
class YieldUntilTask : public ClonableTask<YieldUntilTask>
{
public:
    YieldUntilTask(const YieldUntilTask &other)
        : m_query(other.m_query)
    {
    }

    YieldUntilTask(std::function<bool(const BeliefContext&)> func)
        : m_query(func)
    {
    }

    YieldUntilTask(const std::string &query)
        : m_query(query)
    {
    }

    Task::State execute(BeliefContext&                     context,
                        [[maybe_unused]] const GoalHandle& goal,
                        [[maybe_unused]] const UniqueId&   intentionId,
                        [[maybe_unused]] std::string_view  plan) override
    {
        return m_query(context) ? Task::DONE : Task::YIELD;
    }

    Task::State resume(BeliefContext&                     context,
                       [[maybe_unused]] const GoalHandle& goal,
                       [[maybe_unused]] const UniqueId&   intentionId,
                       [[maybe_unused]] std::string_view  plan) override
    {
        // will keep trying until this func returns true
        return m_query(context) ? Task::DONE : Task::YIELD;
    }

  protected:

    BeliefQuery m_query;
};

}} // namespace aos::jack

#endif // JACK_YIELDUNTIL_H
