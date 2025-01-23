#include <jack/resource.h>
#include <jack/agent.h>

namespace aos { namespace jack {

Resource::Resource(std::string_view name)
    : m_name(name)
    , m_count(1)
    , m_max(1)
    , m_min(0)
    , m_reserved(0)
    , m_agent(nullptr)
{}

Resource::Resource(std::string_view name, int max, int min)
    : m_name(name)
    , m_count(max)
    , m_max(max)
    , m_min(min)
    , m_reserved(0)
    , m_agent(nullptr)
{}

Resource::Resource(const Resource &other)
    : m_name(other.m_name)
    , m_count(other.m_count)
    , m_max(other.m_max)
    , m_min(other.m_min)
    , m_reserved(other.m_reserved)
    , m_agent(nullptr)
{}

Resource& Resource::operator=(const Resource &other)
{
    m_name = other.m_name;
    m_count = other.m_count;
    m_max = other.m_max;
    m_min = other.m_min;
    m_reserved = other.m_reserved;
    m_agent = other.m_agent;
    return *this;
}

void Resource::set(int count)
{
    if (m_count == count) {
        /// \note Guards against consuming 0 resources causing spurious
        /// percept events.
        return;
    }

    m_count = count;
    if (m_agent) {
        /// \todo We don't implement reserved amounts yet
#if 0
        if (m_count < m_reserved) {
            // if the amount of resource is decreased below the reserved amount
            // we need to - trigger an event to handle a replan.
        }
#else
        // \todo This is a work around. See agent.cpp:addResourcePerceptEvent
        // comment. Ideally we take the agent's dispatch queue and write
        // a percept event directly to it.
        m_agent->addResourcePerceptEvent(m_name, m_count);
#endif
    }
}

int Resource::consume(int count)
{
    int result = m_count - count;
    set(result);
    return result;
}

}} // namespace aos::jack
