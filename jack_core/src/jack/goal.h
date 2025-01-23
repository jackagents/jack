#ifndef JACK_GOAL_H
#define JACK_GOAL_H

#include <jack/corelib.h>
#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/beliefquery.h>    // for BeliefQuery
#include <jack/engine/uuid.h>    // for UniqueId
#include <jack/handles.h>        // for GoalHandle
#include <jack/future.h>         // for Future
#include <jack/promise.h>        // for PromisePtr

#include <functional>       // for function, _Func_class
#include <memory>           // for shared_ptr
#include <vector>           // for vector
#include <string>           // for string

namespace aos::jack
{
class Task;
class IntentionExecutor;

enum GoalPersistent
{
    GoalPersistent_No,
    GoalPersistent_Yes,
};

struct GoalPursue
{
    GoalHandle handle;  ///< Handle to the goal that was pursued
    PromisePtr promise; ///< Promise that is fulfilled when the goal pursued is failed or succeeded
};

/*! ***********************************************************************************************
 * @class   Goal
 *
 * @brief The Goal entity
 *
 * ************************************************************************************************/
class Goal
{
public:
    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    Goal(std::string_view name)
        : m_handle({std::string(name), UniqueId::random()})
    {}

    /// Create a new goal using 'other' goal as a template
    Goal(const Goal* other)
        : m_planSelection(other->m_planSelection)
        , m_engine(other->m_engine)
        , m_precondition(other->m_precondition)
        , m_satisfied(other->m_satisfied)
        , m_dropWhen(other->m_dropWhen)
        , m_priority(other->m_priority)
        , m_heuristic(other->m_heuristic)
        , m_parent(other->m_parent)
        , m_future(other->m_future)
        , m_finishState(other->m_finishState)
        , m_handle({other->m_handle.m_name, UniqueId::random()})
        , m_messageSchema(other->m_messageSchema)
        , m_persistent(other->m_persistent)
        , m_delegated(other->m_delegated)
    {
    }

    virtual ~Goal() {}

    Goal(const Goal &other) = delete;

    Goal& operator=(const Goal &other) = delete;

    /// Create a new goal using this one as a template
    virtual Goal *clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const { return globalHeapAllocator.newInternal<Goal>(JACK_CALL_SITE_INPUT this); }

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/
    /// Get a lightweight non-owning handle referencing this goal.
    const GoalHandle &handle() const { return m_handle; }

    /// Retrieve the JACK engine instance this service belongs to.
    /// @return The JACK engine instance.

    void setEngine(Engine& engine) { m_engine = &engine; }
    Engine &engine() { assert(m_engine); return *m_engine; }
    const Engine &engine() const { assert(m_engine); return *m_engine; }

    /// Evaluate the precondition of the goal. In absence of the precondition,
    /// true is always returned.
    /// @param context The context to evaluate the precondition against
    /// @return True if the precondition passes, false otherwise.
    bool isValid(const BeliefContext& context) const;

    /// Evaluate the precondition of the goal. In absence of the precondition,
    /// true is always returned.
    /// @return True if the precondition passes, false otherwise.
    bool isValid() const { return isValid(m_context); }

    /// Evaluate the satisfied condition of the goal. In absence of the
    /// satisfied condition, false is always returned.
    /// @param context The context to evaluate the satisfied condition against
    /// @return True if the satisfied condition passes, false otherwise.
    bool isSatisfied(const BeliefContext& context) const;

    /// Evaluate the satisfied condition of the goal. In absence of the
    /// satisfied condition, false is always returned.
    /// @return True if the satisfied condition passes, false otherwise.
    bool isSatisfied() const { return isSatisfied(m_context); }

    /// Check if the goal is an achievement goal. An achievement goal is a goal
    /// that can be satisfied (e.g. it has a satisfied condition).
    /// @return True if this goal is an achievement goal, false otherwise.
    bool isAchievement() const { return m_satisfied.isValid(); }

    /// Evaluate the drop condition of the goal. The drop condition determines
    /// when the goal should be removed from the execution list. In absence of
    /// a drop condition the drop condition always returns false.
    /// @return True if this goal should currently drop itself and all sub goals
    bool shouldDrop() const;

    /// Set the precondition for the goal. The precondition is evaluated to
    /// determine if the goal is valid for execution.
    /// @param func The function to evaluate for the precondition
    void setPrecondition(const BeliefQuery &func) { m_precondition = func; }

    /// Set the satisfied condition for the goal. The satisfied condition is
    /// evaluated to determine if the goal has been completed and should not be
    /// re-attempted.
    /// @param func The function to evaluate for the satisfied condition
    void setSatisfied(const BeliefQuery &func) { m_satisfied = func; }

    /// Set the drop condition for the goal. The drop condition is evaluated to
    /// determine if the goal should be dropped and execution should stop.
    /// @param func The function to evaluate for the drop condition
    void setDropWhen(const BeliefQuery &func) { m_dropWhen = func; }

    /// Set the goal's unique id
    /// @param id The identifier to assign to the goal
    void setId(const UniqueId &id) { m_handle.m_id = id; }

    /// @return The ID of the goal which is unique to this instance.
    const UniqueId &id() const { return m_handle.m_id; }

    /// @return The name of the goal
    const std::string &name() const { return m_handle.m_name; }

    /// Set the priority of the goal
    /// \todo Currently unused
    void setPriority(int priority) { m_priority = priority; }

    /// Set if this goal is persistent. When a goal is persistent, the
    /// goal will permanently be re-attempted even if the goal fails or
    /// succeeds.
    void setPersistent(bool persistent) { m_persistent = persistent; }

    /// Check if the goal is persistent goal. When a goal is persistent, the
    /// goal will permanently be re-attempted even if the goal fails or
    /// succeeds.
    /// @return True if this goal is persistent, false otherwise
    bool isPersistent() const { return m_persistent; }

    /// Set the heuristic of the goal which estimates the completion of this
    /// goal from a given context. The heuristic is ideally admissible (i.e. it
    /// doesn't over estimate the cost of completing the goal).
    /// @param func The function to evaluate for the heuristic
    void setHeuristic(const std::function<float(const BeliefContext &)> func) { m_heuristic = func; }

    /// @return The priority of the goal
    int priority() const { return m_priority; }

    /// Evaluate the heuristic of the goal. The heuristic is evaluated to
    /// determine the cost of completing the goal when planning against the
    /// given context.
    /// @param context The context to evaluate the heuristic against
    /// @return The estimated cost to completing the goal.
    float heuristic(const BeliefContext &context) const;

    /// Determine if the goal has a heuristic associated with it which estimates
    /// the completion of this goal.
    /// @return True if the goal has a heuristic associated with it. False
    /// otherwise
    bool hasHeuristic() const { return static_cast<bool>(m_heuristic); }

    /// Retrieve this goal's context that the various goal query functions
    /// evaluate their conditions against.
    BeliefContext& context() { return m_context; }

    /// Retrieve this goal's context that the various goal query functions
    /// evaluate their conditions against.
    const BeliefContext& context() const { return m_context; }

    struct Parent
    {
        GoalHandle handle;
        UniqueId   planTaskId; ///< Tracks the ID of the task that initiated this sub-goal.
    };

    void setParent(const Parent& parent) { m_parent = parent; }

    const Parent& parent() const { return m_parent; }

    /// Set the promise for the goal that is evaluated when the goal is failed
    /// or succeeded
    /// @param promise The promise to execute
    void setPromise(const PromisePtr &promise) { m_future = JACK_INIT_SHARED_PTR(Future, JACK_NEW(Future, promise)); }

    void toString() const;

    /// Mark the goal as having finished
    /// @param[in] success Indicate if the goal finished successfully or failed
    void finish(FinishState finish);

    FinishState finished() const { return m_finishState; }

    /// Query the message that the goal must have for execution of the goal.
    /// This can be empty if no message is necessary for execution.
    std::string_view messageSchema() const { return m_messageSchema; }

    /// Set the message that the goal must have for execution.
    void setMessageSchema(std::string_view messageSchema) { m_messageSchema = std::string(messageSchema); }

    /// Set a status flag indicating that the agent that owns the goal must
    /// delegate the achieving of the goal because the agent does not have
    /// a plan for the goal. This is always false for the template goal.
    void setDelegated(bool delegated) { m_delegated = delegated; }

    /// Check if the achieving of this goal must be delegated to member agent
    /// because the owning agent does not have a plan for the goal.
    /// @return True if the goal must be delegated, false otherwise
    bool delegated() const { return m_delegated; }

    struct PlanHistory
    {
        std::string toString() const;

        std::string plan;              /// The plan the history is relevant for
        uint32_t    successCount;      /// The number of times the plan has succeeded
        uint32_t    failCount;         /// The numbef of times the plan has failed
        uint32_t    lastLoopIteration; /// The last loop iteration that this plan was executed in
    };

    class PlanSelection
    {
    public:
        /// Find the execution history of the plan for the goal this plan
        /// selection is associated with.
        /// @param plan The plan to find the execution history for
        /// @return The plan execution history. If the plan has never been
        /// executed for the goal, this function returns a null pointer
        /// indicating that the plan has never been executed for this goal yet.
        PlanHistory* findHistory(std::string_view plan);

        /// Find the execution history of the plan or and if it does not exist
        /// create the entry.
        /// @param plan The plan to find or make the execution history for
        /// @return The plan execution history entry
        PlanHistory& findOrMakeHistory(std::string_view plan);

        std::string toString() const;

        /// The tactic the goal was pursued with
        std::string              m_tactic;

        /// The current iteration of attempting to solve the plan. This value is
        /// always zero if the plan is not allowed to loop.
        uint32_t                 m_planLoopIteration;

        /// The index into the fixed plan list indicating what the next plan
        /// that must be executed is. This value is always modulo the fixed plan
        /// list size. If there's no plan list, this value is always 0.
        uint32_t                 m_planListIndex;

        /// The execution history of the plans attempted for this goal.
        std::vector<PlanHistory> m_history;
    };

    /// The goal plan selection book-keeping state for this instance of the
    /// goal.
    PlanSelection m_planSelection = {};

protected:

    /// The engine running this service
    Engine* m_engine = nullptr;

    /// A custom function for evaluating the precondition of the goal, whether
    /// or not the goal is valid to be executed or not.
    BeliefQuery m_precondition;

    /// A custom function for evaluating the satisfied condition of the goal,
    /// whether or not the goal is complete and should no longer be executed.
    BeliefQuery m_satisfied;

    /// A custom function for evaluating when the goal should be dropped.
    BeliefQuery m_dropWhen;

    /// \todo Unused
    int         m_priority = 0;

    /// The execution context of the goal including all the variables made
    /// available to the goal for executing it (the goal) successfully such as
    /// the agent beliefs, goal parameters and plan parameters.
    BeliefContext m_context;

    /// A custom function for evaluating the heuristic of the goal, the cost of
    /// completing the goal.
    std::function<float(const BeliefContext &)> m_heuristic;

    /// The parent intention
    Parent m_parent = {};

    /// The future for fulfiling a promise
    std::shared_ptr<Future> m_future;

    /// Track if success/fail has been called yet to prevent double triggered of
    /// the completion handlers
    FinishState m_finishState = FinishState::NOT_YET;

    /// The light weight handle for this goal
    GoalHandle m_handle = {};

    /// Name of the message schema required for the perform/pursue of the goal
    std::string m_messageSchema;

    /// The goal is permanently desired and reattempted even if the goal
    /// succeeds or fails.
    bool m_persistent = false;

    /// Flag that indicates this goal must be delegated because the owning agent
    /// does not have a plan to achieve the goal.
    bool m_delegated = false;
};
} // namespace aos::jack
#endif // JACK_GOAL_H
