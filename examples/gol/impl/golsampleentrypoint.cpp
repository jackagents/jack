#include <gol/meta/gol.h>

int main(int /*argc*/, char **/*argv*/)
{
    gol bdi; /// Initializing the bdi application

    /// Create the agents
    aos::jack::AgentHandle gameoflifeagentHandle = bdi.createAgent("gol.Game Of Life Agent Template", "gol.game of life agent");

    /// Start the agents
    aos::jack::Agent* gameoflifeagent = bdi.getAgent(gameoflifeagentHandle);
    gameoflifeagent->start();

    bdi.execute(); /// Execute the bdi application synchronously
    return 0;
}
