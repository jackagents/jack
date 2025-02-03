// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/impl/agents/gameoflifeagentimpl.h>

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
GameOfLifeAgent::GameOfLifeAgent(aos::jack::Engine& bdi, std::string_view name)
: GameOfLifeAgentMeta(bdi, name) {
}

GameOfLifeAgent::GameOfLifeAgent(const GameOfLifeAgent* other, std::string_view name)
: GameOfLifeAgentMeta(other, name) {
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/
