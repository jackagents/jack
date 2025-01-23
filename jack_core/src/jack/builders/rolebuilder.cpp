#include <jack/builders/rolebuilder.h>
#include <jack/builders/goalbuilder.h>    // for GoalBuilder
#include <jack/engine.h>                  // for Engine
#include <jack/role.h>                    // for Role, RoleBeliefSet, RoleBeli...

namespace aos::jack
{
RoleBuilder::RoleBuilder(Engine& engine, std::string_view name)
    : Builder(engine, name)
    , m_role(std::string(name))
{
}

RoleBuilder& RoleBuilder::goals(const Span<GoalBuilder> items)
{
    std::vector<std::string>& goals = m_role.goals();
    goals.reserve(goals.size() + items.size());
    for(const GoalBuilder& item : items) {
        goals.push_back(item.name());
    }
    return *this;
}

RoleBuilder& RoleBuilder::goalNames(const Span<std::string_view> items)
{
    std::vector<std::string>& goals = m_role.goals();
    goals.insert(goals.end(), items.begin(), items.end());
    return *this;
}

RoleBuilder& RoleBuilder::beliefs(std::string_view name,
                                  RoleBeliefSet::ReadAccess read,
                                  RoleBeliefSet::WriteAccess write)
{
    m_role.addBeliefset({std::string(name), read, write});
    return *this;
}

RoleBuilder& RoleBuilder::commit()
{
    m_engine.commitRole(&m_role);
    return *this;
}
} // namespace aos::jack
