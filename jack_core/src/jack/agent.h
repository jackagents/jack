#ifndef JACK_AGENT_H
#define JACK_AGENT_H

#include <jack/agentexecutor.h>    // for AgentExecutor, AgentExecutor::RunningState
#include <jack/corelib.h>
#include <jack/beliefcontext.h>    // for BeliefContext
#include <jack/engine/dispatch.h>  // for Dispatch
#include <jack/engine/uuid.h>      // for UniqueId
#include <jack/handles.h>          // for AgentHandle
#include <jack/promise.h>          // for PromisePtr
#include <jack/schedule.h>
#include <jack/service.h>          // for Service
#include <jack/tactic.h>           // for Tactics

#include <chrono>      // for milliseconds, seconds, system_clock
#include <cstdint>     // for uint8_t
#include <functional>  // for function
#include <list>
#include <map>         // for map, map<>::mapped_type
#include <memory>      // for shared_ptr, unique_ptr, static_pointer_...
#include <mutex>       // for mutex
#include <queue>       // for priority_queue
#include <string>      // for string, operator<
#include <vector>      // for vector

#define AGENT_REMOVE_IMPOSSIBLE_GOALS_AFTER_SCHEDULING_WORK_AROUND 1

namespace aos::jack
{
class Engine;
class Goal;
class GoalBuilder;
class IntentionExecutor;
class Plan;
class Schedule;
class Task;
class Team;
struct DelegationEvent;
struct AuctionEventBid;
struct Event;
struct TimerEvent;
struct TacticHandle;

/*! ***********************************************************************************************
 * \class   CompareTimerEvent
 * Sorting mechanism to sort the timer events into the agent's priority queue
 * ************************************************************************************************/
class CompareTimerEvent
{
public:
    bool operator() (const TimerEvent *a, const TimerEvent *b);
};
/*! ***********************************************************************************************
 * @class Agent
 *
 * @brief The core BDI Agent Entity
 *
 * ************************************************************************************************/
class Agent : public Service
{
public:
    /**************************************************************************
     * Constructor & destructor
     **************************************************************************/
    Agent(Engine& engine, std::string_view name);

    Agent(const Agent* other, std::string_view newName);

    virtual ~Agent();

    /// @private
    Agent(const Agent &other) = delete;

    /// @private
    Agent& operator=(const Agent &other) = delete;

    /**************************************************************************
     * Public Functions
     **************************************************************************/
    /// @return Read only list of plans this agent can use
    const std::vector<std::string>& plans() const { return m_plans; }

    /// @return Read only list of roles this agent supports
    const std::vector<std::string>& roles() const { return m_roles; }

    /// @return Read only list of resources this agent supports
    const std::vector<std::string>& resources() const { return m_resourceIds; }

    /// @return List of goals this agent supports
    std::vector<std::string> goals() const;

    /// @return A list of the goals this agent desires
    std::vector<std::string> desires() const;

    /// @return True if the agent has a desire with the given id
    const Goal* getDesire(const UniqueId &id) const;

    /// @return True if the agent has a desire with the given id
    Goal* getDesire(const UniqueId &id);

    /// @return True if the agent has a desire with the given name
    const Goal* getDesire(std::string_view name) const;

    /// @return True if the agent has a desire with the given name
    Goal* getDesire(std::string_view name);

    /// @return A read only list of the initial desires this agent will pursue
    const std::vector<std::string>& initialDesires() const { return m_initialDesires; }

    /// @return A read only list of the resource to goal (triggered when resource is violated) mapping
    const std::map<std::string, std::string>& resourcesToGoals() const { return m_resourcesToGoals; }

    /// The action handler type
    using ActionHandlerFunc = std::function<Event::Status(Agent&, Message&, Message&, ActionHandle)>;

    using ActionHandlerTable = std::map<std::string, ActionHandlerFunc, std::less<>>;

    /// @return A read only list of action handlers this agent supports
    const ActionHandlerTable &actionHandlers() const { return m_actionHandlers; }

    /// @return A read only list of message handlers this agent supports
    const std::map<std::string, std::function<void(Agent&, const Message&)>> &messageHandlers() const { return m_messageHandlers; }

    /// Query the list of minimum set of services required by this agent to be
    /// able to enact all goals that this agent is capable of achieving.
    /// \todo This should not be in the agent, we should query the template from
    /// the agent.
    /// @return List of the minimum set of services that must be attached to
    /// this agent for full functionality.
    const std::vector<std::string>& serviceNames() const { return m_services; }

    /// @return The current tactic for a goal
    const Tactic* currentTactic(std::string_view goal) const;

    /// @return The list of belief sets this agent uses
    const std::vector<std::string>& beliefSetNames() const { return m_beliefsetIds; }

    /// @return The list of intentions this agent is running
    Span<const IntentionExecutor*> intentions() const;

    const AgentExecutor *executor() const { return &m_executor; }

    const IntentionExecutor *intentionBeingExecuted() const { return m_executor.intentionBeingExecuted(); }

    void forceReschedule() { m_scheduleDirty |= ScheduleDirty_FORCE; }

    /// Query the list of Servics attached to the agent.
    /// @return The list of Servics currently attached to the agent.
    const std::vector<ServiceHandle>& attachedServices() const { return m_attachedServices; }

    /// Link a Service to this agent which allows the agent to forward any
    /// unhandled actions to it if it is capable of handling the action.
    ///
    /// If the agent has a pre-existing Service attached with the same type
    /// (e.g. action capabilities) this function will not overwrite the
    /// pre-existing attachment unless 'force' is set.
    ///
    /// Duplicate Service names with different action capabilities (e.g.
    /// different Service type) are permitted but discouraged to aid
    /// explainability.
    ///
    /// @param[in] service The handle to the Service to associate with the
    /// agent.
    /// @param[in] force Flag to indicate if the given service must always be
    /// attached if there is a pre-existing service that is a different instance
    /// but has the same type.
    bool attachService(const ServiceHandle& service, bool force);

    /// Unlink a Service from the agent.
    /// @param[in] service The handle to the Service to disassociate with the
    /// agent.
    bool detachService(const ServiceHandle& service);

    /// Instruct the agent to pursue a goal (e.g. attempt to achieve it)
    /// @param goal The name of the goal to pursue
    /// @param persistent The goal is permanently desired and reattempted even 
    /// if the goal succeeds or fails.
    /// plan selection policy
    /// @param parameters (Optional) The starting parameters to parameterise the
    /// execution of the goal with
    /// @param id The unique identifier to associate with the goal
    /// @return A promise the is triggered when the pursuit has succeeded or failed
    GoalPursue pursue(std::string_view goal,
                      GoalPersistent   persistent,
                      std::shared_ptr<Message> parameters = {},
                      const UniqueId&  id         = UniqueId::random());

    /// Instruct the agent to pursue a goal (e.g. attempt to achieve it)
    /// @param goal The goal to pursue
    /// @param persistent The goal is permanently desired and reattempted even 
    /// if the goal succeeds or fails.
    /// @param parameters (Optional) The starting parameters to parameterise the
    /// execution of the goal with
    /// @param id The unique identifier to associate with the goal
    /// @return A promise the is triggered when the pursuit has succeeded or failed
    GoalPursue pursue(const GoalBuilder& goal,
                      GoalPersistent     persistent,
                      std::shared_ptr<Message> parameters = {},
                      const UniqueId&    id         = UniqueId::random());

    /// Instruct the agent to pursue a goal (e.g. attempt to achieve it) and
    /// mark the intention that triggered the (sub) goal.
    /// @param goal The name of the goal to pursue
    /// @param parent The intention that triggered this pursuit of a goal. If an
    /// intention has not triggered this goal `IntentionExecutor::NULL_ID` can
    /// be used.
    /// @param persistent The goal is permanently desired and reattempted even
    /// if the goal succeeds or fails.
    /// @param parameters (Optional) The starting parameters to parameterise the
    /// execution of the goal with
    /// @param id (Optional) The unique identifier to associate with the goal
    /// @param parentTaskId (Optional) If the goal was triggered as a sub-goal
    /// pass in the ID of the task in the plan that requested this sub-goal for
    /// metrics and tracking purposes.
    /// @return A promise the is triggered when the pursuit has succeeded or
    /// failed
    GoalPursue pursueSub(std::string_view      goal,
                         IntentionExecutor::Id parent,
                         GoalPersistent        persistent,
                         std::shared_ptr<Message> parameters,
                         const UniqueId*       id,
                         const UniqueId*       parentTaskId);

    /// Sets a current tactic for the agent
    /// The agent can have multiple tactics active but only one per goal
    /// @todo We should use an id for the tactic selection and not a string
    bool selectTactic(std::string_view tactic);

    /// Drop a goal with the ability to set the mode of the drop
    /// @param handle[in] The handle of the goal to drop
    /// @param dropMode[in] Specify the behavior of the drop, force drops allow
    /// permit dropping of persistent goals, otherwise only non-persistent goals
    /// can be dropped.
    /// @param reason[in] An optional reason explaining why the drop occurred
    void dropWithMode(const GoalHandle& handle, protocol::DropMode dropMode, std::string_view reason = "");

    /// Drop a goal, forcibly, using its handle
    /// @param handle[in] The handle of the goal to drop
    /// @param reason[in] An optional reason explaining why the drop occurred
    void drop(const GoalHandle& handle, std::string_view reason = "");

    /// @return The state of the executor collectively determined by all the
    /// intentions that are currently running in this executor since the last
    /// invocation to execute(...)
    AgentExecutor::RunningState runningState() const { return m_executor.runningState(); }

    /// @return Always false for agents. If the agent was initially instantiated
    /// as a team, true.
    virtual bool isTeam() const { return false; }

    /// @copydoc Message::asPtr
    template <typename TypedMessage>
    std::shared_ptr<TypedMessage> messageAsPtr() const;

    /// @copydoc Message::asCopy
    template <typename TypedMessage>
    std::shared_ptr<TypedMessage> messageAsCopy() const;

    /// @return The message with a given name
    std::shared_ptr<Message> message(const std::string& name) const;

    /// Send a message to this agent to be handled by the configured handler
    /// @deprecate Old style handlers had a lambda per message, however we are
    /// transitioning to goal's handling messages. An agent that receives
    /// a message will trigger a goal, not a lambda.
    void sendMessageToHandler(std::unique_ptr<Message> msg);
    /// @todo recieveMessage? handleMessage? processMessage?

    /// @return The BeliefContext of the agent
    BeliefContext &context() { return m_context; }

    /// \todo This is a workaround for a MSVC compiler issue. See comment in
    /// agent.cpp:addResourcePerceptEvent()
    /// Add a percept event for when a resource is updated
    void addResourcePerceptEvent(const std::string &resourceName, int value);

    const std::vector<Team *> &teamMemberships() const { return m_teamMemberships; }

    /// The list of teams that this agent is a member of
    /// @return If the team could be added to the list, false if we already have the team
    bool addTeamMembership(Team *newTeam);

    struct SharedBeliefSet
    {
        std::shared_ptr<Message>  m_beliefSet;   // The beliefset data
        UniqueId                  m_memberId;    // The UUID of the agent that this BeliefSet came from
        std::string               m_memberName;  // The name of the agent that this BeliefSet came from
        std::chrono::microseconds m_lastUpdated; // Uses the engine internal clock. The last engine tick that this beliefset was updated
        std::chrono::microseconds m_prevLastUpdated;
        bool operator==(const UniqueId &other) const { return m_memberId == other; }
    };

    /// Get the mapping of the shared beliefsets that are received by this agent.
    using BeliefSetName = std::string;
    const std::map<BeliefSetName, std::vector<SharedBeliefSet>> &sharedBeliefSets() const { return m_sharedBeliefSets; }

    /// Get the most recently used (MRU) beliefset from a list of shared beliefsets
    /// @param list The list of beliefsets to search
    /// @param bSet (Optional) Stores the MRU beliefset if found, otherwise it leaves the parameter untouched. WARNING: Returns a pointer to the list container.
    /// @return True if the MRU beliefset was found in the list. False otherwise.
    static bool findMRUFromSharedBeliefSetList(std::vector<SharedBeliefSet> &list, SharedBeliefSet **bSet);

    struct DebugState
    {
        /// Stores if the agent has had start(...) called on it before throughout
        /// the agent's lifetime.
        bool m_startedAtLeastOnce;

        /// The number of events processed whilst the agent is halted.
        uint64_t m_eventBackLog;

        /// Tracks if a warning has been logged before for events that are being
        /// processed whilst the agent is halted.
        bool     m_eventBackLogWarning;

        /// True if the agent was instantiated via create(), false if it wasn't.
        bool m_instantiatedFromTemplate;

        size_t m_searchNodeOpenHighWaterMark;
        size_t m_searchNodeClosedHighWaterMark;
        size_t m_searchNodePendingHighWaterMark;
    };

    struct ScheduleList
    {
        const Schedule* planner;
        const Schedule* executor;
    };

    ScheduleList scheduleList() const { return ScheduleList{m_schedule, m_executor.schedule()}; }

    /// @return the plans applicable for the given goal
    const std::vector<const Plan*>& getGoalPlans(const Goal* goal);

    /// @return The plans supported by the goal taking into account the active
    /// tactic (if any).
    ///
    /// This list does not take into consideration the goal's plan selection
    /// state, only the plans that the tactic supports
    const std::vector<const Plan*>& getGoalTacticPlans(const Goal* goal);

    struct HandlesActionResult
    {
        bool          agent;   ///< The agent can handle the action
        ServiceHandle service; ///< The service that can handle the action
    };

    enum HandlesActionSearch
    {
        HandlesActionSearch_SERVICE = 1 << 0, ///< Check if a service can handle the action
        HandlesActionSearch_AGENT   = 1 << 1, ///< Check if the agent itself can handle the action
        HandlesActionSearch_ALL     = HandlesActionSearch_SERVICE | HandlesActionSearch_AGENT,
    };

    /// Evaluate if an agent can handle an action either directly or by some
    /// service attached to it.
    /// @param[in] search Customise the behaviour in which the agent will
    /// evaluate if it can handle an action or not.
    HandlesActionResult handlesAction(std::string_view name, HandlesActionSearch search) const;

    struct HandlesPlanResult
    {
        bool             success; ///< True if the agent is able to handle the plan, false otherwise
        std::string_view action;  ///< Name of the action that was not handled if 'success' was false
    };

    HandlesPlanResult handlesPlan(Plan const *plan, HandlesActionSearch search);

    struct GetDelegatesResult
    {
        const std::vector<Agent*>& delegates; ///< The agents that can be delegated to
        std::string delegationOutcome;        ///< String describing which delegates were considered and successful or failed
    };

    /// No-op for agents. Note, only teams can get a delegation list.
    /// @return A list of agents that can handle this goal delegation
    virtual GetDelegatesResult getDelegates([[maybe_unused]] const GoalHandle& goal);

    /**************************************************************************
     * Protected Functions
     **************************************************************************/
protected:
    /// @return True if the tactic was valid and able to be set, false otherwise
    bool setTactic(const TacticHandle &handle);

    /// Delegate a goal to the delegate agent. No-op for Agents. Note, only teams can delegate goals to agents
    virtual void delegateGoal(const GoalHandle& goalHandle, Agent* delegate, const std::shared_ptr<Message>& parameters);

    /// Analyse a delegation goal. No-op for Agents. Note, only teams can delegate goals to agents
    /// @param goal The goal to delegate out (i.e auction out).
    /// @param scheduleId The ID of the schedule the auctions will be generated for
    virtual bool analyseDelegation(Goal *goal, size_t scheduleId);

    /// Drop a delegated goal. No-op for Agents. Note, only teams can drop delegations
    virtual void dropDelegation(const GoalHandle &goalHandle, Agent *delegate);

    /// @param name The new name of the cloned agent.
    /// @return A clone of this agent with the new name
    Agent *clone(std::string_view name) const override { return JACK_NEW(Agent, this, name); }

    /// Delete the desire from the agent's list of desires immediately
    /// @return True if the desire was found and erased otherwise false
    bool eraseDesire(const GoalHandle &handle);

    /// Setup a custom handler for an action
    /// @param name The name of the action that when triggered will call the handler
    /// @param func The handler/callback to be triggered when an action is requested
    void addActionHandler(const std::string &name, const ActionHandlerFunc &func)
    {
        m_actionHandlers[name] = func;
    }

    /// Setup a custom handler for an message
    /// @param name The name of the message that when triggered will call the handler
    /// @param func The handler/callback to be triggered when an message is requested
    void addMessageHandler(const std::string &name, const std::function<void(Agent&, const Message&)> &func)
    {
        m_messageHandlers[name] = func;
    }

    /// Wake up the scheduler to tick the engine
    void notifyScheduler();

    /// Process an event to and trigger the handler for an incoming event
    void handleMessageEvent(Event *event);

    /// Execute one tick of the BDI process for this agent
    void run() override;

    /// Execute the BDI planner to create a new intention schedule
    Schedule* plan();

    /// @return Generate a list of the active goals
    std::vector<Goal*> activateGoals(BeliefContext *context);

    /// \todo Don't return a heap allocated pointer, doesn't need to be
    /// allocated. plan() and by proxy generate schedule sit on the hot path in
    /// any non-trivial application.
    ///
    /// Generate a new schedule from the current agent's context
    /// @return The generated schedule
    Schedule *generateSchedule(const std::vector<Goal*> &activeGoals);

    /// Execute a schedule
    void execute(Schedule *schedule);

    /// Handle an incoming event
    void eventDispatch(Event *event) override;

    /// Add an event to the engine's dispatch queue for the recipient
    PromisePtr addEvent(Event *event);

    struct DelegationEventBackLogEntry
    {
        /// \todo Re-evaluate all pointers here.
        /// 1. Schedule shouldn't need to be one- stack allocate and move into it.
        /// 2. Goals are weakly owned (?) by the schedule? I think?
        JACK_UNIQUE_PTR(Schedule) schedule = JACK_INIT_UNIQUE_PTR(Schedule, nullptr);
        bool                      delegationGoalAlreadyBeingExecuted = false;
        struct DelegationEvent *  delegationEvent;

        /// \todo This is a hack. See DelegationEvent where we populate this
        /// array for explanation.
        std::vector<JACK_UNIQUE_PTR(Goal)> desires_HACK;
    };

    struct CurrentAuction
    {
        /// The goal the auction was triggered for
        GoalHandle                goal;

        /// The schedule ID for which this auction ID was initiated for
        size_t                    scheduleId;

        /// The total number of delegations that were dispatched for this auction
        size_t                    totalDelegations;

        /// The bids that we have received back for the auction
        std::vector<AuctionEventBid> bids;

        /// A time point specified in terms of the engine clock
        std::chrono::milliseconds expiryTimePoint;

        /// @return True if all the bids have been returned or the auction has
        /// expired
        bool finished(std::chrono::milliseconds clock) const;
    };

    /// \todo Temporary, this gives us container resize with stable entries
    /// which is causing some lifetime grief.
    std::list<CurrentAuction> m_currentAuctions;

    /// Goals delegated by this agent are stored in this list and processed as
    /// the auctions are returned from the agents we delegated to.
    std::vector<DelegationEventBackLogEntry> m_delegationEventBackLog;

    /// Stores the plan names the agent can use
    std::vector<std::string> m_plans;

    /// Stores the role names the agent can use
    std::vector<std::string> m_roles;

    /// Stores the initial desires the agent will pursue on start-up
    std::vector<std::string> m_initialDesires;

    /// Goal that will be used when a resource is violated
    std::map<std::string, std::string> m_resourcesToGoals;

    /// Map the name of an action to its handler
    ActionHandlerTable m_actionHandlers;

    /// Map the name of an message to its handler
    std::map<std::string, std::function<void(Agent&, const Message&)>> m_messageHandlers;

    /// The current desires of the agent
    std::vector<Goal*> m_desires;

    /// Map the name of a goal to plans applicable for that goal
    std::map<std::string, std::vector<const Plan*>> m_goalPlans;

    std::map<std::string, std::vector<const Plan*>> m_goalTacticPlans;

    /// \todo Better description
    /// Active timer events
    std::priority_queue<TimerEvent*, std::vector<TimerEvent*>, CompareTimerEvent> m_timerEvents;

    /// Stores the belief set names the agent can use
    std::vector<std::string> m_beliefsetIds;

    /// The list of resource IDs that this agent needs to instantiate while running
    std::vector<std::string> m_resourceIds;

    /// Services that this agent depends on
    std::vector<std::string> m_services;

    /// The list of Services currently associated with the agent and are able to
    /// handle actions on behalf of the agent if they do not handle the action
    /// themselves.
    std::vector<ServiceHandle> m_attachedServices;

    /// The agent's belief context
    BeliefContext m_context;

    /// The executor for managing and running intentions
    AgentExecutor m_executor;

    /// The planning search space that stores the potential sequence of
    /// intentions to execute. In this search space the optimal sequence of
    /// execution is also recorded and used to direct the agent to achieve their
    /// desired goals.
    Schedule* m_schedule;

    /// Denotes which change in the agent caused a schedule to become dirty and
    /// hence requiring a new schedule to be generated
    ///
    /// A schedule must be imemdiately replaced with a new schedule when the
    /// schedule becomes dirty from
    ///
    /// - GOAL_REMOVED: The schedule may reference a goal to solve that no
    /// longer exists
    /// - MEMBER_REMOVED: The schedule may reference a team member in an auction
    /// that no longer exists.
    /// - FORCE: The user requested a reschedule that must be respected
    ///
    /// All other dirty states will cause the schedule to be replaced only after
    /// any pre-existing schedule has completed planning.
    enum ScheduleDirty : uint16_t {
        ScheduleDirty_NONE            = 0,
        ScheduleDirty_AGENT_STARTED   = 1 << 0,
        ScheduleDirty_PERCEPT         = 1 << 1,
        ScheduleDirty_MESSAGE         = 1 << 2,
        ScheduleDirty_GOAL_ADDED      = 1 << 3,
        ScheduleDirty_GOAL_REMOVED    = 1 << 4,
        ScheduleDirty_MEMBER_ADDED    = 1 << 5,
        ScheduleDirty_MEMBER_REMOVED  = 1 << 6,
        ScheduleDirty_TACTICS_CHANGED = 1 << 7,
        ScheduleDirty_FORCE           = 1 << 8,
    };

    /// Set when the schedule is dirty, i.e. invalidated by some change because
    /// of changing percepts/new goals
    uint16_t m_scheduleDirty = ScheduleDirty_NONE;

    std::chrono::milliseconds m_lastTimeBeliefSetsDirtied = std::chrono::milliseconds(0);

    std::chrono::milliseconds m_lastTimeBeliefSetsShared = std::chrono::milliseconds(0);

    /// \todo Better documentation
    std::mutex m_guardAPI;

    friend class Engine;
    friend class AgentBuilder;
    friend class IntentionExecutor;
    friend class Schedule;
    friend class ScheduleLane;
    friend class AgentExecutor;

    // TODO: should just have the base here
    friend class ActionTask;
    friend class PursueTask;
    friend class SleepTask;
    friend class Task;

private:

    /// @return The next timed event's time to execute relative to the engine's internal clock
    std::chrono::milliseconds getNextEventTimePoint() const;

    /// @return If the agent has any timer events
    bool hasTimerEvents() const { return !m_timerEvents.empty(); }

    /// Initialise constants on construction of the Agent
    void initConstants();

    /// Send beliefsets out to the team or team-members if applicable.
    void processSharedBeliefs();

    /// Update all the schedules in the delegation event queue. This backlog is
    /// generated from incoming auctions (i.e. DelegationEvents)- where
    /// simulating the cost of completing the delegated goal may take multiple
    /// ticks of the scheduler.
    void processCurrentDelegationEvents();

    /// Auctions that are dispatched by this agent and are awaiting bids results
    /// from the agents we delegated to are processed iteratively as the results
    /// return in this function
    void processCurrentAuctions();

    /// Monotonically increasing id whose value gets snapshotted by newly
    /// created schedules primarily to track outdated auction bids which
    /// reference old schedules.
    size_t m_scheduleIdCounter;

    /// The shared beliefs received by the agent, mapping a beliefset to the
    /// list agents they received it from.
    /// \note We use a the name of the beliefset as the key, as we may receive
    /// multiple of the same beliefsets from different agents.
    std::map<BeliefSetName, std::vector<SharedBeliefSet>> m_sharedBeliefSets;

    /// Which team this agent is a member of
    std::vector<Team *> m_teamMemberships;

    /// Current enabled Tactics lookup via goal
    std::map<std::string, const Tactic *, std::less<>> m_currentTactics;

    DebugState m_debugState = {};

    std::unordered_map<const Plan*, HandlesPlanResult> m_handlesPlanCache;

    size_t m_handlesPlanCachedOnTick = 0;
};

template <typename TypedMessage>
std::shared_ptr<TypedMessage> Agent::messageAsPtr() const
{
    return std::dynamic_pointer_cast<TypedMessage>(message(TypedMessage::schema().m_name));
}

template <typename TypedMessage>
std::shared_ptr<TypedMessage> Agent::messageAsCopy() const
{
    if (std::shared_ptr<Message> msg = message(TypedMessage::schema().m_name)) {
        return std::move(msg->clone());
    } else {
        return {};
    }
}
} // namespace aos::jack
#endif
