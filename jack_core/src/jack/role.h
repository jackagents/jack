#ifndef JACK_ROLE_H
#define JACK_ROLE_H

#include <jack/corelib.h>

#include <string>
#include <vector>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * @class Role
 *
 * @brief A Role define the interface between an Agent and a Team
 *
 * ************************************************************************************************/
class RoleBeliefSet
{
public:
    enum struct ReadAccess  { NO, YES };
    enum struct WriteAccess { NO, YES };

    std::string m_name;
    ReadAccess  m_readFromTeam;
    WriteAccess m_writeToTeam;
};

class Role
{
public:
    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    Role() = default;

    /// Constructs a default Role with a name
    /// @param name The name of this Role
    Role(const std::string& name);

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/
    bool valid() const { return m_name.size(); }

    /// Add a Goal for this Role to support
    /// @param goal The Goal to add to this Role
    void addGoal(std::string_view goal) { m_goals.push_back(std::string(goal)); }

    /// Add a Beliefset to this role that assigned agents will support reading and writing
    /// @param entry The beliefSet to add to this role
    void addBeliefset(RoleBeliefSet &&entry) { m_beliefsets.emplace_back(entry); }

    /// @return The name of this Role
    const std::string& name() const { return m_name; }

    /// @return The list of Beliefsets for this role
    const std::vector<RoleBeliefSet>& beliefsets() const { return m_beliefsets; }

    /// @return The list of Goals applicable for this Role
    std::vector<std::string>& goals() { return m_goals; }

    /// @return The list of Goals applicable for this Role
    const std::vector<std::string>& goals() const { return m_goals; }

    bool canWriteBeliefSetToTeam(const std::string &beliefSetName) const;

    bool canReadBeliefSetFromTeam(const std::string &beliefSetName) const;

protected :
    /* ****************************************************************************************
     * Attributes
     * ****************************************************************************************/
    /// The name of the role
    std::string m_name;

    /// A list of the goals that this role will support
    std::vector<std::string> m_goals;

    /// A list of beliefsets this role will support
    std::vector<RoleBeliefSet> m_beliefsets;
};

}} // namespace aos::jack

#endif
