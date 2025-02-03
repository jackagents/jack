// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/plan.h>
#include <jack/resource.h>
#include <jack/agent.h>

/// Third Party
#include <tracy/Tracy.hpp>
#include <memory>

namespace aos::jack
{
Plan::Plan(std::string_view name)
    : m_name(std::string(name))
{}

Plan::Plan(const Plan* other)
    : m_engine(other->m_engine)
    , m_name(other->m_name)
    , m_goal(other->m_goal)
    , m_dropCoroutine(other->m_dropCoroutine ? JACK_NEW(Coroutine, *other->m_dropCoroutine) : nullptr)
    , m_precondition(other->m_precondition)
    , m_dropWhen(other->m_dropWhen)
    , m_effects(other->m_effects)
    , m_resourceLocks(other->m_resourceLocks)
{
    setBody(other->m_body);
}

Plan::~Plan()
{
    if (m_body) {
        JACK_DELETE(m_body);
    }

    if (m_dropCoroutine) {
        JACK_DELETE(m_dropCoroutine);
    }
}

void Plan::set(const Plan* other)
{
    m_engine = other->m_engine;
    m_name = other->m_name;
    m_goal = other->m_goal;
    setBody(other->m_body);
    m_dropCoroutine = other->m_dropCoroutine ? JACK_NEW(Coroutine, *other->m_dropCoroutine) : nullptr;
    m_precondition = other->m_precondition;
    m_dropWhen = other->m_dropWhen;
    m_effects = other->m_effects;
    m_resourceLocks = other->m_resourceLocks;
}

void Plan::setBody(CoroutinePtr func)
{
    JACK_DELETE(m_body);
    m_body = nullptr;
    if (func) {
        m_body = JACK_NEW(Coroutine, *func);
    }
}

void Plan::setDropCoroutine(CoroutinePtr func)
{
    JACK_DELETE(m_dropCoroutine);
    m_dropCoroutine = nullptr;
    if (func) {
        m_dropCoroutine = JACK_NEW(Coroutine, *func);
    }
}


// reset the "intention" for execution
// pass in the agent ( the coroutine needs this )
// pass in the parent's goal
void Plan::reset(Agent* agent)
{
    // reset coroutine
    if (m_body) {
        m_body->reset(agent, this);
    }

    if (m_dropCoroutine) {
        m_dropCoroutine->reset(agent, this);
    }
}

Plan::Status Plan::status() const
{
    Plan::Status result  = {};
    result.dropRequested = m_dropped != DropState::NONE;
    result.finishState   = FinishState::NOT_YET;
    if (result.dropRequested) {
        if (!m_dropCoroutine || m_dropCoroutine->finished()) {
            result.finishState = FinishState::DROPPED;
        }
    } else {
        if (m_body) {
            if (m_body->finished()) {
                if (m_body->succeeded()) {
                    result.finishState = FinishState::SUCCESS;
                } else {
                    result.finishState = FinishState::FAILED;
                }
            }
        } else {
            result.finishState = FinishState::SUCCESS;
        }
    }

    result.bodyFinished = !m_body || (m_body->finished() && m_body->succeeded());
    return result;
}

void Plan::tick(BeliefContext &context, const GoalHandle& desire, const UniqueId& intentionId)
{
    ZoneScoped;
    m_body->tick(context, desire, intentionId, m_name);
}

void Plan::applyEffects(BeliefContext &context) const
{
    ZoneScoped;
    if (m_effects) {
        m_effects(context);
    }
}

bool Plan::applyResources()
{
    /// \todo implement correctly
    return false;
}

void Plan::unlockResources(BeliefContext &context) const
{
    for (auto res : m_resourceLocks)
    {
        auto resource = context.resource(res);

        if (resource) {
            resource->unlock();
        } else {
            /// \todo unknown resource - log warning
        }
    }
}

bool Plan::valid(const BeliefContext& context) const
{
    assert(m_engine);

    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    bool result = m_precondition.isValid() ? m_precondition(context) : true;
    return result;
}

bool Plan::shouldDrop(const BeliefContext& context) const
{
    ZoneScoped;
    ZoneName(name().c_str(), name().size());
    bool result = (m_dropWhen.isValid() ? m_dropWhen(context) : false);
    return result;
}

void Plan::lockResources(BeliefContext &context) const
{
    for (auto res : m_resourceLocks)
    {
        auto resource = context.resource(res);

        if (resource) {
            resource->lock();
        } else {
            /// \todo unknown resource - log warning
        }
    }
}

void Plan::drop(BeliefContext& context, const GoalHandle& desire, const UniqueId& intentionId)
{
    ZoneScoped;
    /// \todo redo this for the new executor
    /// the intention executor should probably handle this
    //JACK_WARNING("Plan::drop not implemented...");

    // execute the plan body coroutine
    if (!m_dropCoroutine || m_dropCoroutine->finished()) {
        m_dropped = DropState::FINISHED;
        context.flushActionReplyMessages();
    } else {
        m_dropped = DropState::DROPPING;
        m_dropCoroutine->tick(context, desire, intentionId, m_name);
    }
}

bool Plan::onTaskComplete(const UniqueId &taskId, bool success)
{
    CoroutinePtr coroutine = body();
    if (status().dropRequested) {
        coroutine = m_dropCoroutine;
    }

    bool result = coroutine ? coroutine->onTaskComplete(taskId, success) : false;
    return result;
}

} // namespace aos::jack
