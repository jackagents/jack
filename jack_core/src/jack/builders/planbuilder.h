#ifndef JACK_PLAN_BUILDER_H
#define JACK_PLAN_BUILDER_H

/// JACK
#include <jack/beliefquery.h>
#include <jack/builders/builder.h>
#include <jack/corelib.h>

/// Third Party
#include <functional>
#include <vector>
#include <string>
#include <string_view>

namespace aos::jack
{
class BeliefContext;
class Coroutine;
class Engine;
class GoalBuilder;
class CoroutineBuilder;
class ResourceBuilder;
class Plan;

/******************************************************************************
 * @class PlanBuilder
 * A plan builder class
 * Used to construct a plan within the jack core's declarative model
 ******************************************************************************/
class PlanBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// @param engine The engine instance
    /// @param name The name to assign the plan built by the builder
    PlanBuilder(Engine& engine, std::string_view name);

    /// @param other The plan builder to copy from
    PlanBuilder(const PlanBuilder &other);

    PlanBuilder& operator=(const PlanBuilder &other) = delete;

    ~PlanBuilder();

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Set the goal this plan can handle from a goal name.
    PlanBuilder& handles(std::string_view goal);

    /// Set the goal this plan can handle from a goal
    PlanBuilder& handles(const GoalBuilder& goal);

    /// Define a plan's precondition
    PlanBuilder& pre(const std::string& query);

    /// Define a plan's precondition
    PlanBuilder& pre(const std::function<bool(const BeliefContext&)> &func);

    /// Define a plan's drop condition
    PlanBuilder& dropWhen(const std::string& query);

    /// Define a condition when the plan should drop itself
    /// @param func The function to invoke to determine if the drop should occur
    PlanBuilder& dropWhen(const std::function<bool(const BeliefContext&)> func);

    /// Set the routine to execute when the mission body executes
    /// @param builder The coroutine builder to instantiate as the body routine
    PlanBuilder& body(const CoroutineBuilder& builder);

    /// Set the routine to execute when the mission is dropped
    /// @param builder The coroutine builder to instantiate as the drop routine
    PlanBuilder& onDrop(const CoroutineBuilder& builder);

    /// \todo Need better documentation
    /// Set the effects for this plan
    PlanBuilder& effects(const std::function<void(BeliefContext&)> func);

    /**************************************************************************
     * Lock Functions
     **************************************************************************/
    /// Add an array of resources that this plans locks upon execution
    PlanBuilder& locks(const Span<ResourceBuilder> items);

    /// Add an array of resources names that this plans locks upon execution
    PlanBuilder& lockNames(const Span<std::string_view> items);

    /// Add a resource that this plans locks upon execution
    PlanBuilder& lock(const ResourceBuilder& item) { return locks(Span<ResourceBuilder>(&item, 1)); }

    /// Add a resource by name that this plan locks upon execution
    PlanBuilder& lockName(std::string_view item) { return lockNames(Span<std::string_view>(&item, 1)); }

    /// Save the changes to the BDI engine
    template <typename PlanT = Plan>
    PlanBuilder& commit()
    {
        PlanT plan(m_name);
        commitInternal(&plan);
        return *this;
    }

private:
    /// Assign the properties to the plan passed in and commit the plan to the
    /// engine.
    Plan* commitInternal(Plan* plan);

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    std::string                         m_goal;
    Coroutine*                          m_body;
    Coroutine*                          m_dropCoroutine;
    BeliefQuery                         m_precondition;
    BeliefQuery                         m_dropWhen;
    std::function<void(BeliefContext&)> m_effects;
    std::vector<std::string>            m_locks;

};
} /// namespace aos::jack
#endif /// JACK_PLAN_BUILDER_H
