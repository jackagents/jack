#include <jack/builders/resourcebuilder.h>
#include <jack/engine.h>
#include <jack/resource.h>

namespace aos::jack
{
ResourceBuilder::ResourceBuilder(Engine& engine, std::string_view name)
    : Builder(engine, name)
    , m_min(0)
    , m_max(0)
{}

ResourceBuilder::ResourceBuilder(const ResourceBuilder &other)
    : Builder(other.m_engine, other.m_name)
{}

ResourceBuilder& ResourceBuilder::max(int max)
{
    m_max = max;
    return *this;
}

ResourceBuilder& ResourceBuilder::min(int min)
{
    m_min = min;
    return *this;
}

ResourceBuilder& ResourceBuilder::commit()
{
    Resource resource(name(), m_max, m_min);
    m_engine.commitResource(&resource);
    return *this;
}
} // namespace aos::jack
