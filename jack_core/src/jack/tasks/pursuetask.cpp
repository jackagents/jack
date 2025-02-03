// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/tasks/pursuetask.h>
#include <jack/agent.h>          // for Agent
#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/corelib.h>
#include <jack/plan.h>           // for Plan
#include <jack/engine.h>

#include <functional>       // for _Func_impl_no_alloc<>::_Mybase
#include <memory>           // for shared_ptr
#include <utility>          // for pair
#include <vector>           // for _Vector_iterator, vector, _Vector_const_i...

namespace aos::jack
{
Task::State PursueTask::execute(BeliefContext&                    context,
                                const GoalHandle&                 goal,
                                [[maybe_unused]] const UniqueId&  intentionId,
                                [[maybe_unused]] std::string_view plan)
{
    /*! ***************************************************************************************
     * Execute a pursue task
     *
     * 1. Pack parameters into the goal event
     * 2. Send the pursue goal event
     * 3. Hook up the callback to inform this task when the goal is done
     * ****************************************************************************************/

    /// \todo In end user applications, we currently accept beliefsets allocated
    /// from another allocator than the ones in the engine, which means we can't
    /// use any memory macros here since we will have a mix of parameters
    /// allocated from our memory system and parameters allocated by the
    /// external applications memory system.
    ///
    /// The engine in PursueEvent will treat this as an untracked allocation for
    /// now.
    const Goal* realGoal = m_agent->engine().getGoal(m_goal);
    if (!realGoal) {
        const IntentionExecutor *executor = m_agent->intentionBeingExecuted();
        JACK_WARNING("Sub-goal referenced in plan does not exist in the JACK node [agent={}, goal={}, plan={}]",
                     m_agent->handle().toString(),
                     goal.toString(),
                     executor->currentIntention()->name());
        fail();
        return Task::WAIT;
    }

    const MessageSchema* schema = m_agent->engine().getMessageSchema(realGoal->messageSchema());
    if (realGoal->messageSchema().size() && !schema) {
        const IntentionExecutor *executor = m_agent->intentionBeingExecuted();
        JACK_WARNING("Sub-goal referenced in plan requires a message that was not found in the JACK node [agent={}, goal={}, plan={}, message={}]",
                     m_agent->handle().toString(),
                     goal.toString(),
                     executor->currentIntention()->name(),
                     realGoal->messageSchema());
        fail();
        return Task::WAIT;
    }

    std::unique_ptr<Message> parameterPack;
    if (schema) {
        /// \note If the sub-goal message matches a message in the context then
        /// we can automatically map it into the sub-goal task.
        /// \todo If it doesn't we need to look into message remapping, right
        /// now we only remap on a per-parameter and same-name-message basis.
        auto msgQuery               = BeliefContext::MessageQuery(*schema);
        const auto messageToMap = context.getMessage(msgQuery);
        if (messageToMap) {
            parameterPack = messageToMap->clone();
        } else {
            parameterPack = schema->createMessage();
        }
    } else {
        /// \todo We should be able to not specify a message.
        if (m_literals.size() || m_parameters.size()) {
            JACK_WARNING("A subgoal was initiated with some starting parameters but this goal does not have a message schema associated with it to assign the parameters to [agent={}, goal={}]",
                         m_agent->handle().toString(),
                         goal.toString());
        }
    }

    /// Pull default/literal values into the message
    for (const auto &param : m_literals) {
        if(!parameterPack->setField(param.first, param.second)) {
            JACK_WARNING("Failed to set literal field from mapping [agent={}, goal={}, field={}]",
                                m_agent->handle().toString(),
                                goal.toString(),
                                param.first);
        }
    }

    /// Next pull parameters that exist from the context (this overwrites the
    /// default/literal values if they exist in the context)
    for (const ParameterToContextMapping &mapping : m_parameters) {
        /// Pull parameters from the environment in the following precedence
        const std::string &paramName        = mapping.paramName;
        const std::string &contextParamName = mapping.contextParamName.empty()
                                                    ? mapping.paramName
                                                    : mapping.contextParamName;

        /// Check the belief context, which checks, in order, points 1, 2, 3 as
        /// referred to earlier.
        Variant variant = context.get(contextParamName);

        if (variant.has_value()) {
            if (!parameterPack->setField(paramName, variant)) {
                JACK_WARNING("Failed to set pursue message field from mapping [agent={}, goal={}, field={}]",
                                m_agent->handle().toString(),
                                goal.toString(),
                                contextParamName);
            }
        }
    }

    // we are going to assume that we are being executed inside a valid agent and intention
    assert(m_agent);
    assert(m_intention);

    /// 2. Send the pursue sub goal event
    GoalPursue goalPursue = m_agent->pursueSub(m_goal,
                                               m_agent->intentionBeingExecuted()->id(),
                                               GoalPersistent_No,
                                               std::move(parameterPack) /*parameters*/,
                                               nullptr /*id*/,
                                               &m_id);
    m_promise = goalPursue.promise;

    /// 3.  Hook up the callback to inform this task when the goal is done
    m_promise->then([this]() {
        // sub goal succeeded
        succeed();
    },
    [this]() {
        // sub goal failed
        fail();
    });

    return m_wait ? Task::WAIT : Task::ASYNC;
}
} // namespace aos::jack
