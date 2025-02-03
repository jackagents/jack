// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <gol/impl/services/gameoflifeserviceimpl.h>
#include <gol/meta/messages/cellcommandmeta.h>
#include <gol/impl/agents/gameoflifeagentimpl.h>

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
GameOfLifeService::GameOfLifeService(aos::jack::Engine& bdi, std::string_view name)
: GameOfLifeServiceMeta(bdi, name) {
}

GameOfLifeService::GameOfLifeService(const GameOfLifeService* other, std::string_view name)
: GameOfLifeServiceMeta(other, name) {
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/

aos::jack::Event::Status GameOfLifeService::onLiveAction(const CellCommand& request, aos::jack::ActionHandle handle)
{
    m_gol->m_grid[request.cell_index] = true;

    return aos::jack::Event::Status::SUCCESS;
}


aos::jack::Event::Status GameOfLifeService::onDieAction(const CellCommand& request, aos::jack::ActionHandle handle)
{
    m_gol->m_grid[request.cell_index] = false;

    return aos::jack::Event::Status::SUCCESS;
}

