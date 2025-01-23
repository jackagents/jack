#include <jack/goal.h>
#include <jack/event-protocol/busadapter.h>
#include <jack/engine.h>

#include <tracy/Tracy.hpp>
#include <cassert>               // for assert
#include <string>                // for char_traits

/// Third Party
#include <fmt/core.h>

namespace aos::jack
{
bool Goal::isValid(const BeliefContext& context) const
{
    assert(m_engine);

    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    bool result = m_precondition.isValid() ? m_precondition(context) : true;
    return result;
}

bool Goal::isSatisfied(const BeliefContext& context) const
{
    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    bool result = m_satisfied.isValid() ? m_satisfied(context) : false;
    return result;
}

bool Goal::shouldDrop() const
{
    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    bool result = m_dropWhen.isValid() ? m_dropWhen(m_context) : false;
    return result;
}

float Goal::heuristic(const BeliefContext& context) const
{
    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    float32 result = {};
    if (m_heuristic) {
        result = m_heuristic(context);
    }
    return result;
}
void Goal::toString() const
{
    fmt::println("Goal: {}", m_handle.m_name);
    if (const std::shared_ptr<Message>& msg = context().goal()) {
        fmt::println("Goal message: {}", msg->toString());
    }
}

void Goal::finish(FinishState finishState)
{
    JACK_ASSERT_MSG(finishState != FinishState::NOT_YET,
                    "Setting a goal to not finished is most likely a bug. The "
                    "goal stores state that would need to be reversed if we "
                    "can set it back to *not* finished.");

    /// \todo Move the double invocation check to the promise to catch all
    /// cases. Making this change will break many tests and it seems like we're
    /// doing this liberally.
    JACK_ASSERT_MSG(
        m_finishState == FinishState::NOT_YET,
        "Success should not be called on an already failed/success goal "
        "otherwise the failed/success callback triggers twice");

    m_finishState = finishState;
    if (m_future) {
        if (m_finishState == FinishState::SUCCESS) {
            m_future->success();
        } else {
            m_future->fail();
        }
    }
}

Goal::PlanHistory* Goal::PlanSelection::findHistory(std::string_view plan)
{
    Goal::PlanHistory *result = nullptr;
    for (size_t resultIndex = 0;
         !result && resultIndex < m_history.size();
         resultIndex++)
    {
        Goal::PlanHistory& entry = m_history[resultIndex];
        result                   = plan == entry.plan ? &entry : nullptr;
    }

    return result;
}

Goal::PlanHistory& Goal::PlanSelection::findOrMakeHistory(std::string_view plan)
{
    Goal::PlanHistory *result = findHistory(plan);
    if (!result) {
        m_history.push_back({});
        result       = &m_history.back();
        result->plan = plan;
    }
    return *result;
}

std::string Goal::PlanSelection::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder          builder = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("PlanSelection{{tactic={}, planLoopIteration={}, planListIndex={}, history={{"),
                   m_tactic,
                   m_planLoopIteration,
                   m_planListIndex);

    if (m_history.size()) {
        builder.appendRef("\n");
        for (size_t index = 0; index < m_history.size(); index++) {
            const aos::jack::Goal::PlanHistory& history = m_history[index];
            if (index) {
                builder.appendRef(",\n");
            }
            builder.append(FMT_STRING(" {}"), history.toString());
        }
        builder.appendRef("\n");
    }
    builder.appendRef("}");
    std::string result = builder.toString();
    return result;
}

std::string Goal::PlanHistory::toString() const
{
    std::string result = JACK_FMT("PlanHistory{{plan={}, successCount={}, failCount={}, lastLoopIteration={}}}",
                                  plan,
                                  successCount,
                                  failCount,
                                  lastLoopIteration);
    return result;
}
} /// namespace aos::jack
