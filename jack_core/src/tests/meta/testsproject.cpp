/// Project
#include <tests/meta/testsproject.h>
#if defined(JACK_WITH_SIM)
#include <tests/meta/testsservicecomponents.h>
#include <tests/meta/testsevents.h>
#endif

/// Project
#include <tests/meta/messages/pingmeta.h>
#include <tests/meta/messages/pongmeta.h>
#include <tests/meta/messages/pingpongbeliefsetmeta.h>
#include <tests/meta/messages/agentplanswitchmeta.h>
#include <tests/meta/messages/dummyflagmeta.h>

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
    registry.registerType<Ping>(
        "tests.Ping",
        &Ping::anyToMessage,
        nullptr,
        &Ping::anyToJSON,
        nullptr,
        &Ping::anyToNlohmannJSON);
    registry.registerType<Pong>(
        "tests.Pong",
        &Pong::anyToMessage,
        nullptr,
        &Pong::anyToJSON,
        nullptr,
        &Pong::anyToNlohmannJSON);
    registry.registerType<PingPongBeliefSet>(
        "tests.PingPong BeliefSet",
        &PingPongBeliefSet::anyToMessage,
        nullptr,
        &PingPongBeliefSet::anyToJSON,
        nullptr,
        &PingPongBeliefSet::anyToNlohmannJSON);
    registry.registerType<AgentPlanSwitch>(
        "tests.AgentPlanSwitch",
        &AgentPlanSwitch::anyToMessage,
        nullptr,
        &AgentPlanSwitch::anyToJSON,
        nullptr,
        &AgentPlanSwitch::anyToNlohmannJSON);
    registry.registerType<DummyFlag>(
        "tests.DummyFlag",
        &DummyFlag::anyToMessage,
        nullptr,
        &DummyFlag::anyToJSON,
        nullptr,
        &DummyFlag::anyToNlohmannJSON);
    registry.registerType<std::vector<Ping>>(
        "tests.Ping[]",
        nullptr,
        &Ping::anyArrayToMessage,
        nullptr,
        &Ping::anyArrayToJSON,
        &Ping::anyToNlohmannJSON);
    registry.registerType<std::vector<Pong>>(
        "tests.Pong[]",
        nullptr,
        &Pong::anyArrayToMessage,
        nullptr,
        &Pong::anyArrayToJSON,
        &Pong::anyToNlohmannJSON);
    registry.registerType<std::vector<PingPongBeliefSet>>(
        "tests.PingPong BeliefSet[]",
        nullptr,
        &PingPongBeliefSet::anyArrayToMessage,
        nullptr,
        &PingPongBeliefSet::anyArrayToJSON,
        &PingPongBeliefSet::anyToNlohmannJSON);
    registry.registerType<std::vector<AgentPlanSwitch>>(
        "tests.AgentPlanSwitch[]",
        nullptr,
        &AgentPlanSwitch::anyArrayToMessage,
        nullptr,
        &AgentPlanSwitch::anyArrayToJSON,
        &AgentPlanSwitch::anyToNlohmannJSON);
    registry.registerType<std::vector<DummyFlag>>(
        "tests.DummyFlag[]",
        nullptr,
        &DummyFlag::anyArrayToMessage,
        nullptr,
        &DummyFlag::anyArrayToJSON,
        &DummyFlag::anyToNlohmannJSON);

    /// Create message schemas
    bdi.commitMessageSchema(&Ping::schema());
    bdi.commitMessageSchema(&Pong::schema());
    bdi.commitMessageSchema(&PingPongBeliefSet::schema());
    bdi.commitMessageSchema(&AgentPlanSwitch::schema());
    bdi.commitMessageSchema(&DummyFlag::schema());
}

static void initActions([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initRoles([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initResources([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initGoals([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initPlans([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initServices([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initTactics([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initAgents([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the agent templates to the engine under their runtime name
}

static void initTeams([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the team templates to the engine under their runtime name
}

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
tests::tests() : aos::jack::Engine()
{
    aos::jack::Engine& bdi = static_cast<aos::jack::Engine&>(*this);
    bdi.setName("tests");

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

tests::~tests() { }

/******************************************************************************
 * Functions
 ******************************************************************************/










/******************************************************************************
 * Static Functions
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
void tests::initSimModel(aos::sim::SimulationBase* sim)
{
    if (!sim) {
        return;
    }
    /// @todo: The editor uses the model name but it strips spaces from the
    /// name. Here I mangle the name of the component so its uniform. We however 
    /// should only use the model name verbatim.
    sim->addJsonComponentCreator(Ping::MODEL_NAME, Ping::JsonConfig::parseJson);
    sim->addJsonComponentCreator(Pong::MODEL_NAME, Pong::JsonConfig::parseJson);
    sim->addJsonComponentCreator(PingPongBeliefSet::MODEL_NAME, PingPongBeliefSet::JsonConfig::parseJson);
    sim->addJsonComponentCreator(AgentPlanSwitch::MODEL_NAME, AgentPlanSwitch::JsonConfig::parseJson);
    sim->addJsonComponentCreator(DummyFlag::MODEL_NAME, DummyFlag::JsonConfig::parseJson);
}

bool tests::addComponentToEntity(aos::jack::Engine& engine, aos::sim::EntityWrapper entity, std::string_view componentName, const aos::sim::JsonParsedComponent *config)
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
    return false;

}
#endif /// defined(JACK_WITH_SIM)
