#include <gol/meta/services/gameoflifeservicemeta.h>

/// Project
#include <gol/impl/services/gameoflifeserviceimpl.h>
#include <gol/meta/messages/cellcommandmeta.h>
#include <gol/meta/messages/cellinfometa.h>

/// JACK
#include <jack/corelib.h>
#include <jack/engine.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
GameOfLifeServiceMeta::GameOfLifeServiceMeta(aos::jack::Engine& bdi, std::string_view name)
: aos::jack::Service(bdi, name)
{
    setupHandlers();
}

GameOfLifeServiceMeta::GameOfLifeServiceMeta(const GameOfLifeServiceMeta* other, std::string_view name)
: aos::jack::Service(other, name)
{
    setupHandlers();
}

/******************************************************************************
 * Action Handlers
 ******************************************************************************/
aos::jack::Service* GameOfLifeServiceMeta::clone(std::string_view name) const
{
    return JACK_NEW(GameOfLifeService, static_cast<const GameOfLifeService*>(this), name);
}

void GameOfLifeServiceMeta::setupHandlers()
{
    addActionHandler("gol.Live Action", [](aos::jack::Service& service, [[maybe_unused]] aos::jack::Message& in, [[maybe_unused]] aos::jack::Message& out, aos::jack::ActionHandle handle)
    {
        const CellCommand& request = static_cast<CellCommand&>(in);
        aos::jack::Event::Status result = static_cast<GameOfLifeServiceMeta&>(service).onLiveAction(request, handle);
        return result;
    });
    addActionHandler("gol.Die Action", [](aos::jack::Service& service, [[maybe_unused]] aos::jack::Message& in, [[maybe_unused]] aos::jack::Message& out, aos::jack::ActionHandle handle)
    {
        const CellCommand& request = static_cast<CellCommand&>(in);
        aos::jack::Event::Status result = static_cast<GameOfLifeServiceMeta&>(service).onDieAction(request, handle);
        return result;
    });
}

/**************************************************************************
 * Static Functions
 **************************************************************************/
aos::jack::ServiceHandle GameOfLifeServiceMeta::create(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto result = bdi.createService(MODEL_NAME, name, proxy, uuid);
    JACK_ASSERT(result.valid());
    return result;
}

GameOfLifeService* GameOfLifeServiceMeta::createInstance(aos::jack::Engine& bdi, std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<GameOfLifeService*>(bdi.createServiceInstance(MODEL_NAME, name, proxy, uuid));
    JACK_ASSERT(result);
    return result;
}
