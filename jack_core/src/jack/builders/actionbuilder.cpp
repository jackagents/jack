/// JACK
#include <jack/builders/actionbuilder.h>
#include <jack/engine.h>

namespace aos::jack
{
ActionBuilder::ActionBuilder(Engine& engine, std::string_view name)
: Builder(engine, name)
{
}

ActionBuilder::ActionBuilder(const ActionBuilder &other)
    : Builder(other.m_engine, other.m_name)
{
}

ActionBuilder& ActionBuilder::commit()
{
    Action action   = {};
    action.request  = m_request;
    action.reply    = m_reply;
    action.name     = m_name;
    action.feedback = m_feedback;
    m_engine.commitAction(&action);
    return *this;
}
} // namespace aos::jack
