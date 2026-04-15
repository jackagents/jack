#include <simple/meta/agents/thermostatagentmeta.h>

/// Project
#include <simple/impl/agents/thermostatagentimpl.h>
#include <simple/meta/messages/temperaturereadingmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/engine.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
ThermostatAgentMeta::ThermostatAgentMeta(aos::jack::Engine& bdi, std::string_view name)
: aos::jack::Agent(bdi, name)
{
    setupHandlers();
}

ThermostatAgentMeta::ThermostatAgentMeta(const ThermostatAgentMeta* other, std::string_view name)
: aos::jack::Agent(other, name)
{
    setupHandlers();
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::GoalPursue ThermostatAgentMeta::pursueMaintainComfort(aos::jack::GoalPersistent persistent, const aos::jack::UniqueId& id)
{
    aos::jack::GoalPursue result = pursue("simple.Maintain Comfort", persistent, {} /*message*/, id);
    return result;
}

aos::jack::Agent* ThermostatAgentMeta::clone(std::string_view name) const
{
    return JACK_NEW(ThermostatAgent, static_cast<const ThermostatAgent*>(this), name);
}

void ThermostatAgentMeta::setupHandlers()
{
}

/**************************************************************************
 * Static Functions
 **************************************************************************/
aos::jack::AgentHandle ThermostatAgentMeta::create(aos::jack::Engine& bdi, std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto result = bdi.createAgent(MODEL_NAME, name, uuid);
    JACK_ASSERT(result.valid());
    return result;
}

ThermostatAgent* ThermostatAgentMeta::createInstance(aos::jack::Engine& bdi, std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<ThermostatAgent*>(bdi.createAgentInstance(MODEL_NAME, name, uuid));
    JACK_ASSERT(result);
    return result;
}
