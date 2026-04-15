#include <simple/meta/simple.h>

int main(int /*argc*/, char **/*argv*/)
{
    simple bdi; /// Initializing the bdi application

    /// Create the agents
    aos::jack::AgentHandle thermostatagentHandle = bdi.createAgent("simple.Thermostat Agent Template", "simple.thermostat agent");

    /// Start the agents
    aos::jack::Agent* thermostatagent = bdi.getAgent(thermostatagentHandle);
    thermostatagent->start();

    bdi.execute(); /// Execute the bdi application synchronously
    return 0;
}
