// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_TEAM_H
#define JACK_TEAM_H

#include <jack/agent.h>     // for Agent
#include <jack/corelib.h>

#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <vector>      // for vector
#include <string>      // for string

namespace aos { namespace jack {

class Engine;
class Goal;
struct AuctionEvent;

/*! ***********************************************************************************************
 * @class   Team
 *
 * JACK's team agent type - derived from agent
 * ************************************************************************************************/


class Team : public Agent
{
public:

    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/

    /// Constructs a Team with a name
    /// @param engine[in] The JACK engine instance
    /// @param name[in] The name of the Team instance
    Team(Engine& engine, std::string_view name);

    /// Constructs a Team from a template and name
    /// @param other[in] The Team template
    /// @param name[in] The name for the constructed Team instance
    Team(const Team* other, std::string_view name);

    virtual ~Team() {}

    /// @private
    Team(const Team &other) = delete;

    /// @private
    Team& operator=(const Team &other) = delete;

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/
    /// Add a member agent to this team
    /// @param member The Agent member to add
    void addMemberAgent(Agent *member);

    /// Remove a member agent from this team
    /// @param member The Agent member to remove
    void removeMemberAgent(Agent *member);

    /// Set the list of member agents for this team
    void setMemberAgents(const std::vector<Agent *> &members);

    /// Get a list of the current member agents for this team
    const std::vector<Agent *> &memberAgents() const { return m_members; }

    /// Get a list of the current member agents for this team
    std::vector<Agent *> &memberAgents() { return m_members; }

    /// register a handler for call backs when the team member list changes
    void setMembersChangedHandler(const std::function<void()> &func) {
        m_membersChangedHandler = func;
    }

    /// @return Returns whether or not the agent was instantiated as a team.
    /// Always true for the Team class.
    bool isTeam() const override final { return true; }

    /// Query the team members that can handle the goal
    GetDelegatesResult getDelegates(const GoalHandle &goal) override final;

protected:
    /// Delegate a goal to the delegate agent. No-op for Agents. Note, only teams can delegate goals to agents
    void delegateGoal(const GoalHandle &goalHandle, Agent* delegate, const std::shared_ptr<Message>& parameters) override;

    /// Analyse a delegation goal. No-op for Agents. Note, only teams can delegate goals to agents
    /// @param goal The goal to delegate out (i.e auction out).
    /// @param scheduleId The ID of the schedule the auctions will be generated for
    bool analyseDelegation(Goal *goal, size_t scheduleId) override;

    /// Drop a delegated goal. No-op for Agents. Note, only teams can drop delegations
    void dropDelegation(const GoalHandle &goalHandle, Agent *delegate) override;

    // clone this agent and return the object
    Agent *clone(std::string_view name) const override { return JACK_NEW(Team, this, name); }

    // weak pointers to member agents
    std::vector<Agent *> m_members;

    // handler for the member list changing
    std::function<void ()> m_membersChangedHandler;

    std::vector<AuctionEvent *> m_auctions;

private:
    struct GetDelegatesCache
    {
        size_t cachedOnEngineTick = 0;  ///< Engine tick that getDelegates was last called and cached for
        bool init = false;              ///< Track if the cache has been set at-least once.
        std::vector<Agent *> delegates; ///< Delegates for the engine tick
        std::string delegationOutcome;  ///< String describing which delegates were considered and successful or failed
    };

    /// Cached result of calling get delegates on the same tick.
    std::unordered_map<std::string /*Goal Name*/, GetDelegatesCache> m_getDelegatesCache;
};

}} // namespace aos::jack

#endif
