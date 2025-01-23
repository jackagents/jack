#ifndef JACK_SCHEDULE_H
#define JACK_SCHEDULE_H

#include <jack/goal.h>    // for Goal::PlanSelection
#include <jack/handles.h>
#include <jack/corelib.h>

#include <limits>         // for numeric_limits
#include <map>            // for map
#include <memory>         // for shared_ptr
#include <queue>          // for priority_queue
#include <set>            // for set
#include <unordered_map>  // for _Umap_traits<>::allocator_type, unor...
#include <utility>        // for pair
#include <vector>         // for vector, _Vector_iterator, _Vector_co...

namespace aos::jack
{
struct AuctionEvent;
class Agent;
class BeliefContext;
class Plan;

/******************************************************************************
 * @class SearchNode
 *
 ******************************************************************************/
struct GoalPlanInfo
{
    Goal::PlanSelection      planSelection;
    std::vector<const Plan*> plans;
};

struct SearchNode
{
    /**************************************************************************
     * Definitions
     **************************************************************************/
    enum class Failure : uint8_t
    {
        /// The goal could not be achive because executing this goal would cause
        /// a resource violation in the agent.
        RESOURCE_VIOLATION  = 1 << 0,

        /// The goal could not be achieved by the delegate because we did not
        /// receive an auction bid from them before the auction timeout.
        AUCTION_BID_TIMEOUT = 1 << 1,

        /// The goal could not be achieved because the heuristic was impossible
        /// (e.g. returned the prohibitive sentinel cost value).
        HEURISTIC_FAILED    = 1 << 2,

        /// The plan could not be achieved in this node because its precondition
        /// did not pass.
        PLAN_INVALID        = 1 << 3,

        /// The plan cannot be achieved in this node because a required service
        /// on the agent is not available.
        SERVICE_UNAVAILABLE = 1 << 4,

        /// Delegate is already allocated an intention in the graph already.
        /// Delegates (team members) may only execute one goal at a time for the
        /// team.
        DELEGATE_ALLOCATED  = 1 << 5,
    };

    enum class State
    {
        PENDING,
        CLOSED,
        OPEN,
        FAILED,
    };

    struct GoalListItem
    {
        size_t       goalIndex;
        GoalPlanInfo goalPlanInfo;
    };

    struct GoalList
    {
        std::vector<GoalListItem> expandable;

        /// These goals have plans that do not have effects and is sorted in order
        /// of lowest to highest cost as per the heuristic.
        std::vector<GoalListItem> effectless;
    };

    /**************************************************************************
     * Functions
     **************************************************************************/
    ~SearchNode();

    /// Log the search node info out
    void print() const;

    bool isDelegation() const { return plan == nullptr; }

    /**************************************************************************
     * Fields
     **************************************************************************/
    /// Specifies if this node in the schedule was successfully planned in the
    /// graph of intentions to execute or it failed due to some constraints.
    State   state   = State::PENDING;

    /// If the `state` is `Failed`, this enum will be set to the reason that the
    /// search node could not be put into the planned graph of intentions to
    /// execute.
    Failure failure = {};

    Goal*                     goal                = nullptr; ///< The goal to achieve for this search node
    Agent*                    delegate            = nullptr; ///< Team member that is to achieve the goal. Null if this goal is not delegated.
    const Plan*               plan                = nullptr; ///< Non-owning pointer, referencing the template plan in the Engine
    bool                      contextIsCloned     = false;   ///< Flag that indicates if the context was cloned (and must be freed)
    BeliefContext*            context             = nullptr; ///< The execution context after plan effects are applied
    float32                   costFromStart       = 0.f;     ///< The cost so far from the start of the search
    float32                   costOfNode          = 0.f;     ///< The cost of this node in the search tree
    float32                   estimateToEnd       = 0.f;     ///< The estimated cost of the this node to the end of the search tree
    float32                   costTotal           = 0.f;     ///< The total cost including the cost so far
    SearchNode*               parent              = nullptr; ///< The parent search node in the n-ary tree of nodes
    SearchNode*               next                = nullptr; ///< The next node in the n-ary tree of nodes
    SearchNode*               firstChild          = nullptr; ///< The first child node in the n-ary tree of nodes
    SearchNode*               lastChild           = nullptr; ///< The last child node in the n-ary tree of nodes
    std::chrono::milliseconds startCreationTimeMs = {};      ///< Time at start of node creation
    std::chrono::milliseconds terminationTimeMs   = {};      ///< Time at node termination
    std::chrono::milliseconds processingTimeMs    = {};      ///< Processing time of node

    /// 1:1 memoization with the schedule's root goal regarding its precondition
    /// passed. This is copied and reused in children nodes but is recalculated
    /// when the context has changed.
    std::vector<uint8_t> valid;

    /// List of indicies indicating the remaining goals to expand in the search
    /// space as of this current node.
    std::vector<uint32_t> goalsRemaining;

    /// Divides the list of goals into goals that should be expanded
    /// (because they mutate the context) and goals that have no effects.
    GoalList goalList;

    /// The current plan selection state, i.e. the list of plans already
    /// attempted by this SearchNode, the selection policies and e.t.c
    Goal::PlanSelection planSelection = {};
};

struct CompareSearchNode
{
    bool operator() (SearchNode *a, SearchNode *b)
    {
        return a->costTotal > b->costTotal;
    }
};
typedef std::priority_queue<SearchNode*, std::vector<SearchNode*>, CompareSearchNode> SearchNodeQueue;

/******************************************************************************
 * \class   Schedule
 *
 * A Schedule of Intentions for execution.
 ******************************************************************************/
class Schedule
{
    /**************************************************************************
     * Definitions
     **************************************************************************/
public:
    enum ScheduleState
    {
        START,      // The start of the schedule
        SELECT,     // Select the next best search point
        EXPAND,     // Expand the current best into a set of pending nodes
        AUCTION,    // Start an auction for costing delegated goals
        COST,       // Cost the pending nodes
        PENDING_COST,
        DECONFLICT, // Deconflict the pending nodes
        BIND,       // Move the costed and deconflicted pending nodes to the open list
        END         // The schedule is done (solution found or maxed out)
    };

    struct DelegateCost
    {
        Agent*  delegate = nullptr;
        float32 cost     = 0.f;
    };

    /**************************************************************************
     * Functions
     **************************************************************************/
public:
    Schedule(Agent *agent, const std::vector<Goal*>& goals);

    ~Schedule();

    /// Expand a goal into a list of candidate plan nodes to explore in the
    /// scheduler.
    /// @param rootGoalIndex The index in the schedule's list of root goals to
    /// use to expand from.
    /// @return The list of candidate nodes
    std::vector<SearchNode*> expandGoalToPlans(SearchNode* parent, size_t rootGoalIndex, const GoalPlanInfo& goalPlanInfo);

    /// Expand this schedule's open list. i.e. the list of candidate plans for
    /// consideration
    void expand();

    void cost();

    void auction();

    void deconflict();

    void bind();

    // print the whole schedule
    void printSchedule() const;

    /// Produce the list of intentions to execute based off the planning that is
    /// completed in the schedule. Each intention is assigned an agent and the
    /// list is sorted in the order of execution (e.g. the first intention/node
    /// to execute is stored at the start of the list).
    ///
    /// @return Get the list of search nodes/intentions to execute in sequence
    /// based on the result of the scheduler's optimising forward planner.
    std::vector<SearchNode*> getBestIntentions() const;

    bool isFinished() const { return m_state == Schedule::END || m_searchDepth <= 0; }

    bool isWaitingForAuctions() const { return m_state == Schedule::PENDING_COST; }

    float getBestCost() const;

    void processAuction(const AuctionEvent* auctionEvent);

    /// The ID of the schedule. It is currently used to map returning auction
    /// bids to their schedule.
    size_t id() const { return m_id; };

    struct PlannerGoal
    {
        Goal* goal;
        bool  canBePlanned;
    };

    /// @return The goals that the schedule is trying to achieve.
    const std::vector<PlannerGoal>& plannerGoals() const { return m_plannerGoals; };

    /// @return List of goals that the schedule is trying to solve into an
    /// ordered chain of intentions to execute.
    std::vector<PlannerGoal>& plannerGoals() { return m_plannerGoals; };

    const SearchNodeQueue& openNodes() const { return m_openNodes; }

    const std::vector<SearchNode*>& closedNodes() const { return m_closedNodes; }

    const std::vector<SearchNode*>& pendingNodes() const { return m_pendingNodes; }

    const std::vector<SearchNode*>& failureNodes() const { return m_failureNodes; }

    /// Get the root node of the schedule
    /// @return The root node of this schedule. The root node will never be null
    /// and always present.
    const SearchNode* root() const { return &m_root; }

    /// Get the agent that owns the schedule
    /// @return The agent that owns the schedule.
    const Agent* agent() const { return m_agent; }

protected:
    float32 estimateCostFrom(const Goal*                  currGoal,
                             BeliefContext*               context,
                             const std::vector<uint32_t>& goalsRemaining) const;

private:
    void addNodeToFailureList(SearchNode* node, SearchNode::Failure failure);

    void tryFinalizeAuction();

    /**************************************************************************
     * Fields
     **************************************************************************/
public:
    ArenaAllocator m_arena;

    /// The cost value to assign if a JACK plan in the planner's schedule
    /// cannot be completed/executed successfully (e.g. because the plan is
    /// impossible to achieve, there are no agents available or services are
    /// missing e.t.c);
    static constexpr inline float FAILED_COST = std::numeric_limits<float>::max();

    /// A monotonically increasing id. Auction events reference the age of the
    /// schedule to know when a slow auction result no-longer references
    /// a schedule known by the agent holding the schedule.
    size_t m_id;

    /// A handle back to the agent that triggered this schedule. This is only
    /// set when a team auctions out to its team member. The recipient team
    /// member will generate a schedule to simulate the cost of completing the
    /// delegation. In that schedule, this handle will be set to the team
    /// agent.
    AgentHandle m_delegator = {};

    /// The schedule ID of the team that triggered the auction on this agent.
    /// `m_delegator` will be set and return `valid()` on the handle for this to
    /// be set to a value other than 0.
    size_t m_delegatorScheduleID = 0;

    /// The time stamp taken from the engine's internal clock to mark when the
    /// schedule was started
    std::chrono::high_resolution_clock::duration m_startTime = {};

protected:
    // a list of pending uncosted search nodes
    std::vector<SearchNode*> m_pendingNodes;

    /// A mapping from an index in the schedule's root goal list to the best
    /// agent to complete a goal (i.e. the agent/delegate with the least cost to
    /// complete the goal) in a team. Determined after having received back
    /// auction bids from the team members.
    std::vector<DelegateCost> m_bestDelegation;

    /// A table of all the (goals, agents) and their cost to complete the goal.
    std::map<std::pair<GoalHandle, Agent*>, float32> m_delegationCostCache;

    /// A list of weakly-owned goals that the scheduled plan must start with (it
    /// can start with any one of them, but it has to be one of these goals).
    std::vector<PlannerGoal> m_plannerGoals;

    // the owning agent to this schedule
    Agent *m_agent;

    // the root of the search tree
    SearchNode m_root;

    // priority queue of open search nodes
    // the top search node of this priority queue contains the best plan
    // grap top and traverse the search nodes backward to form the best plans of intentions.
    SearchNodeQueue m_openNodes;

    std::vector<SearchNode*> m_closedNodes;

    std::vector<SearchNode*> m_clonedContextNodes;

    /// Keep track of node failures (e.g. plan precondition failed or a
    /// necessary service for the plan was unavailable e.t.c) for metrics.
    std::vector<SearchNode*> m_failureNodes;

    // the current depth of the search
    int32_t m_searchDepth = 1000;

    ScheduleState m_state = START;

    int32_t m_pendingAuctions = 0;

    /// Records the failures as bit flags corresponding to `SearchNode::Failure`
    /// If any node in this graph encountered failures the corresponding bit
    /// will be set.
    uint8_t m_searchNodeFailureFlags = 0;
};

} // namespace aos::jack
#endif // JACK_SCHEDULE_H
