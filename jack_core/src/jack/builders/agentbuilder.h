#ifndef JACK_AGENT_BUILDER_H
#define JACK_AGENT_BUILDER_H

/// JACK
#include <jack/agent.h>             // for Agent
#include <jack/builders/builder.h>  // for Builder
#include <jack/message.h>           // For Message::Status
#include <jack/handles.h>
#include <jack/corelib.h>           // for Span

/// Third Party
#include <functional>          // for function
#include <map>                 // for map, map<>::mapped_type
#include <string>              // for string, operator<
#include <string_view>
#include <vector>

namespace aos::jack
{
class Engine;
class ServiceBuilder;
class RoleBuilder;
class PlanBuilder;
class GoalBuilder;
class TacticBuilder;
class ResourceBuilder;

/******************************************************************************
 * @brief A helper class for configuring an agent template and
 * creating an agent instance
 *
 * An agent builder is requested from the engine instance by calling the agent
 * member with the template name
 *
 * @see Engine
 * @code
 * bdi.agent("WorkTeam")
 * @endcode
 *
 * Once constructed the builder can be used to configure the Agent template
 *
 * Builders support a fluent style API for easier configuration
 *
 * Example:
 * @code
 * bdi.agent("Agent")
 *    .plans({agentPlan})
 *    .beliefsName("messageName")
 *    .handleAction("PerformAction", actionHandler)
 *    .commit();
 * @endcode
 ******************************************************************************/
class AgentBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// Constructs an unconfigured AgentBuilder with a name
    /// @param engine The JACK engine instance
    /// @param name The name of this Agent template
    AgentBuilder(Engine& engine, std::string_view name);

    /// Constructs a AgentBuilder for an existing Agent
    /// @param engine The JACK engine instance
    /// @param agent The name of the Agent
    /// \todo add builder unit test to make sure we can re-confige an Agent template
    AgentBuilder(Engine& engine, const Agent& agent);

    /// Add a handler for an action
    /// @param action The type of the action
    /// @param func The handler of the action
    /// @return This AgentBuilder
    AgentBuilder& handleAction(const std::string& action, Agent::ActionHandlerFunc func);

    /// Add a handler for an incoming message
    /// @param msg The type of the message
    /// @param func The handler of the message
    /// @return This AgentBuilder
    AgentBuilder& handleMessage(const std::string& msg, std::function<void(Agent&, const Message&)> func);

    /**************************************************************************
     * Services Functions
     **************************************************************************/
    /// Add an array of services to this agent
    AgentBuilder& services(const Span<ServiceBuilder> items);

    /// Add an array of service names to this agent
    AgentBuilder& serviceNames(const Span<std::string_view> items);

    /// Add a service to the agent
    AgentBuilder& service(const ServiceBuilder& item) { return services(Span<ServiceBuilder>(&item, 1)); }

    /// Add a service by name to this agent
    AgentBuilder& serviceName(std::string_view item) { return serviceNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Roles Functions
     **************************************************************************/
    /// Add an array of roles to this agent
    AgentBuilder& roles(const Span<RoleBuilder> items);

    /// Add an array of role names to this agent
    AgentBuilder& roleNames(const Span<std::string_view> items);

    /// Add a role to the agent
    AgentBuilder& role(const RoleBuilder& item) { return roles(Span<RoleBuilder>(&item, 1)); }

    /// Add a role by name to this agent
    AgentBuilder& roleName(std::string_view item) { return roleNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Plans Functions
     **************************************************************************/
    /// Add an array of plans to this agent
    AgentBuilder& plans(const Span<PlanBuilder> items);

    /// Add an array of plan names to this agent
    AgentBuilder& planNames(const Span<std::string_view> items);

    /// Add a plan to the agent
    AgentBuilder& plan(const PlanBuilder& item) { return plans(Span<PlanBuilder>(&item, 1)); }

    /// Add a plan by name to this agent
    AgentBuilder& planName(std::string_view item) { return planNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Desires Functions
     **************************************************************************/
    /// Add an array of desires to this agent
    AgentBuilder& desires(const Span<GoalBuilder> items);

    /// Add an array of desire names to this agent
    AgentBuilder& desireNames(const Span<std::string_view> items);

    /// Add a desire to the agent
    AgentBuilder& desire(const GoalBuilder& item) { return desires(Span<GoalBuilder>(&item, 1)); }

    /// Add a desire by name to this agent
    AgentBuilder& desireName(std::string_view item) { return desireNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Beliefs Functions
     **************************************************************************/
    /// Add an array of belief names to this agent
    AgentBuilder& beliefNames(const Span<std::string_view> items);

    /// Add a belief by name to this agent
    AgentBuilder& beliefName(std::string_view item) { return beliefNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Tactic Functions
     **************************************************************************/
    /// \todo Only tactics accepts a handle for now, see the comment in
    /// TacticBuilder::commit() for more information.

    /// Add an array of tactics to this agent
    AgentBuilder& tactics(const Span<TacticHandle> items);

    /// Add an array of tactic names to this agent
    AgentBuilder& tacticNames(const Span<std::string_view> items);

    /// Add a tactic to the agent
    AgentBuilder& tactic(const TacticHandle& item) { return tactics(Span<TacticHandle>(&item, 1)); }

    /// Add a tactic by name to this agent
    AgentBuilder& tacticName(std::string_view item) { return tacticNames(Span<std::string_view>(&item, 1)); }

    /**************************************************************************
     * Resource Functions
     **************************************************************************/
    /// Add an array of resources to this agent
    AgentBuilder& resources(const Span<ResourceBuilder> items);

    /// Add an array of resource names to this agent
    AgentBuilder& resourceNames(const Span<std::string_view> items);

    /// Add a resource to the agent
    AgentBuilder& resource(const ResourceBuilder& item) { return resources(Span<ResourceBuilder>(&item, 1)); }

    /// Add a resource by name to this agent
    AgentBuilder& resourceName(std::string_view item) { return resourceNames(Span<std::string_view>(&item, 1)); }

    /// Optionally add a resource to goal mapping (activated when the resource is violated)
    /// @param resource The resource of the goal mapping to add
    /// @param goal The goal of the resource-mapping to add
    /// @return This AgentBuilder
    AgentBuilder& maintains(const ResourceBuilder& resource, const GoalBuilder& goal);

    /**************************************************************************
     * Create Functions
     **************************************************************************/
    /// Create an instance of this team. The team must be committed before
    /// creation as a team otherwise this function will return a null pointer.
    /// @param[in] name The name of the new Agent.
    /// @param[in] uuid The UniqueId to assign to the new Agent.
    /// @return A pointer to the newly created Agent. Null pointer if creation
    /// failed.
    Team* createTeamInstance(std::string_view name, const UniqueId& uuid = UniqueId::random());

    /// Create an instance of this team. The team must be committed before
    /// creation as a team otherwise this function will return a null pointer.
    /// @param[in] name The name of the new Agent.
    /// @param[in] uuid The UniqueId to assign to the new Agent.
    /// @return The handle to the newly created Service. Invalid handle if
    /// creation failed.
    AgentHandle createTeam(std::string_view name, const UniqueId& uuid = UniqueId::random());

    /// Create an instance of this agent. The agent must be committed before
    /// creation.
    /// @param[in] name The name of the new Agent.
    /// @param[in] uuid The UniqueId to assign to the new Agent.
    /// @return A pointer to the newly created Agent. Null pointer if creation
    /// failed.
    Agent* createAgentInstance(std::string_view name, const UniqueId& uuid = UniqueId::random());

    /// Create an instance of this agent. The agent must be committed before
    /// creation.
    /// @param[in] name The name of the new Agent.
    /// @param[in] uuid The UniqueId to assign to the new Agent.
    /// @return The handle to the newly created Service. Invalid handle if
    /// creation failed.
    AgentHandle createAgent(std::string_view name, const UniqueId& uuid = UniqueId::random());

    /// Commit this Agent template into jack model
    /// Once commited this template can be used to create an Agent instance
    /// @return The AgentBuilder
    template <typename AgentT = Agent>
    AgentBuilder& commitAsAgent() {
        AgentT agent(m_engine, m_name);
        commitInternal(&agent);
        return *this;
    }

    /// Commit this Agent template into jack model
    /// Once commited this template can be used to create an Agent instance
    /// @return The AgentBuilder
    template <typename TeamT = Team>
    AgentBuilder& commitAsTeam() {
        TeamT agent(m_engine, m_name);
        JACK_ASSERT(agent.isTeam());
        commitInternal(&agent);
        return *this;
    }

private:
    /// Assign the properties to the agent passed in and commit the agent to
    /// the engine.
    void commitInternal(Agent *agent);

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    std::vector<std::string>                                           m_plans;
    std::vector<std::string>                                           m_roles;
    std::vector<std::string>                                           m_desires;
    Agent::ActionHandlerTable                                          m_actionHandlers;
    std::map<std::string, std::function<void(Agent&, const Message&)>> m_messageHandlers;
    std::vector<std::string>                                           m_beliefs;
    std::vector<std::string>                                           m_resources;
    std::map<std::string, std::string>                                 m_resourcesToGoals;
    std::vector<std::string>                                           m_tactics;
    std::vector<std::string>                                           m_services;
};
} // namespace aos::jack
#endif
