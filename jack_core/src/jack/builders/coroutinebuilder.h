// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_COROUTINE_BUILDER_H
#define JACK_COROUTINE_BUILDER_H

#include <jack/builders/builder.h> // for Builder
#include <jack/coroutine.h>        // for Coroutine, CoroutinePtr

#include <functional>
#include <string>
#include <string_view>

namespace aos::jack
{
class BeliefContext;
class Engine;

/******************************************************************************
 * @brief A helper class for creating coroutines that are executed for a plan
 ******************************************************************************/
class CoroutineBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// @param engine The engine instance
    CoroutineBuilder(Engine& engine);

    /// @param other The other coroutine builder to copy
    CoroutineBuilder(const CoroutineBuilder &other);

    CoroutineBuilder& operator=(const CoroutineBuilder &other) = delete;

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Queue an action to the coroutine task list
    /// @param name The name of the action to add to the coroutine
    CoroutineBuilder& action(std::string_view name, const UniqueId& id = UniqueId::random());

    /// Add a parameter to the most recent task in the coroutine that gets
    /// it's value assigned from the parameter named 'fromContextParam' from
    /// the BeliefContext of the goal (typically used for configuring actions or
    /// pursue tasks).
    /// @param param The name of the parameter to add
    /// @param contextParamName The name of the parameter to pull from the beliefcontext.
    CoroutineBuilder& paramFrom(std::string_view param, std::string_view contextParamName);

    /// Add a parameter with a default value to the most recent task in the
    /// coroutine that gets it's value assigned from the same named parameter
    /// from the BeliefContext of the goal (typically used for configuring
    /// actions or puruse tasks).
    /// @param param The name of the parameter to add
    /// @param value The value to associate with the value
    template<typename T>
    CoroutineBuilder& param(std::string_view param, const T& value)
    {
        m_coroutine.addLiteralParam(param, std::any(value));
        return *this;
    }

    /// Queue a sleep during execution to the coroutine task list
    /// @param milliseconds The duration to sleep in milliseconds
    CoroutineBuilder& sleep(int32_t milliseconds, const UniqueId& id = UniqueId::random());

    /// Queue a message print to the coroutine task list
    /// @param message The message to print
    CoroutineBuilder& print(std::string_view message, const UniqueId& id = UniqueId::random());

    /// Queue a pursuit of a goal to the coroutine task list
    /// @param goal The goal to pursue
    CoroutineBuilder& goal(std::string_view goal, const UniqueId& id = UniqueId::random());

    /// Queue a drop of a goal to the coroutine task list
    /// @param handle[in] The handle of the goal to drop
    CoroutineBuilder& drop(const GoalHandle& handle, const UniqueId& id = UniqueId::random());

    /// Queue a conditional to the coroutine task list. lets the coroutine
    /// branch to a true or false target for the given function
    /// @param func The function to perform the conditional query
    CoroutineBuilder& cond(std::function<bool(const BeliefContext &)> func, const UniqueId& id = UniqueId::random());

    /// Apply 'nowait' to the most recent task, undefined behaviour if the
    /// coroutine is empty. A 'nowait' causes the most recent task to execute
    /// immediately.
    CoroutineBuilder& nowait();

    /// Queue a yield to the coroutine task list. Yielding lets the coroutine
    /// wait for the given function to become true before proceeding
    /// @param func The function to wait on before the coroutine can proceed
    CoroutineBuilder& yieldUntil(std::function<bool(const BeliefContext &)> func, const UniqueId& id = UniqueId::random());

    /// Reset the builder back to a labelled task
    /// @param targetLabel The task Label to reset the builder back to
    CoroutineBuilder& configure(int32_t targetLabel);

    // Set the branching destination for this current task when Successful
    // or True(for a conditional task)
    CoroutineBuilder& onSuccess(int32_t targetLabel);

    // Set the branching destination for this current task when Failed
    // or False(for a conditional task)
    CoroutineBuilder& onFail(int32_t label);

    // Get the label for the current task
    /// @param label The task Label to retrieve
    CoroutineBuilder& label(int32_t& label);

    /// Create an instance of the coroutine
    /// @return The coroutine as a pointer
    CoroutinePtr create() const;

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    Coroutine m_coroutine;
    int       m_currentLabel;
};
} /// namespace aos::jack
#endif // JACK_COROUTINE_BUILDER_H
