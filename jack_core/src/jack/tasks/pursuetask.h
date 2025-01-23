#ifndef JACK_PURSUETASK_H
#define JACK_PURSUETASK_H

#include <jack/promise.h>                   // for PromisePtr
#include <jack/tasks/task.h>                // for ClonableTask

/// Third Party
#include <string>
#include <string_view>

namespace aos::jack
{
class BeliefContext;

/******************************************************************************
 * \class   PursueTask
 * This task issues an pursue goal event to the agent
 ******************************************************************************/
class PursueTask : public ClonableTask<PursueTask>
{
public:
    PursueTask(const PursueTask &other) : m_goal(other.m_goal) { m_wait = other.m_wait; }

    PursueTask(std::string_view goal): m_goal(goal) { }

    Task::State execute(BeliefContext&    context,
                        const GoalHandle& goal,
                        const UniqueId&   intentionId,
                        std::string_view  plan) override final;

    const std::string &goal() const { return m_goal; }

protected:
    std::string m_goal;
    PromisePtr  m_promise;
};
} // namespace aos::jack
#endif // JACK_PURSUETASK_H
