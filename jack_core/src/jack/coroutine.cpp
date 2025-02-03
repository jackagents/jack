// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/coroutine.h>

#include <jack/handles.h>   // for GoalHandle, PlanHandle...
#include <jack/tasks/actiontask.h>
#include <jack/tasks/conditionaltask.h>
#include <jack/tasks/droptask.h>
#include <jack/tasks/printtask.h>
#include <jack/tasks/pursuetask.h>
#include <jack/tasks/sleeptask.h>
#include <jack/tasks/yielduntiltask.h>

namespace aos { namespace jack {


Coroutine::Coroutine()
        : m_current(0)
        , m_succeeded(true)
        , m_numAsyncToWaitFor(0)
{}

Coroutine::Coroutine(const Coroutine &other)
    : m_current(0)
    , m_succeeded(true)
    , m_numAsyncToWaitFor(0)
{
    m_tasks.reserve(other.m_tasks.size());
    for (auto t : other.m_tasks) {
        m_tasks.push_back(t->clone());
    }
}

Coroutine::~Coroutine()
{
    // clean up all coroutine tasks
    for(Task* t : m_tasks) {
        JACK_DELETE(t);
    }
}

void Coroutine::reset(Agent* agent, Plan *intention)
{
    // we just start with the first task at the moment
    /// \todo support multiple starting tasks - somehow
    m_current = 0;
    m_succeeded = true;

    // reset all coroutine tasks
    for(Task* t : m_tasks) {
        t->reset(agent, intention);
    }
}

/// @return if the coroutine has finished
bool Coroutine::finished()
{
    // Are we still waiting for async tasks to finish?
    if (m_numAsyncToWaitFor > 0) {
        return false;
    }

    /// fast path: if the last tasks are done then we are also finished
    /// @todo This method should really be const so hopefully this fast pathing will
    /// be removed at some point
    /// If removed the inverse pre-condition unit tests fails

    // if not finished yet
    if (m_tasks.size() > m_current) {
        Task *task = m_tasks[m_current];
        assert(task);

        // the current task is done
        if (task->state() == Task::DONE) {

            // task is successful and pointing to the end
            if (task->status() == Task::SUCCEEDED && task->successTarget() == Coroutine::TERMINAL_LABEL) {
                // finish the plan
                m_succeeded = true;
                m_current = m_tasks.size();
            }

            // task has failed and pointing to the end
            if (task->status() == Task::FAILED && task->failTarget() == Coroutine::TERMINAL_LABEL) {
                // finish the plan
                m_succeeded = false;
                m_current = m_tasks.size();
            }
        }
    }

    return m_tasks.size() <= m_current;
}

bool Coroutine::waiting() const
{
    if (m_tasks.size() > m_current) {
        Task *task = m_tasks.at(m_current);
        return (task->state() == Task::WAIT);
    }

    return false;
}

// mark an async task as being done
void Coroutine::markAsyncDone()
{
    m_numAsyncToWaitFor--;
}

bool Coroutine::tick(BeliefContext&    context,
                     const GoalHandle& desire,
                     const UniqueId&   intentionId,
                     std::string_view  plan)
{
    if (m_tasks.size() > m_current) {
        Task *task = m_tasks[m_current];
        if (task->tick(context, desire, intentionId, plan)) {  // either done or async
            if (task->status() == Task::SUCCEEDED) {
                if (task->state() == Task::ASYNC) {
                    // we need to wait for this async task
                    m_numAsyncToWaitFor++;
                }

                // get the success label
                int label = task->successTarget();

                if (label != Coroutine::TERMINAL_LABEL) {
                    // it's not the end
                    m_current = label;
                    if (Task *nextTask = m_tasks[m_current]) {
                        nextTask->setReady();
                    }
                } else {
                    // finish
                    m_current = m_tasks.size();
                }
            } else {

                // get the fail label
                int label = task->failTarget();

                if (label != Coroutine::TERMINAL_LABEL) {
                    // it's not the end
                    m_current = label;
                    if (Task *nextTask = m_tasks[m_current]) {
                        nextTask->setReady();
                    }
                } else {

                    // finish
                    m_current = m_tasks.size();

                    // the plan failed
                    m_succeeded = false;
                }

            }
        }
    }
    return finished();
}

void Coroutine::addParamFrom(std::string_view param, std::string_view contextParamFrom)
{
    if (!m_tasks.empty()) {
        Task* t = m_tasks.back();
        t->paramFrom(param, contextParamFrom);
    }
}

void Coroutine::addParam(std::string_view param)
{
    addParamFrom(param, "" /*contextParamFrom*/);
}

void Coroutine::addLiteralParam(std::string_view name, const std::any& b)
{
    if (m_tasks.size()) {
        Task* t = m_tasks.back();
        t->param(name, b);
    }
}

void Coroutine::nowait()
{
    if (m_tasks.size()) {
        Task* t = m_tasks.back();
        t->nowait();
    }
}

int32_t Coroutine::addTask(Task* task, const UniqueId& id)
{
    int32_t result = TERMINAL_LABEL;
    if (!task) {
        return result;
    }

    result             = static_cast<int32_t>(m_tasks.size());
    Task* previousTask = m_tasks.empty() ? nullptr : m_tasks.back();
    task->m_id         = id;
    /// \note By default the previous task will point at this task if it's
    /// pointing at end
    if (previousTask && previousTask->successTarget() == TERMINAL_LABEL) {
        previousTask->setSuccessTarget(result);
    }

    m_tasks.push_back(task);
    return result;
}

int32_t Coroutine::addActionTask(std::string_view actionName, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(ActionTask, actionName), id);
    return result;
}

int32_t Coroutine::addCondTask(std::function<bool(const BeliefContext &)> func, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(ConditionalTask, func), id);
    return result;
}

int32_t Coroutine::addDropTask(const GoalHandle& handle, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(DropTask, handle), id);
    return result;
}

int32_t Coroutine::addGoalTask(std::string_view goalName, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(PursueTask, goalName), id);
    return result;
}

int32_t Coroutine::addYieldTask(std::function<bool(const BeliefContext &)> func, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(YieldUntilTask, func), id);
    return result;
}

int32_t Coroutine::addSleepTask(int milliseconds, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(SleepTask, milliseconds), id);
    return result;
}

int32_t Coroutine::addPrintTask(std::string_view message, const UniqueId& id)
{
    int32_t result = addTask(JACK_NEW(PrintTask, message), id);
    return result;
}

void Coroutine::setSuccessEdge(int32_t sourceLabel, int32_t targetLabel)
{
    /*! ***************************************************************************************
    * 1. Validate label indicies
    * 2. Set the fail edge from the source task to the target
    * ****************************************************************************************/

    // 1. Validate label indicies
    assert((targetLabel >= 0 && (size_t)targetLabel < m_tasks.size()) || targetLabel == Coroutine::TERMINAL_LABEL);
    assert((sourceLabel >= 0 && (size_t)sourceLabel < m_tasks.size()) || sourceLabel == Coroutine::TERMINAL_LABEL);

    // set the success edge from the source task to the target
    Task* t = m_tasks[sourceLabel];
    t->setSuccessTarget(targetLabel);
}

void Coroutine::setFailEdge(int32_t sourceLabel, int32_t targetLabel)
{
    /*! ***************************************************************************************
    * 1. Validate label indicies
    * 2. Set the fail edge from the source task to the target
    * ****************************************************************************************/

    // 1. Validate label indicies
    assert((targetLabel >= 0 && (size_t)targetLabel < m_tasks.size()) || targetLabel == Coroutine::TERMINAL_LABEL);
    assert((sourceLabel >= 0 && (size_t)sourceLabel < m_tasks.size()) || sourceLabel == Coroutine::TERMINAL_LABEL);   // The source can't be a terminal

    // 2. Set the fail edge from the source task to the target
    Task* t = m_tasks[sourceLabel];
    t->setFailTarget(targetLabel);
}

bool Coroutine::onTaskComplete(const UniqueId &taskId, bool success)
{
    bool result = false;
    Task *task = currentTask();
    if (task && task->id() == taskId) {
        if (success) {
            task->succeed();
        } else {
            task->fail();
        }
        result = true;
    }
    return result;
}

bool Coroutine::isCurrentTaskId(const UniqueId &id) const
{
    const Task *task = currentTask();
    if (!task) {
        return false;
    }
    return task->id() == id;
}

}} // namespace aos::jack

