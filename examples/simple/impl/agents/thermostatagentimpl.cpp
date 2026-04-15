#include <simple/impl/agents/thermostatagentimpl.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
ThermostatAgent::ThermostatAgent(aos::jack::Engine& bdi, std::string_view name)
: ThermostatAgentMeta(bdi, name) {
}

ThermostatAgent::ThermostatAgent(const ThermostatAgent* other, std::string_view name)
: ThermostatAgentMeta(other, name) {
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/
