#include <jack/engine/intentionexecutor.h>

#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/goal.h>           // for Goal
#include <jack/plan.h>           // for Plan
#include <jack/agent.h>          // for Agent
#include <jack/tasks/task.h>     // for Task
#include <jack/engine/uuid.h>    // for UniqueId, operator<<
#include <jack/engine.h>
#include <jack/event-protocol/busadapter.h>
#include <jack/corelib.h>        // for JACK_LOG, JACK_CRITICAL, JACK_BDI, JACK_INFO

/// Third Party
#include <tracy/Tracy.hpp>
#include <atomic>           // for atomic, _Atomic_integral
#include <cassert>          // for assert
#include <ostream>          // for operator<<, basic_ostream
#include <string>           // for char_traits, to_string
#include <type_traits>      // for move
#include <utility>          // for max, min

namespace aos::jack
{
static std::atomic<IntentionExecutor::Id> gCount = {0};

// copy from another IntentionExecutor and specific context
// the goal will be copied in a reset state
IntentionExecutor::IntentionExecutor(const IntentionExecutor& other)
    : m_agent(other.m_agent)
    , m_goal(JACK_CLONE_BDI_OBJECT(other.m_goal))
    , m_desireHandle(other.desireHandle())
    , m_id(++gCount)
    , m_delegated(other.m_delegated)
{
    JACK_DEBUG_MSG("Copy IntentionExecutor");
}

IntentionExecutor::IntentionExecutor(Agent *agent, Goal *goal, bool delegated)
    : m_agent(agent)
    , m_goal(JACK_CLONE_BDI_OBJECT(goal))
    , m_desireHandle(goal->handle())
    , m_id(++gCount)
    , m_delegated(delegated)
{
    // add listener to the goal state
    // initially this executor will be inactive
    // when the owning goal becoming ready
    // the executor will be woken up and continue
    // towards a running state
}

IntentionExecutor::~IntentionExecutor()
{
    JACK_DELETE(m_currentIntention);
    JACK_DELETE(m_targetIntention);

    /// \todo Double check why this is a clone, check the comment in the header
    /// file for this member variable.
    JACK_DELETE(m_goal);

    /// \todo Clean up the context?
    /*
    BeliefContext& context = m_goal->context();
    BeliefSet*     gptr    = context.goal();
    BeliefSet*     pptr    = context.plan();
    if (gptr != nullptr) { delete gptr; }
    if (pptr != nullptr) { delete pptr; }
    */
}

BeliefContext &IntentionExecutor::context()
{
    return m_goal->context();
}

const BeliefContext &IntentionExecutor::context() const
{
    return m_goal->context();
}

void IntentionExecutor::setContext(BeliefContext& agentContext, const std::shared_ptr<Message>& goalContext)
{
    /*! ***************************************************************************************
     * Set the context for this goal executor
     * ****************************************************************************************/
    BeliefContext &context = m_goal->context();
    context.setAgentContext(agentContext, m_agent->handle());
    context.setGoalContext(goalContext); // setup the goal context
}

void IntentionExecutor::setSimulated(bool simulated)
{
    /*! ***************************************************************************************
     * Switch this goal executor between real and simulated modes
     * If a goal executor is simulated it won't issues any real concrete actions
     * and also won't effect the agents state
     * ****************************************************************************************/
    m_simulated = simulated;

    // hack to stop the id being incremented by the planner
    if(simulated) --gCount;

    // stop goals from looping while planning/being simulated
    // this will ensure that each plan is only evaluated once

    /// \todo This would change the global tactic, re-evaluate but we don't use
    /// simulated intentions atm.
    /// m_goal->m_planSelection.m_policy_.planLoop = false;
}

void IntentionExecutor::setPlan(Plan *plan)
{
    JACK_DELETE(m_targetIntention);
    m_targetIntention = plan;
    setState(ExecutorState::DROPPING);
}

/***************************************************************************************************
 Executor event handling
***************************************************************************************************/
void IntentionExecutor::checkPrecondition()
{
    if (!m_goal->delegated()) {
        /// \note If the goal is satisfied we will not mark the intention as
        /// failed even if the goal is invalid (e.g. the precondition failed).
        /// This way we plan for "success" in the general case.
        if (!m_goal->isSatisfied() && !m_goal->isValid()) {
            JACK_DEBUG("Intention just started but goal precondition failed, goal concluding [agent={}, goal={}]",
                       m_agent->handle().toString(),
                       m_goal->handle().toString());
            onGoalDone(FinishState::FAILED);
            return;
        }
    }

    // Check if the intention has already succeeded. If so, mark the intention complete and early
    // exit.
    const int  attempts          = m_succeededIntentions + m_failedIntentions;
    const bool goalIsAchievement = m_goal->isAchievement();
    const bool finished          = goalIsAchievement ? m_goal->isSatisfied() : m_lastPlanFinishState == FinishState::SUCCESS;
    if (finished) {
        if (goalIsAchievement) {
            JACK_DEBUG("Achievement goal is satisfied [agent={}, goal={}]",
                       m_agent->handle().toString(),
                       m_goal->handle().toString());
        } else {
            JACK_DEBUG("Perform goal succeeded [agent={}, goal={}]",
                       m_agent->handle().toString(),
                       m_goal->handle().toString());
        }
        onGoalDone(FinishState::SUCCESS);
        return;
    }

    // Intention is not complete, update the intention state machine to be ready
    // for executing a plan.
    if (attempts > 0) {
        if (goalIsAchievement) {
            JACK_DEBUG_MSG("Achievement goal was not satisfied, intention getting ready to try again");
        } else {
            JACK_DEBUG_MSG("Perform goal's plan failed, intention getting ready to try again");
        }
    }
    setState(ExecutorState::RUNNING);
}

void IntentionExecutor::onIntentionDone(FinishState finishState)
{
    if (finishState == FinishState::NOT_YET) {
        JACK_ASSERT(finishState != FinishState::NOT_YET);
        return;
    }

    m_lastPlanFinishState = finishState;
    protocol::BDILogGoalIntentionResult intentionResult = {};
    if (finishState == FinishState::SUCCESS) {
        intentionResult = protocol::BDILogGoalIntentionResult_SUCCESS;
        m_succeededIntentions++;
    } else if (finishState == FinishState::DROPPED) {
        intentionResult = protocol::BDILogGoalIntentionResult_DROPPED;
        m_failedIntentions++;
    } else {
        intentionResult = protocol::BDILogGoalIntentionResult_FAILED;
        m_failedIntentions++;
    }
    logToBusIntention(&intentionResult);
    m_currentIntentionStarted = false;

    /// we *might* need a new plan, so we need to dirty the agent's schedule
    /// @todo There should be a new schedule state for this. i/e/ intention_failed.
    /// this is a work around for now
    m_agent->forceReschedule();
}

void IntentionExecutor::onGoalDone(FinishState finishState)
{
    ExecutorState prevState = m_state;
    (void)prevState; /// \note Make previous state available for debugging

    if (!setState(ExecutorState::CONCLUDED)) {
        return;
    }

    /// \note We do not log a goal finish on the bus. This goal is a clone of
    /// the desire that this intention is for so we'd be reporting that the
    /// cloned goal has finished which can be confusing because this is
    /// different from the desire.
    ///
    /// \note We do not log that the intention is finished either, that happens
    /// immediately after the plan has ticked into finish (or been dropped).
    /// This will hopefully make the logging a lot more accurate as log as soon
    /// as possible when the intention is finished. We now also only log goal
    /// finish when it's ready to be deleted from the agent which makes the
    /// semantics clearer.
    m_goal->finish(finishState);

    JACK_DEBUG("Goal is done [agent={}, goal={}]",
               m_agent->handle().toString(),
               m_goal->handle().toString());
}

void IntentionExecutor::drop(protocol::DropMode mode, std::string_view reason)
{
    /**************************************************************************
     * Drop the current intention (reason: direct request)
     **************************************************************************/
    if (setState(ExecutorState::FORCE_DROPPING)) {
        JACK_DEBUG("Drop intention (direct request) [agent={}, desire={}, plan={}, nextPlan={}, reason={}]",
                   m_agent->handle().toString(),
                   m_desireHandle.toString(),
                   m_currentIntention ? m_currentIntention->name() : "(none)",
                   m_targetIntention  ? m_targetIntention->name() : "(none)",
                   reason);
    }

    m_dropRequest.mode = mode;
    m_dropRequest.reason = reason;
}

/// Given the completion of a plan, apply the result onto the goal plan
/// selection policy and update the necessary book-keeping state for the
/// original desire.
///
/// @param agent The agent executing the plan
/// @param goal The instance of the desire used in the executor whose plan is
/// being achieved.
/// @param intention The plan that was just completed to achieve the goal
/// @param intentionSucceeded A flag indicating if the intention was completed
/// successfully or not
/// @param desire The root desire of the agent, this is the high-level
/// motivation of the agent that triggered the scheduler to spawn an instance of
/// the goal to achieve.
///
/// @return This function returns true if, after applying the result of the plan
/// on the goal policy book-keeping state, all plans as dictated by the goal policy
/// have been attempted. False otherwise, i.e. there are still more plans that
/// are valid to be selected for the achievement of this goal.
///
/// This function also returns false if any parameters are invalid.
static bool evaluatePlanResultForGoalPolicy(Agent* agent,
                                            const Goal* goal,
                                            const Plan* intention,
                                            const Tactic* tactic,
                                            bool intentionSucceeded,
                                            Goal* desire)
{
    ZoneScoped;
    bool result = true;
    if (!agent || !goal || !intention || !desire || !tactic) {
        JACK_INVALID_CODE_PATH;
        return result;
    }

    /// \note Update the plan execution history of the goal
    Goal::PlanSelection& planSelection = desire->m_planSelection;
    Goal::PlanHistory& entry  = planSelection.findOrMakeHistory(intention->name());
    entry.lastLoopIteration   = desire->m_planSelection.m_planLoopIteration;
    entry.successCount       += static_cast<unsigned int>(intentionSucceeded);
    entry.failCount          += static_cast<unsigned int>(!intentionSucceeded);

    /// \note First  branch covers the strict plan goal policy
    ///       Second branch covers the non-strict plan goal policy
    if (tactic->isUsingPlanList() && tactic->planOrder() == Tactic::PlanOrder::Strict) {
        const std::vector<std::string>& tacticPlans = tactic->plans();
        unsigned int& planIndex = planSelection.m_planListIndex;
        if (planIndex > tacticPlans.size()) {
            JACK_ERROR("Fixed plan selection policy is indexing out of bounds [agent={}, goal={}, index={}, bounds={}, planSelection={}]",
                       agent->handle().toString(),
                       goal->handle().toString(),
                       planIndex,
                       tacticPlans.size(),
                       planSelection.toString());
        } else {
            std::string_view expectedPlan = tacticPlans[planIndex];
            if (expectedPlan != intention->name()) {
                JACK_ERROR("A plan was given to the executor to execute for a goal, but, this goal "
                           "has a fixed plan policy and this plan is not the plan that was expected "
                           "to execute [agent={}, goal={}, currPlan={}, expectedPlan={}, planSelection={}]",
                           agent->handle().toString(),
                           goal->handle().toString(),
                           intention->name(),
                           expectedPlan,
                           planSelection.toString());
            }
        }

        /// \note Advance the fixed plan list
        planSelection.m_planListIndex = (planSelection.m_planListIndex + 1) % tacticPlans.size();

        /// \note We have attempted all plans if we've wrapped around the fixed
        /// plan list.
        result = planSelection.m_planListIndex == 0;
    } else {
        /// \note Check if we've attempted all the plans available to us for
        /// this goal for the current round of achieving the goal.
        const std::vector<const Plan*>& plansToCheck = agent->getGoalTacticPlans(goal);
        for (size_t planIndex = 0; planIndex < plansToCheck.size(); planIndex++) {
            const Plan* plan                 = plansToCheck[planIndex];
            const Goal::PlanHistory* history = planSelection.findHistory(plan->name());
            if (!history) {
                /// \note We have an applicable plan that we've
                /// never executed. We can still try to achieve this
                /// goal.
                result = false;
                break;
            }

            if (history->lastLoopIteration != desire->m_planSelection.m_planLoopIteration) {
                /// \note If the last time the plan was executed is
                /// prior to the current attempt to solve the goal,
                /// we havent executed the plan for this round of
                /// the goal yet.
                ///
                /// We still have plans to try before calling it
                /// quits.
                result = false;
                break;
            }
        }
    }

        return result;
}


void IntentionExecutor::execute()
{
    ZoneScoped;
    if (!isRunning() && !isDropping()) {
        return;
    }

    /// \note Check if the executor can execute the plan without conflicts
    bool planCanExecute = false;
    if (m_currentIntention) {
        JACK_ASSERT_MSG(
            isDelegated() == false,
            "If the intention has a plan to execute but it has been marked as "
            "delegated then this is a bug. Intentions are delegated when we "
            "have no plans for the goal, hence, this intention executor should "
            "be delegated to an agent. This executor itself is a proxy for the "
            "actual agent's intention that is executing it. In this case we "
            "*cannot* have a plan (current intention) assigned to this "
            "executor.");

        /// \note It is possible to have a resource violation, even after
        /// scheduling. As part of converting the scheduler's search nodes
        /// (intentions) to a DAG (sequenced intention execution chain), we try
        /// to parallelise the execution of this chain queueing any search nodes
        /// that have resource conflicts as sub-intentions to be executed after
        /// the parent intention has finished.
        ///
        /// Some actions may be completed faster than another and resources used
        /// across branches of the execution chain may cause a resource
        /// violation as they are actually run in the executor. Hence here we
        /// double check at point of execution that the intention is still
        /// possible to be run, and if there's a violation we don't execute
        /// until it's vacant again.
        ///
        /// i.e. Consider the final DAG tree
        ///
        /// 1. Intention 00 & 01 run in parallel
        ///
        ///     Executing
        ///     V
        ///     [Intention 00] --> [Intention 02]
        ///     [Locks(A)    ]     [Locks(A, B) ]
        ///
        ///     Executing
        ///     V
        ///     [Intention 01] --> [Intention 03]
        ///     [Locks(B)    ]     [Locks(B)    ]
        ///
        /// 2. Intention 00 finishes
        /// 3. Intention 02 starts executing
        ///
        ///                        Executing
        ///                        V
        ///     [Intention 00] --> [Intention 02]
        ///     [Locks(A)    ]     [Locks(A, B) ]
        ///
        ///     Executing
        ///     V
        ///     [Intention 01] --> [Intention 03]
        ///     [Locks(B)    ]     [Locks(B)    ]
        ///
        /// 4. Intention 01 completes
        /// 5. Intention 03 starts executing *but* Intention 02 is executing and
        /// has locked
        ///    (A, B) and conflicts with Intention 03 which need to lock (B).
        ///
        ///                        Executing
        ///                        V
        ///     [Intention 00] --> [Intention 02]
        ///     [Locks(A)    ]     [Locks(A, B) ]
        ///
        ///                        Executing (blocked by Intention 02 executor).
        ///                        V
        ///     [Intention 01] --> [Intention 03]
        ///     [Locks(B)    ]     [Locks(B)    ]
        ///
        BeliefContext &context = m_goal->context();
        planCanExecute         = !context.hasResourceViolation(nullptr /*violatedResources*/);
        if (!planCanExecute) {
            JACK_DEBUG("Resource violation in executor, skipping task execution for one tick [executor={}, desire={}, plan={}]",
                       m_id,
                       m_desireHandle.toString(),
                       m_currentIntention->name());
        }
    }

    /// \note Log that the intention started
    if ((planCanExecute || isDelegated()) && !m_currentIntentionStarted) {
        m_currentIntentionStarted = true;
        logToBusIntention(nullptr /*result*/);
    }

    /// \note Execute the plan
    /// Goal policy failure here tracks if the tactic for the plan has been
    /// exhausted (e.g. all plans tried, all plan loops exhausted). If so then
    /// this flag is set to true which will be used to transition the executor
    /// to flush the tactic from this executor.
    bool goalPolicyFailure = false;
    if (planCanExecute) {
        JACK_ASSERT(isDelegated() == false);

        /// \note Handle dropping of the plan or otherwise execute the plan
        BeliefContext &context = m_goal->context();
        if (isDropping() || m_currentIntention->shouldDrop(context)) {

            // transistion to a dropping state if required
            if (isRunning()) {
                setState(ExecutorState::DROPPING);
            }

            /// @todo: When we stop() an agent with subgoals there's a
            /// high chance that the intentions in the executor have a
            /// dependency chain on each other (parent intention sub-goals a
            /// child intention and so forth). If we are not careful about what
            /// we drop and delete we can easily delete a parent's intention
            /// memory before we deleted the child.
            ///
            /// Whether or not this happens all depends on the order of the
            /// intentions in the executor. In unit test
            /// 'GoalsTest.F23_ExecuteSubGoals' we appear to create a bunch of
            /// intentions that eventually go out of order before stopping the
            /// agent.
            ///
            /// Executing a plan normally works fine because in the plan body we
            /// track the number of dependent sub-goals before plan completion.
            ///
            /// We completely side step that in the drop coroutine. Most of the
            /// time we don't even have a drop coroutine which means that the
            /// drop() call returns immediately and deletes the intention
            /// potentially causing dangling pointers.
            ///
            /// We can guard against this by checking to ensure that the desires
            /// that are subgoal'ed from the intention no longer exist before we
            /// can consider the intention as finished dropping.
            uint16_t childrenExists = 0;
            for (const UniqueId& subGoal : m_subGoalDesireIds) {
                childrenExists += (m_agent->getDesire(subGoal) != nullptr);
            }

            if (!childrenExists) {
                m_currentIntention->drop(context, m_desireHandle, m_goal->id());
            }
        } else {
            m_currentIntention->tick(context, m_desireHandle, m_goal->id());
        }

        /// \note Log that the intention finished
        Plan::Status planStatus = m_currentIntention->status();
        if (planStatus.finishState != FinishState::NOT_YET) {

            /// @todo: In unit test
            /// IntentionsTestF.InversePreconditionPlans, there's a case where
            /// on completion of the action we send a message dirtying the
            /// agent's schedule and the alternative precondition-ed plan to
            /// kick in.
            ///
            /// The test expects that because the executor has finished
            /// executing, the executor will transition into concluding. The
            /// plan status will return the 'DROPPED' state because the new
            /// schedule will cause the current plan (that is finished!) to
            /// transition to dropping.
            ///
            /// Here we update the state to match said behaviour. This seems
            /// fairly dubious to have to do, we should revisit this quirk in
            /// the executor. It feels like the scheduler *should* take into
            /// account the executor and whether or not the goal is satisfied.

            FinishState planFinishState = planStatus.finishState;
            if (planFinishState == FinishState::DROPPED) {
                if (planStatus.bodyFinished) {
                    planFinishState = FinishState::SUCCESS;
                }
            }
            onIntentionDone(planFinishState);

            /// \note Calculate the goal policy failure flag
            Goal* desire = m_agent->getDesire(m_desireHandle.m_id);
            if (desire) {
                const Tactic* tactic = m_agent->currentTactic(desire->name());
                bool allPlansTriedForGoal =
                    evaluatePlanResultForGoalPolicy(m_agent,
                                                    m_goal,
                                                    m_currentIntention,
                                                    tactic,
                                                    planStatus.finishState == FinishState::SUCCESS,
                                                    desire);

                if (allPlansTriedForGoal) {
                    Goal::PlanSelection& planSelection = desire->m_planSelection;
                    uint32_t loopMaxCount = tactic->loopPlansCount();
                    if (loopMaxCount == Tactic::LOOP_PLANS_INFINITELY || loopMaxCount < planSelection.m_planLoopIteration) {
                        /// \note We are allowed to loop the plan selection.
                        /// Increment loop counter indicating we can and try
                        /// again in the scheduler.
                        planSelection.m_planLoopIteration++;
                    } else {
                        /// \note If the policy was to not allow loops, the goal
                        /// has failed if all plans have been attempted.
                        goalPolicyFailure = true;
                    }
                }

            } else {
                JACK_WARNING("Executor finished but desire no longer exists [agent={}, desire={}, plan={}]",
                             m_agent->handle().toString(),
                             m_goal->handle().toString(),
                             m_currentIntention->name());
            }
        }
    }

    /// \note Handle the completion of an intention, either the intention's
    /// coroutine was complete OR the intention's drop routine has completed.
    if (!m_currentIntention || m_currentIntention->finished()) {

        /// \note Transition the executor to the new intention if there is any
        if (isDelegated()) {
            JACK_ASSERT_MSG(!m_currentIntention, "Delegations can *not* have intentions");
            JACK_ASSERT_MSG(!m_targetIntention,  "Delegations can *not* have intentions");
        } else {
            JACK_DELETE(m_currentIntention);
            m_currentIntention = m_targetIntention;
            m_targetIntention  = nullptr;
        }

        /// \note Advance the executor based on the state of the executor.
        if (m_state == ExecutorState::FORCE_DROPPING || goalPolicyFailure) {
            JACK_DEBUG("Intention finished dropping [agent={}, goal={}]",
                       m_agent->handle().toString(),
                       m_goal->handle().toString());
            onGoalDone(m_lastPlanFinishState == FinishState::NOT_YET ? FinishState::DROPPED : m_lastPlanFinishState);
        } else {
            /// \note State is DROPPING or RUNNING. The previous plan
            /// (intention) has finished OR it was dropped and marked finished,
            /// we will re-evaluate if the desire has been achieved by getting
            /// the executor to re-evaluate the precondition.
            ///
            /// In the precondition, if the desire has not been achieved and
            /// there was a plan to transition into, this has already occured
            /// above and the executor will prepare to execute the new plan.
            checkPrecondition();
        }
    }
}

bool IntentionExecutor::setState(ExecutorState newState)
{
    if (m_state == ExecutorState::CONCLUDED) {
        return false;
    }

    /// \note We can't transition into dropping from force dropping because
    /// force dropping has a higher priority. Typically force dropping is the
    /// result of a "direct" drop request which means the intention should
    /// not stick around.
    if (m_state == ExecutorState::FORCE_DROPPING && newState == ExecutorState::DROPPING) {
        return false;
    }

    m_state = newState;
    return true;
}

bool IntentionExecutor::isWaiting() const
{
    /*! ***************************************************************************************
     * Check if this goal executor is in a waiting state
     * ****************************************************************************************/
    bool busyWaitingOnTaskInPlan     = isRunning()   && m_currentIntention    && m_currentIntention->waiting();
    bool waitingOnDelegatedIntention = isDelegated() && m_lastPlanFinishState == FinishState::NOT_YET;
    bool result                      = isWaitingForPlan() || busyWaitingOnTaskInPlan || waitingOnDelegatedIntention;
    return result;
}

void IntentionExecutor::addSubGoalDesireId(const UniqueId &otherId)
{
    assert(m_desireHandle.m_id != otherId);
    m_subGoalDesireIds.push_back(otherId);
}

void IntentionExecutor::logToBusIntention(protocol::BDILogGoalIntentionResult *result)
{
    /// \note We use the goal ID for the intention ID, this is different
    /// from the ID in the desire handle. Each intention gets assigned
    /// a unique instance of the goal so it can serve as the unique id of
    /// the intention.
    ///
    /// The desire is the instance of the goal that was originally pursued
    /// in the agent (intentions clone the original desire, the following
    /// assert ensures that this is the case.).
    assert(m_desireHandle.m_name == m_goal->name());
    assert(m_desireHandle.m_id != m_goal->id());

    Engine& engine = m_agent->engine();
    if (!engine.haveBusAdapter()) {
        return;
    }

    std::string_view intentionName;
    if (isDelegated()) {
        intentionName = "<Delegated to team member>";
    } else {
        if (m_currentIntention) {
            intentionName = m_currentIntention->name();
        } else {
            intentionName = "<No suitable plans>";
        }
    }

    protocol::BDILogHeader header = m_agent->bdiLogHeader(protocol::BDILogLevel_NORMAL);
    protocol::BDILog logEvent = {};
    if (result) {
        logEvent = engine.makeBDILogIntentionFinished(header,
                                                      m_desireHandle.m_name,       /// goal
                                                      m_desireHandle.m_id.toString(), /// goalId
                                                      m_goal->id().toString(),        /// intentionId
                                                      intentionName,               /// plan
                                                      *result);                    /// success
    } else {
        logEvent = engine.makeBDILogIntentionStarted(header,
                                                     m_desireHandle.m_name,       /// goal
                                                     m_desireHandle.m_id.toString(), /// goalId
                                                     m_goal->id().toString(),        /// intentionId
                                                     intentionName);              /// plan
    }
    engine.sendBusEvent(&logEvent);
}
} /// namespace aos::jack
