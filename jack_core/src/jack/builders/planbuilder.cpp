#include <jack/builders/planbuilder.h>
#include <jack/builders/coroutinebuilder.h>
#include <jack/builders/goalbuilder.h>
#include <jack/builders/resourcebuilder.h>
#include <jack/engine.h>
#include <jack/corelib.h>
#include <jack/plan.h>

namespace aos::jack
{
PlanBuilder::PlanBuilder(Engine& engine, std::string_view name)
    : Builder(engine, name)
    , m_body(nullptr)
    , m_dropCoroutine(nullptr)
{
}

PlanBuilder::PlanBuilder(const PlanBuilder& other)
    : Builder(other.m_engine, other.m_name)
    , m_goal(other.m_goal)
    , m_body(other.m_body ? JACK_NEW(Coroutine, *other.m_body) : nullptr)
    , m_dropCoroutine(other.m_dropCoroutine ? JACK_NEW(Coroutine, *other.m_dropCoroutine) : nullptr)
    , m_precondition(other.m_precondition)
    , m_dropWhen(other.m_dropWhen)
{
}

PlanBuilder::~PlanBuilder()
{
    if (m_body) {
        JACK_DELETE(m_body);
    }

    if (m_dropCoroutine) {
        JACK_DELETE(m_dropCoroutine);
    }
}

PlanBuilder& PlanBuilder::handles(std::string_view goal)
{
    m_goal = goal;
    return *this;
}

PlanBuilder& PlanBuilder::handles(const GoalBuilder& goal)
{
    m_goal = goal.name();
    return *this;
}

PlanBuilder& PlanBuilder::pre(const std::string& query)
{
    m_precondition = BeliefQuery(query);
    return *this;
}

PlanBuilder& PlanBuilder::pre(const std::function<bool(const BeliefContext&)>& func)
{
    m_precondition = BeliefQuery(func);
    return *this;
}

PlanBuilder& PlanBuilder::dropWhen(const std::string &query)
{
    m_dropWhen = BeliefQuery(query);
    return *this;
}

PlanBuilder& PlanBuilder::dropWhen(const std::function<bool(const BeliefContext&)> func)
{
    m_dropWhen = func;
    return *this;
}

PlanBuilder& PlanBuilder::body(const CoroutineBuilder& builder)
{
    m_body = builder.create();
    return *this;
}

PlanBuilder& PlanBuilder::onDrop(const CoroutineBuilder& builder)
{
    m_dropCoroutine = builder.create();
    return *this;
}

PlanBuilder& PlanBuilder::effects(const std::function<void(BeliefContext&)> func)
{
    m_effects = func;
    return *this;
}

/******************************************************************************
 * Lock Functions
 ******************************************************************************/
PlanBuilder& PlanBuilder::locks(const Span<ResourceBuilder> items)
{
    m_locks.reserve(m_locks.size() + items.size());
    for (const ResourceBuilder& item : items) {
        m_locks.push_back(item.name());
    }
    return *this;
}

PlanBuilder& PlanBuilder::lockNames(const Span<std::string_view> items)
{
    m_locks.insert(m_locks.end(), items.begin(), items.end());
    return *this;
}

Plan* PlanBuilder::commitInternal(Plan* plan)
{
    Plan* result = nullptr;
    if (plan) {
        /// @todo: Validation should move into the commit function in the
        /// engine as that's the catch-all step.
        if (m_goal.empty()) {
            JACK_WARNING("Plan added to engine does not handle a goal, it is invalid and can't be used. [plan='{}']", m_name);
        }
        plan->setGoal(m_goal);
        plan->setBody(m_body);
        plan->setDropCoroutine(m_dropCoroutine);
        plan->setPrecondition(m_precondition);
        plan->setDropWhen(m_dropWhen);
        plan->setEffects(m_effects);
        plan->setResourceLocks(m_locks);
        result = m_engine.commitPlan(plan);
    }
    return result;
}
} // namespace aos::jack
