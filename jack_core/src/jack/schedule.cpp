// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/agent.h>
#include <jack/beliefcontext.h>          // for BeliefContext
#include <jack/plan.h>                   // for Plan
#include <jack/schedule.h>
#include <jack/corelib.h>
#include <jack/events/auctionevent.h>    // for AuctionEvent
#include <jack/events/delegationevent.h> // for DelegationEvent
#include <jack/tasks/actiontask.h>
#include <jack/engine.h>

/// Third Party
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <filesystem>
#include <cstring>
#include <cstddef>      // for size_t
#include <list>         // for _List_const_iterator, _List_iter...
#include <type_traits>  // for forward
#include <unordered_set>

namespace aos::jack
{
static float32 saturatingF32Add(float32 lhs, float32 rhs)
{
    float32 result = std::min(Schedule::FAILED_COST, lhs + rhs);
    return result;
}

SearchNode::~SearchNode()
{
    if (contextIsCloned) {
        JACK_DELETE(context);
    }
}

void SearchNode::print() const
{
    // debug message
    fmt::print("SearchNode: ");
    if (plan) {
        fmt::print("  plan: {}", plan->name());
    } else  {
        if (delegate != nullptr) {
            fmt::println("delegated to {}", delegate->name());
        } else {
            fmt::println("not delegated");
        }
    }

    // print the beliefset names
    if(context) {
        std::vector<std::string> msgList = context->messageList();
        for(const std::string& msgName : msgList) {
            std::shared_ptr<Message> msg = context->message(msgName);
            fmt::print(msg->toString());
        }
    }

    if (goal) {
        fmt::println("  parent goal: ", goal->name(), (goal->isPersistent() ? " (Desire)" : " (Perform)"));
        if (goal->context().goal()) {
            const std::shared_ptr<Message>& goalMsg = goal->context().goal();
            fmt::println(goalMsg->toString());
        }
    } else  {
        fmt::println("  no goal");
    }

    fmt::println("Node cost: {} Total cost: {} Accum cost:  {} Est: {} ", estimateToEnd, costOfNode, costTotal, costFromStart);
}

/// @param plans Pass in the list of plans that are valid for this goal.
/// @param parent The parent node of the new node to compute plan selection
/// for
/// @param rootNode The root node of the schedule
/// @param plannerGoals The root goals of the schedule. This is used to initialise
/// the plan selection policy for the first time goals are expanded.
/// @param goal The goal the compute a plan selection list for
/// @param filteredPlans This plan list filtered to remove plans that are not
/// valid to be scheduled accordingly to the current plan selection policy.
static GoalPlanInfo computeGoalPlanInfo(const std::vector<const Plan*>&           plans,
                                        const SearchNode*                         parent,
                                        const SearchNode*                         rootNode,
                                        const std::vector<Schedule::PlannerGoal>& plannerGoals,
                                        const Goal*                               goal,
                                        const Tactic*                             tactic)
{
    ZoneScoped;
    GoalPlanInfo                    result      = {};
    const std::vector<std::string>& tacticPlans = tactic->plans();

    /// \note Find the most up-to-date plan selection policy for this goal from
    /// the scheduler's search space so far.
    if (parent == rootNode) {
        /// \note When adding a top level search node the plan selection state
        /// is inherited from the root goal.
        result.planSelection = goal->m_planSelection;
    } else {
        /// \note Making a new search node that is not a top level node. Search
        /// up the scheduler's tree for the last time this goal was executed and
        /// inherit the plan selection history which contains the plan selection
        /// state informing us what we've already tried.
        const SearchNode* ancestorGoalNode = parent;
        while (ancestorGoalNode != rootNode && ancestorGoalNode->goal->handle() != goal->handle()) {
            ancestorGoalNode = ancestorGoalNode->parent;
        }

        if (ancestorGoalNode != rootNode) {
            result.planSelection = ancestorGoalNode->planSelection;

            /// \note Advance the fixed plan list if it exists
            if (tactic->isUsingPlanList() && tactic->planOrder() == Tactic::PlanOrder::Strict) {
                result.planSelection.m_planListIndex = (result.planSelection.m_planListIndex + 1) % tacticPlans.size();
            }
        } else {
            /// \note This schedule's search space branch has never expanded
            /// this goal before. We need to pull the plan history from the root
            /// desire.
            bool found = false;
            for (size_t rootIndex = 0; !found && rootIndex < plannerGoals.size(); rootIndex++) {
                const Goal* rootGoal = plannerGoals[rootIndex].goal;
                found = rootGoal->name() == goal->name();
                if (found) {
                    result.planSelection = rootGoal->m_planSelection;
                }
            }

            JACK_ASSERT(found && "Can't expand a goal that is not a part of the root goals.");
        }
    }

    /// \note Filter the plans according to the plan selection policy.
    ///
    /// A fixed plan list supercedes the exclusion policy. A fixed plan list
    /// dictates an order for the plans to be expanded and executed in.
    ///
    /// An exclusion policy removes plans as they are attempted. It does not
    /// make sense to exclude a plan from the fixed plan list, rather you
    /// already have an ordered list, you just need to iterate it.
    uint32_t loopMaxCount = tactic->loopPlansCount();
    if (loopMaxCount != Tactic::LOOP_PLANS_INFINITELY && result.planSelection.m_planLoopIteration >= loopMaxCount) {
        return result;
    }

    if (tactic->isUsingPlanList() && tactic->planOrder() == Tactic::PlanOrder::Strict) {
        JACK_ASSERT(result.planSelection.m_planListIndex < tacticPlans.size());
        /// \note Get the next plan from the fixed plan list, and add it to the
        /// filtered plan list.

        std::string nextPlanName = tacticPlans[result.planSelection.m_planListIndex % tacticPlans.size()];
        for (const Plan *plan : plans) {
            if (plan->name() == nextPlanName) {
                result.plans.push_back(plan);
                break;
            }
        }

        /// @todo: I think there's a bug here where we don't increment
        /// the plan loop iteration for this branch.
    } else {
        /// \note Exclude policy means that plans are only tried once per loop.
        /// Remove plans that are attempted already from the candidate plan
        /// list.
        if (tactic->planOrder() == Tactic::PlanOrder::ExcludePlanAfterAttempt) {
            ZoneNamedN(debugTracyEvaluateGoal, "Filter plans for goal info", true);
            auto                   filteredPlans = plans;
            for (auto it = filteredPlans.begin(); it != filteredPlans.end(); ) {
                const Plan*              plan    = *it;
                const Goal::PlanHistory* history = result.planSelection.findHistory(plan->name());

                bool firstTime = history == nullptr;
                if (firstTime || history->lastLoopIteration < result.planSelection.m_planLoopIteration) {
                    /// \note Plans that have not been executed yet for this round's
                    /// attempt of trying to "achieve" the goal will stay as they
                    /// are candidates for executing.
                    it++;
                } else {
                    /// \note The plan has been executed for this round in the goal
                    /// and the goal hasn't been achieved yet. We must 'exclude' it
                    /// from the candidate plan list.
                    it = filteredPlans.erase(it);
                }
            }

            if (result.planSelection.m_planLoopIteration < tactic->loopPlansCount() && filteredPlans.empty()) {
                /// Reset the filterd plan list, all plans were exhausted, so we
                /// loop and allow full plan selection again.
                result.planSelection.m_planLoopIteration++;
                result.plans = plans;
            } else {
                result.plans = std::move(filteredPlans);
            }
        } else {
            ZoneNamedN(debugTracyEvaluateGoal, "Copy plan list for goal info", true);
            result.plans = plans;
        }
    }

    return result;
}

struct GoalWithCost
{
    size_t       index;
    float32      cost;
    GoalPlanInfo goalPlanInfo;
};

/// Split the list of 'plannerGoals' into 2 lists of goals. Goals that must be
/// expanded into the search space from the current node and a list of goals that
/// have no effects.
///
/// In the schedule we use 1 goal from the list of goals with no effects and
/// append it to the search space to ensure we check atleast one of these goals
/// in the planning stage. The 1 goal that we choose is prioritised in order
/// from the goal with the cheapest heuristic AND that we had at least one plan
/// that was valid to achieve the goal.
static SearchNode::GoalList makeListOfGoalsToExpand(SearchNode const *root, SearchNode *node, Agent *agent, std::vector<Schedule::PlannerGoal>& plannerGoals)
{
    ZoneScoped;
    SearchNode::GoalList result = {};
    std::vector<GoalWithCost> effectlessGoals;

    /// Determine if a goal is plannable (e.g. should be added to the search
    /// space or not). A goal that is plannable (in the schedule) is defined as
    /// a goal that can model effects OR a goal that has a heuristic AND the
    /// goal has plans.
    ///
    /// If a goal is not plannable, then, we only keep one of the goals for
    /// expansion in the scheduler initially.
    for (size_t goalIndex : node->goalsRemaining) {
        ZoneNamedN(debugTracyEvaluateGoal, "Evaluate goal", true);
        Goal* goal = plannerGoals[goalIndex].goal;
        ZoneNameV(debugTracyEvaluateGoal, goal->name().c_str(), goal->name().size());

        /// \note Sanity check
        if (JACK_SLOW_DEBUG_CHECKS && goal->delegated()) {
            JACK_ASSERT(agent->getGoalTacticPlans(goal).empty());
        }

        /// \note We can't expand a goal in the schedule's search space if it is
        /// *not* delegated and their precondition has failed.
        if (!goal->delegated()) {

            /// \note We only need to evaluate the precondition if the context
            /// has changed or we're the first node in the search space.
            /// \todo Calling the precondition (or any goal query function is
            /// very expensive because of the implementation of our messages as
            /// javascript-esque dictionaries) so we need complicated caching
            /// logic like this to minimise calls where possible to clawback
            /// some performance.
            bool evaluatePrecondition = !node->parent || node->contextIsCloned;
            if (evaluatePrecondition) {
                goal->context().setAgentContext(*node->context, node->delegate ? node->delegate->handle() : agent->handle());
                node->valid[goalIndex] = goal->isValid();
                goal->context().setAgentContext(agent->context(), agent->handle());
            }

            if (!node->valid[goalIndex]) {
                continue;
            }
        }

        /// \todo Getting the plans with respect to the current tactic directly
        /// from the agent is highly suspect since changing tactics can
        /// invalidate prior branches in the search space.

        /// \note Calculate the list of plans valid for solving the goal at this
        /// point of the search space.
        const std::vector<const Plan*>& candidatePlans = agent->getGoalTacticPlans(goal);
        const Tactic*                   tactic         = agent->currentTactic(goal->name());
        GoalPlanInfo                    goalPlanInfo   = computeGoalPlanInfo(candidatePlans, node, root, plannerGoals, goal, tactic);

        /// \note Check if the goal has plans with effects
        bool hasPlanWithEffects = false;
        for (auto planIt = goalPlanInfo.plans.begin(); !hasPlanWithEffects && planIt != goalPlanInfo.plans.end(); planIt++) {
            hasPlanWithEffects |= (*planIt)->canModelEffect();
        }

        /// \note 1st branch: If a goal is to be delegated, or, it has a plan
        /// with effects then we can expand this goal into search nodes (1 for
        /// each plan) to determine the best course of action.
        ///
        /// 2nd branch: The goal has no effects, then, in the eyes of JACK
        /// achieving this goal has *no* consequences or influence on the
        /// ability to achieve other goals in the schedule.
        ///
        /// We only add the cheapest cost goal that is effectless to the search
        /// space, one at a time. This prevents the schedule churning through
        /// many levels of redundant planning since, at the end of the day,
        /// there are no side-effects to the execution of these goals which
        /// practically means that the order in which they are executed does
        /// *not* matter.
        ///
        /// We can pick the lowest cost one to add to the search space and move
        /// on.
        if (goal->delegated() || hasPlanWithEffects) {
            ZoneNamedN(debugTracyAddGoalToExpandList, "Add goal to expand list", true);
            if (JACK_SLOW_DEBUG_CHECKS && goal->delegated()) {
                JACK_ASSERT(goalPlanInfo.plans.empty());
            }
            result.expandable.push_back({goalIndex, std::move(goalPlanInfo)});
        } else {
            ZoneNamedN(debugTracyAddGoalToExpandList, "Add goal to effectless list", true);
            auto oldGoalContext = node->context->goal();
            node->context->setGoalContext(goal->context().goal());
            float32 cost = goal->heuristic(*node->context);
            node->context->setGoalContext(oldGoalContext);
            effectlessGoals.push_back({goalIndex, cost, std::move(goalPlanInfo)});
        }
    }

    std::sort(effectlessGoals.begin(), effectlessGoals.end(), [](const GoalWithCost& lhs, const GoalWithCost& rhs) {
        bool result = lhs.cost < rhs.cost;
        return result;
    });

    result.effectless.reserve(effectlessGoals.size());
    for (const auto& item : effectlessGoals) {
        result.effectless.push_back({item.index, std::move(item.goalPlanInfo)});
    }

    /// \note Sanity check that the list is sorted in from lowest to highest cost
    if (effectlessGoals.size() >= 2) {
        JACK_ASSERT(effectlessGoals[0].cost <= effectlessGoals[1].cost);
    }
    return result;
}

/******************************************************************************
 * Public Ctor & Dtor
 ******************************************************************************/
Schedule::Schedule(Agent *agent, const std::vector<Goal *> &goals) : m_agent(agent)
{
    ZoneScoped;
    /// \note Setup the root note and add root goals
    m_plannerGoals.reserve(goals.size());
    m_root.startCreationTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_startTime);
    m_root.contextIsCloned     = true;
    m_root.context             = JACK_CLONE_BDI_OBJECT(&agent->context());
    m_root.valid.resize(goals.size());
    m_root.goalsRemaining.resize(goals.size());

    /// \note Initialise the list of goals remaining
    for (size_t goalIndex = 0; goalIndex < goals.size(); goalIndex++) {
        Goal* goal                       = goals[goalIndex];
        m_root.goalsRemaining[goalIndex] = static_cast<uint32_t>(goalIndex);
        m_plannerGoals.push_back({goal, false /*canBePlanned*/});
    }

    /// @todo Does it violate strict-aliasing to potentially pass in
    /// 'm_root' as the 'parent' and the 'node'?
    m_root.goalList = makeListOfGoalsToExpand(&m_root, &m_root, m_agent, m_plannerGoals);

    /// \note Setup state
    m_startTime = agent->engine().internalClock();
    m_bestDelegation.resize(goals.size());
    m_openNodes.push(&m_root);
    m_id = m_agent->m_scheduleIdCounter++;

    /// \note Expand the newly added goals to generate some pending nodes to explore
    m_state = Schedule::EXPAND;
    expand();

    /// \note Auction out newly added goals that can only be delegated
    m_state = Schedule::AUCTION;
    auction();
}

Schedule::~Schedule()
{
    ZoneScoped;
    for (SearchNode* node : m_clonedContextNodes) {
        JACK_DELETE(node->context);
    }

    ThreadScratchAllocator            scratch            = getThreadScratchAllocator(nullptr);
    [[maybe_unused]] std::string_view debugTracyZoneText = JACK_FMT_ALLOC(scratch.arena,
                                                                          "Schedule deallocating {} nodes",
                                                                          m_openNodes.size() + m_closedNodes.size() + m_pendingNodes.size() + m_failureNodes.size());
    ZoneText(debugTracyZoneText.data(), debugTracyZoneText.size());
    m_arena.freeAllocator(false /*clearMemory*/);
}

/******************************************************************************
 * Public Accessors & Mutators
 ******************************************************************************/
static const uint64_t FNV1A64_SEED = 14695981039346656037ULL;
static uint64_t fnv1A64Hash(void const *bytes, size_t size, uint64_t hash)
{
    auto* buffer = reinterpret_cast<const unsigned char *>(bytes);
    for (size_t i = 0; i < size; i++)
        hash = (buffer[i] ^ hash) * 1099511628211 /*FNV Prime*/;
    return hash;
}

static void writeAgentCantHandleActionInPlanReason(StringBuilder& reason, const Agent* agent, const Plan* plan, std::string_view action)
{
    reason.append(FMT_STRING("  {}: Action '{}' cannot be executed because there are no "
                  "services to handle it.\n\nThe agent is attached to "
                  "the following services:\n"), plan->name(), action);

    const Engine& engine                       = agent->engine();
    const std::vector<ServiceHandle>& services = agent->attachedServices();
    for (auto it = services.begin(); it != services.end(); it++) {
        const Service* service = engine.getService(*it);
        if (it != services.begin()) {
            reason.appendRef("\n");
        }
        reason.append(FMT_STRING("    {}: {}"), it->toHumanString(), service->isAvailable() ? "available" : "unavailable");
    }
}

static void logAgentExpandGoalFailed(StringBuilder& reason, Agent *agent, const Goal *goal, const Tactic *tactic, const SearchNode *parent, const Goal::PlanSelection& nextPlanSelection)
{
    static std::unordered_map<uint64_t, std::chrono::milliseconds> hashCache;
    static std::thread::id threadId = std::this_thread::get_id();
    (void)threadId;

    JACK_ASSERT(threadId == std::this_thread::get_id() &&
           "We cache hashes of the logs we print to prevent log spam. This only works in a "
           "single threaded context.");

    uint64_t hash = fnv1A64Hash(agent->name().data(), agent->name().size(), FNV1A64_SEED);
    hash          = fnv1A64Hash(goal->name().data(), goal->name().size(), hash);

    std::chrono::milliseconds& lastHitTime      = hashCache[hash];
    std::chrono::milliseconds  timeSinceLastHit = agent->engine().getUpTime() - lastHitTime;
    if (timeSinceLastHit > std::chrono::seconds(30)) {
        lastHitTime = agent->engine().getUpTime();

        reason.append(FMT_STRING("Desired goal in the planner could not be achieved.\n\nThere were no valid plans to choose from when considering the active tactic and the available plans:\n"));
        reason.append(FMT_STRING("\nThe active tactic was: {}\n"
                                 "  Plan loop counter: {}\n"
                                 "  Using plan list: {}\n"
                                 "  Plan Order: {}\n\n"),
                      tactic->handle().toHumanString(),
                      tactic->loopPlansCount(),
                      tactic->isUsingPlanList(),
                      Tactic::planOrderHumanPrint(tactic->planOrder()));

        if (goal->delegated()) {
            Agent::GetDelegatesResult getDelegatesResult = agent->getDelegates(goal->handle());
            reason.append("{}\n", getDelegatesResult.delegationOutcome);

            if (getDelegatesResult.delegates.size()) {
                reason.append("The current delegates are being tasked or are unavailable:\n\n");
                for (size_t index = 0; index < getDelegatesResult.delegates.size(); index++) {
                    Agent* delegate = getDelegatesResult.delegates[index];
                    if (index) {
                        reason.appendRef("\n");
                    }

                    std::string_view allocatedGoal;
                    std::string_view allocatedPlan;
                    for (const SearchNode* walker = parent; walker; walker = walker->parent) {
                        if (walker->delegate == delegate) {
                            allocatedGoal = walker->goal->name();
                            if (walker->plan) {
                                allocatedPlan = walker->plan->name();
                            }
                            break;
                        }
                    }

                    if (allocatedGoal.size()) {
                        reason.append(FMT_STRING("  {}: {}"), delegate->handle().toHumanString(), allocatedGoal);
                    }
                    if (allocatedPlan.size()) {
                        reason.append(FMT_STRING(": {}"), allocatedPlan);
                    }
                }
                reason.appendRef("\n\n");
            }
        }

        reason.append(FMT_STRING("[agent={}, goal={}, planSelection={}]"), agent->handle().toString(), goal->handle().toString(), nextPlanSelection.toString());
        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        std::string_view       log     = reason.toStringArena(scratch.arena);
        JACK_WARNING("{}", log);
    }
}

std::vector<SearchNode*> Schedule::expandGoalToPlans(SearchNode* parent, size_t rootGoalIndex, const GoalPlanInfo& goalPlanInfo)
{
    ZoneScoped;
   /***************************************************************************
     * Find plans that are applicable for the goal and attach them to the search
     * tree (by attaching to the parent). Teams will attach and delegate to
     * their members.
     *
     * 1. Pre-amble expansion by getting the goal plans
     * 1. Branch for teams or agents
     *    1.1 Delegate goal/plan to members
     *    1.2 If there are no team members, consider the null allocation for the
     *        search
     * 2. Otherwise we are an agent, search and assign plans that are applicable
     *    for this intention
    ***************************************************************************/
    /// 1. Branch for teams or agents
    std::vector<SearchNode *> result;
    if (rootGoalIndex >= m_plannerGoals.size()) {
        JACK_ASSERT(rootGoalIndex < m_plannerGoals.size() && "Root goals have changed within the same schedule after initialisation, index out of bounds");
        return result;
    }

    Goal *goal = m_plannerGoals[rootGoalIndex].goal;

    /// \note Retrieve the plans that are valid for the current tactic that is
    /// set.
    TracyCZoneN(debugTracyGetGoalTacticPlans, "Get plan list for goal according to tactic", true);
    const std::vector<const Plan*>& candidatePlans = m_agent->getGoalTacticPlans(goal);
    if (goal->delegated()) {
        JACK_ASSERT(candidatePlans.empty());
    }
    TracyCZoneEnd(debugTracyGetGoalTacticPlans);

    /// \note Generate the list of plans that are valid according to the plan
    /// selection policy.
    bool                     hasDelegates = false;
    ThreadScratchAllocator   scratch      = getThreadScratchAllocator(nullptr);
    StringBuilder            reason       = StringBuilder(scratch.arena);
    auto                     failureNodes = StaticArray<SearchNode*>(scratch.arena, goalPlanInfo.plans.size());
    auto                     ageMs        = std::chrono::duration_cast<std::chrono::milliseconds>(m_agent->engine().internalClock());
    if (goal->delegated()) {
        // We are a team, delegation to members of the team (agents) is required
        // Create a delegation analysis event and switch to a pending state whilst waiting for the
        // team member's analysis to come back.

        // 1.1 Delegate goal/plan to members
        const std::vector<Agent *>& delegates = m_agent->getDelegates(goal->handle()).delegates;
        hasDelegates = delegates.size();
        ZoneNamedN(debugTracyAddDelegatesToSearchSpace, "Add delegate(s) to planning search space", true);
        for (Agent *delegate : delegates) {
            ZoneNamedN(debugTracyCreateDelegateSearchNode, "Create search node for delegate", true);
            // Create a new search node for each valid plan
            SearchNode* planNode = JACK_ALLOCATOR_ALLOC(&m_arena, SearchNode);
            planNode->delegate   = delegate;
            ZoneNamedN(debugTracyAppendNode, "Append delegated search node", true);
            result.push_back(planNode);
            JACK_ASSERT(!planNode->plan && "No plan for a search node by definition means it's delegated");
        }

        // 1.2 If there are no team members, consider the null allocation for the search
        /// \todo Verify what the null-allocation means? If we don't have
        /// a delegate for the agent .. why do we insert it into the graph for
        /// consideration?
        ///
        /// Subsequent iterations in the schedule may potentially change the
        /// context such that an unavailable delegate becomes available, so at
        /// that point, then we will consider the delegate for the goal and add
        /// it into the search graph for planning.
        #if !AGENT_REMOVE_IMPOSSIBBLE_GOALS_AFTER_SCHEDULING_WORK_AROUND
        if (delegates.empty()) {
            ZoneNamedN(debugTracyNoDelegates, "Create the nil-search node where no delegates are availble", true);
            SearchNode *planNode = JACK_ALLOCATOR_ALLOC(&m_arena, SearchNode);
            JACK_ASSERT(!planNode->plan && "No plan for a search node by definition means it's delegated");
            result.push_back(planNode);
        }
        #endif
    } else {
        ZoneNamedN(debugTracyCreateAgentSearchNode, "Add agent's plans search space", true);
        /// 2. Otherwise we are an agent, search and assign plans that are applicable
        /// for this intention
        for (const Plan *plan : goalPlanInfo.plans) {
            ZoneNamedN(debugTracyEvaluatePlan, "Evaluate plan", true);
            ZoneNameV(debugTracyEvaluatePlan, plan->name().c_str(), plan->name().size());

            bool planPreconditionPassed = plan->valid(*parent->context);
            if (!planPreconditionPassed) {
                reason.append("  {}: Plan precondition failed\n", plan->name());
            }

            /// \note Check if the agent can handle the action in the plans
            TracyCZoneN(debugTracyCheckAgentCanHandle, "Check if the agent can handle the action in the plans", true);
            Agent::HandlesPlanResult handlesPlanResult = m_agent->handlesPlan(plan, Agent::HandlesActionSearch_ALL);
            if (!handlesPlanResult.success) {
                writeAgentCantHandleActionInPlanReason(reason, m_agent, plan, handlesPlanResult.action);
            }
            TracyCZoneEnd(debugTracyCheckAgentCanHandle);

            ZoneNamedN(debugTracyCreateSearchNode, "Create search node for the valid plan", true);
            SearchNode* planNode = JACK_ALLOCATOR_ALLOC(&m_arena, SearchNode);
            planNode->plan       = plan;
            bool impossiblePlan  = !planPreconditionPassed || !handlesPlanResult.success;

            if (impossiblePlan) {
                planNode->state = SearchNode::State::FAILED;
                if (!planPreconditionPassed) {
                    planNode->failure = SearchNode::Failure::PLAN_INVALID;
                } else {
                    JACK_ASSERT(!handlesPlanResult.success);
                    planNode->failure = SearchNode::Failure::SERVICE_UNAVAILABLE;
                }
                ZoneNamedN(debugTracyAppendNode, "Append failed search node", true);
                failureNodes.push_back(planNode);
            } else {
                ZoneNamedN(debugTracyAppendNode, "Append search node to result", true);
                result.push_back(planNode);
            }
        }
    }

    /// \todo What happens when we have a fixed plan list, but the plan is not
    /// valid. Is this a problem? Looks like the agent might stall and be unable
    /// to plan successfully.
    if (result.empty()) {
        const Tactic* tactic = m_agent->currentTactic(goal->name());
        logAgentExpandGoalFailed(reason, m_agent, goal, tactic, parent, goalPlanInfo.planSelection);
    }

    /// \note Mark the root goal as being plannable, note that we may have 0
    /// plan nodes expanded if we have delegates but they're all allocated.
    if (result.size() || hasDelegates) {
        /// This is used to determine which goals are not executable at all in
        /// the current context (e.g. no delegates, or no plans). They're
        /// impossible to achieve and should be dropped if after all the
        /// scheduling, we determine that we don't have a way to to execute this
        /// goal.
        m_plannerGoals[rootGoalIndex].canBePlanned = true;
    }

    /// \note Create a n-ary tree of search nodes by attaching the expanded
    /// nodes as the children of the active parent.
    for (auto list : {Span(result), Span(failureNodes)}) {
        for (SearchNode *node : list) {
            if (parent->lastChild) {
                parent->lastChild->next = node;
            }
            parent->lastChild = node;
            if (!parent->firstChild) {
                parent->firstChild = node;
            }
        }
    }

    /// \note Fill out the search node with the common parameters
    for (SearchNode *node : result) {
        node->goal                = goal;
        node->parent              = parent;
        node->startCreationTimeMs = ageMs;
        node->parent              = parent;
        node->context             = parent->context;
        node->planSelection       = goalPlanInfo.planSelection;
        node->valid               = parent->valid;
        node->goalsRemaining      = parent->goalsRemaining;

        /// Stop the goal from being reattempted if it is delegated or the plan
        /// doesn't have the capability to estimate the effects of completing
        /// the plan on the belief context.
        bool goalsRemainingChanged = false;
        if (goal->delegated() || !goal->isPersistent() || (node->plan && !node->plan->canModelEffect())) {
            auto it = std::lower_bound(node->goalsRemaining.begin(), node->goalsRemaining.end(), rootGoalIndex);
            JACK_ASSERT(*it == rootGoalIndex);
            node->goalsRemaining.erase(it);
            goalsRemainingChanged = true;
        }

        /// \todo The first context in the root is cloned but the list of goals
        /// was created with the cloned context which iresults in reconstructing
        /// the list again on the first level of the search space unnecessarily.

        if (parent->contextIsCloned) {
            /// \note When the context is cloned, this indicates that the
            /// context has mutated, the list of goals that we have memoized are
            /// invalidated in the node, we cannot reuse the list from the
            /// parent and so we must recalculate it.
            node->goalList = makeListOfGoalsToExpand(&m_root, node, m_agent, m_plannerGoals);
        } else {
            /// \note Context is the same, we can inherit the goal list from our
            /// parent to save computation.
            node->goalList = parent->goalList;

            /// \note If the goals remaining changed, we removed one goal from
            /// the candidate list. Since we copied the list from our parent we
            /// need to patch up the list again and remove the goal we just
            /// eliminated.
            ///
            /// This is an optimisation to prevent having reconstruct the list
            /// again which is expensive.
            if (goalsRemainingChanged) {
                bool found = false;
                auto expandIt = std::lower_bound(node->goalList.expandable.begin(), node->goalList.expandable.end(), rootGoalIndex,
                [](const SearchNode::GoalListItem& lhs, size_t rootGoalIndex) {
                    bool result = lhs.goalIndex < rootGoalIndex;
                    return result;
                });

                if (expandIt != node->goalList.expandable.end() && expandIt->goalIndex == rootGoalIndex) {
                    node->goalList.expandable.erase(expandIt);
                    found = true;
                }

                if (!found) {
                    for (auto it = node->goalList.effectless.begin(); it != node->goalList.effectless.end(); it++) {
                        if (it->goalIndex == rootGoalIndex) {
                            node->goalList.effectless.erase(it);
                            break;
                        }
                    }
                }
            }

            JACK_ASSERT_FMT(node->goalList.expandable.size() + node->goalList.effectless.size() <= node->goalsRemaining.size(),
                            "The list of goals remaining is always a superset of the expand and "
                            "effectless list. The superset list for example may contain goals that "
                            "failed their precondition which belong in neither list. Expand had {}"
                            ", effectless had {} nodes but there were {} goals remaining",
                            node->goalList.expandable.size(),
                            node->goalList.effectless.size(),
                            node->goalsRemaining.size());
        }
    }

    m_failureNodes.insert(m_failureNodes.end(), failureNodes.begin(), failureNodes.end());
    return result;
}

static void finaliseNodeTimings(SearchNode* node, SearchNode& rootNode, std::chrono::milliseconds timeMs)
{
    node->terminationTimeMs = timeMs - rootNode.startCreationTimeMs;
    node->processingTimeMs  = timeMs - node->startCreationTimeMs - (node->parent ? node->parent->processingTimeMs : std::chrono::milliseconds(0));
    if (node->parent) {
        node->parent->processingTimeMs += node->processingTimeMs;
    }
}

// expand this schedule's open list. i.e. the list of candidate plans for consideration
void Schedule::expand()
{
   /***************************************************************************
    * Expand the search space from the current best candidate search node using the nodes
    * context
    *
    * 1. Select best cadidate seach node to search from
    * 2. Deliberate goals
    * 3. Select candiate plans for the goals and add to costing list
    * ?. consider resources
    * ?. model effects
    ***************************************************************************/
    ZoneScoped;
    if (m_state != Schedule::EXPAND) { ///< We are not in the expand state quit
        return;
    }

    /// 1. Select best cadidate seach node to search from
    SearchNode *node = !m_openNodes.empty() ? m_openNodes.top() : nullptr;
    JACK_ASSERT(m_pendingNodes.empty());

    if (node == nullptr) { ///< No search nodes - we're done
        m_state = Schedule::END;
        return;
    }

    /// 2. Deliberate goals
    for (const auto *list : {&node->goalList.expandable, &node->goalList.effectless}) {
        for (const SearchNode::GoalListItem& listItem : *list) {
            Goal* goal = m_plannerGoals[listItem.goalIndex].goal;

            std::vector<SearchNode*> candidates;
            // override the context of the goal during the deliberation
            goal->context().setAgentContext(*node->context, node->delegate ? node->delegate->handle() : m_agent->handle());

            if (goal->delegated() || !goal->isSatisfied()) {
                // backup the old goal context and set the new goal context
                auto oldGoalContext = node->context->goal();
                node->context->setGoalContext(goal->context().goal());
                candidates = expandGoalToPlans(node, listItem.goalIndex, listItem.goalPlanInfo);
                node->context->setGoalContext(oldGoalContext);
            }

            // restore the context of the goal
            goal->context().setAgentContext(m_agent->context(), m_agent->handle());
            m_pendingNodes.insert(m_pendingNodes.end(), candidates.begin(), candidates.end());

            /// \note We only accept the first goal from the list of goals with
            /// no effects that successfully contributed to the search space.
            /// This means that we always atleast consider one effect-less goal
            /// at all levels of the search space (because it still might be the
            /// cheapest cost goal to achieve irrespective of effects!).
            if (list == &node->goalList.effectless && candidates.size()) {
                break;
            }
        }
    }

    m_searchDepth--;
    if (m_pendingNodes.empty()) {
        m_state = Schedule::END;
    } else {
        m_state     = Schedule::COST;
        node->state = SearchNode::State::CLOSED;
        auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_agent->engine().internalClock());
        finaliseNodeTimings(node, m_root, ageMs);
        m_closedNodes.push_back(node);
        m_openNodes.pop();
    }
}

// cost the plan nodes
void Schedule::auction()
{
    ZoneScoped;
    if (m_state != Schedule::AUCTION) {
        // we are not in the auction state
        return;
    }

   /***************************************************************************
    * Process A.2.3.3 Auction Phase
    * for each uncosted delegated pending node perform an auction
    * 1. For all desires or perform goal start an auction
    * 2. Wait if required
    ***************************************************************************/

    m_pendingAuctions = 0;

    /// 1. Auction any delegate goals
    for (PlannerGoal& plannerGoal : m_plannerGoals) {
        Goal* desire = plannerGoal.goal;
        if (desire->delegated()) {
            if (JACK_SLOW_DEBUG_CHECKS) {
                JACK_ASSERT(m_agent->getGoalTacticPlans(desire).empty());
            }

            if (m_agent->isTeam() && m_agent->analyseDelegation(desire, m_id)) {
                m_pendingAuctions++;
            }
        }
    }

    // 2. Wait for the auctions to finish if we need to
    m_state = Schedule::PENDING_COST;
    tryFinalizeAuction();
}

void Schedule::addNodeToFailureList(SearchNode* node, SearchNode::Failure failure)
{
    if (!node) {
        return;
    }

    node->state   = SearchNode::State::FAILED;
    node->failure = failure;
    m_failureNodes.push_back(node);
}

void Schedule::tryFinalizeAuction()
{
    if (m_pendingAuctions <= 0) {
        m_state = Schedule::COST;

        // estimate the root node
        float32 remainingCost = estimateCostFrom(m_root.goal, m_root.context, m_root.goalsRemaining);
        m_root.costTotal      = remainingCost;
        m_root.estimateToEnd  = remainingCost;
    }
}

float32 Schedule::estimateCostFrom(const Goal*                  currGoal,
                                   BeliefContext*               context,
                                   const std::vector<uint32_t>& goalsRemaining) const
{
    ZoneScoped;
   /***************************************************************************
    * Estimate the cost to achieve the remaining desires given a context and an
    * allocation
    *
    * 1. Split the root goals into desires that we can achieve and can estimate
    *    the cost ourselves and goals that are delegated and need to get their
    *    cost from the auction table.
    *   1.1 Skip already achieved desires
    *     1.1.1 Collect the desires that have been delegated
    *     1.1.2 Accumulate the cost of desires we can achieve
    * 2. Accumulate the cost to complete the remaining desires from our auction
    *    results stored in the delegation cost cache.
    ***************************************************************************/

    struct GoalWithDelegates
    {
        Goal*                       goal = nullptr;
        size_t                      goalIndex = 0;
        const std::vector<Agent *> *delegates;
    };

    /// 1. Split the root goals into desires that we can achieve and can estimate
    /// the cost ourselves and goals that are delegated and need to get their
    /// cost from the auction table.
    std::vector<GoalWithDelegates> remainingDesires;
    float32                          estimateRemainingCost = 0.0f;
    for (size_t goalIndex : goalsRemaining) {
        Goal* g = m_plannerGoals[goalIndex].goal;

        /// 1.1 Skip already achieved desires
        if (currGoal == g) {
            /// Estimating the cost to the end does not need to include the cost
            /// of the current goal. That is costed separately and stored in the
            /// owning search node itself.
            continue;
        }

        if (m_bestDelegation[goalIndex].delegate) {
            /// 1.1.1 Collect the desires that have been delegated
            /// We have a cost for this goal in our delegations (i.e. it was
            /// auctioned out).
            GoalWithDelegates entry = {};
            entry.goal              = g;
            entry.goalIndex         = goalIndex;
            entry.delegates         = &m_agent->getDelegates(g->handle()).delegates;
            remainingDesires.push_back(entry);
        } else {
            /// 1.1.2 Accumulate the cost of the desires we can achieve

            /// \note 1.f cost is very intentional for estimation. If a list of
            /// goals don't have a heuristic, they will return a cost of 0.
            /// Generated search nodes will cost the current node and return 0,
            /// but estimating the cost of the remaining list of heuristic-less
            /// goals will return a cost of 1 each by entering this branch.
            ///
            /// This means that the current search node (and the sum of its
            /// tree) will always be cheaper than checking our neighbours unless
            /// there's a goal with a heuristic within our search space. This
            /// will force the planner to DFS down the current search tree in
            /// the degenerate case (i.e. all nodes and consequently root goals
            /// are heuristic-less goals and return a cost of 0.f).
            float32 cost = 1.f;
            if (g->delegated()) {
                /// \todo How can the team estimate the remaining cost for the
                /// goal if we had no delegates respond to this goal? We assume
                /// a cost of 1.f for now.
            } else {
                if (g->hasHeuristic()) {
                    auto oldGoalContext = context->goal();
                    context->setGoalContext(g->context().goal());
                    cost = g->heuristic(*context);
                    context->setGoalContext(oldGoalContext);
                } else {
                    /// \todo No heuristic? 0 cost is problematic, we can't
                    /// accurately plan into the future. Default to cost of 1.f
                }
            }

            estimateRemainingCost = saturatingF32Add(estimateRemainingCost, cost);
        }
    }

    if (remainingDesires.empty()) {
        return estimateRemainingCost;
    }

    struct DelegationBid
    {
        Agent *agent;  // The agent that is the best fit for the goal (least cost)
        Goal * goal;   // The goal to be completed
        float32  cost;   // The cost to complete the goal
    };

    /// 2. Accumulate the cost to complete the remaining desires from our auction
    ///    results stored in the delegation cost cache.
    while (remainingDesires.size()) {

        /// Go through all all the desires and get the agent <-> desire that has
        /// the lowest cost and store it into the 'best' DelegationBid.

        /// \note We must evaluate all the desires to find the best member
        /// because, a team member might be the best candidate (lowest cost) for
        /// multiple goals, in which case you only want them to be allocated to
        /// the best goal.

        DelegationBid best = {};
        for (GoalWithDelegates &entry : remainingDesires) {

            DelegationBid bid = {};

            auto bestDelegateIt = m_bestDelegation[entry.goalIndex];
            JACK_ASSERT(bestDelegateIt.delegate != nullptr &&
                   "When creating the list of remaining desires we did this based off the "
                   "delegation cache, so it must exist at this point");

            /// \note Optimisation step, we keep track of the best agent for a desire as we
            /// receive the auction bid. We can directly use this agent without having to check
            /// all the delegates if they are available for allocation since they're the best
            /// pick (least cost).

            bid.agent = bestDelegateIt.delegate;
            bid.cost  = bestDelegateIt.cost;
            bid.goal  = entry.goal;

            /// Keep this bid if its the best candidate so far across all the remaining desires.
            if ((bid.agent != nullptr && bid.cost != FAILED_COST) &&
                (best.agent == nullptr || best.cost > bid.cost))
            {
                best = bid;
            }
        }

        /// If no bid was selected, then, we have a desire that was unachievable.
        if (best.goal == nullptr) {
            JACK_ASSERT(best.agent == nullptr);
            JACK_ASSERT(best.cost == 0.f);
            break;
        }

        /// We have the best bid (i.e. team member that has the lowest cost) for
        /// achieving one of our remaining desires. Remove them from the pool of
        /// considered members for the remaining desires.

        estimateRemainingCost = saturatingF32Add(estimateRemainingCost, best.cost);

        /// Remove the desire from the our pool of desires to consider
        for (auto it = remainingDesires.begin(); it != remainingDesires.end(); it++) {
            if (it->goal == best.goal) {
                remainingDesires.erase(it);
                break;
            }
        }
    }

    /// For all the remaining desire just give them a score of 1.0f
    estimateRemainingCost = saturatingF32Add(estimateRemainingCost, static_cast<float32>(remainingDesires.size()));
    return estimateRemainingCost;
}

// cost the plan nodes
void Schedule::cost()
{
    ZoneScoped;
    if (m_state != Schedule::COST)
        return;
    auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_agent->engine().internalClock());

   /***************************************************************************
    * Process A.2.3.3.1 Cost Schedule
    *   1. For each uncosted node
    *   2. Calculate the effects of this plan on the context
    *     2.1 Team delegation search nodes query cost from the
    *     2.2 Non-team delegation search nodes cost the search node using the
    *         goal heuristic
    *   3. Run the modelled context through the remaining desires not completed
    ***************************************************************************/
    // 1. For each uncosted node
    for (auto nodeIt = m_pendingNodes.begin(); nodeIt != m_pendingNodes.end(); ) {
        ZoneNamedN(debugTracyCostSearchNode, "Cost search node", true);

        SearchNode* node = *nodeIt;

        auto oldGoalContext = node->context->goal();
        node->context->setGoalContext(node->goal->context().goal());
        JACK_DEFER { node->context->setGoalContext(oldGoalContext); };

        ThreadScratchAllocator scratch            = getThreadScratchAllocator(nullptr);
        std::string_view       debugTracyZoneName = {};
        const Goal*            goal               = node->goal;
        if (node->isDelegation()) {
            debugTracyZoneName = JACK_FMT_ALLOC(scratch.arena, "Delegated intention {}", goal->name());
        } else {
            const Plan* plan   = node->plan;
            debugTracyZoneName = JACK_FMT_ALLOC(scratch.arena, "Intention {} for {}", goal->name(), plan->name());
        }
        ZoneNameV(debugTracyCostSearchNode, debugTracyZoneName.data(), debugTracyZoneName.size());

        // 2. Calculate the effects of this plan on the context
        bool auctionBidWasMissing = false;
        if(node->isDelegation()) {
            float32 delegatedCostValue = 1.f;
            if (node->delegate) {
                auto key = std::make_pair(node->goal->handle(), node->delegate);
                auto it  = m_delegationCostCache.find(key);
                if (it != m_delegationCostCache.end()) {
                    delegatedCostValue = it->second;  // cost the node from the cache
                } else {
                    JACK_ERROR("Auction bid didn't return in time, marking goal as impossible [agent={}, goal={}, delegate={}, schedule={}]",
                               m_agent->handle().toString(),
                               node->goal->handle().toString(),
                               node->delegate->handle().toString(),
                               m_id);
                    delegatedCostValue = FAILED_COST;
                    auctionBidWasMissing = true;
                }
            } // else a null allocation

            /// 2.1 Team delegation search nodes query cost from the
            node->costOfNode = delegatedCostValue;
        } else {
            /// \todo we need to model the effects without having to create a full copy of the
            /// effects
            /// maybe a push / pop
            /// or checkpoint + revert and commit

            if (node->plan->canModelEffect()) {
                ZoneNamedN(debugTracyCloneDelegatedContext, "Clone context for costing search node", true);
                node->context         = JACK_CLONE_BDI_OBJECT(node->context);
                node->contextIsCloned = true;
                node->plan->applyEffects(*node->context);
                m_clonedContextNodes.push_back(node);
            }

            /// 2.2 Non-team delegation search nodes cost the search node using
            /// the goal heuristic
            node->costOfNode = node->goal->heuristic(*node->context);

            /// \note Update the intention history depending on the cost result
            Goal::PlanHistory& history = node->planSelection.findOrMakeHistory(node->plan->name());
            if (node->costOfNode == FAILED_COST) {
                history.failCount++;
            } else {
                history.successCount++;
            }
        }

        if (node->costOfNode == FAILED_COST) {
            auto failure = auctionBidWasMissing ? SearchNode::Failure::AUCTION_BID_TIMEOUT
                                                : SearchNode::Failure::HEURISTIC_FAILED;
            finaliseNodeTimings(node, m_root, ageMs);
            addNodeToFailureList(node, failure);
            nodeIt = m_pendingNodes.erase(nodeIt);
            continue;
        }

        JACK_ASSERT(node->context);

        /// 3. Run the modelled context through the remaining desires not completed
        /// Estimate the remaining cost from a context with a set of remaining desires
        /// Calculate the cost for the remaining goals - not including this node
        node->estimateToEnd = estimateCostFrom(node->goal, node->context, node->goalsRemaining);

        /// Accumulation of the real cost from the start (no end estimates in
        /// this cost value)
        node->costFromStart = saturatingF32Add(node->parent->costFromStart, node->costOfNode);

        /// The total cost (real cost + estimated cost to the end) to complete
        /// the schedule from this node.
        node->costTotal = saturatingF32Add(node->costFromStart, node->estimateToEnd);
        nodeIt++;

        JACK_ASSERT(node->estimateToEnd >= 0);
        JACK_ASSERT(node->costOfNode    >= 0);
        JACK_ASSERT(node->costFromStart >= 0);
        JACK_ASSERT(node->costTotal     >= 0);
    }

    m_state = Schedule::DECONFLICT;
}

void Schedule::deconflict()
{
    ZoneScoped;
   /***************************************************************************
    * Process A.2.3.4 De-Conflict Schedule
    * Resolve any conflicts with the resource allocation
    *
    *   1. Check for Conflicts (A.2.3.4.1)
    *   2. Trim Violating Plans (A.2.3.4.2)
    *   3. Inject Maintenance (A.2.3.4.3)
    ***************************************************************************/

     if (m_state != Schedule::DECONFLICT)
         return;
     auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(m_agent->engine().internalClock());

     // 1. Check for Conflicts (A.2.3.4.1) do we need this injection crap?
     std::vector<Goal *> maintenanceGoalsToInject;
     for (SearchNode *node : m_pendingNodes) {

         // 2. Trim Violating Plans (A.2.3.4.2)
         bool deleteNode = false;
         if (node->context) {
             std::vector<std::string> violatedResources;
             deleteNode = node->context->hasResourceViolation(&violatedResources);

             const std::map<std::string, std::string> &resourceGoalMap = m_agent->resourcesToGoals();
             for (const std::string &resource : violatedResources) {
                 JACK_ASSERT(deleteNode);
                 auto it = resourceGoalMap.find(resource);
                 if (it != resourceGoalMap.end()) {
                     // 3. Inject Maintenance (A.2.3.4.3)
                     // Found a goal that handles a (violated) resource, add it as a desire

                     /// \todo Maintenance goal injection breaks with our
                     /// perform bugfix which before the fix would persist
                     /// perform goals that are not achievable in the agent.
                     ///
                     /// That bugfix relies on the schedule not introducing new
                     /// goals to the agent because, since the goal was
                     /// violated- it is unplannable (or unachievable).
                     ///
                     /// The bugfix will delete the perform goal from the agent,
                     /// however, if that goal was a violated, it's unplannable
                     /// but we should reconsider that goal with the injected
                     /// maintenance goal.
                     ///
                     /// And only after consideration of the maintenance goal
                     /// should we delete the goal if after that it is failed.
                     ///
                     /// Now this is all speculative on the fact that
                     /// maintenance goals continue to be implemented the way
                     /// they are currently. This will break DeliveryRobots.
                     #if 0
                     const std::string &goalName = it->second;
                     if (!m_agent->getDesire(goalName)) {
                         m_agent->pursue(goalName, GoalPersistent_No);
                     }
                     #endif
                 }
             }
         }

         if (deleteNode) {
             addNodeToFailureList(node, SearchNode::Failure::RESOURCE_VIOLATION);
         } else {
            node->state = SearchNode::State::OPEN;
            finaliseNodeTimings(node, m_root, ageMs);
            m_openNodes.push(node);
         }
     }

     m_pendingNodes.clear();
     m_state = Schedule::EXPAND;
}

void Schedule::bind()
{
   /***************************************************************************
    * Process A.2.3.5 Bind Schedule
    ***************************************************************************/
}

void Schedule::printSchedule() const
{
   /***************************************************************************
    * Print the schedule to standard out
    ***************************************************************************/

    std::vector<SearchNode*> bestIntentions = getBestIntentions();

    /// 2. Traverse the best chain of intentions in the search tree
    if (bestIntentions.empty())
        return;

    // just add all the nodes for now - no dependency
    for(auto node : bestIntentions) {
        node->print();
    }
}

/// Walk the up the n-ary tree of search nodes and record the nodes into a flat
/// array. The order of the walk is preserved from the node to the root, e.g.
/// the root most node is stored as the last node. This function does not return
/// the root of the tree, it stores the final node just before the root.
///
/// In JACK terms, this produces a list of intentions in reverse order (the
/// first intention/node to execute is stored in the last entry).
static Span<SearchNode*> walkSearchNodeToRoot(ArenaAllocator& arena, SearchNode* node)
{
    Span<SearchNode*> result = {};
    if (!node) {
        return result;
    }

    /// \note Calculate the depth of the tree
    size_t depth = 0;
    for (SearchNode* walk = node; walk->parent; walk = walk->parent) {
        depth++;
    }

    /// \note Walk the tree and store the nodes
    result           = Span<SearchNode*>::alloc(arena, depth);
    size_t spanIndex = 0;
    for (SearchNode* walk = node; walk->parent; walk = walk->parent) {
        result.m_data[spanIndex++] = walk;
    }

    JACK_ASSERT(result.m_size == spanIndex);
    return result;
}

std::vector<SearchNode*> Schedule::getBestIntentions() const
{
    /**************************************************************************
     * 1. Enumerate all the root goals that are valid
     * 2. Traverse the best chain of intentions in the search tree
     **************************************************************************/
    std::vector<SearchNode*>           result;
    std::map<const Goal*, SearchNode*> allocation;
    std::set<const Agent*>             agentHasAllocation;

    /// 1. For each valid root goal create an intention allocation slot
    for (const PlannerGoal& plannerGoal : m_plannerGoals) {
        if (plannerGoal.canBePlanned) {
            /// Goal is valid and needs to be allocated an agent for handling
            const Goal* goal = plannerGoal.goal;
            allocation[goal] = nullptr;
        }
    }

    /// 2. Traverse the best chain of intentions in the search tree
    /// The chain is backward - last is first
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    Span<SearchNode*> intentions   = walkSearchNodeToRoot(scratch.arena, m_openNodes.empty() ? nullptr : m_openNodes.top());
    for (size_t intentionIndex = intentions.size() - 1; intentionIndex < intentions.size(); intentionIndex--) {
        SearchNode* node = intentions[intentionIndex];
        const Goal* goal = node->goal; /// The goal to be achieved
        if (allocation.find(goal) != allocation.end() && allocation[goal] == nullptr) {
            allocation[goal] = node;
            result.push_back(node);
        }
    }

    return result;
}

float32 Schedule::getBestCost() const
{
    if (m_openNodes.empty()) {
        return FAILED_COST;
    }

    // the best chain of intentions to run
    SearchNode* node = m_openNodes.top();

    if (node) {
        return node->costTotal;
    } else {
        return FAILED_COST;
    }
}

void Schedule::processAuction(const AuctionEvent* auctionEvent) {

   /***************************************************************************
    * Process an incoming auction event
    * 1. Record all the bids
    * 2. Record the auction as complete if all bids have arrived
    ***************************************************************************/
    /// 1. Record all the bids
    for (const AuctionEventBid &bid : auctionEvent->m_bids) {
        Agent *           bidder     = m_agent->engine().getAgent(bid.bidder);
        const GoalHandle &goalHandle = auctionEvent->m_goal;
        auto              key        = std::make_pair(goalHandle, bidder);

        auto costCacheIt = m_delegationCostCache.find(key);
        if (costCacheIt != m_delegationCostCache.end()) {
            JACK_WARNING("Duplicate bid for goal [goal={}, bidder={}]", goalHandle.toString(), bidder->handle().toString());
            continue;
        }

        m_delegationCostCache[key] = bid.score; /// Cache the cost

        /// \todo This best check here doesn't handle ties in the score. This
        /// changes a scenario's behaviour when the order that the team is
        /// initialized in is changed when this really shouldn't. We should
        /// solve ties using the name or something. Confirm this with the JACK
        /// team ...

        /// Store the best (least cost) bid and the delegate associated with
        /// said bid.
        for (size_t checkIndex = 0; checkIndex < m_plannerGoals.size(); checkIndex++) {
            const Goal* goal = m_plannerGoals[checkIndex].goal;
            if (goal->handle() == goalHandle) {
                DelegateCost &dCost = m_bestDelegation[checkIndex];
                if (!dCost.delegate || dCost.cost > bid.score) {
                    dCost.delegate = bidder;
                    dCost.cost     = bid.score;
                }
                break;
            }
        }
    }

    m_pendingAuctions--;

    /// 2. Record the auction as complete if all bids have arrived
    tryFinalizeAuction();
}

} // namespace aos::jack
