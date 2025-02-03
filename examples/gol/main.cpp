// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/gridworld.h>
#include <gol/gameoflife.h>

#include <gol/meta/golproject.h>
#include <gol/meta/messages/cellinfometa.h>

#include <gol/impl/services/gameoflifeserviceimpl.h>
#include <gol/impl/agents/gameoflifeagentimpl.h>

#include <iostream>

int main(int /*argc*/, char **/*argv*/)
{
    const int gridSize = 50;

    /*
    * 1) Create the agents for each cell
    */
    gol bdi; /// Initializing the bdi application

    // Setup a simepl gridworld rendering
    aos::GridWorld world(bdi,
              gridSize,
              gridSize,
              550 /*windowWidth*/,
              550 /*windowHeight*/,
              "Game Of Life using JACK Agents");

    GameOfLife env(gridSize);

    // Create the Game of Life Service
    GameOfLifeService* golService = bdi.createGameOfLifeServiceInstance("GOL Service", false);
    golService->start();
    golService->setEnvironment(&env);
    auto golServiceHandle = golService->handle();

    // Create the agents for each cell
    std::vector<aos::jack::Agent *> agents(gridSize * gridSize);

    // create the agents for each cell
    for (int i = 0; i < agents.size(); ++i) {

        // create the agent
        std::stringstream ss;
        ss << "agent" << i;
        aos::jack::Agent* agent = bdi.getAgent(bdi.createAgent(GameOfLifeAgentMeta::MODEL_NAME, ss.str()));

        // start the agent
        agent->start();

        agents[i] = agent;

        const auto cellInfo = agent->context().getMessageAsPtr<CellInfo>();
        cellInfo->cell_index = i;
        cellInfo->is_alive   = env.m_grid[i];
        cellInfo->population = env.m_population[i];

        // link the agent to the service
        agent->attachService(golServiceHandle, true);
    }

    std::cout << "All Agents are created" << std::endl;

    sf::Clock clock;
    sf::Time deltaTime;
    bool run = true;
    while(run) {
        run = world.draw(1 /*engineTicks*/, deltaTime.asSeconds());
        deltaTime = clock.restart();

        env.update();

        // update the beliefs of the agents
        for (int i = 0; i < gridSize * gridSize; ++i)
        {
            aos::jack::Agent* agent = agents.at(i);
            const auto cellInfo = agent->context().getMessageAsPtr<CellInfo>();
            cellInfo->cell_index = i;
            cellInfo->is_alive   = env.m_grid[i];
            cellInfo->population = env.m_population[i];
        }

        for (int x = 0; x < gridSize; x++) {
            for (int y = 0; y < gridSize; y++) {
                int i      = y * gridSize + x;
                bool alive = env.m_grid[i];
                int pop    = env.m_population[i];

                sf::Color color = sf::Color(0x00000000);
                if (alive) {
                    if (pop == 0) {
                        color = sf::Color(0x333333FF);
                    } else if (pop == 1) {
                        color = sf::Color(0x666666FF);
                    } else if (pop == 2) {
                        color = sf::Color(0xFFFFFFFF);
                    }  else if (pop == 3) {
                        color = sf::Color(0x00FF00FF);
                    } else {
                        color = sf::Color(0xFF0000FF);
                    }
                }

                world.drawCell(x, y, color);
            }
        }
    }

    return 0;
}
