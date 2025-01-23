#ifndef JACK_TASK_H
#define JACK_TASK_H

#include <jack/engine/uuid.h>  // for UniqueId
#include <jack/corelib.h>

#include <string_view>
#include <string>
#include <vector>
#include <chrono>
#include <any>

namespace aos::jack
{
class Agent;
class BeliefSet;
class BeliefContext;
class Engine;
class Plan;
struct GoalHandle;

// TODO comment
class Task
{
public:
    virtual ~Task() {}

    enum State
    {
        NOTREADY,
        READY,
        WAIT,
        YIELD,
        ASYNC,
        DONE
    };

    enum Status
    {
        SUCCEEDED,
        FAILED
    };

    Engine &engine();

    void reset(Agent* agent, Plan *intention);

    State state() const { return m_state; }

    void setReady() { m_state = READY; }

    Status status() const { return m_status; }

    /// @param context[in] Environment variables available during task execution
    /// @param desire[in] The root motivating goal reponsible for the tick of
    /// this task
    /// @param intentionId[in] ID of the intention associatd with executing the
    /// task.
    /// @param plan[in] Name of the plan responsible for the tick of this task
    bool tick(BeliefContext& context,
              const GoalHandle& desire,
              const UniqueId& intentionId,
              std::string_view plan);

    virtual void nowait() { m_wait = false; }

    void succeed();

    void fail();

    virtual Task *clone() const = 0;

    /// Configure this task with a parameter whose value is pulled from the
    /// coroutine/belief context that matches the name specified in
    /// 'contextParamName'.
    void paramFrom(std::string_view name, std::string_view contextParamName)
    {
        m_parameters.emplace_back(ParameterToContextMapping{std::string(name), std::string(contextParamName)});
    }

    /// Configure this task with a parameter and provide a default value
    /// The value of the parameter takes its value with precedence from the goal
    /// context before defaulting to the default value if it's not available in
    /// the context.
    void param(std::string_view name, const std::any& value)
    {
        m_literals.push_back(std::make_pair(std::string(name), value));
    }

    /// Configure the outgoing success edge for this task
    /// @param label the index of the target
    /// @note any index less than 0 is end
    void setSuccessTarget(int32_t label) { m_successTarget = label; }

    /// Configure the outgoing faile edge for this task
    /// @param label the index of the target
    /// @note any index less than 0 is end
    void setFailTarget(int32_t label) { m_failTarget = label; }

    /// get the success edge
    /// @return the current target index for a success
    int32_t successTarget() const { return m_successTarget; }

    /// get the fail edge
    /// @return the current target index for a fail
    int32_t failTarget() const { return m_failTarget; }

    const Plan* intention() const { return m_intention; }

    const UniqueId& id() const { return m_id; }

    /// Each task requires an ID to register and allow events/messages generated
    /// from the task to be associated with it. Currently this is used for
    /// ActionTasks to allow complete actions to route back to the task that
    /// generated it.
    UniqueId m_id = UniqueId::random();

protected:
    virtual State execute(BeliefContext &, const GoalHandle&, const UniqueId&, std::string_view) = 0;

    virtual State resume(BeliefContext &, const GoalHandle&, const UniqueId&, std::string_view) { return DONE; }

    State      m_state     = NOTREADY;
    Agent *    m_agent     = nullptr;
    Plan *     m_intention = nullptr;
    BeliefSet *m_local     = nullptr;

    /// When true, the coroutine will wait for the task to finish before
    /// proceeding.
    bool   m_wait   = true;
    Status m_status = SUCCEEDED;

    struct ParameterToContextMapping
    {
        /// The name of the parameter that is exposed to the action.
        std::string paramName;

        /// The name of the parameter from the BeliefContext to pull the value
        /// from. If this is an empty string, the 'paramName' is used to search
        /// the BeliefContext for the matching parameter.
        std::string contextParamName;
    };

    /// The parameters of this task that will be pulled from the goal context if
    /// applicable (i.e. typically action and pursue taks)
    std::vector<ParameterToContextMapping> m_parameters;

    /// A mapping from the parameter name to the default value of the parameter.
    /// A matching parameter from the beliefcontext takes precedence over the
    /// default value of a parameter.
    std::vector<std::pair<std::string /*parameter name*/, std::any>> m_literals;

    /// define the outgoing targets
    int32_t m_successTarget = -1;  // -1 is the end label
    int32_t m_failTarget    = -1;  // -1 is the end label
};

template<typename ParentTask>
class ClonableTask : public Task
{
public:
    ~ClonableTask() override {}

    // clone this task
    Task *clone() const override
    {
        ParentTask *task = JACK_NEW(ParentTask, *(static_cast<const ParentTask*>(this)));
        task->reset(m_agent, m_intention);
        /// @note The task ID is unique for the task in that plan. This
        /// ID is hence shared across all instances of the plan. This is useful
        /// for tracking plan execution on the bus as ID's are consistent across
        /// plans and refer to exactly one consistent task across execution.
        task->m_id = m_id;
        task->m_parameters = m_parameters;
        task->m_literals = m_literals;
        task->m_successTarget = m_successTarget;
        task->m_failTarget = m_failTarget;

        return static_cast<Task*>(task);
    }
};
} // namespace aos::jack
#endif // JACK_TASK_H
