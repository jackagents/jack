#include <jack/builders/agentbuilder.h>

#include <jack/corelib.h>
#include <jack/agent.h>      // for Agent
#include <jack/engine.h>     // for Engine
#include <jack/message.h>    // for Message::Status

#include <jack/builders/goalbuilder.h>
#include <jack/builders/planbuilder.h>
#include <jack/builders/rolebuilder.h>
#include <jack/builders/servicebuilder.h>
#include <jack/builders/resourcebuilder.h>
#include <jack/builders/tacticbuilder.h>

namespace aos::jack
{
AgentBuilder::AgentBuilder(Engine& engine, std::string_view name)
: Builder(engine, name)
{}

AgentBuilder::AgentBuilder(Engine& engine, const Agent& agent)
: Builder(engine, agent.name())
, m_plans(agent.plans())
, m_roles(agent.roles())
, m_desires(agent.initialDesires())
, m_actionHandlers(agent.actionHandlers())
, m_messageHandlers(agent.messageHandlers())
, m_beliefs(agent.beliefSetNames())
, m_resources(agent.resources())
, m_resourcesToGoals(agent.resourcesToGoals())
, m_services(agent.serviceNames())
{
}

AgentBuilder& AgentBuilder::handleAction(const std::string& action, Agent::ActionHandlerFunc func)
{
    m_actionHandlers[action] = func;
    return *this;
}

AgentBuilder& AgentBuilder::handleMessage(const std::string& msg, std::function<void(Agent&, const Message&)> func)
{
    m_messageHandlers[msg] = func;
    return *this;
}

/**************************************************************************
 * Services Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::services(const Span<ServiceBuilder> items)
{
    m_services.reserve(m_services.size() + items.size());
    for (const ServiceBuilder& item : items) {
        m_services.push_back(item.name());
    }
    return *this;
}

AgentBuilder& AgentBuilder::serviceNames(const Span<std::string_view> items)
{
    m_services.insert(m_services.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Roles Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::roles(const Span<RoleBuilder> items)
{
    m_roles.reserve(m_roles.size() + items.size());
    for (const RoleBuilder& item : items) {
        m_roles.push_back(item.name());
    }
    return *this;
}

AgentBuilder& AgentBuilder::roleNames(const Span<std::string_view> items)
{
    m_roles.insert(m_roles.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Plans Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::plans(const Span<PlanBuilder> items)
{
    m_plans.reserve(m_plans.size() + items.size());
    for (const PlanBuilder& item : items) {
        m_plans.push_back(item.name());
    }
    return *this;
}

AgentBuilder& AgentBuilder::planNames(const Span<std::string_view> items)
{
    m_plans.insert(m_plans.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Desires Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::desires(const Span<GoalBuilder> items)
{
    m_desires.reserve(m_desires.size() + items.size());
    for (const GoalBuilder& item : items) {
        m_desires.push_back(item.name());
    }
    return *this;
}

AgentBuilder& AgentBuilder::desireNames(const Span<std::string_view> items)
{
    m_desires.insert(m_desires.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Beliefs Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::beliefNames(const Span<std::string_view> items)
{
    m_beliefs.insert(m_beliefs.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Tactics Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::tactics(const Span<TacticHandle> items)
{
    m_tactics.reserve(m_tactics.size() + items.size());
    for (const TacticHandle& item : items) {
        m_tactics.push_back(item.m_name);
    }
    return *this;
}

AgentBuilder& AgentBuilder::tacticNames(const Span<std::string_view> items)
{
    m_tactics.insert(m_tactics.end(), items.begin(), items.end());
    return *this;
}

/**************************************************************************
 * Resources Functions
 **************************************************************************/
AgentBuilder& AgentBuilder::resources(const Span<ResourceBuilder> items)
{
    m_resources.reserve(m_resources.size() + items.size());
    for (const ResourceBuilder& item : items) {
        m_resources.push_back(item.name());
    }
    return *this;
}

AgentBuilder& AgentBuilder::resourceNames(const Span<std::string_view> items)
{
    m_resources.insert(m_resources.end(), items.begin(), items.end());
    return *this;
}

AgentBuilder& AgentBuilder::maintains(const ResourceBuilder& resource, const GoalBuilder& goal)
{
    m_resourcesToGoals[resource.name()] = goal.name();
    return *this;
}

/******************************************************************************
 * Create Functions
 ******************************************************************************/
Team *AgentBuilder::createTeamInstance(std::string_view name, const UniqueId& uuid)
{
    Agent *result = createAgentInstance(name, uuid);
    if (!result->isTeam()) {
        JACK_WARNING("Agent builder created team before the template was "
                     "committed or, template is not a team. Commit the builder "
                     "by calling commit() before calling create() and ensure "
                     "that the committed template is a team. [team={}]", m_name);
        JACK_INVALID_CODE_PATH;
    }
    return dynamic_cast<Team *>(result);
}

AgentHandle AgentBuilder::createTeam(std::string_view name, const UniqueId& uuid)
{
    Team *team = createTeamInstance(name, uuid);
    AgentHandle result = {};
    if (team) {
        result = team->handle();
    }
    return result;
}

Agent *AgentBuilder::createAgentInstance(std::string_view name, const UniqueId& uuid)
{
    Agent *result = m_engine.createAgentInstance(m_name, name, uuid);
    if (!result) {
        JACK_WARNING("Agent builder created agent before the template was "
                     "committed. Commit the builder by calling commit() before "
                     "calling create(). [template={}, agent={}]",
                     m_name, name);
        JACK_INVALID_CODE_PATH;
    }
    return result;
}

AgentHandle AgentBuilder::createAgent(std::string_view name, const UniqueId& uuid)
{
    Agent*      agent  = createAgentInstance(name, uuid);
    AgentHandle result = {};
    if (agent) {
        result = agent->handle();
    }
    return result;
}

void AgentBuilder::commitInternal(Agent *agent)
{
    agent->m_plans = m_plans;
    agent->m_roles = m_roles;
    agent->m_initialDesires = m_desires;
    agent->m_actionHandlers = m_actionHandlers;
    agent->m_messageHandlers = m_messageHandlers;
    agent->m_beliefsetIds = m_beliefs;
    agent->m_resourceIds = m_resources;
    agent->m_resourcesToGoals = m_resourcesToGoals;
    for (const auto& tacticName : m_tactics) {
        const Tactic* tactic = m_engine.getTactic(tacticName);
        agent->m_currentTactics[tactic->goal()] = tactic;
    }
    agent->m_services = m_services;
    m_engine.commitAgent(agent);
}
}  // namespace aos::jack
