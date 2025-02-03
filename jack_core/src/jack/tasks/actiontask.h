// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_ACTIONTASK_H
#define JACK_ACTIONTASK_H

#include <jack/tasks/task.h>

/// Third Party
#include <string_view>
#include <string>

namespace aos::jack
{
/******************************************************************************
 * \class   ActionTask
 * This task issues an action message back to the agent
 ******************************************************************************/
class ActionTask : public ClonableTask<ActionTask>
{
public:
    /// @param action The name of the action task
    ActionTask(std::string_view action);

    /// @param other The action task to copy from
    ActionTask(const ActionTask &other);

    ActionTask &operator=(const ActionTask &other) = delete;

    Task::State execute(BeliefContext& context,
                        const GoalHandle& goal,
                        const UniqueId& intentionId,
                        std::string_view plan) override;

    const std::string& action() const { return m_action; }
protected:
    std::string m_action;
};
} // namespace aos::jack
#endif // JACK_ACTIONTASK_H
