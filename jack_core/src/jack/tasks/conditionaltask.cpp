// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/tasks/conditionaltask.h>

#include <jack/agent.h>
#include <jack/engine.h>
#include <jack/handles.h> /// GoalHandle
#include <jack/event-protocol/busadapter.h> /// BusAdapter

namespace aos::jack
{
ConditionalTask::ConditionalTask(const ConditionalTask &other)
    : m_query(other.m_query)
{
}

ConditionalTask::ConditionalTask(std::function<bool(const BeliefContext&)> func)
    : m_query(func)
{
}

Task::State ConditionalTask::execute(BeliefContext&    context,
                                     const GoalHandle& goal,
                                     const UniqueId&   intentionId,
                                     std::string_view  plan)
{
    /// \todo When we have the reasoning engine(BQL) working this might be async
    /// and require a resume. For now we can just assume sync
    bool result = m_query(context);

    jack::Engine& engine = m_agent->engine();
    if (engine.haveBusAdapter()) {
        protocol::BDILogHeader header = m_agent->bdiLogHeader(protocol::BDILogLevel_NORMAL);
        protocol::BDILog logEvent = engine.makeBDILogConditionResult(header,
                                                                     goal.m_name,                                 /// goal
                                                                     goal.m_id.toString(),                           /// goalId
                                                                     intentionId.toString(),                         /// intentionId
                                                                     plan,                                        /// plan
                                                                     m_id.toString(),                                /// taskId
                                                                     "TODO: Condition string needs to go here"sv, /// condition
                                                                     result);                                     /// success
        engine.sendBusEvent(&logEvent);
    }

    m_status = result ? Task::SUCCEEDED : Task::FAILED;
    return Task::DONE;
}
} // namespace aos::jack
