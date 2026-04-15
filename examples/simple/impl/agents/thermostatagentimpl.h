#ifndef SIMPLE_THERMOSTAT_AGENT_IMPL_H
#define SIMPLE_THERMOSTAT_AGENT_IMPL_H

#include <simple/meta/agents/thermostatagentmeta.h>


/******************************************************************************
 * \class  ThermostatAgent
 * \author jackmake
 ******************************************************************************/
class ThermostatAgent : public ThermostatAgentMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    ThermostatAgent(aos::jack::Engine& bdi, std::string_view name);
    ThermostatAgent(const ThermostatAgent* other, std::string_view name);
    ~ThermostatAgent() override {}

    /**************************************************************************
     * Action Handlers
     **************************************************************************/
};

#endif /// SIMPLE_THERMOSTAT_AGENT_IMPL_H