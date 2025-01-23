#include <jack/builders/tacticbuilder.h>

#include <jack/engine.h>            // for Engine
#include <jack/builders/builder.h>  // for Builder

namespace aos::jack
{
static void initCommonValues(TacticBuilder &builder)
{
    builder.planOrder(Tactic::PlanOrder::ExcludePlanAfterAttempt);
    builder.loopPlansCount(1);
}

TacticBuilder::TacticBuilder(Engine& engine, std::string_view name)
    : Builder(engine, name)
{
    m_tactic.m_handle.m_name = name;
    m_tactic.m_handle.m_id   = UniqueId::random();
    initCommonValues(*this);
}

TacticBuilder::TacticBuilder(Engine& engine, const Tactic& tactic)
    : Builder(engine, tactic.handle().m_name),
      m_tactic(tactic)
{
    initCommonValues(*this);
}

TacticBuilder& TacticBuilder::plans(const Span<PlanBuilder> items)
{
    m_tactic.m_plans.reserve(m_tactic.m_plans.size() + items.size());
    for (const PlanBuilder& item : items) {
        m_tactic.m_plans.push_back(item.name());
    }
    usePlanList(true);
    return *this;
}

TacticBuilder& TacticBuilder::planNames(const Span<std::string_view> items)
{
    m_tactic.m_plans.insert(m_tactic.m_plans.end(), items.begin(), items.end());
    usePlanList(true);
    return *this;
}

TacticHandle TacticBuilder::commit()
{
    JACK_ASSERT(m_tactic.m_handle.m_name.size());
    m_commitError = !m_engine.commitTactic(&m_tactic);

    TacticHandle result = m_tactic.m_handle;

    /// \note Update the tactic handle to ensure that the next commit creates a
    /// new tactic and doesn't overwrite the previous one.
    m_tactic.m_handle.m_id = UniqueId::random();
    return result;
}
} // namespace aos::jack
