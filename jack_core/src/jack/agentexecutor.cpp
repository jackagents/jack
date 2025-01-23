#include <jack/agentexecutor.h>
#include <jack/agent.h>                     // for Agent
#include <jack/beliefcontext.h>             // for BeliefContext
#include <jack/dag.h>                       // for DAGNode, IntentionExecutionDAG
#include <jack/goal.h>                      // for Goal
#include <jack/plan.h>                      // for Plan
#include <jack/schedule.h>                  // for SearchNode, Schedule, JACK_SCH...
#include <jack/event-protocol/protocol.h>
#include <jack/engine.h>                    // for Engine
#include <jack/engine/intentionexecutor.h>  // for IntentionExecutor, IntentionEx...
#include <jack/engine/uuid.h>               // for UniqueId
#include <jack/events/delegationevent.h>    // for DelegationEvent
#include <jack/events/event.h>              // for Event, Event::SUCCESS, Event::...

/// Third Party
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <algorithm>
#include <cstddef>                     // for size_t
#include <cassert>                     // for assert
#include <utility>                     // for max, min, pair, swap
#include <vector>                      // for vector, _Vector_iterator, _Vec...
#include <string>                      // for operator+, operator<<, string

namespace aos::jack
{
/* ************************************************************************************************
 * Public Ctor & Dtor
 * ************************************************************************************************/
AgentExecutor::AgentExecutor(Agent* agent)
    : m_agent(agent)
    , m_scheduleValid(false)
{
}

AgentExecutor::~AgentExecutor()
{
    for (IntentionExecutor* ptr : m_intentions) {
        JACK_ALLOCATOR_DELETE(&m_agent->m_heapAllocator, ptr);
    }

    if (m_schedule != nullptr) {
        JACK_ALLOCATOR_DELETE(&m_agent->m_heapAllocator, m_schedule);
    }

    // delete the intention dag?

   // delete m_main;
}

/* ************************************************************************************************
 * Public Accessors & Mutators
 * ************************************************************************************************/

bool AgentExecutor::done() const
{
    /// \note The executor is considered done if the schedule has been
    /// invalidated
    if (!m_scheduleValid) {
        return true;
    }

    /// \note The executor is not done yet if there are still plans to be
    /// executed according to a our list of best intentions (the DAG).
    if (!m_dag.done()) {
        return false;
    }

    /// \note The executor has assigned all the plans from the list of best
    /// intentions (the DAG). If all the executors no longer have a plan
    /// queued up for execution then we have finished executing the schedule.
    bool allExecutorsIdle = true;
    for (auto it = m_intentions.begin();
         allExecutorsIdle && it != m_intentions.end();
         it++)
    {
        allExecutorsIdle &= (*it)->isWaitingForPlan();
    }

    return allExecutorsIdle;
}

AgentExecutor::RunningState AgentExecutor::runningState() const
{
    if (m_intentions.empty()) {
        return RunningState::IDLE;
    }

    assert(m_intentions.size());
    bool allWaiting = static_cast<std::size_t>(waitingCount()) == m_intentions.size();
    return allWaiting ? RunningState::BUSY_WAITING_ON_EXECUTOR : RunningState::EXECUTING;
}

void AgentExecutor::update()
{
    ZoneScoped;
    m_waitingCount = 0;
    m_workingCount = 0;

    /// \note We have to store the locks from the executor because the act of
    /// executing can transition the underlying plan out and we need to undo
    /// the changes of the lock.
    struct IntentionWithLocks
    {
        IntentionExecutor *      executor;
        std::vector<std::string> locks;
    };

    ThreadScratchAllocator scratchArena = getThreadScratchAllocator(/*otherArena*/ nullptr);
    StaticArray<IntentionWithLocks> intentionAndLocks(scratchArena.arena, m_intentions.size());

    for (size_t intentionIndex = 0; intentionIndex < m_intentions.size(); intentionIndex++) {
        IntentionExecutor* intention = m_intentions[intentionIndex];
        m_intentionBeingExecuted     = intention;

        /// \note If we have already started executing this intention coroutine
        /// we will re-lock resources that are already locked. This is ok! When
        /// the task in the coroutine finishes, the locks will be released
        /// again and the resource violation is resolved here.
        ///
        /// Alternatively, we can look at this condition as, the executor
        /// *must* be able to lock the resources to be able to advance the
        /// coroutine.
        ///
        /// \todo We should be able to preview resource locks on a per-task
        /// basis, and, we should only preview the lock on resources
        /// required by the next task to execute, this way, we block the
        /// executor at a task level and not at the plan level.
        if (intention->currentIntention()) {
            const Plan *plan                              = intention->currentIntention();
            const std::vector<std::string>& resourceLocks = plan->resourceLocks();
            intentionAndLocks.emplace_back(intention, resourceLocks);

            BeliefContext &context = intention->context();
            context.lockResources(resourceLocks);
        }

        intention->execute();

        bool waiting   = intention->isWaiting();
        bool concluded = intention->isConcluded();

        if (waiting) {
            m_waitingCount++;
        }

        if (!concluded) {
            m_workingCount++;
        }

        if (waiting || !concluded) {
            /// \todo This causes excessive log spam, but its actually
            /// occasionally helpful when debugging stalled executors.
            #if 0
            JACK_DEBUG("Agent is working " << (waiting ? "(blocked waiting)" : "") << "[agent="
                                           << m_agent->handle()
                                           << ", goal=" << intention->goal() << ", tick=" << m_agent->engine().pollCount() << "]");
            #endif
        }
    }

    for (const auto &entry : intentionAndLocks) {
        BeliefContext &context = entry.executor->context();
        context.unlockResources(entry.locks);
    }

    m_intentionBeingExecuted = nullptr;
}


// if there is already an intention return that and reuse it.
IntentionExecutor *AgentExecutor::findMatchingIntention(DAGNode* n)
{
    // brute force lookup
    /// \todo: optimise this
    for(IntentionExecutor* intention : m_intentions) {
        if (n->node->goal->handle() == intention->desireHandle()) {
            return intention;
        }
    }

    return nullptr;
}

IntentionExecutor *AgentExecutor::findMatchingIntentionByGoalId(const UniqueId &id)
{
    // brute force lookup
    /// \todo: optimise this
    for(IntentionExecutor* intention : m_intentions) {
        if (id == intention->desireHandle().m_id) {
            return intention;
        }
    }
    return nullptr;
}

IntentionExecutor *AgentExecutor::findMatchingIntentionByIntentionId(IntentionExecutor::Id id)
{
    // brute force lookup
    /// \todo: optimise this
    if (id != IntentionExecutor::NULL_ID) {
        for(IntentionExecutor* intention : m_intentions) {
            if (id == intention->id()) {
                return intention;
            }
        }
    }
    return nullptr;
}

bool AgentExecutor::onActionTaskComplete(const UniqueId& intentionId, const UniqueId& taskId, bool success, std::shared_ptr<Message> reply)
{
    bool result = false;

    for (size_t intentionIndex = 0; intentionIndex < m_intentions.size(); intentionIndex++) {
        IntentionExecutor* intention = m_intentions[intentionIndex];
        if (intention->goalPtr()->id() != intentionId) {
            continue;
        }

        Plan* plan = intention->currentIntention();
        if (!plan) {
            continue;
        }

        if (plan->onTaskComplete(taskId, success)) {
            /// Unlock the resources
            BeliefContext &beliefContext = m_agent->context();
            beliefContext.unlockResources(plan->resourceLocks());

            // Add the reply message into the intention's plan context
            intention->context().addActionReplyMessage(reply);

            /// Advance the coroutine
            result = true;
            break;
        }
    }

    return result;
}

void AgentExecutor::processDAGNode(IntentionExecutor* executor, DAGNode* node, ProcessDAGNodeMode mode)
{
    if (!node || !executor) {
        return;
    }

    const GoalHandle& goalHandle = node->node->goal->handle();
    const Plan*       nextPlan   = node->node->plan;

    /// Check if we need to transition the executor into the new plan
    bool executorNeedsToChangePlan = true;
    if (mode == ProcessDAGNodeMode::ON_NEW_SCHEDULE) {
        /// \note When we have a new schedule, we don't change plans unless the
        /// plan name has changed. This prevents churning of the executor on
        /// dirty schedules when the plan has remained the same.
        const Plan* currPlan = executor->currentIntention();
        if (currPlan && nextPlan) {
            executorNeedsToChangePlan = currPlan->name() != nextPlan->name();
        }
    } else {
        /// \note When closing a node in the DAG for an intention means that
        /// the intention was concluded or all plans queued to execute were
        /// dropped. The DAG tells us what the next plan is that we can execute.
    }

    /// \note There are no harmful side-effects when the DAG node is a
    /// delegation, the executor will respect delegations when you call
    /// `setPlan` (e.g. the plan will be null which it handles gracefully).
    if (executorNeedsToChangePlan) {
        Plan* intentionPlan = JACK_CLONE_BDI_OBJECT(nextPlan);
        if (intentionPlan) {
            intentionPlan->reset(m_agent);
        }
        executor->setPlan(intentionPlan);
    }

    /// \note Set the agent/goal context
    std::shared_ptr<Message> goalMsg = node->node->goal->context().goal();
    executor->setContext(m_agent->context(), goalMsg);

    /// \note Update book-keeping
    node->intention       = executor;
    m_dagLookup[executor] = node;

    /// \note Create a delegation event to execute this in another agent
    if (!executor->isConcluded() && node->node->delegate) {
        auto it = m_delegations.find(goalHandle);
        if (it == m_delegations.end()) {
            JACK_DEBUG("Intention delegated [team={}, delegate={}, goal={}]", m_agent->handle(), node->node->delegate->handle(), goalHandle);
            m_agent->delegateGoal(goalHandle, node->node->delegate, node->node->goal->context().goal());
            m_delegations[goalHandle] = node->node->delegate; ///< Store the delegation mapping
        }
    }
}

void AgentExecutor::setSchedule(Schedule *schedule)
{
    ZoneScoped;
    /**************************************************************************
     * Process A.3 - Execute
     * Responsible for taking a new schedule and preparing it for execution and
     * transitioning the current agent's intentions to align with the start of
     * this schedule
     *
     * Sub-Processes:
     *   1. Initialise DAG (A.3.1)
     *   2. Generate Transition Events(A.3.2)
     *   3. Execute DAG (A.3.3)
     *   4. Monitor Actions(A.3.4)
     *
     **************************************************************************/
    TracyCZoneN(debugTracyValidateSchedule, "Validate the new schedule", true);
    if (m_schedule != nullptr) {
        ZoneNamedN(debugTracyDeleteOldSchedule, "Delete old schedule", true);
        JACK_ALLOCATOR_DELETE(&m_agent->m_heapAllocator, m_schedule);
    }

    m_schedule      = schedule;
    m_scheduleValid = true;

    // * 1. Initialise DAG (A.3.1)
    m_dag.setSchedule(schedule);

    /**************************************************************************
     * Remove impossible goals from the schedule
     **************************************************************************/
    /// \todo This chunk of code will remove a goal if after planning it is
    /// deemed impossible and is not persistent.
    ///
    /// This is change is useful, but guarded by the preprocessor because this
    /// is not robust enough and can cause crashes due to the schedule or
    /// executor holding onto pointers that it shouldn't.
    ///
    /// If we experience crashes elsewhere we should turn this off.
    #if AGENT_REMOVE_IMPOSSIBLE_GOALS_AFTER_SCHEDULING_WORK_AROUND
    {
        ThreadScratchAllocator scratchArena = getThreadScratchAllocator(/*otherArena*/ nullptr);

        std::vector<Schedule::PlannerGoal>& plannerGoals = m_schedule->plannerGoals();
        for (auto it = plannerGoals.begin(); it != plannerGoals.end(); ) {
            const Schedule::PlannerGoal& plannerGoal = *it;
            if (plannerGoal.canBePlanned) {
                it++;
                continue;
            }

            /// \note This goal was not plannable, e.g. the schedule was
            /// unable to put this goal into the search space.
            /// This means this goal is not something we can run in the
            /// current context.
            /// If the goal is persistent, we can keep it around, otherwise
            /// we need to consider dropping it.
            Goal* goal = plannerGoal.goal;
            if (goal->isPersistent()) {
                it++;
                continue;
            }

            bool hasIntention = false;
            #if 0
            for (auto it = intentions().begin(); it != intentions().end(); it++) {
                IntentionExecutor* executor = *it;
                if (executor->desireHandle() != goal->handle()) {
                    continue;
                }

                /// \note Maybe check that the plan hasn't yielded or is pending?
                const Plan* plan = executor->currentIntention();
                hasIntention     = plan && plan->finished();
                break;
            }
            #else
            /// \note Whilst this goal is not-plannable, we already have an
            /// intention for it, we will let the intention finish/terminate
            /// the goal as they have book-keeping they need to clean up.
            ///
            /// We *only* want to terminate goals that were freshly-planned
            /// but were still invalid even after planning.

            /// \todo There's definitely a bug here, if the intention is pending
            /// then these inprogress intentions will never get deleted when
            /// they should. I think the #ifdef branch above might fix this but
            /// it needs some more testing. I have a feeling it might break some
            /// tests.
            for (auto intentionIt = intentions().begin(); !hasIntention && intentionIt != intentions().end(); intentionIt++) {
                hasIntention = (*intentionIt)->desireHandle() == goal->handle();
            }
            #endif

            if (hasIntention) {
                it++;
                continue;
            }

            /// \note We will drop the goal and log the drop
            JACK_INFO("Agent dropped goal because it was not plannable (e.g. no plans, no available members, "
                      "precondition failed) [agent={}, goal={}]", m_agent->handle().toString(), goal->handle().toString());

            ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
            auto                   builder = StringBuilder(scratch.arena);
            builder.appendCopy(m_agent->handle().toHumanString("Agent"));
            builder.appendRef(" dropped goal ");
            builder.appendCopy(goal->handle().toHumanString());
            builder.appendRef(" because it was not plannable (e.g. no plans, no available members, or the precondition failed)");

            std::string_view reason = builder.toStringArena(scratch.arena);
            TracyMessage(reason.data(), reason.size());
            JACK_INFO("{}", reason);

            /// \note Drop and erase the goal
            m_agent->dropWithMode(goal->handle(), protocol::DropMode_NORMAL, reason);
            it = plannerGoals.erase(it);
        }
    }
    #endif

    /**************************************************************************
     * Delete intentions not in the schedule
     **************************************************************************/
    /// \todo Check in the new dag if we generated some new intentions to run
    /// and compare it to the current list of running intentions. If there's
    /// a new intention in the DAG then ideally we pause the intention.
    ///
    /// And in the ideal implementation, we would also be able to resume the
    /// intention from where it left off. However we currently don't
    /// support resuming an intention as there's no way, currently to
    /// indicate to the agent's action that an intention has resumed after
    /// being side tracked with another goal, like a maintenance goal, and
    /// that the action/agent context should switch back to finishing the
    /// paused intention.
    ///
    /// i.e. Eventually the action handling api might look like
    ///
    /// if (onActionResume(actionHandle)) { }
    /// if (onActionStarted(actionHandle)) { }
    ///
    /// Which allows us to detect and update the actual robot to react
    ///
    /// So in this work-around, ideally we drop & delete the intention to
    /// restart the plan from the start (since we don't support resuming).
    for (auto it = m_intentions.begin(); it != m_intentions.end(); it++) {
        bool intentionInNewDag = false;
        for (const DAGNode& n : m_dag.dagNodes()) {
            if (n.node->goal->handle() == (*it)->desireHandle()) {
                intentionInNewDag = true;
                break;
            }
        }

        if (intentionInNewDag) {
            continue;
        }

        IntentionExecutor *intention = *it;
        if (intention->currentIntention()) {
            /// \note If the current intention is finished, then don't
            /// prematurely drop it, since, after this schedule is set, the
            /// AgentExecutor will update and naturally delete the intention
            /// from the agent, updating and notifying necessary subsystems.
            if (intention->currentIntention()->finished()) {
                continue;
            }
        }

        /// \todo We let the team member handle their own intentions. Dropping
        /// intentions that are delegated from the team side will trigger drops
        /// on the agent potentially overriding the agent's maintenance goal and
        /// cause a ping-ponging effect.
        ///
        /// Additionally teams already have their own codepaths for dropping
        /// delegated goals and also that teams do not support maintenance
        /// goals (yet).
        if (intention->isDelegated()) {
            continue;
        }

        /// \note We check if the intention that is still being executed in
        /// the agent is one of the desires of an agent. Even though a
        /// schedule may not have been able to create a successful meta-plan
        /// to achieve all the goals, these intentions we created prior are
        /// still valid to be executed.
        ///
        /// It's possible in the schedule that you have a problematic desire
        /// that has suddenly become impossible (e.g. a service went
        /// offline). This schedule would fail as it couldn't completely
        /// achieve all goals, that does not mean that the remaining valid
        /// goals and their intentions should be deleted.
        ///
        /// \todo: Is the problem here
        /// that we are not correctly generating partial schedules? Not all
        /// goals need to be achieved in the schedule for it to be valid?
        const std::vector<Schedule::PlannerGoal>& plannerGoals  = m_schedule->plannerGoals();
        bool goalDesiredAndIsIntentionButNotInSchedule          = false;
        for (size_t goalIndex = 0; goalIndex < plannerGoals.size(); goalIndex++) {
            const Schedule::PlannerGoal& plannerGoal = plannerGoals[goalIndex];
            const Goal *goal                         = plannerGoal.goal;
            if (intention->desireHandle() == goal->handle() && plannerGoal.canBePlanned) {
                /// \note This goal is currently being executed and it's
                /// plannable. It's not in the schedule but the agent wants
                /// to do it, and, is already doing it. We will not delete
                /// the intention. We don't support partial schedules yet(?)
                goalDesiredAndIsIntentionButNotInSchedule = true;
                break;
            }
        }

        if (goalDesiredAndIsIntentionButNotInSchedule) {
            continue;
        }

        /// \note We pipe the drop through the agent so that we create an
        /// event for traceability.
        const AgentHandle& agentHandle = m_agent->handle();
        const GoalHandle& desireHandle = intention->desireHandle();
        const Plan* plan               = intention->currentIntention();

        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        auto                   builder = StringBuilder(scratch.arena);
        std::string agentHandleLabel   = agentHandle.toHumanString("Agent");
        std::string desireHandleLabel  = desireHandle.toHumanString();

        builder.append(FMT_STRING(
            "{} is dropping an intention because it generated a schedule "
            "that no longer includes this intention for execution. The "
            "intention being dropped is:\n"
            "\n"
            "  {}\n"
            "  Plan: {}\n"
            "\n"),
            agentHandleLabel,
            desireHandleLabel,
            plan ? plan->name() : "none");

        std::set<std::string> scheduledGoals;
        for (const DAGNode& dagNode : m_dag.dagNodes()) {
            scheduledGoals.insert(scheduledGoals.end(), dagNode.node->goal->name());
        }

        if (scheduledGoals.size()) {
            builder.appendRef("The planner has scheduled the following tasks to execute\n\n");
            for (auto setIt = scheduledGoals.begin(); setIt != scheduledGoals.end(); setIt++) {
                if (setIt != scheduledGoals.begin()) {
                    builder.appendRef("\n");
                }
                builder.append(FMT_STRING("  {}"), *setIt);
            }
        } else {
            builder.appendRef("The planner has not scheduled any goals to be executed");
        }

        std::string_view dropReason = builder.toStringArena(scratch.arena);
        internalDrop(intention->desireHandle(), intention->dropRequest().mode, dropReason);
    }
    TracyCZoneEnd(debugTracyValidateSchedule);

    /**************************************************************************
     * Create intentions for the intention executor
     **************************************************************************/
    // * 2. Generate Transition Events(A.3.2)
    // Compare the open nodes with the current intentions and generate the
    // appropriate transitions. In this first loop
    //   1. Delegates that were executing the task and are no-longer chosen will
    //      also be forced to drop the task.
    //   2. Pre-existing delegates that are assigned a new task will have to
    //      drop their current tasks. Any
    TracyCZoneN(debugTracyRemoveDelegations, "Remove delegations from members not in the new schedule", true);
    const std::vector<DAGNode*> &open = m_dag.open();
    for (DAGNode* n : open) {

        JACK_ASSERT(n->intention == nullptr);
        #if defined(LET_TEAM_DELEGATE_FAILED_AUCTIONS)
        #else
        JACK_ASSERT_MSG(n->node->costOfNode != Schedule::FAILED_COST,
                        "A node that has a failed cost should never have been added to the DAG");
        #endif

        if (n->node->isDelegation()) {
            // 1. Delegates that were executing the task and are no-longer
            //    chosen will also be forced to drop the task.
            const GoalHandle &goalHandle = n->node->goal->handle();
            auto existingDelegation = m_delegations.find(goalHandle);
            if (existingDelegation != m_delegations.end()) {
                aos::jack::Agent *existingAgent = existingDelegation->second;
                if (existingAgent != n->node->delegate) {

                    // Drop the delegation from the previous agent that was doing the task.
                    m_agent->dropDelegation(goalHandle, existingAgent);
                    m_delegations.erase(goalHandle);
                }
            }
        }
    }
    TracyCZoneEnd(debugTracyRemoveDelegations);

    /// \todo No intention transitioning yet
    /// In the second loop, generate the new intentions and dispatch them to the
    /// agents.
    TracyCZoneN(debugTracyUpdateOrCreateNewIntentionExecutors, "Update or create new intention executors", true);
    for (DAGNode* n : open) {
        // \todo this doesn't handle team member allocation and resource allocations
        IntentionExecutor* executor = findMatchingIntention(n);
        if (executor == nullptr) {
            // First time we are executing this goal/plan (intention). Setup an
            // executor to run the intention.
            executor = JACK_ALLOCATOR_NEW(&m_agent->m_heapAllocator, IntentionExecutor, m_agent, n->node->goal, static_cast<bool>(n->node->delegate));
            m_intentions.push_back(executor);
            JACK_DEBUG("New intention exec [agent={}, goal={}, intentions={}]",
                       m_agent->handle().toString(),
                       executor->desireHandle().toString(),
                       m_intentions.size());
        }
        processDAGNode(executor, n, ProcessDAGNodeMode::ON_NEW_SCHEDULE);
    }
    TracyCZoneEnd(debugTracyUpdateOrCreateNewIntentionExecutors);

    // * 2. Generate Transition Events(A.3.2)

    /// \todo implement transition phase

    // * 3. Execute DAG (A.3.3)
}

bool AgentExecutor::internalDrop(const GoalHandle &handle, protocol::DropMode mode, std::string_view reason)
{
    bool result = false;
    IntentionExecutor *intention = nullptr;
    for(IntentionExecutor* check : m_intentions) {
        if (check->desireHandle() == handle) {
            intention = check;
            break;
        }
    }

    if (intention) {
        result = true;
        intention->drop(mode, reason);
        m_dagLookup.erase(intention);
        JACK_DEBUG("Goal drop [agent={}, goal={}, intentionId={}]", m_agent->handle(), handle, intention->goalPtr()->handle().m_id);
    } else {
        JACK_DEBUG("Goal drop requested but no intention to drop [agent={}, goal={}]", m_agent->handle(), handle);
    }

    // drop any team delegations
    auto it = m_delegations.find(handle);
    if (it != m_delegations.end()) {
        m_agent->dropDelegation(handle, it->second);
        m_delegations.erase(it /*DelegationEvent*/);
    }

    // need a new schedule
    m_scheduleValid = false;
    return result;
}

void AgentExecutor::handleDelegationEvent(const DelegationEvent *event)
{
    for (IntentionExecutor* executor : m_intentions) {
        if (event->goalHandle != executor->desireHandle())
            continue;

        if (event->status == Event::Status::SUCCESS) {
            executor->onIntentionDone(FinishState::SUCCESS);
        } else {
            // If the delegation failed, reset the intention to allow
            // replanning again and another attempt

            /// \note Although this works at the moment it has to wait for the goal to fail or
            /// replan. It certainly doesn't work well for delivery robots because it has multiple
            ///  missions generated from the delegation action. This means this won't re-allocate
            /// until the team planner runs and creates a new allocation trigger when all missions
            /// have finished for a single goal allocation this might cause a re-plane
            /// ( need to check )

            //intention->onGoalPrepareForNextIteration();

            /// \note Also bear in mind that failing plans don't currently work.

            /// \note This will just force the goal to drop if the delegation fails.
            /// This is instead switching to waiting for new plan above
            executor->drop(executor->dropRequest().mode, "Team delegation to member failed");

            // \todo This will cause the team to perpetually continue to replan
            // any delegated plan that has failed. In the future, we may wish to
            // have a policy for allowing the team to eventually drop an
            // intention that consistently fails.
            //
            // For now we will continue to replan until the intention has
            // succeeded.
        }
        executor->execute();

        // Find and remove the delegation + allocation
        auto it = m_delegations.find(event->goalHandle);
        if (it != m_delegations.end()) {
            m_delegations.erase(it);
        }
        break;
    }
}

// close this execution node and execute the next node(s)
// we may keep the intention
void AgentExecutor::close(IntentionExecutor* intention)
{
    ZoneScoped;
    auto it = m_dagLookup.find(intention);
    if (it == m_dagLookup.end()) {
        return;
    }
    std::vector<DAGNode*> openingNodes = m_dag.close(it->second);
    // just remove it for now
    // not optimal - we should only remove this if we remove the intention
    m_dagLookup.erase(it);

    /// Process the intentions that are now unlocked by closing this node.
    for (DAGNode* n : openingNodes) {

        /// \todo I think we can use `findMatchingIntention` in `processDAGNode`
        /// instead of checking if the node matches the intention. Otherwise as
        /// it is currently this might cause us to generate multiple intentions
        /// for the same desire.
        ///
        /// Needs some very careful thought. But I've preserved this behaviour
        /// because this is what the old code did. It unconditionally made new
        /// executors for downstream nodes.

        IntentionExecutor* executor = nullptr;
        Goal*              goal     = n->node->goal;
        if (goal->handle() == intention->desireHandle()) {
            executor = intention;
        } else {
            executor = JACK_ALLOCATOR_NEW(&m_agent->m_heapAllocator, IntentionExecutor, m_agent, goal, static_cast<bool>(n->node->delegate));
            m_intentions.push_back(executor);
        }

        processDAGNode(executor, n, ProcessDAGNodeMode::ON_CLOSE);
    }
}

void AgentExecutor::stop()
{
    /*! ***************************************************************************************
     * Stop the Executor
     * This will occur when the agent is stopping
     * \todo Test this on stopping a Team - will need to drop delegations too
     * ****************************************************************************************/

    /// Delete all the desires that didn't have an intention, the ones with an
    /// intention will transition to drop and delete themselves when it's valid
    /// to do so.
    for (auto it = m_agent->m_desires.begin(); it != m_agent->m_desires.end();) {
        bool hasIntention = false;
        for (IntentionExecutor* intention : m_intentions) {
            if ((*it)->handle() == intention->desireHandle()) {
                hasIntention = true;
                break;
            }
        }

        if (hasIntention) {
            it++;
        } else {
            it = m_agent->m_desires.erase(it);
        }
    }

    /// Triggering a drop will queue the intentions to shutdown gracefully and
    /// delete themselves when they're ready to be deleted. We do not need to
    /// delete it here.
    for (IntentionExecutor* intention : m_intentions) {
        /// \note We could already be in the process of dropping or finishing
        /// the intention, we don't want to double-stop.
        if (intention->isDropping() || intention->isConcluded()) {
            continue;
        }
        intention->drop(intention->dropRequest().mode, "Stopping agent");
    }

    m_scheduleValid = false;
    m_dag.reset();
    m_dagLookup.clear();
}


/// a process for handling a new schedule
/// \todo rename this, it's too vague
void AgentExecutor::execute()
{
    ZoneScoped;
    /*! ***************************************************************************************
     * Process A.3 - Execute
     * Responsible for taking a new schedule and preparing it for execution and transitioning
     * the current agent's intentions to align with the start of this schedule
     *
     *
     * Sub-Processes:
     * 3. Execute DAG (A.3.3)
     * 4. Monitor Actions(A.3.4)
     *
     * Inputs:
     *
     * Outputs:
     *
     * Tools:
     *
     * ****************************************************************************************/

    /// 1.  Execute the intentions
    update();

    // if the dag is empty and an intention is waiting at this point then we're done
    // and need a new schedule.

    /// Just take the best intention from the schedule and execute only that at the moment.
    /// \todo execute the proper schedule - only re-schedule if the something fails

    ThreadScratchAllocator scratchArena = getThreadScratchAllocator(/*otherArena*/ nullptr);
    StaticArray<IntentionExecutor*> toRemove(scratchArena.arena, m_intentions.size());

    /// \note Snap the number of intentions to iterate. `close` may modify the
    /// intentions vector and the new ones inserted do *not* need to be
    /// checked in this loop.
    size_t intentionCount = m_intentions.size();

    for (size_t intentionIndex = 0; intentionIndex < intentionCount; intentionIndex++) {
        IntentionExecutor* intention = m_intentions[intentionIndex];

        if (intention->isConcluded() || intention->isWaitingForPlan()) {
            if (m_scheduleValid) {
                /// \note Close the DAG node associated with the intention and
                /// unlock the next nodes in the DAG
                /// \note WARNING This may modify the `m_intentions` vector!
                close(intention);
            }
        }

        if (intention->isConcluded()) {
            JACK_DEBUG("Removing intention [goal={}]", intention->desireHandle().toString());
            const IntentionExecutor::DropRequest& dropRequest = intention->dropRequest();
            toRemove.push_back(intention);
            if (!intention->goalPtr()->isPersistent() || dropRequest.mode == protocol::DropMode_FORCE || m_agent->m_state == Service::STOPPING) {
                Engine& engine = m_agent->engine();
                Goal* desire   = m_agent->getDesire(intention->desireHandle().m_id);
                if (!desire) {
                    continue;
                }
                JACK_DEFER { m_agent->eraseDesire(desire->handle()); };

                /// \todo It's not very obvious why we don't finish the
                /// desire. The cloned instance copies the promise and
                /// triggers the promise/future of the goal for us. I'm not
                /// sure that's correct, maybe it is. I find it kind of
                /// confusing, needs consideration.
                ///
                /// desire->finish(desireSuccess);

                /// \note Record the completion of the goal onto the bus
                if (!engine.haveBusAdapter()) {
                    continue;
                }


                FinishState goalFinish = intention->goalPtr()->finished();
                JACK_ASSERT(goalFinish != FinishState::NOT_YET);
                protocol::BDILogGoalIntentionResult finish = protocol::finishStateToBDILogGoalIntentionResult(goalFinish);

                protocol::BDILogHeader header = m_agent->bdiLogHeader(protocol::BDILogLevel_NORMAL);
                protocol::BDILog logEvent     = {};
                if (desire->parent().handle.valid()) {
                    logEvent = engine.makeBDILogSubGoalFinished(
                        header,
                        desire->handle().m_name,                 /// goal
                        desire->handle().m_id.toString(),        /// goalId
                        desire->parent().handle.m_id.toString(), /// intentionId
                        desire->parent().planTaskId.toString(),  /// taskId
                        dropRequest.reason,                      /// dropReason
                        finish);
                } else {
                    logEvent = engine.makeBDILogGoalFinished(
                        header,
                        desire->handle().m_name,
                        desire->handle().m_id.toString(),
                        dropRequest.reason,
                        finish);
                }

                engine.sendBusEvent(&logEvent);
            }
        }
    }

    for(const IntentionExecutor* intention : toRemove) {
        auto it = std::find(m_intentions.begin(), m_intentions.end(), intention);
        JACK_ASSERT(it != m_intentions.end());
        m_intentions.erase(it);
        JACK_ALLOCATOR_DELETE(&m_agent->m_heapAllocator, intention);
    }
}
} // namespace aos::jack
