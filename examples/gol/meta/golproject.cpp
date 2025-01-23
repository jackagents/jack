/// Project
#include <gol/meta/golproject.h>
#if defined(JACK_WITH_SIM)
#include <gol/meta/golservicecomponents.h>
#include <gol/meta/golevents.h>
#endif

/// Project
#include <gol/impl/agents/gameoflifeagentimpl.h>
#include <gol/impl/services/gameoflifeserviceimpl.h>
#include <gol/meta/messages/cellcommandmeta.h>
#include <gol/meta/messages/cellinfometa.h>
#include <gol/impl/goals/modellifeimpl.h>
#include <gol/impl/plans/liveplanimpl.h>
#include <gol/impl/plans/dieplanimpl.h>

/// Sim
#if defined(JACK_WITH_SIM)
#include <sim/sim.h>
#include <sim/simjson.h>
#include <sim/utils.h>
#endif /// defined(JACK_WITH_SIM)

/// JACK
#include <jack/builders/agentbuilder.h>
#include <jack/builders/actionbuilder.h>
#include <jack/builders/rolebuilder.h>
#include <jack/builders/coroutinebuilder.h>
#include <jack/builders/planbuilder.h>
#include <jack/builders/servicebuilder.h>


static void initMessages([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Register the custom type to use in message schemas
    [[maybe_unused]] aos::jack::FieldRegistry& registry = aos::jack::FieldRegistry::getInstance();
    registry.registerType<CellCommand>(
        "gol.Cell Command",
        &CellCommand::anyToMessage,
        nullptr,
        &CellCommand::anyToJSON,
        nullptr,
        &CellCommand::anyToNlohmannJSON);
    registry.registerType<CellInfo>(
        "gol.Cell Info",
        &CellInfo::anyToMessage,
        nullptr,
        &CellInfo::anyToJSON,
        nullptr,
        &CellInfo::anyToNlohmannJSON);
    registry.registerType<std::vector<CellCommand>>(
        "gol.Cell Command[]",
        nullptr,
        &CellCommand::anyArrayToMessage,
        nullptr,
        &CellCommand::anyArrayToJSON,
        &CellCommand::anyToNlohmannJSON);
    registry.registerType<std::vector<CellInfo>>(
        "gol.Cell Info[]",
        nullptr,
        &CellInfo::anyArrayToMessage,
        nullptr,
        &CellInfo::anyArrayToJSON,
        &CellInfo::anyToNlohmannJSON);

    /// Create message schemas
    bdi.commitMessageSchema(&CellCommand::schema());
    bdi.commitMessageSchema(&CellInfo::schema());
}

static void initActions([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    bdi.action("gol.Live Action")
        .request("gol.Cell Command")
        .commit();
    bdi.action("gol.Die Action")
        .request("gol.Cell Command")
        .commit();
}

static void initRoles([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initResources([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initGoals([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    bdi.goal("gol.Model Life")
       .message("")
       .commit<ModelLife>();
}

static void initPlans([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    { /// LivePlan
        aos::jack::CoroutineBuilder coroutine = bdi.coroutine();
        coroutine.action("gol.Live Action", aos::jack::UniqueId(0x66a80c7a9a0848f7ULL, 0x811e51cf65934f40ULL));

        int32_t label0gol_LiveAction = {};
        coroutine.label(label0gol_LiveAction);

        /// \note Configure task execution graph
        coroutine.configure(label0gol_LiveAction).onSuccess(aos::jack::Coroutine::TERMINAL_LABEL);

        bdi.plan("gol.Live Plan")
           .handles("gol.Model Life")
           .body(coroutine)
           .commit<LivePlan>();
    }
    { /// DiePlan
        aos::jack::CoroutineBuilder coroutine = bdi.coroutine();
        coroutine.action("gol.Die Action", aos::jack::UniqueId(0x865d82cabda246b1ULL, 0x88cdc7c83badfb3fULL));

        int32_t label0gol_DieAction = {};
        coroutine.label(label0gol_DieAction);

        /// \note Configure task execution graph
        coroutine.configure(label0gol_DieAction).onSuccess(aos::jack::Coroutine::TERMINAL_LABEL);

        bdi.plan("gol.Die Plan")
           .handles("gol.Model Life")
           .body(coroutine)
           .commit<DiePlan>();
    }
}

static void initServices([[ maybe_unused ]] aos::jack::Engine& bdi)
{

    { /// GameOfLifeService
        const std::string_view msgList[] = {
            "gol.Cell Info"sv,
        };

        bdi.service(GameOfLifeService::MODEL_NAME)
            .messageNames(msgList)
            .commit<GameOfLifeService>();
    }
}

static void initTactics([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initAgents([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the agent templates to the engine under their runtime name

    { /// GameOfLifeAgent
        const std::string_view serviceList[] = {
            "gol.Game Of Life Service"sv,
        };
        const std::string_view planList[] = {
            "gol.Live Plan"sv,
            "gol.Die Plan"sv,
        };
        const std::string_view desireList[] = {
            "gol.Model Life"sv,
        };
        const std::string_view beliefList[] = {
            "gol.Cell Info"sv,
        };

        /// Configure the agent
        bdi.agent(GameOfLifeAgent::MODEL_NAME)
            .serviceNames(serviceList)
            .planNames(planList)
            .desireNames(desireList)
            .beliefNames(beliefList)
            .commitAsAgent<GameOfLifeAgent>();
    }
}

static void initTeams([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the team templates to the engine under their runtime name
}

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
gol::gol() : aos::jack::Engine()
{
    aos::jack::Engine& bdi = static_cast<aos::jack::Engine&>(*this);
    bdi.setName("gol");

    initMessages(bdi);
    initActions(bdi);
    initResources(bdi);
    initGoals(bdi);
    initPlans(bdi);
    initRoles(bdi);
    initServices(bdi);
    initTactics(bdi);
    initAgents(bdi);
    initTeams(bdi);
}

gol::~gol() { }

/******************************************************************************
 * Functions
 ******************************************************************************/

aos::jack::AgentHandle gol::createGameOfLifeAgent(std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto result = createAgent(GameOfLifeAgent::MODEL_NAME, name, uuid);
    return result;
}

GameOfLifeAgent* gol::createGameOfLifeAgentInstance(std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<GameOfLifeAgent*>(createAgentInstance(GameOfLifeAgent::MODEL_NAME, name, uuid));
    return result;
}






aos::jack::ServiceHandle gol::createGameOfLifeService(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto result = createService(GameOfLifeService::MODEL_NAME, name, proxy, uuid);
    return result;
}



GameOfLifeService* gol::createGameOfLifeServiceInstance(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<GameOfLifeService*>(createServiceInstance(GameOfLifeService::MODEL_NAME, name, proxy, uuid));
    JACK_ASSERT(result);
    return result;
}


/******************************************************************************
 * Static Functions
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
void gol::initSimModel(aos::sim::SimulationBase* sim)
{
    if (!sim) {
        return;
    }
    /// @todo: The editor uses the model name but it strips spaces from the
    /// name. Here I mangle the name of the component so its uniform. We however 
    /// should only use the model name verbatim.
    sim->addJsonComponentCreator(CellCommand::MODEL_NAME, CellCommand::JsonConfig::parseJson);
    sim->addJsonComponentCreator(CellInfo::MODEL_NAME, CellInfo::JsonConfig::parseJson);
    sim->addJsonComponentCreator(GameOfLifeServiceComponent::COMPONENT_NAME, GameOfLifeServiceComponent::JsonConfig::parseJson);
}

bool gol::addComponentToEntity(aos::jack::Engine& engine, aos::sim::EntityWrapper entity, std::string_view componentName, const aos::sim::JsonParsedComponent *config)
{
    if (!entity.valid()) {
        return false;
    }

    /// \todo We can remove the JSON config since we code generate to/from JSON
    /// serialisation routines in the base class anyway. JSON config is used
    /// here temporarily.
    bool handledAsComponent = false;

    if (handledAsComponent) {
        return true;
    }

    bool handledAsService = true;
    if (componentName == GameOfLifeService::COMPONENT_NAME || componentName == aos::sim::removeSpaces(GameOfLifeService::COMPONENT_NAME)) {
        auto& service = *entity.m_handle.assign<GameOfLifeServiceComponent>();
        if (const auto *c = dynamic_cast<const GameOfLifeServiceComponent::JsonConfig *>(config)) {
            aos::jack::UniqueId id = aos::jack::UniqueId::initFromString(c->uuid);
            if (c->uuid.size()) {
                JACK_ASSERT(id.valid());
            } else {
                id = aos::jack::UniqueId::random();
            }
            service = GameOfLifeServiceComponent(entity, engine, c->templateName, c->name, c->startService, c->proxyService, id);
        }
    } else {
        handledAsService = false;
    }
    return handledAsService;

}
#endif /// defined(JACK_WITH_SIM)
