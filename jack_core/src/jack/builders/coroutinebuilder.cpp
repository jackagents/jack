#include <jack/builders/coroutinebuilder.h>
#include <jack/coroutine.h>             // for Coroutine, CoroutinePtr
#include <jack/builders/builder.h>      // for Builder
#include <jack/corelib.h>

namespace aos::jack
{
CoroutineBuilder::CoroutineBuilder(Engine& engine)
    : Builder(engine, "")
    , m_currentLabel(Coroutine::TERMINAL_LABEL)
{}

CoroutineBuilder::CoroutineBuilder(const CoroutineBuilder &other)
    : Builder(other.m_engine, other.m_name)
    , m_coroutine(other.m_coroutine)
    , m_currentLabel(Coroutine::TERMINAL_LABEL)
{
}

CoroutineBuilder& CoroutineBuilder::action(std::string_view name, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addActionTask(name, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::paramFrom(std::string_view param, std::string_view contextParamName)
{
    if (!JACK_CHECK(m_currentLabel != Coroutine::TERMINAL_LABEL)) {
        return *this;
    }
    m_coroutine.addParamFrom(param, contextParamName);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::sleep(int milliseconds, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addSleepTask(milliseconds, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::print(std::string_view message, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addPrintTask(message, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::goal(std::string_view goal, const UniqueId& id)
{
    m_currentLabel =  m_coroutine.addGoalTask(goal, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::drop(const GoalHandle& handle, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addDropTask(handle, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::nowait()
{
    assert(m_currentLabel != Coroutine::TERMINAL_LABEL);
    if (m_currentLabel == Coroutine::TERMINAL_LABEL) return *this;

    m_coroutine.nowait();
    return *this;
}

CoroutineBuilder& CoroutineBuilder::yieldUntil(std::function<bool(const BeliefContext &)> func, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addYieldTask(func, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::cond(std::function<bool(const BeliefContext &)> func, const UniqueId& id)
{
    m_currentLabel = m_coroutine.addCondTask(func, id);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::configure(int32_t label)
{
    int32_t maxLabel = static_cast<int32_t>(m_coroutine.tasks().size());
    if (!JACK_CHECK(label < maxLabel)) {
        return *this;
    }

    /// \note Any label < 0 is considered the start node
    m_currentLabel = std::max(label, 0);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::onSuccess(int targetLabel)
{
    int32_t maxLabel = static_cast<int32_t>(m_coroutine.tasks().size());
    if (!JACK_CHECK(targetLabel <= maxLabel)) {
        return *this;
    }

    m_coroutine.setSuccessEdge(m_currentLabel, targetLabel);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::onFail(int targetLabel)
{
    int32_t maxLabel = static_cast<int32_t>(m_coroutine.tasks().size());
    if (!JACK_CHECK(targetLabel <= maxLabel)) {
        return *this;
    }

    /// \note The starting label can never fail
    if (!JACK_CHECK(m_currentLabel != Coroutine::TERMINAL_LABEL)) {
        return *this;
    }

    m_coroutine.setFailEdge(m_currentLabel, targetLabel);
    return *this;
}

CoroutineBuilder& CoroutineBuilder::label(int& label)
{
    label = m_currentLabel;
    return *this;
}

CoroutinePtr CoroutineBuilder::create() const
{
    CoroutinePtr p = JACK_NEW(Coroutine, m_coroutine);
    return p;
}
} /// namespace aos::jack
