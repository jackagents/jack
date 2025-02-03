// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/role.h>

namespace aos { namespace jack {

Role::Role(const std::string& name)
    : m_name(name)
{
}

/// @param checkReadAccess If true, check if we have read permission, otherwise check if we have write access
static bool canAccessTeamBeliefSet(const std::vector<RoleBeliefSet> &roleBSets, const std::string &beliefSetName, bool checkReadAccess)
{
    for (const RoleBeliefSet &roleBSet : roleBSets) {
        if (roleBSet.m_name != beliefSetName) {
            continue;
        }

        if (checkReadAccess) {
            if (roleBSet.m_readFromTeam == RoleBeliefSet::ReadAccess::YES) {
                return true;
            }
        } else {
            if (roleBSet.m_writeToTeam == RoleBeliefSet::WriteAccess::YES) {
                return true;
            }
        }
    }

    return false;
}

bool Role::canWriteBeliefSetToTeam(const std::string &beliefSetName) const
{
    bool result = canAccessTeamBeliefSet(m_beliefsets, beliefSetName, false /*checkReadAccess*/);
    return result;
}

bool Role::canReadBeliefSetFromTeam(const std::string &beliefSetName) const
{
    bool result = canAccessTeamBeliefSet(m_beliefsets, beliefSetName, true /*checkReadAccess*/);
    return result;
}

}} // namespace aos::jack
