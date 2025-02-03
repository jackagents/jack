// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/builders/goalbuilder.h>

#include <jack/engine.h>

namespace aos::jack
{
GoalBuilder::GoalBuilder(Engine& engine, std::string_view name)
    : Builder(engine, name)
{}

GoalBuilder::GoalBuilder(const GoalBuilder &other)
    : Builder(other.m_engine, other.m_name)
    , m_precondition(other.m_precondition)
    , m_satisfied(other.m_satisfied)
    , m_dropWhen(other.m_dropWhen)
    , m_priority(other.m_priority)
    , m_heuristic(other.m_heuristic)
{
}

GoalBuilder& GoalBuilder::pre(const std::function<bool(const BeliefContext&)> func)
{
    m_precondition = func;
    return *this;
}

GoalBuilder& GoalBuilder::pre(const std::string &query)
{
    m_precondition = BeliefQuery(query);
    return *this;
}

GoalBuilder& GoalBuilder::satisfied(const std::function<bool(const BeliefContext&)> func)
{
    m_satisfied = BeliefQuery(func);
    return *this;
}

GoalBuilder& GoalBuilder::satisfied(const std::string &query)
{
    m_satisfied = BeliefQuery(query);
    return *this;
}

GoalBuilder& GoalBuilder::dropWhen(const std::function<bool(const BeliefContext&)> func)
{
    m_dropWhen = func;
    return *this;
}

GoalBuilder& GoalBuilder::priority(int priority)
{
    m_priority = priority;
    return *this;
}

GoalBuilder& GoalBuilder::heuristic(const std::function<float(const BeliefContext&)> func)
{
    m_heuristic = func;
    return *this;
}

GoalBuilder& GoalBuilder::message(std::string_view message)
{
    m_messageSchema = message;
    return *this;
}

Goal* GoalBuilder::commitInternal(Goal* goal)
{
    Goal* result = nullptr;
    if (goal) {
        goal->setMessageSchema(m_messageSchema);
        goal->setPrecondition(m_precondition);
        goal->setSatisfied(m_satisfied);
        goal->setDropWhen(m_dropWhen);
        goal->setPriority(m_priority);
        goal->setHeuristic(m_heuristic);
        result = m_engine.commitGoal(goal);
    }
    return result;
}
} // namespace aos::jack
