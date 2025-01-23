#ifndef JACK_GOAL_BUILDER_H
#define JACK_GOAL_BUILDER_H

#include <jack/beliefquery.h>
#include <jack/goal.h>
#include <jack/builders/builder.h>

#include <functional>
#include <vector>
#include <string>
#include <string_view>

namespace aos::jack
{
class BeliefContext;
class Engine;

/******************************************************************************
 * @class   GoalBuilder
 *
 * A goal builder class
 * Used to construct a goal within the jack core's declarative model
 ******************************************************************************/
class GoalBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    GoalBuilder(Engine& engine, std::string_view name);

    /// Copy constructor
    /// @param other The goal builder to copy from
    GoalBuilder(const GoalBuilder &other);

    GoalBuilder& operator=(const GoalBuilder &other) = delete;

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Define a goal's precondition
    /// @param func The function to execute to when checking for precondition
    GoalBuilder& pre(const std::function<bool(const BeliefContext&)> func);

    /// Define a goal's precondition
    GoalBuilder& pre(const std::string &query);

    /// Define a goal's satisfied condition
    /// @param func The function to execute to when checking for satisfaction
    GoalBuilder& satisfied(const std::function<bool(const BeliefContext&)> func);

    /// Define a goal's satisfied condition
    /// @param query
    GoalBuilder& satisfied(const std::string &query);

    /// Define a condition when the goal should drop itself
    GoalBuilder& dropWhen(const std::function<bool(const BeliefContext&)> func);

    /// Set the goal's template priority
    GoalBuilder& priority(int priority);

    /// @todo The goal needs to take a message schema. For now, JACK will look
    /// for a schema that has the same name as the goal name.
    /// Define the input message for the goal
    GoalBuilder& heuristic(const std::function<float(const BeliefContext&)> func);

    /// Set the message the goal must be performed/pursued with.
    GoalBuilder& message(std::string_view message);

     /// Commit this goal to the BDI engine model
    template <typename GoalT = Goal>
    GoalBuilder& commit()
    {
        GoalT item(m_name);
        commitInternal(&item);
        return *this;
    }

private:
    /// Assign the properties to the goal passed in and commit the goal to the
    /// engine.
     Goal* commitInternal(Goal* goal);

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    std::string                                m_messageSchema;
    BeliefQuery                                m_precondition;
    BeliefQuery                                m_satisfied;
    BeliefQuery                                m_maintain;
    BeliefQuery                                m_dropWhen;
    int                                        m_priority;
    std::function<float(const BeliefContext&)> m_heuristic;
};
} // namespace aos::jack
#endif
