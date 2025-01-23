#ifndef JACK_ACTIONEVENT_H
#define JACK_ACTIONEVENT_H

#include <jack/events/event.h>

#include <jack/handles.h>
#include <jack/message.h>

#include <string>      // for string, char_traits, to_string, stod, stof, stoi
#include <vector>      // for vector
#include <string_view>

namespace aos::jack
{
struct Action;

/*! ***********************************************************************************************
 * @struct  ActionEvent
 *
 * An event to delegate an action
 * ************************************************************************************************/

// Action Message
struct ActionEvent : public Event
{
    /// Construct an action, if construction fails because of an invalid action
    /// passed in then 'valid' will return false on the returned object.
    ActionEvent(Engine&           engine,
                const Action&     action,
                const GoalHandle& goal,
                const UniqueId&   intentionId,
                std::string_view  plan,
                const UniqueId&   taskId);

    ActionEvent(std::string_view     action,
                const MessageSchema* request,
                const MessageSchema* reply,
                const GoalHandle&    goal,
                const UniqueId&      intentionId,
                std::string_view     plan,
                const UniqueId&      taskId);

    /**************************************************************************
     * Public Accessors & Mutators
     **************************************************************************/
    /// @return True if the event was constructed validly, false otherwise
    bool valid() const { return m_handle.valid(); }

    /// @return The name of the action
    std::string_view name() const { return m_handle.m_name; }

    /// Query a value from the parameters stored in this action
    /// @param str The name of the property to find
    /// @return The parameter value, if it's present, return a default value
    /// initialized value.
    template<typename T>
    T get(std::string_view &value) const { return m_request->get<T>(value); }

    template<typename T>
    T* getPtr(std::string_view &value) const { return m_request->getPtr<T>(value); }

    /// Get a lightweight reference to the action
    const ActionHandle &handle() const { return m_handle; }

    /// @todo Not sure if a reference is suitable here. Flesh this out when
    /// Services are implemented, how are messages sent, their lifetimes e.t.c
    /// for now as a reference, this mirrors the old action handlers keeping
    /// a pointer to the actual concrete action.
    /// @return The parameters of the action
    std::shared_ptr<Message> request() { return m_request; }

    /// @return ref to reply message
    std::shared_ptr<Message> reply() { return m_reply; }

private:
    /// Assign the parameters to the message. No validation is done on the
    /// parameters at this point.
    void finishInit(std::string_view     action,
                    const MessageSchema* request,
                    const MessageSchema* reply,
                    const GoalHandle&    goal,
                    const UniqueId&      intentionId,
                    std::string_view     plan,
                    const UniqueId&      taskId);

    /**************************************************************************
     * Member Fields
     **************************************************************************/
public:
    std::vector<std::string> m_resourceLocks; ///< Track the resource usage of the action
    GoalHandle               m_goal;          ///< Goal that triggered this task
    std::string              m_plan;          ///< Plan that the action was triggered from
    UniqueId                 m_intentionId;   ///< ID of the intention that triggered this task
    UniqueId                 m_taskId;        ///< ID of the task in intention's plan that triggered this task

protected:
    std::shared_ptr<Message> m_request; ///< Request parameters, parameters required for the execution of the action
    std::shared_ptr<Message> m_reply;   ///< Reply parameters, parameters generated on the completion of the action
    ActionHandle             m_handle;  ///< A lightweight reference to the action
};
} // namespace aos::jack
#endif // JACK_ACTIONEVENT_H
