#ifndef JACK_SERVICE_H
#define JACK_SERVICE_H

#include <jack/event-protocol/protocol.h>
#include <jack/handles.h>             // for ServiceHandle
#include <jack/engine/dispatch.h>     // for Dispatch
#include <jack/engine/uuid.h>         // for UniqueId
#include <jack/events/event.h>        //
#include <jack/events/perceptevent.h> //
#include <jack/corelib.h>           //

#include <string>                // for string, operator<
#include <vector>                // for vector
#include <map>                   //

namespace aos { namespace jack {

class Engine;
class Message;

struct ActionEvent;

/*!
 * @brief A JACK Service that allows Agent's to interface with the external
 * environment through actions and percepts.
 *
 * Agents can send actions to a Service to enact some change in the environment
 * and perceive changes in the environment by percepts generated from the
 * Service.
 */
class Service : public Dispatch
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    Service(Engine& engine, std::string_view name);

    Service(const Service* other, std::string_view newName);

    virtual ~Service();

    Service(const Service &other) = delete; ///< @private

    Service& operator=(const Service &other) = delete; ///< @private

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Get a lightweight non-owning handle referencing this agent.
    const ServiceHandle& handle() const { return m_handle; }

    /// Retrieve the JACK engine instance this service belongs to.
    /// @return The JACK engine instance.
    Engine &engine() { return m_engine; }

    const Engine &engine() const { return m_engine; }

    /// Query the running uptime of this service.
    /// @return The number of milliseconds this service has been running for.
    std::chrono::milliseconds getUpTime() const;

    /// The list of beliefsets this service has.
    /// @return The list of BeliefSet names this service supports.
    const std::vector<std::string>& beliefSetNames() const { return m_beliefsetIds; }

    /// The function signature required to handle incoming actions.
    using ActionHandlerFunc = std::function<Event::Status(Service&, Message&, Message&, ActionHandle)>;

    using ActionHandlerTable = std::map<std::string, ActionHandlerFunc, std::less<>>;

    /// The list of actions that this service can handle.
    /// @return A read only list of action handlers this service supports.
    const ActionHandlerTable actionHandlers() const { return m_actionHandlers; }

    /// Queue the service to start on the next tick of the engine.
    void start();

    /// Queue the service to stop on the next tick of the engine. The service
    /// will transition into a stopping state where all desires and intentions
    /// are dropped before entering State::STOPPED.
    void stop();

    /// Queue the service to pause on the next tick of the engine.
    void pause();

    /// Query if the service is currently stopped.
    /// @return True if this service is in the stopped state.
    bool stopped() const { return m_state == Service::STOPPED; }

    /// Query if the service is currently running.
    /// @return True if this service is still executing code (e.g. running).
    bool running() const { return m_state == Service::RUNNING || m_state == Service::STOPPING; }

    bool stopping() const { return m_state == Service::STOPPING; }

    /// Query if the service is currently paused.
    /// @return True if this service is in the paused state.
    bool paused() const { return m_state == Service::PAUSED; }

    // The states that the service can be in
    enum State
    {
        STOPPED,  ///< All desires and intentions have been dropped and no new ones can be started.
        STOPPING, ///< Desires and intentions are signalled to stop and are in the process of stopping
        RUNNING,  ///< Desires and intentions are accepted and executed.
        PAUSED,   ///< Desires and intentions are paused and can be resumed later.
    };

    /// Query the execution state of the service.
    /// @return The state that the service is in.
    State state() const { return m_state; }

    /// Set the state of the service
    /// @return True if the state transitioned, false if the agent was already
    /// in the state requested.
    bool setState(State state);

    /// Query the name of the service.
    /// @return The name of the service
    const std::string& name() const { return m_handle.m_name; }

    /// Set the service's unique identifier. The identifier is set irrespective
    /// of if the id is valid or not.
    /// @param[in] id The identifier to set.
    void setUUID(const UniqueId &id);

    /// Query the unique identifier of this service.
    /// @return The unique identifier of the service.
    const UniqueId &UUID() const { return m_handle.m_id; }

    /// Query whether or not this service is currently a proxy instance.
    /// @return True if this instance is a proxy instance, false otherwise.
    bool isProxy() const { return m_isProxy; }

    /// Set the service to be a proxy instance. Future messages sent to this
    /// instance will be forwarded on the bus to the concrete instance if there
    /// is one available otherwise the messages are dropped.
    /// @param[in] isProxy The proxy state to assign to this instance.
    void setProxy(bool isProxy) { m_isProxy = isProxy; }

    /// Query the BusAddress of this service.
    /// @return An address that this entity can be contacted on if messages are
    /// addressed to this address on the bus.
    const protocol::BusAddress& busAddress() const { return m_busAddress; }

    /// Make a BDI log header for this instance
    protocol::BDILogHeader bdiLogHeader(protocol::BDILogLevel level, const UniqueId& id = UniqueId::random()) const;

    /// Set if the service is available to be contacted.
    void setAvailability(bool available);

    /// \todo Currently unused
    /// @return True if the service is currently available
    bool isAvailable() const { return m_isAvailable; }

    /// Create a percept event that is sent to all agents linked to this
    /// service.
    /// param[in] message Name of the message (or belief) to send a percept for.
    /// param[in] key     Name of the field in the message to update
    /// param[in] value   The new value of the field
    void percept(std::string_view message, std::string_view key, const std::any& value);

    /// Send a message to update an agent's beliefs.
    /// @param[in] msg The message to send
    /// @param[in] broadcastToBus When true the message will be pushed onto the
    /// bus if a bus adapter is available on the next tick
    void sendMessage(std::shared_ptr<Message> msg, bool broadcastToBus, Service *recipient = nullptr);

    /// Query if this service can handle the queried action.
    /// @param[in] action The action to check.
    /// @return True if this service handles the action, false otherwise.
    bool handlesAction(const std::string& action) const { return m_actionHandlers.find(action) != m_actionHandlers.end(); }

    /// Mark the action represented by the action handle as having completed
    /// successfully or not.
    /// @param[in] handle The action to finish.
    /// @param[in] success Flag to indicate whether or not the action handle was
    /// completed successfully or not.
    /// @param[in] reply Reply message that is applied to the action and
    /// consequently written into the plan context for subsequent actions to
    /// use.
    bool finishActionHandle(const ActionHandle& handle, bool success, const jack::Message* reply = nullptr);

    /// Get the allocator used for constructing events in the service.
    /// \todo This is temporary due to a cyclical include with the percept()
    /// function requiring accessing the engine's allocator- but the engine
    /// depends on Service, and us accessing the engine's allocator makes us
    /// dependent on the Engine.
    /// @return The chunk allocator for events.
    ChunkAllocator* eventAllocator();

protected:
    /// Actions that are marked success/failed are dispatched to the event
    /// queue. No-op if the action is not complete yet.
    /// @param[in] action The completed action event
    /// @return True if the action is marked complete and an event was
    /// dispatched to the queue.
    void processCurrentActions();

    /// Handle completion of the given action by creating the matching action 
    /// complete event.
    bool processCompletedAction(ActionEvent* action);

    // @return True is this service handles the given action name
    bool handlesAction(std::string_view action) const { return m_actionHandlers.find(action) != m_actionHandlers.end(); }

    /// Notify the engine to wake up if it is currently in a blocking sleep.
    void notifyScheduler();

    /// Execute 1 iteration of the BDI process for this service.
    virtual void run();

    /// Handle an incoming event, rerouting the event if necessary to the
    /// correct recipient.
    /// @param[in] event The event to handle.
    virtual void eventDispatch(Event *event) override;

    /// Post an event to this service's dispatch queue for handling on the
    /// next run() step.
    /// @param[in] event The event to add.
    /// @return A promise that is triggered when the event is marked as having
    /// succeeded or failed.
    PromisePtr addEvent(Event *event);

    /// Clone this instance of the Service. The clone will generate a new
    /// UniqueId for cloned instance.
    /// @param[in] name The new name of the cloned agent.
    /// @return A clone of this service with the new name
    virtual Service *clone(std::string_view name) const { return JACK_NEW(Service, this, name); }

    /// Setup a custom handler for an action
    /// @param name The name of the action that when triggered will call the handler
    /// @param func The handler/callback to be triggered when an action is requested
    void addActionHandler(const std::string &name, const ActionHandlerFunc &func) { m_actionHandlers[name] = func; }

    virtual void onStart() { /* default nothing*/ }

public:
    /**************************************************************************
     * Member Variables
     **************************************************************************/
    /// The primary heap allocator for the agent for generic dynamic allocations
    /// that have non-linear object lifetimes.
    HeapAllocator m_heapAllocator;

    /// The allocator for per-tick lifetime objects.
    /// \todo This is currently unused but would be helpful in all the spots
    /// where we construct temporary vectors or strings that live in a frame,
    /// for this though we need to expose a std::allocator compatible interface
    /// from the arena. We may also want much saner StaticString/StaticArray
    /// classes that represent non-growing strings/objects.
    ArenaAllocator m_tickAllocator;

protected:
    /// The list of concrete actions the service is currently trying to execute
    std::vector<ActionEvent*> m_currentActions;

    /// The engine running this service
    Engine &m_engine;

    /// The handle for this service
    ServiceHandle m_handle;

    /// The current service state
    std::atomic<State> m_state;

    /// The bus address that this entity is contactable from
    protocol::BusAddress m_busAddress;

    /// Map the name of an action to its handler
    ActionHandlerTable m_actionHandlers;

    /// Stores the belief set names the service will support
    std::vector<std::string> m_beliefsetIds;

    /// The start time of this service
    std::chrono::system_clock::time_point m_startTime;

    friend class Engine;
    friend class ServiceBuilder;

    /// Stop command requested, the agent will stop future planning and begin
    /// transitioning to STOPPED state.
    /// \todo consider this should just be another state, see m_state
    bool m_isStopping = false;

    /// The template that the service was instantiated from.
    std::string m_templateName;

private:
    /// Flag to determine if the service is available for interacting with.
    /// \todo Currently unused.
    std::atomic<bool> m_isAvailable;

    /// True if this instance is a proxy for the real version. Commands routed
    /// through a proxy instance will be redirected to the concrete instance.
    bool m_isProxy = false;

    friend class Engine;

    friend class ServiceBuilder;

    friend class Agent;
};

}} // namespace aos::jack

#endif
