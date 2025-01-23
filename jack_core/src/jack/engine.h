#ifndef JACK_ENGINE_H
#define JACK_ENGINE_H

/*! \mainpage JACK
 *
 * This document contains commercially sensitive information and is provided in
 * confidence to licensees, and shall not be disclosed to anyone outside this
 * distribution without the prior written consent of Agent Oriented Software Pty
 * Ltd.
 *
 * \copyright © Agent Oriented Software 2020
 */

#include <jack/agent.h>                    // for Agent
#include <jack/builders/agentbuilder.h>    // for AgentBuilder
#include <jack/builders/goalbuilder.h>     // for GoalBuilder
#include <jack/builders/planbuilder.h>     // for PlanBuilder
#include <jack/builders/servicebuilder.h>  // for ServiceBuilder
#include <jack/builders/tacticbuilder.h>   // for TacticBuilder
#include <jack/engine/dispatch.h>          // for Dispatch
#include <jack/goal.h>                     // for Goal
#include <jack/fieldregistry.h>            // for FieldRegistry
#include <jack/message.h>                  // for Message, Prop, Propbase (ptr only)
#include <jack/plan.h>                     // for Plan
#include <jack/service.h>                  // for Service
#include <jack/team.h>                     // for Team
#include <jack/corelib.h>

#include <cstddef>                      // for size_t
#include <atomic>                       // for atomic
#include <chrono>                       // for milliseconds, high_resolution...
#include <condition_variable>
#include <map>                          // for map
#include <mutex>                        // for condition_variable, mutex
#include <thread>                       // for thread
#include <type_traits>                  // for forward, move
#include <utility>                      // for max, exchange, min, pair
#include <vector>                       // for vector
#include <string>                       // for string, operator<
#include <string_view>

using namespace std::literals;
using namespace std::string_view_literals;

#include <jack/event-protocol/busadapter.h>

namespace aos::jack
{
class ActionBuilder;
class CoroutineBuilder;
class MessageSchema;
class FieldRegistry;
class ProxyAgent;
class Resource;
class ResourceBuilder;
class Role;
class RoleBuilder;
struct Event;

/// \todo Move to own file, probably
struct Action
{
    std::string name;
    std::string request;
    std::string reply;
    std::string feedback;

    std::string toString() const;
};

enum ActionMessageSchemasFlag
{
    ActionMessageSchemasFlag_MISSING_REQUEST = 1 << 0,
    ActionMessageSchemasFlag_MISSING_REPLY   = 1 << 1,
};

struct ActionMessageSchemas
{
    /// ActionMessageSchemaFlag bitset representing if this struct was generated
    /// from an invalid action (e.g. one of the the required message schemas are
    /// missing)
    uint8_t invalidFlags;

    /// Request schema, null pointer if the the action did not have a request
    /// schema or, the action specified a message that does not exist.
    MessageSchema *request;

    /// Reply schema, null pointer if the the action did not have a reply
    /// schema or, the action specified a message that does not exist.
    MessageSchema *reply;
};


/*! ***********************************************************************************************
 * \class   Engine
 *
 * JACK's core BDI engine. Responsible for the execution of all BDI Agents
 * ************************************************************************************************/
class Engine : public Dispatch
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    Engine(const std::string& name = {});

    virtual ~Engine();

    /// @private
    Engine(const Engine&) = delete;

    /// @private
    Engine& operator=(const Engine&) = delete;

    /**************************************************************************
     * Functions
     **************************************************************************/
    static std::chrono::seconds heartbeatTimerPeriod() { return std::chrono::seconds(4); };

    /// @return The human readable name of this JACK node
    const std::string& name() const { return m_name; }

    /// Set the human readable name of this JACK node
    void setName(const std::string& name);

    /// Return the status of the engine ( is it running )
    bool getStatus() const { return m_running; }

    /**************************************************************************
     * Execute Functions
     **************************************************************************/
    /// Return the internal clock. The internal clock only accumulates when the engine is polled or
    /// executing
    std::chrono::high_resolution_clock::duration internalClock() const { return m_internalClock; }

    /// Return the engine's poll count, an accumulated value representing the
    /// amount of times JACK has been ticked.
    size_t pollCount() const { return m_pollCount; }

    /// @return The number of milliseconds the engine has been running, 0 if it
    /// has not been running
    std::chrono::milliseconds getUpTime() const;

    /// Execute the jack engine in it's own thread
    void start();

    /// Wait for the main engine's execution thread to end, it must only be
    /// called when execute(...) or start(...) and after exit(...) is called.
    void join();

    /// Execute the jack engine in the calling thread
    void execute();

    // When POLL_AUTO_DELTA_TIME is given to poll(...), poll will use the
    // a deltaTime based on the time the last poll(...) was called.
    static constexpr std::chrono::milliseconds POLL_AUTO_DELTA_TIME = std::chrono::milliseconds(-1);

    struct PollResult
    {
        int agentsRunning;
        int agentsExecuting;
    };

    /// Process the event queue and tick the engine a number of steps
    PollResult poll(std::chrono::milliseconds deltaTime = POLL_AUTO_DELTA_TIME);

    /// Signal the engine to quit
    void exit() { m_running = false; notify(); }

    /// Remove runtime agents/services from the engine, flush event queues and
    /// various engine states to their initial state.
    ///
    /// Templates and bus adapters are not reset and preserved in the engine.
    ///
    /// \todo Introduce enum flags to decide whether or not bus adapters get
    /// disconnected and if templates are flush or not e.t.c
    void reset();

    /// Delete an agent from the engine
    /// @return True if the matching agent was found and destroyed, false
    /// otherwise
    bool destroyAgent(const AgentHandle& handle);

    /// Configure the bdi engine to stop when there are no longer any active agents
    void exitWhenDone() { m_exitWhenNoActiveAgents = true; }

    /**************************************************************************
     * Builders
     **************************************************************************/
    /// Create a plan template builder
    /// @param[in] name The name of the template
    PlanBuilder plan(std::string_view name);

    /// Create a goal template builder
    /// @param[in] name The name of the template
    GoalBuilder goal(std::string_view name);

    /// Create an agent template builder
    /// @param[in] name The name of the template
    AgentBuilder agent(std::string_view name);

    /// Create a service template builder
    /// @param[in] name The name of the template
    ServiceBuilder service(std::string_view name);

    /// Create a service template builder
    /// @param[in] name The name of the template
    TacticBuilder tactic(std::string_view name);

    /// Create a coroutine builder
    CoroutineBuilder coroutine();

    /// Create an action template builder
    /// @param[in] name The name of the template
    ActionBuilder action(std::string_view name);

    /// Create a resource template builder
    /// @param[in] name The name of the template
    ResourceBuilder resource(std::string_view name);

    /// Create a role template builder
    /// @param name[in] The name of the template
    RoleBuilder role(std::string_view name);

    /**************************************************************************
     * Commit Functions
     **************************************************************************/
    /// Add the goal template to the engine overwriting any prior matching
    /// instance
    /// @return The goal instance belonging to the engine, null pointer if
    /// committing of the goal failed (e.g. invalid goal)
    Goal* commitGoal(const Goal* goal);

    /// Add the plan template to the engine overwriting any prior matching
    /// instance
    /// @return The plan instance belonging to the engine, null pointer if
    /// committing of the plan failed (e.g. invalid plan)
    Plan* commitPlan(const Plan* plan);

    /// Add the agent template to the engine overwriting any prior matching
    /// instance
    /// @return The agent instance belonging to the engine, null pointer if
    /// committing of the agent failed (e.g. invalid agent)
    Agent* commitAgent(const Agent* agent);

    /// Add the service template to the engine overwriting any prior matching
    /// instance
    /// @return The service instance belonging to the engine, null pointer if
    /// committing of the service failed (e.g. invalid service)
    Service* commitService(const Service* service);

    /// Add the action template to the engine overwriting any prior matching
    /// instance
    /// @return The action instance belonging to the engine, null pointer if
    /// committing of the action failed (e.g. invalid action)
    Action* commitAction(const Action* action);

    /// Add the resource template to the engine overwriting any prior matching
    /// instance
    /// @return The resource instance belonging to the engine, null
    /// pointer if committing of the resource failed (e.g. invalid resource)
    Resource* commitResource(const Resource* resource);

    /// Add the message schema template to the engine overwriting any prior
    /// matching instance
    /// @return The message schema instance belonging to the engine, null
    /// pointer if committing of the message schema failed (e.g. invalid message
    /// schema)
    MessageSchema* commitMessageSchema(const MessageSchema* schema);

    /// Add the role template to the engine overwriting any prior matching
    /// instance
    /// @return The role instance belonging to the engine, null pointer if
    /// committing of the role failed (e.g. invalid role)
    Role* commitRole(const Role* role);

    /// Add the tactic template to the engine overwriting any prior matching
    /// instance
    /// @return The tactic instance belonging to the engine, null pointer if
    /// committing of the tactic failed (e.g. invalid tactic)
    Tactic* commitTactic(const Tactic* tactic);

    /**************************************************************************
     * Create Functions
     **************************************************************************/
    /// Create a service from a template. This variant returns the raw pointer
    /// and should be used with care, external applications integrating JACK
    /// should prefer the handle which guards against invalidated instances.
    /// @param templateName The name of the service template to create the new service from
    /// @param serviceName The unique name of the service
    /// @param proxy Set the service to be a proxy which forwards its events to the concrete instance
    /// @param uuid The unique ID of the service
    /// @return A pointer to the instance of the service
    Service *createServiceInstance(std::string_view templateName, std::string_view serviceName, bool proxy, const UniqueId& uuid = UniqueId::random());

    /// Create a service from a template
    /// @param templateName The name of the service template to create the new service from
    /// @param serviceName The unique name of the service
    /// @param proxy Set the service to be a proxy which forwards its events to the concrete instance
    /// @param uuid The unique ID of the service
    /// @return A handle to the instance of the service
    ServiceHandle createService(std::string_view templateName, std::string_view serviceName, bool proxy, const UniqueId& uuid = UniqueId::random());

    /// Create an agent from a template. This variant returns the raw pointer
    /// and should be used with care, external applications integrating JACK
    /// should prefer the handle which guards against invalidated instances.
    /// @param templateName The name of the agent template to create the new agent from
    /// @param agentName The unique name of the agent
    /// @param uuid The unique ID of the agent
    /// @return A pointer to the instance of the agent
    Agent *createAgentInstance(std::string_view templateName, std::string_view agentName, const UniqueId& uuid = UniqueId::random());

    /// Create an team from the template. This variant returns the raw pointer
    /// and should be used with care, external applications integrating JACK
    /// should prefer the handle which guards against invalidated instances.
    ///
    /// Ths specified agent to instantiate must be a team otherwise, a null
    /// pointer will be returned and an assert is triggered.
    ///
    /// @param templateName The name of the agent template to create the new
    /// agent from
    /// @param agentName The unique name of the agent
    /// @param uuid The unique ID of the agent
    /// @return A pointer to the instance of the agent
    Team *createTeamInstance(std::string_view templateName, std::string_view agentName, const UniqueId& uuid = UniqueId::random());

    /// Create an agent from a template
    /// @param templateName The name of the agent template to create the new agent from
    /// @param agentName The unique name of the agent
    /// @param uuid Apply a custom unique Id to the agent
    /// @return A handle to the instance of the agent
    AgentHandle createAgent(std::string_view templateName, std::string_view agentName, const UniqueId& uuid = UniqueId::random());

    /// Enqueue the creation of an agent to the next engine ick.
    /// @param templateName The name of the agent template to create the new agent from
    /// @param agentName The unique name of the agent
    /// @param uuid Apply a custom unique Id to the agent
    /// @param start Whether or not the agent should be started when it is
    /// created.
    /// @param team (Optional) The team that the agent should become a member of
    /// when it is created.
    /// @return A handle to the instance of the agent. This handle is invalid
    /// until the agent is instantiated by the engine.
    AgentHandle queueCreateAgent(std::string_view templateName, std::string_view agentName, const UniqueId &uuid, bool start, const AgentHandle *team);

    /// Create a proxy agent
    /// \todo Better explanation of what this does
    ProxyAgent* createProxyAgent(std::string_view templateName, std::string_view agentName);
    ProxyAgent* createProxyAgent(std::string_view templateName, std::string_view agentName, const UniqueId& uuid);

    /// Create a message from a message schema
    ///
    /// This message will be instantiated with the fields specified in the
    /// message schema. Schema's must be registered prior to this call using
    /// MessageBuilder or commitMesageSchema().
    ///
    /// @param[in] schema The name of the schema to instantiate the message from
    /// @return The created message, Message::valid() returns false if the
    /// message schema does not exist and instantiation failed.
    /// @todo make this a std::unique_ptr<Message>
    std::unique_ptr<Message> createMessage(std::string_view schema);

    /// Create a resource from a template
    /// @param templateName The name of the template to create the new resource from
    Resource *createResource(std::string_view templateName);

    /**************************************************************************
     * Get Functions
     **************************************************************************/
    /// @return the service associated with the given handle or null
    Service* getService(const ServiceHandle& handle) const;

    /// @return the service associated with the given handle or null
    Service* getServiceByUUID(const UniqueId& uuid) const;

    /// Query a saved agent by their uuid
    /// @return Null pointer if the agent is no longer available
    Agent* getAgent(const AgentHandle& handle) const;

    /// Query a saved agent by their uuid
    Agent* getAgentByUUID(const UniqueId& uuid) const;

    /// Get the list of committed agent templates in the engine
    const std::map<std::string, Agent*, std::less<>>& getAgentTemplates() const { return m_agentTemplates; }

    /// Get the list of committed goal templates in the engine
    const std::map<std::string, Goal*, std::less<>>& getGoalTemplates() const { return m_goals; }

    /// Get the list of committed plan templates in the engine
    const std::map<std::string, Plan*, std::less<>>& getPlanTemplates() const { return m_plans; }

    /// Get the list of committed resource templates in the engine
    const std::map<std::string, Resource, std::less<>>& getResourceTemplates() const { return m_resourceTemplates; }

    /// Get the list of committed role templates in the engine
    const std::map<std::string, Role, std::less<>>& getRoleTemplates() const { return m_roleTemplates; }

    /// Get the list of committed message templates in the engine
    const std::map<std::string, MessageSchema, std::less<>>& getMessageTemplates() const { return m_messageSchemas; }

    /// Get the list of committed service templates in the engine
    const std::map<std::string, Service*, std::less<>>& getServiceTemplates() const { return m_serviceTemplates; }

    /// Get the list of committed tactic templates in the engine
    const std::map<std::string, Tactic, std::less<>>& getTacticTemplates() const { return m_tacticTemplates; }

    /// Get the backing goal by name
    /// @param name The name of the goal to query
    const Goal* getGoal(std::string_view name) const;

    /// Get the backing plan by name
    /// @param name The name of the plan to query
    const Plan* getPlan(std::string_view name) const;

    /// Get the backing role by name
    /// @param name The name of the role to query
    const Role* getRole(std::string_view name) const;

    /// Get the backing tactic by name
    /// @param name The name of the tactic to query
    const Tactic* getTactic(std::string_view name) const;

    /// Get the builtin tactics that is created by the engine by default on goal
    /// commit.
    ///
    /// @return Handle to the tactic. The handle is valid if the tactic exists
    /// (all goals create their builtin tactic on commit) otherwise the handle
    /// returned will be invalid.
    TacticHandle getBuiltinTactic(std::string_view goal) const;

    /// Get the backing action by name
    /// @param name The name of the action to query
    const Action* getAction(std::string_view name) const;

    /// Retrieve the message schema(s) for the request and reply message
    /// associated with the action.
    ActionMessageSchemas getActionMessageSchema(const Action* action);

    /// Query a message schema by name
    /// @return The message schema, otherwise null pointer if it does not exist.
    const MessageSchema* getMessageSchema(std::string_view name) const;

    /// @copydoc getMessageSchema
    MessageSchema* getMessageSchema(std::string_view name);

    /// Query the list of saved agent names
    std::vector<AgentHandle> agentList() const;

    /// Query the list of runtime agents
    const std::vector<Agent*>& agents() const { return m_agents; }

    /// Query the list of runtime agents
    std::vector<Agent*>& agents() { return m_agents; }

    /// Query the list of saved service names
    std::vector<ServiceHandle> serviceList() const;

    /// Query the list of runtime services
    const std::vector<Service*>& services() const { return m_services; }

    /// Query the list of runtime services
    std::vector<Service*>& services() { return m_services; }

    /// Query the list of saved tactics
    /// @return The list of tactics by name
    /// @todo return a list of tactics by id
    std::vector<std::string> tacticList() const;

    /**************************************************************************
     * Bus Functions
     **************************************************************************/
    /// Add a bus adapter for sending and receiving events generated by this
    /// and other connected JACK instances respectively.
    /// @param adapter
    void addBusAdapter(BusAdapter* adapter);

    /// Remove a bus adapter from the engine
    /// @param adapter to remove
    void removeBusAdapter(const BusAdapter* adapter);

    /// Send a protocol event to all buses
    /// @param event The event to send
    void sendBusEvent(protocol::Event* event);

    /// Setup a protocol event from `protocol.h` with the header filled out with
    /// this engine's bus addresses instance.
    template <typename T>
    T makeProtocolEvent(const protocol::BusAddress& sender    = {},
                        const protocol::BusAddress& recipient = {},
                        const UniqueId& eventId               = UniqueId::random());

    protocol::BDILog makeBDILogGoalStarted(const protocol::BDILogHeader& header,
                                           std::string_view              goal,
                                           std::string_view              goalId);

    protocol::BDILog makeBDILogGoalFinished(const protocol::BDILogHeader&       header,
                                            std::string_view                    goal,
                                            std::string_view                    goalId,
                                            std::string_view                    dropReason,
                                            protocol::BDILogGoalIntentionResult intentionResult);

    protocol::BDILog makeBDILogSubGoalStarted(const protocol::BDILogHeader& header,
                                              std::string_view              goal,
                                              std::string_view              goalId,
                                              std::string_view              intentionId,
                                              std::string_view              taskId);

    protocol::BDILog makeBDILogSubGoalFinished(const protocol::BDILogHeader&       header,
                                               std::string_view                    goal,
                                               std::string_view                    goalId,
                                               std::string_view                    intentionId,
                                               std::string_view                    taskId,
                                               std::string_view                    dropReason,
                                               protocol::BDILogGoalIntentionResult intentionResult);

    protocol::BDILog makeBDILogIntentionStarted(const protocol::BDILogHeader& header,
                                                std::string_view              goal,
                                                std::string_view              goalId,
                                                std::string_view              intentionId,
                                                std::string_view              plan);

    protocol::BDILog makeBDILogIntentionFinished(const protocol::BDILogHeader&       header,
                                                 std::string_view                    goal,
                                                 std::string_view                    goalId,
                                                 std::string_view                    intentionId,
                                                 std::string_view                    plan,
                                                 protocol::BDILogGoalIntentionResult intentionResult);

    protocol::BDILog makeBDILogActionStarted(const protocol::BDILogHeader& header,
                                             std::string_view              goal,
                                             std::string_view              goalId,
                                             std::string_view              intentionId,
                                             std::string_view              plan,
                                             std::string_view              taskId,
                                             std::string_view              action);

    /// \todo Complete the conversion of other functions to take in native JACK
    /// types instead of destructuring from the call-site.
    protocol::BDILog makeBDILogActionFinished(const protocol::BDILogHeader& header,
                                              const struct ActionEvent*     event,
                                              std::string_view              reasoning);

    protocol::BDILog makeBDILogSleepStarted(const protocol::BDILogHeader& header,
                                            std::string_view              goal,
                                            std::string_view              goalId,
                                            std::string_view              intentionId,
                                            std::string_view              plan,
                                            std::string_view              taskId,
                                            uint64_t                      sleepMs);

    protocol::BDILog makeBDILogSleepFinished(const protocol::BDILogHeader& header,
                                             std::string_view              goal,
                                             std::string_view              goalId,
                                             std::string_view              intentionId,
                                             std::string_view              plan,
                                             std::string_view              taskId);

    protocol::BDILog makeBDILogConditionResult(const protocol::BDILogHeader& header,
                                               std::string_view              goal,
                                               std::string_view              goalId,
                                               std::string_view              intentionId,
                                               std::string_view              plan,
                                               std::string_view              taskId,
                                               std::string_view              condition,
                                               bool                          success);

    /// @return List of bus adapters currently attached to the engine
    std::vector<BusAdapter*>& busAdapters() { return m_busAdapters; }

    /// @return if the engine has at least one bus adapter currently
    bool haveBusAdapter() const { return m_busAdapters.size(); }

    struct BusAddressableEntity
    {
        std::chrono::milliseconds m_lastMessageClockTime;
    };

    /// The bus directory containing a list of bus addresses that this node is
    /// aware of.
    const std::unordered_map<protocol::BusAddress, BusAddressableEntity> &busDirectory() const { return m_busDirectory; }

    /// Dump the bus directory to a string for printing.
    std::string dumpBusDirectory() const;

    /// @return An address that this entity can be
    /// contacted on if messages are addressed to this on the bus.
    const protocol::BusAddress& busAddress() const { return m_busAddress; }

    /// All events that are *last* handled by this agent is allocated through
    /// the chunk allocator (i.e. delegation events go from team -> agent ->
    /// team, so the team handles/owns that event).
    ChunkAllocator m_eventAllocator;

    struct Settings {
        /// When an agent executes an action, if it's not handled by the agent
        /// directly, this flag allows the engine to search the global
        /// list of services and forward the action to the first service that
        /// can handle the action on behalf of the agent.
        ///
        /// If set false, then, actions can be only be handled by the agent and
        /// or any compatible service that has explicitly been attached to the
        /// agent.
        ///
        /// To preserve behaviour as per version <=0.4.X, this flag should be
        /// set to true.
        bool unhandledActionsForwardedToFirstApplicableService = true;
        
    } settings;

private:
    /**************************************************************************
     * Private Functions
     **************************************************************************/
    /// Queue an event onto the engine's event queue
    void eventDispatch(Event *event);

    bool protocolEventHandler(protocol::Event *event);

    /// Return an event to the calling agent
    void returnEvent(Event *event);

    /// Let the engine know there is work to be done
    void notify() { m_condition.notify_all(); }

    /// Internally called before createAgent and createProxyAgent are complete.
    void finishCreateAgent(std::string_view templateName, Agent* agent, const UniqueId* uuid);

    // All the agent templates saved to the engine
    std::map<std::string, Agent*, std::less<>> m_agentTemplates;

    // All the service templates saved to the engine
    std::map<std::string, Service*, std::less<>> m_serviceTemplates;

    // All the resource templates saved to the engine
    std::map<std::string, Resource, std::less<>> m_resourceTemplates;

    /// All the role templates saved to the engine
    std::map<std::string, Role, std::less<>> m_roleTemplates;

    /// All the tactic templates saved to the engine
    std::map<std::string, Tactic, std::less<>> m_tacticTemplates;

    /// Stores the default tactic for each goal. These tactics are created on
    /// goal commit and are used if a goal is pursued without an equipped
    /// tactic.
    std::map<std::string /*goal name*/, TacticHandle, std::less<>> m_builtinTacticTemplates;

    /// Action templates saved to the engine
    std::map<std::string, Action, std::less<>> m_actions;

    /// All the plans saved to the engine
    std::map<std::string, Plan*, std::less<>> m_plans;

    /// All the goals saved to the engine
    std::map<std::string, Goal*, std::less<>> m_goals;

    /// All the agents saved to the engine
    std::vector<Agent*> m_agents;

    /// All the service running in this engine instance
    std::vector<Service*> m_services;

    /// All the message schemes saved to the engine
    std::map<std::string, MessageSchema, std::less<>> m_messageSchemas;

    std::condition_variable m_condition;

    std::atomic<bool> m_running;

    /// When true, the poll and execute functions will terminate early if no
    /// agents are running
    bool m_exitWhenNoActiveAgents;

    /// The thread running the engine when start(...) is called
    std::thread m_mainEngineThread;

    /// \todo make lock free with events
    /// \todo Revise the mutex, JACK is not multithreaded yet it's meant to
    /// guard the public API from multi-thread access but not all functions
    /// abide by this yet.
    ///
    /// We also have internal functions that call the public API and previously,
    /// doubly locks the API causing a hang. A recurive mutex is a fairly safe
    /// option in that it allows recurrent accesses within the *same* thread,
    /// just that it encourages bad design.
    std::recursive_mutex m_guardAPI;

    /// The name of JACK node
    std::string m_name;

    /// An internal clock that only accumulates when the engine is being executed or ticked.
    std::chrono::high_resolution_clock::duration m_internalClock = {};

    /// A countdown timer for emitting a heartbeat onto the bus
    std::chrono::microseconds m_heartbeatTimer;

    /// When the engine is running via execute, this duration represents how
    /// long to sleep when there's no agents executing and running.
    std::chrono::milliseconds m_onIdleSleepDuration = std::chrono::milliseconds(100);

    std::mutex m_onIdleSleepMutex;

    /// Accumulates the number of times to JACK engine has been ticked
    size_t m_pollCount;

    /// The last time the engine was polled
    std::chrono::high_resolution_clock::time_point m_lastPollTimePoint;

    std::chrono::system_clock::time_point m_startTime;

    /// Critical errors that have occurred which will prevent the engine from starting
    std::vector<std::string> m_criticalErrors;

    /// The list of curent bu adapters
    std::vector<BusAdapter*> m_busAdapters;

    /// A directory of registered entities on the bus this engine is aware of
    std::unordered_map<protocol::BusAddress, BusAddressableEntity> m_busDirectory;

    /// A unique identifier for the node
    UniqueId m_id = UniqueId::random();

    /// The bus address that this entity is contactable from
    protocol::BusAddress m_busAddress;

    /// The timestamp of the last protocol event that was sent. This is used for debugging to track.
    std::chrono::microseconds m_lastProtocolTimestampUs = {};

#if defined(JACK_SHARED_MEMORY_DEBUGGING)
    // the shared memory model for this engine
    shared::SharedMemoryModel m_shared;
#endif

    /// \todo This is a work-around for the simulator. When the simulator starts
    /// up and loads a scenario, it ticks the scenario exactly once to process
    /// all the events that are queued to be executed at time = 0.
    ///
    /// This is currently necessary to do because typically what exists at
    /// time = 0 is all the events that construct the runtime agent model and
    /// entities.
    ///
    /// If we don't process these events, what ends up happening is the
    /// simulator has no entities initialised and so you are presented with a
    /// blank screen. Playing the scenario would then cause the entities to be
    /// populated on the screen.
    ///
    /// So instead the simulator executes all those T = 0 events when it loads
    /// the scenario so that all the initial entities are visible on screen
    /// providing immediate feedback when a user loads a scenario.
    ///
    /// The side effect of this is these events are loaded and processed which
    /// may include JACK events. These will produce bus events that get shot
    /// onto the network.
    ///
    /// The simulator has a feature to load scenarios on startup which means it
    /// can initialise the scenario and submit bus-events even before any
    /// websocket clients are attached meaning they miss crucial events that
    /// form the agent runtime model.
    ///
    /// One work-around for this is that we remove the loading a scenario on
    /// startup. Ensure that the clients are connected before a scenario is
    /// loaded. This is ok but the UX sucks.
    ///
    /// Alternatively, we need to implement the snapshot protocol that allows
    /// the client, no matter how late they connect to JACK get an up to date
    /// situational picture of the current scenario.
    ///
    /// In lieu of that, a cheap work-around that allows the scenario to
    /// continue to be loaded on startup is to capture the first ticks worth of
    /// events into this backlog here. Websocket clients must connect before
    /// the scenario is played. When the scenario is played, this backlog will
    /// then contain all the events that were meant to trigger at T = 0, and
    /// they will be flushed out first prior to the subsequent events for that
    /// frame.
    ///
    /// This list will be flushed and cleared and no longer accumulate any
    /// further events until the next reset.
    std::vector<protocol::Event*> m_busEventOnSimStartupBacklog;

    friend class Agent;
    friend class Message;
    friend class Team;
    friend class Service;
};

template <typename T>
T Engine::makeProtocolEvent(const protocol::BusAddress& sender, const protocol::BusAddress& recipient, const UniqueId& eventId)
{
    /// \todo This is stupid, we should not have to encode this. It's always the
    /// case, we should aim to make the transport layer as compact as possible.
    JACK_ASSERT(m_busAddress.type == protocol::NodeType_NODE);

    T result           = {};
    result.senderNode  = m_busAddress;
    result.eventId     = eventId;
    result.sender      = sender;
    result.recipient   = recipient;
    return result;
}
} // namespace aos::jack
#endif
