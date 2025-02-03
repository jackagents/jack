// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/events/actionevent.h>
#include <jack/engine.h>

namespace aos::jack
{

ActionEvent::ActionEvent(Engine&           engine,
                         const Action&     action,
                         const GoalHandle& goal,
                         const UniqueId&   intentionId,
                         std::string_view  plan,
                         const UniqueId&   taskId)
    : Event(Event::ACTION)
{
    /// \note Validate the given action
    if (action.name.empty()) {
        JACK_WARNING_MSG("Action event created with an empty name, created event is invalid");
        return;
    }

    ActionMessageSchemas actionSchemas = engine.getActionMessageSchema(&action);
    if (actionSchemas.invalidFlags) {
        ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
        StringBuilder builder          = StringBuilder(scratch.arena);
        builder.append(FMT_STRING("Action requires the following message(s) that does not exist in the engine. [name={}"), action.name);
        if (actionSchemas.invalidFlags & ActionMessageSchemasFlag_MISSING_REQUEST) {
            builder.append(FMT_STRING(", requestMsg={}"), action.request);
        }
        if (actionSchemas.invalidFlags & ActionMessageSchemasFlag_MISSING_REPLY) {
            builder.append(FMT_STRING(", replyMsg={}"), action.reply);
        }
        builder.append(FMT_STRING("]"));

        std::string_view line = builder.toStringArena(scratch.arena);
        JACK_WARNING("{}", line);
        return;
    }

    finishInit(action.name, actionSchemas.request, actionSchemas.reply, goal, intentionId, plan, taskId);
}

ActionEvent::ActionEvent(std::string_view     action,
                         const MessageSchema* request,
                         const MessageSchema* reply,
                         const GoalHandle&    goal,
                         const UniqueId&      intentionId,
                         std::string_view     plan,
                         const UniqueId&      taskId)
    : Event(Event::ACTION)
{
    if (action.empty()) {
        JACK_WARNING_MSG("Action event created with an empty name, created event is invalid");
        return;
    }

    /// \todo We should validate the action against the action in the engine
    finishInit(action, request, reply, goal, intentionId, plan, taskId);
}

void ActionEvent::finishInit(std::string_view     action,
                             const MessageSchema* request,
                             const MessageSchema* reply,
                             const GoalHandle&    goal,
                             const UniqueId&      intentionId,
                             std::string_view     plan,
                             const UniqueId&      taskId)
{
    if (request) {
        m_request = std::shared_ptr<Message>(request->createMessage());
    }

    if (reply) {
        m_reply = std::shared_ptr<Message>(reply->createMessage());
    }

    /// \note If we reach this branch, the action was constructed validly, set
    /// the handle ensuring that the event returns true for validity checks
    m_handle      = {std::string(action), eventId};
    m_goal        = goal;
    m_intentionId = intentionId;
    m_plan        = plan;
    m_taskId      = taskId;
}
} // namespace aos::jack
