#ifndef JACK_ROLE_BUILDER_H
#define JACK_ROLE_BUILDER_H

/// JACK
#include <jack/role.h>             // for RoleBeliefSet, Role, RoleBeliefSet::Re...
#include <jack/builders/builder.h>
#include <jack/corelib.h>

/// Third Party
#include <vector>
#include <string>

namespace aos::jack
{
class Engine;
class GoalBuilder;
class MessageBuilderBase;

/******************************************************************************
 * @class   RoleBuilder
 *
 * A helper class for configuring a Role template
 *
 * A Role is an interface between an agent and a team and defines
 * how the two will interact
 *
 * Example:
 * @code
 *  bdi.role("AgentRole")
 *     .goals({roleGoal})
 *     .beliefs({roleBeliefs})
 *     .commit();
 * @endcode
 ******************************************************************************/
class RoleBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// Constructs an unconfigured RoleBuilder with a name
    /// @param engine The JACK engine instance
    /// @param name The name of this Role template
    RoleBuilder(Engine& engine, std::string_view name);

    /**************************************************************************
     * Goal Functions
     **************************************************************************/
    /// Add an array of goals supported by this role
    RoleBuilder& goals(const Span<GoalBuilder> items);

    /// Add an array of goal names supported by this role
    RoleBuilder& goalNames(const Span<std::string_view> items);

    /// Add a goal to the role
    RoleBuilder& goal(const GoalBuilder& item) { return goals(Span<GoalBuilder>(&item, 1)); }

    /// Add a goal by name to this role
    RoleBuilder& goalName(std::string_view item) { return goalNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Add a belief set for this Role
    /// The roles beliefs created when the roles is being performed
    RoleBuilder& beliefs(std::string_view name, RoleBeliefSet::ReadAccess read, RoleBeliefSet::WriteAccess write);

    /// Commit this Role into the JACK engine
    RoleBuilder& commit();

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    Role m_role;
};
} // namespace aos::jack
#endif /// JACK_ROLE_BUILDER_H
