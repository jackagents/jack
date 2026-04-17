/// Project
#include <simple/meta/simpleproject.h>
#if defined(JACK_WITH_SIM)
#include <simple/meta/simpleservicecomponents.h>
#include <simple/meta/simpleevents.h>
#endif

/// Project
#include <simple/impl/agents/thermostatagentimpl.h>
#include <simple/impl/services/fanserviceimpl.h>
#include <simple/meta/messages/temperaturereadingmeta.h>
#include <simple/meta/messages/fancontrolmeta.h>
#include <simple/meta/messages/fanstatusmeta.h>
#include <simple/impl/goals/maintaincomfortimpl.h>
#include <simple/impl/plans/turnfanonplanimpl.h>
#include <simple/impl/plans/turnfanoffplanimpl.h>

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
    registry.registerType<TemperatureReading>(
        "simple.Temperature Reading",
        &TemperatureReading::anyToMessage,
        nullptr,
        &TemperatureReading::anyToJSON,
        nullptr,
        &TemperatureReading::anyToNlohmannJSON);
    registry.registerType<FanControl>(
        "simple.Fan Control",
        &FanControl::anyToMessage,
        nullptr,
        &FanControl::anyToJSON,
        nullptr,
        &FanControl::anyToNlohmannJSON);
    registry.registerType<FanStatus>(
        "simple.Fan Status",
        &FanStatus::anyToMessage,
        nullptr,
        &FanStatus::anyToJSON,
        nullptr,
        &FanStatus::anyToNlohmannJSON);
    registry.registerType<std::vector<TemperatureReading>>(
        "simple.Temperature Reading[]",
        nullptr,
        &TemperatureReading::anyArrayToMessage,
        nullptr,
        &TemperatureReading::anyArrayToJSON,
        &TemperatureReading::anyToNlohmannJSON);
    registry.registerType<std::vector<FanControl>>(
        "simple.Fan Control[]",
        nullptr,
        &FanControl::anyArrayToMessage,
        nullptr,
        &FanControl::anyArrayToJSON,
        &FanControl::anyToNlohmannJSON);
    registry.registerType<std::vector<FanStatus>>(
        "simple.Fan Status[]",
        nullptr,
        &FanStatus::anyArrayToMessage,
        nullptr,
        &FanStatus::anyArrayToJSON,
        &FanStatus::anyToNlohmannJSON);

    /// Create message schemas
    bdi.commitMessageSchema(&TemperatureReading::schema());
    bdi.commitMessageSchema(&FanControl::schema());
    bdi.commitMessageSchema(&FanStatus::schema());
}

static void initActions([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    bdi.action("simple.Turn Fan On")
        .request("simple.Fan Control")
        .commit();
    bdi.action("simple.Turn Fan Off")
        .request("simple.Fan Control")
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
    bdi.goal("simple.Maintain Comfort")
       .message("")
       .commit<MaintainComfort>();
}

static void initPlans([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    { /// TurnFanOnPlan
        aos::jack::CoroutineBuilder coroutine = bdi.coroutine();
        coroutine.action("simple.Turn Fan On", aos::jack::UniqueId(0x66a80c7a9a0848f7ULL, 0x811e51cf65934f40ULL));

        int32_t label0simple_TurnFanOn = {};
        coroutine.label(label0simple_TurnFanOn);

        /// \note Configure task execution graph
        coroutine.configure(label0simple_TurnFanOn).onSuccess(aos::jack::Coroutine::TERMINAL_LABEL);

        bdi.plan("simple.Turn Fan On Plan")
           .handles("simple.Maintain Comfort")
           .body(coroutine)
           .commit<TurnFanOnPlan>();
    }
    { /// TurnFanOffPlan
        aos::jack::CoroutineBuilder coroutine = bdi.coroutine();
        coroutine.action("simple.Turn Fan Off", aos::jack::UniqueId(0x865d82cabda246b1ULL, 0x88cdc7c83badfb3fULL));

        int32_t label0simple_TurnFanOff = {};
        coroutine.label(label0simple_TurnFanOff);

        /// \note Configure task execution graph
        coroutine.configure(label0simple_TurnFanOff).onSuccess(aos::jack::Coroutine::TERMINAL_LABEL);

        bdi.plan("simple.Turn Fan Off Plan")
           .handles("simple.Maintain Comfort")
           .body(coroutine)
           .commit<TurnFanOffPlan>();
    }
}

static void initServices([[ maybe_unused ]] aos::jack::Engine& bdi)
{

    { /// FanService
        const std::string_view msgList[] = {
            "simple.Fan Status"sv,
        };

        bdi.service(FanService::MODEL_NAME)
            .messageNames(msgList)
            .commit<FanService>();
    }
}

static void initTactics([[ maybe_unused ]] aos::jack::Engine& bdi)
{
}

static void initAgents([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the agent templates to the engine under their runtime name

    { /// ThermostatAgent
        const std::string_view serviceList[] = {
            "simple.Fan Service"sv,
        };
        const std::string_view planList[] = {
            "simple.Turn Fan On Plan"sv,
            "simple.Turn Fan Off Plan"sv,
        };
        const std::string_view desireList[] = {
            "simple.Maintain Comfort"sv,
        };
        const std::string_view beliefList[] = {
            "simple.Temperature Reading"sv,
        };

        /// Configure the agent
        bdi.agent(ThermostatAgent::MODEL_NAME)
            .serviceNames(serviceList)
            .planNames(planList)
            .desireNames(desireList)
            .beliefNames(beliefList)
            .commitAsAgent<ThermostatAgent>();
    }
}

static void initTeams([[ maybe_unused ]] aos::jack::Engine& bdi)
{
    /// Add the team templates to the engine under their runtime name
}

/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
simple::simple() : aos::jack::Engine()
{
    aos::jack::Engine& bdi = static_cast<aos::jack::Engine&>(*this);
    bdi.setName("simple");

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

simple::~simple() { }

/******************************************************************************
 * Functions
 ******************************************************************************/

aos::jack::AgentHandle simple::createThermostatAgent(std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto result = createAgent(ThermostatAgent::MODEL_NAME, name, uuid);
    return result;
}

ThermostatAgent* simple::createThermostatAgentInstance(std::string_view name, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<ThermostatAgent*>(createAgentInstance(ThermostatAgent::MODEL_NAME, name, uuid));
    return result;
}






aos::jack::ServiceHandle simple::createFanService(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto result = createService(FanService::MODEL_NAME, name, proxy, uuid);
    return result;
}



FanService* simple::createFanServiceInstance(std::string_view name, bool proxy, const aos::jack::UniqueId& uuid)
{
    auto* result = dynamic_cast<FanService*>(createServiceInstance(FanService::MODEL_NAME, name, proxy, uuid));
    JACK_ASSERT(result);
    return result;
}


/******************************************************************************
 * Static Functions
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
void simple::initSimModel(aos::sim::SimulationBase* sim)
{
    if (!sim) {
        return;
    }
    /// @todo: The editor uses the model name but it strips spaces from the
    /// name. Here I mangle the name of the component so its uniform. We however 
    /// should only use the model name verbatim.
    sim->addJsonComponentCreator(TemperatureReading::MODEL_NAME, TemperatureReading::JsonConfig::parseJson);
    sim->addJsonComponentCreator(FanControl::MODEL_NAME, FanControl::JsonConfig::parseJson);
    sim->addJsonComponentCreator(FanStatus::MODEL_NAME, FanStatus::JsonConfig::parseJson);
    sim->addJsonComponentCreator(FanServiceComponent::COMPONENT_NAME, FanServiceComponent::JsonConfig::parseJson);
}

bool simple::addComponentToEntity(aos::jack::Engine& engine, aos::sim::EntityWrapper entity, std::string_view componentName, const aos::sim::JsonParsedComponent *config)
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
    if (componentName == FanService::COMPONENT_NAME || componentName == aos::sim::removeSpaces(FanService::COMPONENT_NAME)) {
        auto& service = *entity.m_handle.assign<FanServiceComponent>();
        if (const auto *c = dynamic_cast<const FanServiceComponent::JsonConfig *>(config)) {
            aos::jack::UniqueId id = aos::jack::UniqueId::initFromString(c->uuid);
            if (c->uuid.size()) {
                JACK_ASSERT(id.valid());
            } else {
                id = aos::jack::UniqueId::random();
            }
            service = FanServiceComponent(entity, engine, c->templateName, c->name, c->startService, c->proxyService, id);
        }
    } else {
        handledAsService = false;
    }
    return handledAsService;

}
#endif /// defined(JACK_WITH_SIM)
