#ifndef JACK_TACTIC_BUILDER_H
#define JACK_TACTIC_BUILDER_H

#include <jack/builders/builder.h>
#include <jack/tactic.h>
#include <jack/corelib.h>           // for Span

#include <string>
#include <string_view>
#include <vector>

namespace aos::jack
{
class Engine;
class Tactic;
class PlanBuilder;

/******************************************************************************
 * @class TacticBuilder
 * @brief A helper class for configuring a tactic in the JACK model.
 *
 * A tactic customises the behaviour that an agent adopts when trying to achieve
 * a goal by restricting the permitted plans and the manner in which those plans
 * are considered and executed.
 ******************************************************************************/
class TacticBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// @param engine The JACK engine instance
    /// @param name The name of this Team template
    TacticBuilder(Engine& engine, std::string_view name);

    /// Create a builder from a copy of the tactic
    TacticBuilder(Engine& engine, const Tactic& tactic);

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Add a goal that this tactic can use for handling the goal
    TacticBuilder& goal(std::string_view goal) { m_tactic.m_goal = goal; return *this; }

    /**************************************************************************
     * Plan Functions
     **************************************************************************/
    /// Add a plan that this tactic can use for handling the goal
    ///
    /// The order of the plans in the tactic dictates the plan execution order
    /// if this tactic's plan order is set to the strict policy.
    ///
    /// By setting a plan this tactic will automatically set the flag for this
    /// tactic that indicates it uses an explicit plan list equivalent, to
    /// calling 'usePlanList(true)'.
    TacticBuilder& plans(const Span<PlanBuilder> item);

    /// @copydoc plans
    TacticBuilder& planNames(const Span<std::string_view> item);

    /// Add a plan by name to this tactic
    ///
    /// @see TacticBuilder::plans
    TacticBuilder& plan(const PlanBuilder& item);

    /// @copydoc TacticBuilder::plan
    TacticBuilder& planName(std::string_view item) { return planNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Tactic Policies
     **************************************************************************/
    /// @copybrief Tactic::loopPlansCount
    TacticBuilder& loopPlansInfinitely() { m_tactic.m_loopPlansCount = Tactic::LOOP_PLANS_INFINITELY; return *this; }

    /// @copybrief Tactic::loopPlansCount
    TacticBuilder& loopPlansCount(uint32_t max) { m_tactic.m_loopPlansCount = max; return *this; }

    /// @copybrief Tactic::m_usingPlanList
    ///
    /// This API does not normally need to be called as adding a plan will
    /// automatically transition the tactic to use the explicit plan list.
    ///
    /// However if you wish to make a tactic with no plans, this method can be
    /// invoked on an empty tactic to enforce no plans permitted for the agent
    /// adopting this tactic.
    TacticBuilder& usePlanList(bool use) { m_tactic.m_usePlanList = use; return *this; }

    /// @copybrief Tactic::m_planOrder
    TacticBuilder& planOrder(Tactic::PlanOrder order) { m_tactic.m_planOrder = order; return *this; }

    /// The list of plans supported by the tactic
    const std::vector<std::string>& planList() const { return m_tactic.m_plans; }

    /// @copydoc TacticBuilder::planList
    std::vector<std::string>& planList() { return m_tactic.m_plans; }

    const Tactic& tactic() { return m_tactic; }

    /// @todo: All the other builders return the builder, tactics return
    /// the handle. All other builders should also return a handle as the
    /// default.
    ///
    /// It gets extra difficult with goals and agents where you have the
    /// template and you spawn runtime instances from it whereas tactics don't
    /// need to spawn a tactic instance from the template because it's globally
    /// usable across all agents, no runtime data is stored here.
    ///
    /// So we may need distinct handles types for templates to allow the user to
    /// create the item from the engine with the distinct handle *and* also have
    /// a handle for managing runtime instances.
    ///
    /// For now I've implemented a tactic with a handle as an experiment but
    /// this needs to be unified.

    /// Commit this tactic template into the JACK model
    ///
    /// All plans that are committed for this tactic must handle the same goal
    /// otherwise commit will not add the tactic to the BDI model and an error
    /// will be raised.
    ///
    /// Consequently, all plans that are referenced in this tactic must exist
    /// JACK model before commit to succeed.
    TacticHandle commit();

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    /// Set to true when there is an error on commit
    bool   m_commitError = false;

    /// The tactic to construct from the builder
    Tactic m_tactic      = {};
};
} // namespace aos::jack
#endif
