// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_AGENT_EXECUTOR_H
#define JACK_AGENT_EXECUTOR_H

#include <jack/engine/intentionexecutor.h>  // for IntentionExecutor, IntentionEx...
#include <jack/dag.h>
#include <jack/handles.h>
#include <jack/corelib.h>

#include <unordered_map>               // for map
#include <memory>                      // for unique_ptr
#include <vector>                      // for vector

namespace aos::jack
{
class Agent;
class Goal;
class IntentionExecutionDAG;
class Message;
class Schedule;
class UniqueId;
struct DAGNode;
struct DelegationEvent;

/*! ***********************************************************************************************
 * @class AgentExecutor
 * @brief Responsible for executing the agent's current schedule of intentions
 * ************************************************************************************************/
class AgentExecutor
{
public:

    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    AgentExecutor(Agent* agent);

    virtual ~AgentExecutor();

    AgentExecutor(const AgentExecutor &other) = delete;
    AgentExecutor& operator=(const AgentExecutor &other) = delete;

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/

    void setSchedule(Schedule *schedule);

    const Schedule *schedule() const { return m_schedule; };

    const IntentionExecutionDAG *dag() const { return &m_dag; };

    /// @return Is the executor done. i.e. nothing to do
    bool done() const;

    /// @return The number of intentions waiting on a plan or task in the last
    /// call to execute(...)
    int waitingCount() const { return m_waitingCount; }

    /// @return The number of intentions is being executed in the last called to
    /// execute(...) (i.e. not finished yet).
    int workingCount() const { return m_workingCount; }

    enum struct RunningState
    {
        IDLE,                     /// There are no intentions in the executor
        EXECUTING,                /// Intentions are being executed
        BUSY_WAITING_ON_EXECUTOR, /// All intentions in the executor are waiting
    };

    /// @return The state of the executor collectively determined by all the
    /// intentions that are currently running in this executor since the last
    /// invocation to execute(...).
    RunningState runningState() const;

    /// @return The list of running of intentions
    Span<const IntentionExecutor*> intentions() const { return Span<const IntentionExecutor*>(m_intentions.data(), m_intentions.size()); }

    /// The current intention, if any, that is currently being executed in the
    /// update(...) step
    /// @return The current intention or nullptr if no intention is being
    /// executed
    const IntentionExecutor *intentionBeingExecuted() const { return m_intentionBeingExecuted; }

    /* ****************************************************************************************
     * Public Functions
     * ****************************************************************************************/

    // stop the executor
    void stop();

    // tick the exection of the intentions
    void execute();

    void update();

    /// End the DAG node associated with the intention and unlock downstream
    /// nodes in the DAG. Downstream nodes are nodes that for example had a
    /// resource dependency on the intention and these nodes could not be
    /// executed until the intention was complete (i.e. the agent can only use
    /// their 1 arm for one intention at a time).
    ///
    /// This function may modify the list of intentions `m_intentions` and
    /// invalidate any iterators to that container.
    void close(IntentionExecutor* intention);

    /// Request the intention for the specified goal handle to be dropped.
    /// @return True if an the handle had an intention executor to drop and drop
    /// was triggered on that executor, false otherwise.
    bool internalDrop(const GoalHandle &handle, protocol::DropMode mode, std::string_view reason);

    // Update the team's intentions for when a goal that was delegated to a team
    // member was received back
    // @param event The event that was received back from the team member
    void handleDelegationEvent(const DelegationEvent *event);

    // Find a valid current intention from a DAG node
    // @return nullptr If intention not found from DAG Node's goal
    IntentionExecutor *findMatchingIntention(DAGNode* n);

    // Find a valid current intention from a goal ID
    // @id The goal id to search for in the intention's desire
    // @return nullptr If intention not found from DAG Node's goal
    IntentionExecutor *findMatchingIntentionByGoalId(const UniqueId &id);

    // Find a valid current intention from an Intention ID
    // @id The goal id to search for in the intention's desire
    // @return nullptr If intention not found from DAG Node's goal
    IntentionExecutor *findMatchingIntentionByIntentionId(IntentionExecutor::Id id);

    /// Invoked when an action task has been completed allowing the coroutine to
    /// advance
    /// @return True if a matching intention with the task ID is found and the
    /// coroutine is advanced, false otherwise.
    bool onActionTaskComplete(const UniqueId& intentionId,
                              const UniqueId& taskId,
                              bool success,
                              std::shared_ptr<Message> reply);
private:
    enum class ProcessDAGNodeMode
    {
        ON_CLOSE,
        ON_NEW_SCHEDULE,
    };

    /// Apply the DAG node to the intention, transitioning the agent's executor
    /// into the newly requested plan.
    void processDAGNode(IntentionExecutor* executor, DAGNode* node, ProcessDAGNodeMode mode);

protected:
    IntentionExecutionDAG m_dag = {};

    /// All the intentions currently being executed
    std::vector<IntentionExecutor*> m_intentions;

    /// The current intention, if any, that is currently being executed in the
    /// update(...) step. This is a nullptr otherwise if nothing is being
    /// executed. Set to a pointer from the m_intentions array.
    IntentionExecutor *m_intentionBeingExecuted = nullptr;

    std::unordered_map<IntentionExecutor*, DAGNode*> m_dagLookup;

    /// \todo We should just make Goal -> WrapperAgent
    /// Where WrapperAgent looks like
    /// {
    ///     jack::Agent *agent;
    ///     jack::GoalHandle goalHandle;
    /// }

    std::unordered_map<GoalHandle, Agent*> m_delegations;

    Agent *m_agent;

    /// The number of intentions waiting on a plan or task in the last call to
    /// execute(...)
    int m_waitingCount = 0;

    /// The number of intentions is being executed in the last called to
    /// execute(...) (i.e. not finished yet).
    int m_workingCount = 0;

    bool m_scheduleValid = false;

    Schedule *m_schedule = nullptr;
};
} // namespace aos::jack
#endif /// JACK_AGENT_EXECUTOR_H
