#include "tactic.h"

namespace aos::jack
{
void Tactic::removePlan(std::string_view plan, RemoveMode mode)
{
    for (auto it = m_plans.begin(); it != m_plans.end(); it++) {
        if (*it == plan) {
            it = m_plans.erase(it);
            if (mode == RemoveMode::FirstInstance) {
                break;
            }
        } else {
            it++;
        }
    }
}

bool Tactic::planAllowed(std::string_view plan) const
{
    if (!isUsingPlanList()) {
        return true;
    }

    bool result = false;
    for (const std::string& check : m_plans) {
        if (plan == check) {
            result = true;
            break;
        }
    }

    return result;
}

std::string_view Tactic::planOrderPrint(PlanOrder order)
{
    switch (order) {
        case Tactic::PlanOrder::ExcludePlanAfterAttempt: return "ExcludePlanAfterAttempt";
        case Tactic::PlanOrder::Strict:                  return "Strict";
        case Tactic::PlanOrder::ChooseBestPlan:          return "ChooseBestPlan";
    }
    return "TACTIC_PLAN_ORDER_BAD_ENUM_VALUE"sv;
}

std::string_view Tactic::planOrderHumanPrint(PlanOrder order)
{
    switch (order) {
        case Tactic::PlanOrder::ExcludePlanAfterAttempt: return "Exclude Plan After Attempt";
        case Tactic::PlanOrder::Strict:                  return "Strict";
        case Tactic::PlanOrder::ChooseBestPlan:          return "Choose Best Plan";
    }
    return "TACTIC_PLAN_ORDER_BAD_ENUM_VALUE"sv;
}

std::string Tactic::print() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("Tactic{{goal={}, plans={{"), m_goal);

    for (size_t index = 0; index < m_plans.size(); index++) {
        if (index) {
            builder.appendRef(", ");
        }
        builder.appendCopy(m_plans[index]);
    }

    builder.append(FMT_STRING("}}, handle={}, loopPlansCount={}, usePlanList={}, planOrder='{}'}}"),
                   m_handle.toString(),
                   m_loopPlansCount,
                   (m_usePlanList ? "true" : "false"),
                   Tactic::planOrderHumanPrint(m_planOrder));

    std::string result = builder.toString();
    return result;
}
} // namespace aos::jack
