#ifndef GOL_GAME_OF_LIFE_AGENT_IMPL_H
#define GOL_GAME_OF_LIFE_AGENT_IMPL_H

// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/meta/agents/gameoflifeagentmeta.h>

/******************************************************************************
 * \class  GameOfLifeAgent
 * \author jackmake
 ******************************************************************************/
class GameOfLifeAgent : public GameOfLifeAgentMeta
{
public:
    /**************************************************************************
     * Constructor/Destructors
     **************************************************************************/
    GameOfLifeAgent(aos::jack::Engine& bdi, std::string_view name);
    GameOfLifeAgent(const GameOfLifeAgent* other, std::string_view name);
    ~GameOfLifeAgent() override {}

    /**************************************************************************
     * Action Handlers
     **************************************************************************/
};

#endif /// GOL_GAME_OF_LIFE_AGENT_IMPL_H