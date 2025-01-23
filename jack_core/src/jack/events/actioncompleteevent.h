#ifndef JACK_ACTIONCOMPLETEEVENT_H
#define JACK_ACTIONCOMPLETEEVENT_H

#include <jack/events/event.h>
#include <jack/events/actionevent.h>

#include <string_view>
#include <string>

namespace aos { namespace jack {

/*! ***********************************************************************************************
 * \struct  ActionCompleteEvent
 *
 * An event to update the status of an action
 * ************************************************************************************************/

struct ActionCompleteEvent : public Event
{
    ActionCompleteEvent(std::string_view  name,
                        const UniqueId&   taskId,
                        Event::Status     status,
                        const GoalHandle& goal,
                        const UniqueId&   intentionId,
                        std::string_view  plan,
                        std::shared_ptr<Message> replyMsg)
        : Event(Event::ACTIONCOMPLETE),
          name(name),
          taskId(taskId),
          actionStatus(status),
          goal(goal),
          intentionId(intentionId),
          plan(plan),
          m_reply(replyMsg)
    {
    }

    /// @return message
    const std::shared_ptr<Message> &reply() const { return m_reply; }

    std::string              name;
    UniqueId                 taskId;
    Event::Status            actionStatus;
    GoalHandle               goal;
    UniqueId                 intentionId;
    std::string              plan;
    std::vector<std::string> resourceLocks;

protected:
    std::shared_ptr<Message> m_reply;
};

}} // namespace aos::jack

#endif // JACK_ACTIONCOMPLETEEVENT_H
