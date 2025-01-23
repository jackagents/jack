#ifndef JACK_COROUTINE_H
#define JACK_COROUTINE_H

/// Local
#include <jack/corelib.h>
#include <jack/beliefcontext.h>

/// Third Party
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <string_view>

namespace aos::jack
{

class Agent;
class Task;
class Plan;
struct GoalHandle;

/******************************************************************************
 * \class   Coroutine
 * JACK's core coroutine type. Used to perform asyncronous task execution
 ******************************************************************************/
class Coroutine
{
public:
    Coroutine();
    Coroutine(const Coroutine &other);

    virtual ~Coroutine();

    /// reset this coroutine
    void reset(Agent* agent, Plan *intention);

    /// @return if the coroutine has finished
    bool finished();

    /// @return if the coroutine succeeded
    bool succeeded() const { return m_succeeded; }

    /// @return if the coroutine succeeded
    bool waiting() const;

    /// Mark an async task as being done
    /// \todo we will have to specify exactly which goal is done
    /// otherwise it won't be explainable
    void markAsyncDone();

    /// Tick the coroutine
    /// @return if the coroutine has finished
    bool tick(BeliefContext& context,
              const GoalHandle& desire,
              const UniqueId& intentionId,
              std::string_view plan);

    // Mark a task as having completed successfully or failed
    bool onTaskComplete(const UniqueId& taskId, bool success);

    /// @return True if the given ID matches the current task's ID
    bool isCurrentTaskId(const UniqueId& id) const;

    static const int32_t TERMINAL_LABEL = -1; /// The coroutine terminating task label

    /// @return List of tasks in this coroutine
    const std::vector<Task*> &tasks() const { return m_tasks; }

protected:
    /**************************************************************************
     * Protected Functions
     **************************************************************************/
    void addParamFrom(std::string_view param, std::string_view contextParamFrom);

    void addParam(std::string_view param);

    void addLiteralParam(std::string_view name, const std::any& b);

    /// Mark the most recently added task as being executed asynchronously
    void nowait();

    /// Add an action as the next task to execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addActionTask(std::string_view actionName, const UniqueId& id = UniqueId::random());

    /// Add a conditional belief query as the next task to execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addCondTask(std::function<bool(const BeliefContext &)> func, const UniqueId& id = UniqueId::random());

    /// Add a force drop request on the specified goal as the next task to
    /// execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addDropTask(const GoalHandle& goalid, const UniqueId& id = UniqueId::random());

    /// Add a sub-goal as the next task to execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addGoalTask(std::string_view goalName, const UniqueId& id = UniqueId::random());

    /// Add a print message as the next task to execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addPrintTask(std::string_view message, const UniqueId& id = UniqueId::random());

    /// Add a sleep in milliseconds as the next task to execute in the coroutine
    /// @return An integer label that is associated with this task
    int32_t addSleepTask(int milliseconds, const UniqueId& id = UniqueId::random());

    /// Add a yield from coroutine execution as the next task to execute in the
    /// coroutine
    /// @return An integer label that is associated with this task
    int32_t addYieldTask(std::function<bool(const BeliefContext &)> func, const UniqueId& id = UniqueId::random());

    /// Set the next to task to execute when the source task succeeds
    void setSuccessEdge(int32_t sourceLabel, int32_t targetLabel);

    /// Set the next to task to execute when the source task fails
    void setFailEdge(int32_t sourceLabel, int32_t targetLabel);

    /**************************************************************************
     * Protected Members
     **************************************************************************/

    std::vector<Task*> m_tasks;
    size_t m_current;
    bool m_succeeded;

    /// The number of async tasks to wait for
    int32_t m_numAsyncToWaitFor;

private:
    Task *currentTask() { return m_current < m_tasks.size() ? m_tasks[m_current] : nullptr; }

    const Task *currentTask() const { return m_current < m_tasks.size() ? m_tasks[m_current] : nullptr; }
    friend class CoroutineBuilder;

private:
    /// Add a new task to this coroutine to be executed after the most recently
    /// added task
    int32_t addTask(Task *task, const UniqueId& id = UniqueId::random());

};
typedef Coroutine* CoroutinePtr;
} // namespace aos::jack
#endif // JACK_COROUTINE_H
