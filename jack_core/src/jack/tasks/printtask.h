#ifndef JACK_PRINTTASK_H
#define JACK_PRINTTASK_H

#include <jack/tasks/task.h>

/// Third Party
#include <string_view>
#include <string>
#include <fmt/core.h>

namespace aos::jack
{
class BeliefContext;
struct GoalHandle;
class UniqueId;
/******************************************************************************
 * \class   PrintTask
 * This task prints a message to the standard out stream
 ******************************************************************************/
class PrintTask : public ClonableTask<PrintTask>
{
public:
    PrintTask(const PrintTask& other) : m_message(other.m_message)
    {
        m_wait = other.m_wait;
        /// \todo extract the parameter markers
        /// \todo split message into substrings between the markers
        /// or format the message in a traditional printf style format string
        /// or boost::format
    }

    PrintTask(std::string_view message) : m_message(message) {}
    Task::State execute([[maybe_unused]] BeliefContext&    context,
                        [[maybe_unused]] const GoalHandle& goal,
                        [[maybe_unused]] const UniqueId&   intentionId,
                        [[maybe_unused]] std::string_view  plan) override final
    {
        fmt::println("{}", m_message);
        return Task::DONE;
    }

protected:
    std::string m_message;
};
} /// namespace aos::jack
#endif // JACK_PRINTTASK_H
