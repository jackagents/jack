// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/tasks/task.h>

#include <jack/plan.h>
#include <jack/agent.h>

namespace aos { namespace jack {

Engine &Task::engine()
{
    return m_agent->engine();
}

void Task::reset(Agent* agent, Plan *intention)
{
    m_agent = agent;
    m_intention = intention;
    m_state = READY;
}

 void Task::succeed()
 {
    if (m_state == ASYNC)
    {
        m_intention->body()->markAsyncDone();
    }

    m_state = DONE; m_status = SUCCEEDED;
}

void Task::fail()
{
    if (m_state == ASYNC)
    {
        m_intention->body()->markAsyncDone();
    }

    m_state = DONE; m_status = FAILED;
}

bool Task::tick(BeliefContext& context,
                const GoalHandle& desire,
                const UniqueId& intentionId,
                std::string_view plan) {

    if (m_state == READY) {
        m_state = execute(context, desire, intentionId, plan);
    } else if (m_state == YIELD) {
        m_state = resume(context, desire, intentionId, plan);
    }
    else {
        /// If 'm_state == WAIT' then the task will only transition out of
        /// waiting when succeed() or fail() is called on the task, i.e.
        /// a PursueTask will call the completion hooks when the pursued goal is
        /// complete allowing the coroutine to advance to the next task.
    }

    return m_state == DONE || m_state == ASYNC;
}
}} // namespace aos::jack
