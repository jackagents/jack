#ifndef JACK_PLAN_H
#define JACK_PLAN_H

#include <functional>           // for function
#include <vector>               // for vector
#include <string>               // for string, operator==
#include <jack/beliefcontext.h> // for BeliefContext
#include <jack/beliefquery.h>   // for BeliefQuery
#include <jack/coroutine.h>     // for CoroutinePtr, Coroutine
#include <jack/corelib.h>

namespace aos::jack
{
class Agent;

/******************************************************************************
 * @class Plan
 * @brief The Plan Entity. A plan is an intention generator
 ******************************************************************************/
class Plan
{
public:
    /**************************************************************************
     * Public Ctor & Dtor
     **************************************************************************/
    /// @param name The name of the plan
    Plan(std::string_view name);

    /// @param other The other plan to copy from
    Plan(const Plan* other);

    virtual ~Plan();

    Plan(const Plan &other) = delete;

    Plan& operator=(const Plan &other) = delete;

    /// Create a new goal using this one as a template
    virtual Plan *clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const { return globalHeapAllocator.newInternal<Plan>(JACK_CALL_SITE_INPUT this); }

    /**************************************************************************
     * Public Accessors & Mutators
     **************************************************************************/
    /// Set this plan from another plan
    virtual void set(const Plan* other);

    // set and get the owning jack engine
    void setEngine(Engine& engine) { m_engine = &engine; }
    Engine &engine() { assert(m_engine); return *m_engine; }
    const Engine &engine() const { assert(m_engine); return *m_engine; }

    /// Set the body of the plan
    /// @param func The coroutine to execute for the body
    void setBody(CoroutinePtr func);

    /// Set the drop coroutine for the plan
    /// @param func The coroutine to execute when drop is called
    void setDropCoroutine(CoroutinePtr func);

    /// Set the supported goal name that this plan can handle
    /// @param goal The goal to set
    void setGoal(const std::string& goal) { m_goal = goal; }

    /// Set the precondition function for the plan which is executed to check if
    /// a plan is viable to be considered for scheduling for a goal.
    void setPrecondition(const BeliefQuery &func) { m_precondition = func; }

    /// Set the dropWhen function for the plan which is executed to check if
    /// a plan should be dropped from the agent's executor.
    void setDropWhen(const BeliefQuery &func) { m_dropWhen = func; }

    /// Set the effects function for the plan which is executed to simulate the
    /// outcome of executing this plan on the given context
    /// @param func The effects function to set
    void setEffects(const std::function<void(BeliefContext &)> &func) { m_effects = func; }

    /// Set the list of resources that this plan will lock/unlock during the execution of the plan.
    /// @param locks The list of resources the plan will lock/unlock
    void setResourceLocks(const std::vector<std::string> &locks) { m_resourceLocks = locks; }

    /**************************************************************************
     * Public Functions
     **************************************************************************/
    /// Reset this plan for execution for the beginning again
    /// @param agent The agent that will run this plan
    void reset(Agent* agent);

    /// @return The goal that is handled by the plan
    std::string_view goal() const { return m_goal; }

    struct Status
    {
        /// Drop was requested on the plan. Whilst the plan finish state is
        /// 'NOT_YET' if the drop flag is set this indicates that the plan is
        /// currently in the process of dropping. On completion, the finish state
        /// will be set to 'DROPPED'.
        bool        dropRequested;

        /// Outcome of the execution of the plan. When a drop is requested on the
        /// plan the finish state will always eventually evaluate to 'DROPPED'
        /// once dropping is complete. 'bodyFinished' can be used to determine if
        /// even though we requested a drop, whether or not all tasks were
        /// completed in the plan at the time of the drop request.
        FinishState finishState;

        bool        bodyFinished;  ///< All tasks in the plan body have been completed
    };

    /// Calculate the status of the plan such as if it was dropped and if the
    /// plan has completed (running or dropping).
    Status status() const;

    /// @return True if this "intention" has finished executing
    bool finished() const { return status().finishState != FinishState::NOT_YET; }

    /// @return True if this "intention" is waiting on something
    bool waiting() const { return m_body->waiting(); }

    /// Advance the plan body by executing the next task in the coroutine.
    void tick(BeliefContext&    context,
              const GoalHandle& desire,
              const UniqueId&   intentionId);

    /// @return True if the plan has a user assigned effects function via 'setEffects'
    bool canModelEffect() const { return m_effects.operator bool(); }

    /// Mutate the context using the effects for this plan
    virtual void applyEffects(BeliefContext &context) const;

    /// \todo Better documentation
    bool applyResources();

    /// Lock the resource locks for this plan and apply to the given context
    /// \todo do we need to pass in the context the plan already has one
    virtual void lockResources(BeliefContext &context) const;

    /// Unlock the resource locks for this plan in the context
    virtual void unlockResources(BeliefContext &context) const;

    /// @return The name of the plan
    const std::string& name() const { return m_name; }

    /// @return True if the precondition passes in the given context, default is true if the precondition is not given
    bool valid(const BeliefContext &context) const;

    /// @return True if the plan should drop itself, default is false is the dropWhen condition is not given
    bool shouldDrop(const BeliefContext& context) const;

    /// Drop this goal and all sub goals
    /// \todo This context should not be passed in. The plan is not stateless,
    /// and iterating the drop coroutine with another context will break the
    /// plan invariants.
    ///
    /// Whatever state is passed in must be the same for every invocation of the
    /// drop coroutine for this instance of the plan to be valid.
    ///
    /// A more reliable way of doing this is storing the context in the plan,
    /// and removing the context parameter here. The only downside to this is
    /// that it is ill-defined what context the plan has and does it have its
    /// own, or does it inherit the context from the parent goal?
    void drop(BeliefContext& context, const GoalHandle& desire, const UniqueId& intentionId);

    /// Get the resources the plan needs to lock when executing
    const std::vector<std::string> &resourceLocks() const { return m_resourceLocks; }

    /// @return The body of the plan, this can be nullptr if the body was not set.
    CoroutinePtr& body() { return m_body; }

    const CoroutinePtr& body() const { return m_body; }

    /// Trigger the task with ID to be completed. The task will be checked on
    /// the drop coroutine if the plan is dropping otherwise it will check on
    /// the plan. If the task ID does not match the current task in the active
    /// coroutine this function is a no-op and returns false to indicate no
    /// action was taken.
    bool onTaskComplete(const UniqueId& id, bool success);

    /// Represents the various states of dropping that the current plan is in.
    /// If the drop state is set to a value other than `NONE` then the drop
    /// coroutine is the active coroutine in the plan.
    enum class DropState
    {
        NONE,
        DROPPING,
        FINISHED,
    };

protected:

    /// The owning engine
    Engine* m_engine = nullptr;

    /**************************************************************************
     * Attributes
     **************************************************************************/
    /// The name of the plan
    std::string                          m_name;

    /// The goal that this plan is relevant for
    std::string                          m_goal;

    // The co-routine to run when the body is executed
    CoroutinePtr                         m_body = nullptr;

    /// The co-routine to run when drop is requested
    CoroutinePtr                         m_dropCoroutine = nullptr;

    /// \todo Better documentation
    BeliefQuery                          m_precondition;

    /// \todo Better documentation
    BeliefQuery                          m_dropWhen;

    /// The effects function used for simulating the outcome of the completed
    /// plan on the given context
    std::function<void(BeliefContext &)> m_effects;

    /// Indicates if the plan is in the process of dropping or not and whether
    /// or not it's finished dropped
    DropState                            m_dropped = DropState::NONE;

    /// The resource locks happening during the life time of this plan
    std::vector<std::string>             m_resourceLocks;
};
} // namespace aos::jack
#endif
