// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

namespace aos
{
inline jack::Engine::PollResult enginePollAndRecordSchedules(
    jack::Engine&             bdi,
    size_t                    pollCount,
    std::chrono::milliseconds pollDuration = jack::Engine::POLL_AUTO_DELTA_TIME)
{
    jack::Engine::PollResult result = {};
    if (pollCount == -1) {
        for (;;) {
            result = bdi.poll(pollDuration);
            if (result.agentsRunning <= 0 && result.agentsExecuting <= 0) {
                break;
            }
        }
    } else {
        for (size_t pollIndex = 0; pollIndex < pollCount; pollIndex++) {
            result = bdi.poll(pollDuration);
        }
    }

    return result;
}

inline jack::Engine::PollResult enginePollUntilDone(
    jack::Engine&             bdi,
    std::chrono::milliseconds pollDuration = jack::Engine::POLL_AUTO_DELTA_TIME)
{
    jack::Engine::PollResult result = {};
    for (;;) {
        result = bdi.poll(pollDuration);
        if (result.agentsRunning <= 0 && result.agentsExecuting <= 0) {
            break;
        }
    }

    return result;
}


}; /// namespace aos

