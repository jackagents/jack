#ifndef JACK_TACTIC_H
#define JACK_TACTIC_H

#include <jack/engine/uuid.h>    // for UUID
#include <jack/handles.h>        // for TacticHandle

#include <string>
#include <vector>

namespace aos::jack
{
/*! ***********************************************************************************************
 * @class Tactic
 *
 * @brief The Tactic Entity. A tactic is a plan filter.
 * ************************************************************************************************/
class Tactic
{
public:
    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/
    /// Get a lightweight non-owning handle referencing this tactic.
    const TacticHandle& handle() const { return m_handle; }

    const std::string& name() const { return m_handle.m_name; }

    /// Get the goal that this tactic supports
    const std::string& goal() const { return m_goal; }

    enum class RemoveMode
    {
        FirstInstance, ///< Remove the first instance of the matched plan encountered
        All,           ///< Remove all encountered matching plan
    };

    /// Remove the plan from the tactic
    /// @param plan The plan to remove
    void removePlan(std::string_view plan, RemoveMode mode);

    /// Check if a plan is supported by this tactic
    /// @param plan The plan to be checked
    bool planAllowed(std::string_view plan) const;

    /// The list of plans supported by this tactic
    const std::vector<std::string>& plans() const { return m_plans; }

    /// The list of plans supported by this tactic
    std::vector<std::string>& plans() { return m_plans; }

    /// The amount of times the plan list is allowd to be looped when executing
    /// before failing the goal.
    uint32_t loopPlansCount() const { return m_loopPlansCount; }

    /// When true this tactic uses the plan list specified in the tactic,
    /// otherwise all plans valid for the goal will be considered.
    bool isUsingPlanList() const { return m_usePlanList; }

    enum class PlanOrder
    {
        /// Each attempt, the best plan is chosen. If the goal is unsatisfied,
        /// the plan is excluded and the next best plan is considered.
        ExcludePlanAfterAttempt,

        /// The plans attempted must follow the order of the plans in the plan
        /// list. Repeated plans are permitted
        Strict,

        /// Only the best plan is considered each attempt. The same plan can be
        /// attempted multiple times if it is the best plan.
        ChooseBestPlan,
    };

    /// An enum that describes how the plan list is evaluated to achieve the
    /// goal when an agent uses this tactic.
    PlanOrder planOrder() const { return m_planOrder; }

    std::string print() const;

    static std::string_view planOrderPrint(PlanOrder order);

    static std::string_view planOrderHumanPrint(PlanOrder order);

    /// Magic value for requesting that a tactic will loop the plan selection
    /// list infinitely. This value must be assigned on the tactic's
    /// 'm_loopPlansCount' variable to take effect.
    static constexpr inline uint32_t LOOP_PLANS_INFINITELY = static_cast<uint32_t>(-1);

    friend class TacticBuilder;

    friend class Agent;

protected:
    /* ****************************************************************************************
     * Attributes
     * ****************************************************************************************/
    /// The goal that this tactic is relevant for
    std::string m_goal;

    /// List of plans supported by this tactic.
    ///
    /// This list can have repeated plans if the plan list is fixed. This
    /// indicates that the tactic enforces a strict ordering of the plans to be
    /// considered during execution.
    ///
    /// If the plan list is *not* fixed, this list will be filtered to remove
    /// duplicates before it is committed to the engine.
    std::vector<std::string> m_plans;

    /// The light weight handle for this tactic
    TacticHandle m_handle;

    uint32_t  m_loopPlansCount = 0;     ///< @copybrief Tactic::loopPlansCount
    bool      m_usePlanList    = false; ///< @copybrief Tactic::isUsingPlanList
    PlanOrder m_planOrder      = {};    ///< @copybrief Tactic::planOrder
};
} /// namespace aos::jack
#endif
