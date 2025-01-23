#include <jack/tasks/actiontask.h>
#include <jack/engine.h>         // for Engine
#include <jack/plan.h>           // for Plan
#include <jack/agent.h>          // for Agent
#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/corelib.h>

#include <jack/events/actionevent.h>

#include <cassert>          // for assert
#include <memory>           // for shared_ptr
#include <utility>          // for pair
#include <vector>           // for _Vector_iterator, vector, _Vector_const_i...

namespace aos::jack
{
class MessageSchema;

ActionTask::ActionTask(std::string_view action)
    : m_action(action)
{
}

ActionTask::ActionTask(const ActionTask &other)
    : m_action(other.m_action)
{
    m_wait = other.m_wait;
}

Task::State ActionTask::execute(BeliefContext&    context,
                                const GoalHandle& goal,
                                const UniqueId&   intentionId,
                                std::string_view  plan)
{
    /*! ***************************************************************************************
    * Send the action message to the agent or service
    *
    * 1. Create an action message from the schema
    * 2. Pack parameters into the action request message
    * 2.1. Map any similar message type into the request
    * 2.2. Map all fields by name
    * 2.3. Map over literals values
    * 2.4. Remapping from the model
    * 3. Dispatch the action message
    * 4. Return false and await the result
    * ****************************************************************************************/

    /// \note Query the action
    jack::Engine& engine = m_agent->engine();
    const Action* action = engine.getAction(m_action);
    if (!action) {
        const IntentionExecutor* executor = m_agent->intentionBeingExecuted();
        JACK_WARNING("Action task failed to generate event because action is missing [agent={}, desire={}, plan='{}', action='{}']",
                     m_agent->handle().toString(),
                     executor->desireHandle().toString(),
                     executor->currentIntention()->name(),
                     m_action);
        fail();
        return Task::WAIT;
    }

    /// 1. Create an action message
    /// \note If the agent doesn't handle the action, this action gets forwaded
    /// to the engine
    ActionEvent *actionEvent = JACK_ALLOCATOR_NEW(&engine.m_eventAllocator,
                                                  ActionEvent,
                                                  engine,
                                                  *action,
                                                  goal,
                                                  intentionId,
                                                  plan,
                                                  m_id);
    actionEvent->caller      = m_agent;
    if (!actionEvent->valid()) {
        fail();
        JACK_CHUNK_ALLOCATOR_GIVE(&engine.m_eventAllocator, ActionEvent, actionEvent, JACK_ALLOCATOR_CLEAR_MEMORY);
        return Task::WAIT;
    }

    /// Push id so the action reply can come back to this task
    actionEvent->m_taskId = m_id;

    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder          builder = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("[agent={}, action={}, intention={}]"),
                   m_agent->handle().toString(),
                   m_action,
                   m_agent->intentionBeingExecuted()->id());

    actionEvent->m_taskId.setTag(builder.toString());

    /// \todo Eventually resource locks should be specified on an action level.
    /// This also means that we don't need to push this onto the bus because
    /// each node will know what resources they need to lock to run an action.
    ///
    /// It's also really weird that some JACK node is telling us what resources
    /// we need to lock ... so this is really just out of place.
    ///
    /// Until we associate the resource locks per action, we have to serialise
    /// this over the bus for now for remote actions.
    actionEvent->m_resourceLocks = m_intention->resourceLocks();

    /// 2. Pack parameters into the action request message
    /// Pull parameters requested by the task from the context into the task.

    /// Setup the request message
    auto request = actionEvent->request();
    if (request && request->schema().size()) {

        const MessageSchema* schema = engine.getMessageSchema(request->schema());
        assert(schema && "The schema must exist otherwise we should have early exited by checking schema.valid() before");

        auto& fields = schema->fields();

        /// 2.1 map any similar message type

        /// \note If the action request message matches a message in the context
        /// then we can automatically map it into action.
        /// \todo If it doesn't we need to look into message remapping, right
        /// now we only remap on a per-parameter and same-name-message basis.
        auto msgQuery               = BeliefContext::MessageQuery(*schema);
        const auto messageToMap = context.getMessage(msgQuery);
        if (messageToMap) {
            //request = messageToMap->clone();
            /// copy over
            *request = *messageToMap;
        }

        /// 2.2 Map all fields by name

        /// \todo Setup the request message by mapping in the fields specified in
        /// the schema from the context. We need to figure out whether or not we
        /// want to just map in whole messages instead of per-parameter, and allow
        /// remapping of parameters or remapping of messages. I suspect the latter.
        ///
        /// However for the bare minimum to get this working, I will map in
        /// parameters.
        for (auto f : fields) {
            std::any data          = context.get(f.m_name);
            if (data.has_value()) {
                if (!request->setField(f.m_name, data)) {
                    JACK_WARNING("Failed to set action message field from mapping [agent={}, action={}, field={}]",
                                m_agent->handle().toString(),
                                m_action,
                                f.toString());
                }
            }
        }

        /// 2.3 Map over literals values
        for (const auto &param : m_literals) {
            if (!actionEvent->request()->setField(param.first, param.second)) {
                JACK_WARNING("Failed to set the default value for a field in the action request "
                            "message, type mismatch or value is referencing a non-existent schema "
                            "field [agent={}, action={}, field={}]",
                            m_agent->handle().toString(),
                            m_action,
                            param.first);
            }
        }

        /// 2.4 Remapping from the model

            /// \todo This is now probably deprecated, unless, there's a desire for
        /// per-parameter remapping.
        ///
        /// With message unification, we probably only want to remap on a per
        /// message basis no parameter.
        ///
        /// \note Next pull parameters that exist from the context (this overwrites
        /// the default/literal values if they exist in the context)
        for (const ParameterToContextMapping &mapping : m_parameters) {
            /// \todo For now the parameters are just belief keys
            /// Pull parameters from the environment in the following precedence
            ///   1. Check the plan context
            ///   2. Check the goal context
            ///   3. Check the global/agent context
            const std::string &paramName        = mapping.paramName;
            const std::string &contextParamName = mapping.contextParamName.empty()
                                                    ? mapping.paramName
                                                    : mapping.contextParamName;

            /// Check the belief context, which checks, in order, points 1, 2, 3 as
            /// referred to earlier.
            Variant variant = context.get(contextParamName);

            if (variant.has_value()) {
                if (!actionEvent->request()->setField(paramName, variant)) {
                    JACK_WARNING("Failed to set action message field from mapping [agent={}, action={}, field={}]",
                                m_agent->handle().toString(),
                                m_action,
                                contextParamName);
                }
            }
        }
    }    

    /// 3. Dispatch the action message
    m_agent->addEvent(actionEvent);

    /// 4. Return false and await the action message reply
    return m_wait ? Task::WAIT : Task::ASYNC;
}
} // namespace aos::jack
