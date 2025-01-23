#include <gol/meta/agents/gameoflifeagentmeta.h>

/// Project
#include <gol/impl/agents/gameoflifeagentimpl.h>
#include <gol/meta/messages/cellinfometa.h>

/// JACK
#include <jack/corelib.h>
#include <jack/engine.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
GameOfLifeAgentMeta::GameOfLifeAgentMeta(aos::jack::Engine& bdi, std::string_view name)
: aos::jack::Agent(bdi, name)
{
    setupHandlers();
}

GameOfLifeAgentMeta::GameOfLifeAgentMeta(const GameOfLifeAgentMeta* other, std::string_view name)
: aos::jack::Agent(other, name)
{
    setupHandlers();
}

/******************************************************************************
 * Functions
 ******************************************************************************/
aos::jack::GoalPursue GameOfLifeAgentMeta::pursueModelLife(aos::jack::GoalPersistent persistent, const aos::jack::UniqueId& id)
{
    aos::jack::GoalPursue result = pursue("gol.Model Life", persistent, {} /*message*/, id);
    return result;
}

aos::jack::Agent* GameOfLifeAgentMeta::clone(std::string_view name) const
{
    return JACK_NEW(GameOfLifeAgent, static_cast<const GameOfLifeAgent*>(this), name);
}

void GameOfLifeAgentMeta::setupHandlers()
{
}

/**************************************************************************
 * Static Functions
 **************************************************************************/
aos::jack::AgentHandle GameOfLifeAgentMeta::create(aos::jack::Engine& bdi, std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto result = bdi.createAgent(MODEL_NAME, name, uuid);
    JACK_ASSERT(result.valid());
    return result;
}

GameOfLifeAgent* GameOfLifeAgentMeta::createInstance(aos::jack::Engine& bdi, std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<GameOfLifeAgent*>(bdi.createAgentInstance(MODEL_NAME, name, uuid));
    JACK_ASSERT(result);
    return result;
}
