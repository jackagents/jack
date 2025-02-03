// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_INTENTION_EXECUTOR_H
#define JACK_INTENTION_EXECUTOR_H

#include <jack/corelib.h>       /// FinishState
#include <jack/engine/uuid.h>   // for UniqueId
#include <jack/handles.h>
#include <jack/plan.h>
#include <jack/event-protocol/protocol.h>

#include <cstddef> // for size_t
#include <memory>  // for shared_ptr
#include <vector>  // for vector
#include <string>  // for string

namespace aos::jack
{
class Agent;
class Message;
class BeliefContext;
class Goal;

/******************************************************************************
 * \class   IntentionExecutor
 *
 * Handle the execution of an intention
 ******************************************************************************/

class IntentionExecutor
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    IntentionExecutor(const IntentionExecutor& other);

    IntentionExecutor(Agent* agent, Goal* goal, bool delegated);

    ~IntentionExecutor();

    /**************************************************************************
     * Static Fields
     **************************************************************************/
    using Id = int;
    static const Id NULL_ID = 0;

    /**************************************************************************
     * Functions
     **************************************************************************/
    void setContext(BeliefContext& context, const std::shared_ptr<Message>& goalContext);

    /// this goal executor is simulated
    void setSimulated(bool simulated);

    // Update the current plan the intention will attempt to execute. If the
    // executor is currently executing a plan, this function will initiate
    // a transition to the new intention that can be begun with subsequent calls
    // to execute(...).
    // @param plan The plan to execute. The intention takes ownership of the pointer.
    void setPlan(Plan *plan);

    /// Mark this intention as a delegated intention (e.g. a team member will
    /// achieve the goal on behalf of this agent).
    void setDelegated() { m_delegated = true; }

    /// Evaluates whether or not the goal of this intention has been achieved
    /// and transitions the executor to completion if possible.
    void checkPrecondition();

    /// Invoke when the intention is complete updating the intention execution
    /// stats in the executor.
    void onIntentionDone(FinishState finishState);

    /// Call when the executing agent is done with this intention. Updates the
    /// intention state machine.
    void onGoalDone(FinishState finishState);

    // Call when the executing agent should block this intention. Updates the
    // intention state machine.
    // Signal to the intention to drop/stop current execution of the goal and plan.
    void drop(protocol::DropMode mode, std::string_view reason);

    /// Update the executor by one step by running the current plan or
    /// transitioning to a new plan if there's one or
    void execute();

    IntentionExecutor::Id id() const { return m_id; }

    // @return Can be nullptr
    Plan *currentIntention() const { return m_currentIntention; }

    enum ExecutorState
    {
        DROPPING,       /// Drop the current plan and transition to a new plan if possible
        FORCE_DROPPING, /// Drop the current plan, direct request- no plan transition
        RUNNING,        /// Advance the plan or drop co-routine in the executor
        CONCLUDED       /// End the executor
    };

    ExecutorState state() const { return m_state; }

    bool isRunning() const { return m_state == RUNNING; }

    bool isConcluded() const { return m_state == CONCLUDED; }

    bool isDropping() const { return m_state == DROPPING || m_state == FORCE_DROPPING; }

    /// Indicates if the intentoin has no plans to executor OR the executor is
    /// running but the executor is waiting for a task to complete.
    bool isWaiting() const;

    /// Indicates if the intention has no plans to execute.
    bool isWaitingForPlan() const { return !m_currentIntention && !m_targetIntention && !isDelegated(); }

    bool isSimulated() const { return m_simulated; }

    bool isDelegated() const { return m_delegated; }

    /// Indicates if the most recently run intention was successful. False if
    /// the intention never actually executed any plan before or the last
    /// intention failed.
    FinishState lastPlanFinishState() const { return m_lastPlanFinishState; }

    BeliefContext& context();

    const BeliefContext& context() const;

    const Goal* goalPtr() const { return m_goal; }

    Goal* goalPtr() { return m_goal; }

    const GoalHandle& desireHandle() const { return m_desireHandle; }

    /// The total number of intentions (plans) executed by this executor.
    /// Exposed for diagnostic purposes.
    int totalIntentions() const { return m_totalIntentions; }

    /// Add a sub-goal ID to the list that represents the sub-goal's spawned
    /// from this executor.
    void addSubGoalDesireId(const UniqueId &otherId);

    /// Get the list of sub-goal ID's spawned from this executor. The returned
    /// list of encompasses only the first level of sub-goals spawned.
    const std::vector<UniqueId>& subGoalDesireIds() const { return m_subGoalDesireIds; }

    struct DropRequest
    {
        /// When the executor is requested to drop the mode dictates if
        /// non-persistent goals are able to be dropped and consequently the
        /// executor.
        protocol::DropMode mode;
        std::string reason;
    };

    const DropRequest& dropRequest() const { return m_dropRequest; }

private:
    /// Log to the bus an intention status
    /// @param[in] result (Optional) Pass in null to log a intention start event,
    /// otherwise this function will log intention finish with the specified
    /// result.
    void logToBusIntention(protocol::BDILogGoalIntentionResult *result);

    /// Transition the state of the executor to the new state.
    /// @return True if the state was successfully transitioned. False if the
    /// executor is currently in a concluded state.
    bool setState(ExecutorState newState);

    /****************************************************************************
     * Fields
     ****************************************************************************/
    /// The agent that owns the executor
    Agent* m_agent = nullptr;

    /// The current intention that is set when set plan is used. The intention
    /// executor owns the pointer.
    Plan* m_currentIntention = nullptr;

    /// The current intention that is set when set plan is used. The intention
    /// executor owns the pointer.
    Plan* m_targetIntention = nullptr;

    /// A cloned instanced of the desire to track this intention's attempt at
    /// achieving the goal.
    Goal* m_goal = nullptr;

    /// The root desire/original goal that this intention was formed from.
    GoalHandle m_desireHandle;

    /****************************************************************************
     * Intention Status Fields
     ****************************************************************************/
    /// Indicates if the last intention that this executor executed succeeded
    /// If this executor was delegated (e.g. the owning agent has no plans to
    /// achieve the goal) this variable will be set when the delegated agent
    /// finishes their intention.
    FinishState m_lastPlanFinishState = {};

    /// The number of intentions that this executor has been assigned to it.
    uint32_t m_totalIntentions = 0;

    /// The number of intentions assigned to it that succeeded
    uint32_t m_succeededIntentions = 0;

    /// The number of intentions assigned to it that failed
    uint32_t m_failedIntentions = 0;

    /// Track if the current assigned intention for the executor has been ticked
    /// at least once. False otherwise.
    bool m_currentIntentionStarted = false;

    /// A unique identifier for this instance of the intention executor
    Id m_id;

    /// The list of sub-goals that were spawned from the execution of this
    /// sub-goal.
    std::vector<UniqueId> m_subGoalDesireIds;

    // Executor is being simulated and will model effects and not generate real
    // actions
    bool m_simulated = false;

    ExecutorState m_state = {};

    /// Indicates if this intention was delegated to a team member. If so this
    /// flag indicates that this intention is a proxy for the team member that is
    /// executing the intention on behalf of this agent.
    bool m_delegated = false;

    DropRequest m_dropRequest = {};
};
} // namespace aos::jack
#endif /// JACK_INTENTION_EXECUTOR_H
