// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_DROPTASK_H
#define JACK_DROPTASK_H

#include <jack/agent.h>
#include <jack/tasks/task.h>

#include <string>
#include <string_view>

namespace aos::jack
{
/*!
 * ***********************************************************************************************
 * \class   DropTask
 *
 * This task is forcing the given goal to be removed from the current agent
 * active goals
 * ************************************************************************************************/
class DropTask : public ClonableTask<DropTask>
{
public:
    DropTask(const DropTask &other)
    : m_goal(other.m_goal)
    {
        m_wait = other.m_wait;
    }

    DropTask(const GoalHandle &goal)
    : m_goal(goal)
    {
    }

    Task::State execute([[maybe_unused]] BeliefContext&    context,
                        [[maybe_unused]] const GoalHandle& goal,
                        [[maybe_unused]] const UniqueId&   intentionId,
                        [[maybe_unused]] std::string_view  plan) override final
    {

        const jack::AgentHandle &agent = m_agent->handle();

        /// \note Construct human friendly string on why the goal was dropped.
        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        StringBuilder builder          = StringBuilder(scratch.arena);
        builder.append(FMT_STRING("{} is being dropped on {} because another goal requested a drop in its plan. The goal that requested this drop was {} executing the plan {}"),
                       m_goal.toHumanString(),
                       agent.toHumanString(),
                       goal.toHumanString(),
                       plan);

        m_agent->dropWithMode(m_goal, protocol::DropMode_NORMAL, builder.toString());
        return Task::DONE; /// \todo This should wait for the drop
    }

protected:
    GoalHandle m_goal = {};
};
} // namespace aos::jack
#endif // JACK_DROPTASK_H
