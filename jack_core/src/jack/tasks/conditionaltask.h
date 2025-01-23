#ifndef JACK_CONDITIONAL_H
#define JACK_CONDITIONAL_H

#include <jack/tasks/task.h>

#include <jack/beliefquery.h>
#include <string>

namespace aos::jack
{
/*! ***********************************************************************************************
 * @class   ConditionalTask
 *
 * @brief This task will branch the coroutine based on the given condition
 * ************************************************************************************************/
class ConditionalTask : public ClonableTask<ConditionalTask>
{
public:
    ConditionalTask(const ConditionalTask &other);
    ConditionalTask(std::function<bool(const BeliefContext&)> func);
    Task::State execute(BeliefContext&    context,
                        const GoalHandle& goal,
                        const UniqueId&   intentionId,
                        std::string_view  plan) override final;

protected:
    BeliefQuery m_query;
};
} // namespace aos::jack
#endif // JACK_CONDITIONAL_H
