#include <jack/team.h>           // for Team
#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/engine.h>
#include <jack/goal.h>           // for Goal
#include <jack/promise.h>        // for PromisePtr, Promise
#include <jack/role.h>           // for Role
#include <jack/schedule.h>
#include <jack/proxyagent.h>
#include <jack/corelib.h>

#include <jack/events/event.h>   // for Event, Event::FAIL, Event::Status
#include <jack/events/auctionevent.h>
#include <jack/events/delegationevent.h>
#include <jack/events/timerevent.h>

/// Third Party
#include <tracy/Tracy.hpp>
#include <cassert>                   // for assert
#include <chrono>                    // for seconds
#include <functional>                // for function, _Func_impl_no_alloc<>:...
#include <memory>                    // for allocator, shared_ptr
#include <type_traits>               // for move
#include <utility>                   // for max, min
#include <vector>                    // for _Vector_iterator, vector, _Vecto...

namespace aos { namespace jack {
Team::Team(Engine& engine, std::string_view name)
    : Agent(engine, name)
{
    m_busAddress.type = protocol::NodeType_TEAM;
}

Team::Team(const Team* other, std::string_view name)
    : Agent(other, name)
{
    m_busAddress.type = protocol::NodeType_TEAM;
}

void Team::addMemberAgent(Agent *member)
{
    /// \note Duplicate member check
    for (const Agent *currentMember : m_members) {
        if (member->handle() == currentMember->handle()) {
            JACK_WARNING("Agent to add to team is already in the team [team={}, member={}]",
                         handle().toString(),
                         member->handle().toString());
            return;
        }
    }

    /// \todo This should be an event
    JACK_DEBUG("Adding team member [team={}, member={}]", handle().toString(), member->handle().toString());
    if (member->roles().empty()) {
        JACK_WARNING("Adding a member to the team with no roles assigned, the team will be unable to delegate goals to it [team={}, member={}]",
                     handle().toString(),
                     member->handle().toString());
    }

    m_members.push_back(member);     /// \todo: validate that this agent is actually useful
    member->addTeamMembership(this); /// Make the member aware of what team it is in

    /// \todo: create a lookup of goals to roles so we only
    /// allocate the goals to member who support a role suitable
    /// right now, all members support all roles
    m_scheduleDirty |= ScheduleDirty_MEMBER_ADDED;

    if (m_membersChangedHandler) {
        m_membersChangedHandler();
    }

    /// \note Add the joining event onto the bus
    if (!dynamic_cast<ProxyAgent *>(member) && m_engine.haveBusAdapter()) {
        protocol::AgentJoinTeam busEvent = m_engine.makeProtocolEvent<protocol::AgentJoinTeam>();
        busEvent.team                    = m_busAddress;
        busEvent.agent                   = member->busAddress();
        m_engine.sendBusEvent(&busEvent);
    }
}

// remove a member agent from this team
void Team::removeMemberAgent(Agent* member)
{
    /// \todo This should be an event
    auto it = std::find(m_members.begin(), m_members.end(), member);
    if (it != m_members.end()) {
        m_members.erase(it);
    }

    m_scheduleDirty |= ScheduleDirty_MEMBER_REMOVED;

    // call the callback
    if (m_membersChangedHandler) {
        m_membersChangedHandler();
    }

    if (!dynamic_cast<ProxyAgent *>(member) && m_engine.haveBusAdapter()) {
        protocol::AgentLeaveTeam busEvent = m_engine.makeProtocolEvent<protocol::AgentLeaveTeam>();
        busEvent.team                     = m_busAddress;
        busEvent.agent                    = member->busAddress();
        m_engine.sendBusEvent(&busEvent);
    }
}

/// set the list of member agents for this team
void Team::setMemberAgents(const std::vector<Agent*>& members)
{
    /// \todo This should be an event

    m_members.clear();

    // copy in the new members
    m_members = members;

    /// \todo trigger a full replan of the team agent

    // trigger the team member being removed to drop all delegations
    m_scheduleDirty |= ScheduleDirty_MEMBER_ADDED | ScheduleDirty_MEMBER_REMOVED;

    // call the callback
    if (m_membersChangedHandler)
        m_membersChangedHandler();
}

// Team Delegation Flow
// - Generate DelegationEvent to members of the team that can handle the goal
// - Team member calculates the cost of completing the goal by simulating
//   a schedule when it receives the event
// - The cost (auction bid) of the schedule is returned to the team as an AuctionEvent
// - The team runs processAuction on each event, collecting all the costs (bids) into the
// schedule
// - When the schedule is processed in processSchedule(..) will assign the costs
//   to the nodes in the planner
// - The schedule is applied in setSchedule and the team will delegateGoal to the appropriate
// agent
// - Returns true if a new auction is added to m_auctions, otherwise false
bool Team::analyseDelegation(Goal* goal, size_t scheduleId)
{
    // let's create the auction
    const GoalHandle&          goalHandle     = goal->handle();
    GetDelegatesResult         delegateResult = getDelegates(goalHandle);
    const std::vector<Agent*>& members        = delegateResult.delegates;
    if (members.size() == 0) {
        JACK_WARNING("Team auction cancelled, no delegates available [team={}, goal={}, reason='{}']",
                     handle().toString(),
                     goal->handle().toString(),
                     delegateResult.delegationOutcome);
        return false; // no auction
    }

    /// \todo We should potentially emit a BDI logging event into the event
    /// queue for explanability that the team initiated an auction.
    CurrentAuction auction   = {};
    auction.expiryTimePoint  = std::chrono::duration_cast<std::chrono::milliseconds>(m_engine.internalClock()) + std::chrono::seconds(2);
    auction.goal             = goal->handle();
    auction.scheduleId       = scheduleId;
    auction.totalDelegations = members.size();
    m_currentAuctions.push_back(std::move(auction));

    for (auto member : members) {
        assert(member->isAvailable() && "The delegate list should already be filtered by availability");
        DelegationEvent *delegationEvent     = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, DelegationEvent, goalHandle, goal->context().goal());
        delegationEvent->recipient           = member;
        delegationEvent->caller              = this;
        delegationEvent->analyse             = true;
        delegationEvent->team                = this->handle();
        delegationEvent->delegatorScheduleID = scheduleId;
        addEvent(delegationEvent);
    }

    return true;
}

// drop a remote delegated goal
void Team::dropDelegation(const GoalHandle &goalHandle, Agent *delegate)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("Team dropping delegation [team={}, delegate={}, goal={}]"),
                   handle().toString(),
                   delegate->handle().toString(),
                   goalHandle.toString());
    std::string reason = builder.toString();
    JACK_DEBUG("{}", reason);

    auto *event                 = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, Event, Event::DROP);
    event->dropEvent.senderNode = engine().busAddress();
    event->dropEvent.goal       = goalHandle.m_name;
    event->dropEvent.goalId     = goalHandle.m_id.toString();
    event->recipient            = delegate;
    event->caller               = this;
    event->dropEvent.sender     = event->caller->busAddress();
    event->dropEvent.recipient  = event->recipient->busAddress();
    event->setReason(std::move(reason));

    m_engine.routeEvent(event);
}

// handle a goal delegation
/// \todo could be delegate event?
void Team::delegateGoal(const GoalHandle &goalHandle, Agent* delegate, const std::shared_ptr<Message>& parameters)
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("Goal delegated [team={}, delegate={}, goal={}"),
                   handle().toString(),
                   delegate->handle().toString(),
                   goalHandle.toString());

    // Find an agent to delegate this event to
    DelegationEvent *delegationEvent = JACK_ALLOCATOR_NEW(&m_engine.m_eventAllocator, DelegationEvent, goalHandle, parameters);
    delegationEvent->setReason(builder.toString());
    delegationEvent->team = this->handle();

    /// \todo:  check the allocation is valid here
    if (delegate != nullptr) {
        delegationEvent->recipient = delegate;
        delegationEvent->caller = this;

        auto p = addEvent(delegationEvent);

        /// \todo Remove this it's a hack to force drop from all the other members too
        /// requried at the moment because for some reason the goal doesn't get dropped
        /// and ends up on multiple team members
        for (auto *member : m_members) {
            if (member != delegate && member->getDesire(goalHandle.m_id)) {
                dropDelegation(goalHandle, member);
            }
        }

        // handle when the delegation is handled
        // NOTE: the promise is owned by the lambda - hence mutable (the shared pointer is moved into)
        // called when the delegation event comes back to this team - so this is safe
        /// \todo don't do this - it's not really obvious
        p->then([p]() mutable {
            // destroy self
            p.reset();
        });
    } else {
        // fail we need to fail this event
        delegationEvent->status = Event::Status::FAIL;
        // reason? bad delegation
        delegationEvent->recipient = this;
        delegationEvent->caller = this;

        routeEvent(delegationEvent);
        JACK_WARNING("Goal delegation failed, planner could not assign a delegate [team={}, goal={}]",
                     m_handle.toString(),
                     goalHandle.toString());
    }
}

Agent::GetDelegatesResult Team::getDelegates(const GoalHandle &goal)
{
    ZoneScoped;
    /// Already pre-calculated the delegates for this tick, return the cache
    GetDelegatesCache &cache     = m_getDelegatesCache[goal.m_name];
    size_t             pollCount = engine().pollCount();

    /// \note We have a edge-case on the first tick of the engine,
    /// m_cachedOnEngineTick is initialised to 0 and pollCount starts from 0
    /// which would normally return the wrong delegates for just the first tick.
    /// The 'init' flag covers this edge-case.
    if (cache.init && cache.cachedOnEngineTick == pollCount) {
        return GetDelegatesResult{cache.delegates, cache.delegationOutcome};
    }

    /// \note Clear the cache
    cache.init = true;
    cache.delegates.clear();
    cache.cachedOnEngineTick = engine().pollCount();
    cache.delegationOutcome.clear();

    /// Recalculate the team delegates for this tick into the cache. Collect all
    /// the agents that have a role supporting the goal into our candidate list.
    std::vector<Agent*> canDelegate;
    std::vector<Agent*> cannotDelegate;

    for (Agent *member : m_members) {
        bool agentHasRoleForGoal = false;

        // Filter the team member's (agents) to members that have a role allowed to execute the goal.
        std::vector<std::string> const &roleNames = member->roles();
        for (std::string const &roleName : roleNames) {
            Role const *role = m_engine.getRole(roleName);
            if (!role) {
                JACK_ERROR("Role in agent does not exist in the engine [role={}, agent={}]",
                           roleName,
                           member->name());
                continue;
            }

            const std::vector<std::string>& goalNames = role->goals();
            for (const std::string& agentGoalName : goalNames) {
                if (goal.m_name == agentGoalName) {
                    agentHasRoleForGoal = true;
                    break;
                }
            }

            if (agentHasRoleForGoal) {
                if (member->isAvailable() && member->running()) {
                    canDelegate.push_back(member);
                } else {
                    cannotDelegate.push_back(member);
                }
                break;
            }
        }
    }

    /// \note Create a human readable description of the outcome of the delegation analysis process
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    if (canDelegate.empty() && cannotDelegate.empty()) {
        builder.append(FMT_STRING("{} has no team members to delegate to"), handle().toHumanString("Team"));
    } else {
        builder.append(FMT_STRING("{} considered {} candidate(s) for the {}.\n"),
                       m_handle.toHumanString("Team"),
                       canDelegate.size() + cannotDelegate.size(),
                       goal.toHumanString("goal"));

        if (canDelegate.size()) {
            builder.append(FMT_STRING("There was {} agent(s) that can be delegated to\n\n"), canDelegate.size());
            for (const Agent* agent : canDelegate) {
                builder.append(FMT_STRING("  {}\n"), agent->handle().toHumanString());
            }

            if (cannotDelegate.size()) {
                builder.appendRef("\n");
            }
        }

        if (cannotDelegate.size()) {
            builder.append(FMT_STRING("There was {} agent(s) that can not be delegated to\n\n"), cannotDelegate.size());
            for (const Agent* agent : cannotDelegate) {
                builder.append(FMT_STRING("  {}:"), agent->handle().toHumanString());
                if (!agent->isAvailable()) {
                    builder.appendRef("is unavailable;");
                }
                if (!agent->running()) {
                    builder.appendRef("is not running");
                }
                builder.appendRef("\n");
            }
        }
    }

    cache.delegates         = std::move(canDelegate);
    cache.delegationOutcome = builder.toString();
    return GetDelegatesResult{cache.delegates, cache.delegationOutcome};
}

}} // namespace aos::jack
