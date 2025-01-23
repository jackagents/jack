#include <jack/beliefcontext.h>
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/resource.h>  // for Resource
#include <jack/message.h>

/// Third Party
#include <tracy/Tracy.hpp>

namespace aos::jack
{
BeliefContext::BeliefContext()
{
    m_messages = std::make_shared<MessageMap>();
    m_resources = std::make_shared<ResourceMap>();
}

/// \todo this doesn't perform a deep copy
BeliefContext::BeliefContext(const BeliefContext &other)
    : m_messages(other.m_messages)
    , m_resources(other.m_resources)
    , m_goal(other.m_goal)
{
}

BeliefContext *BeliefContext::clone(JACK_CALL_SITE_ARGS_NO_TAIL_COMMA) const
{
    ZoneScoped;
    if (m_agent.m_name.size()) {
        ZoneText(m_agent.m_name.c_str(), m_agent.m_name.size());
    }

    // 1. create the new belief context wrapper
    auto* context = globalHeapAllocator.newInternal<BeliefContext>(JACK_CALL_SITE_INPUT_NO_TAIL_COMMA);

    // 1.5 copy over the agent handle
    context->m_agent = m_agent;

    // 2. copy/clone over the beliefsets/message
    if (m_messages) {
        ZoneNamedN(debugTracyCloneMessages, "Clone messages", true);
        for (const auto& kv : *m_messages) {
            [[maybe_unused]] const std::string& name = kv.first;
            ZoneNamedN(debugTracyCloneMessageItem, "Clone message", true);
            ZoneNameV(debugTracyCloneMessageItem, name.c_str(), name.size());

            context->addMessage(kv.second->clone());
        }
    }

    // 3. copy/clone over the resources
    if (m_resources) {
        ZoneNamedN(debugTracyCloneMessages, "Clone resources", true);
        for (const auto& kv : *m_resources) {
            [[maybe_unused]] const std::string& name = kv.first;
            ZoneNamedN(debugTracyCloneResourceItem, "Clone resource", true);
            ZoneNameV(debugTracyCloneResourceItem, name.c_str(), name.size());

            Resource *rs = JACK_NEW(Resource, *kv.second);
            context->addResource(rs);
        }
    }

    // 4. copy/clone over the goal context if there is one
    if (m_goal) {
        [[maybe_unused]] const std::string& name = m_goal->schema();
        ZoneNamedN(debugTracyCloneGoalMessage, "Clone goal message", true);
        ZoneNameV(debugTracyCloneGoalMessage, name.c_str(), name.size());
        context->setGoalContext(m_goal->clone());
    }

    // 5. we don't copy over the action reply messages
    return context;
}

std::vector<std::string> BeliefContext::messageList() const
{
    std::vector<std::string> result;
    for (const auto& kv : *m_messages) {
        result.push_back(kv.first);
    }
    return result;
}

/// @todo: @deprecate this, we have unified getMessage API now that
/// handles all cases. However this API returns a shared pointer which enforces
/// some pretty strong contracts with the code that uses this API so it needs
/// some careful consideration to remove.
std::shared_ptr<Message> BeliefContext::message(const std::string& name) const
{
    auto it = m_messages->find(name);
    if (it != m_messages->end()) {
        return it->second;
    } else {
        return {};
    }
}

void BeliefContext::addMessage(std::shared_ptr<Message> bs) {
    m_messages->operator[](bs->schema()) = bs;
}

void BeliefContext::addResource(Resource *resource)
{
    /// \todo attach the back pointer
    // resource->setContext(this);

    // take ownership of the resource
    m_resources->operator[](resource->name()) = JACK_INIT_SHARED_PTR(Resource, resource);
}

void BeliefContext::addActionReplyMessage(std::shared_ptr<Message> message)
{
    // ignore empty message
    if (!message) {
        return;
    }

    // does the message already exist
    for (auto itr = m_actionReplyMessages.begin(); itr < m_actionReplyMessages.end(); ++itr ) {

        if ((*itr)->schema() == message->schema()) {

            // delete the previous message
            itr = m_actionReplyMessages.erase(itr);
            break;
        }
    }

    /// new messages always go on the end
    /// @note maybe in the future this order won't matter and 
    /// we can just replace the existing pointer
    m_actionReplyMessages.push_back(message);
}

std::shared_ptr<Resource> BeliefContext::resource(std::string_view name) const
{
    auto it = m_resources->find(name);
    if (it != m_resources->end()) {
        return it->second;
    }
    else {
        return {};
    }
}

std::shared_ptr<Message> BeliefContext::getMessage(const MessageQuery& query, const SearchOrder& search) const
{
    for (size_t index = 0; index < search.size; index++) {
        SearchContext searchContext = search.order[index];
        switch (searchContext) {
            case SearchContext::ACTION_REPLY: {
                for (auto it = m_actionReplyMessages.rbegin(); it != m_actionReplyMessages.rend(); ++it) {
                    const std::shared_ptr<Message>& check = (*it);
                    if (query.m_schema) {
                        if (query.m_schema->verifyMessage(*check).success) {
                            return check;
                        }
                    } else {
                        if (check->schema() == query.m_name) {
                            return check;
                        }
                    }
                }
            } break;

            case SearchContext::GOAL: {
                if (!m_goal) {
                    break;
                }

                const std::shared_ptr<Message>& check = m_goal;
                if (query.m_schema) {
                    if (query.m_schema->verifyMessage(*check).success) {
                        return check;
                    }
                } else {
                    if (check->schema() == query.m_name) {
                        return check;
                    }
                }
            } break;

            case SearchContext::AGENT: {
                for (const auto& kv : *m_messages) {
                    const std::shared_ptr<Message>& check = kv.second;
                    if (query.m_schema) {
                        if (query.m_schema->verifyMessage(*check).success) {
                            return check;
                        }
                    } else {
                        if (kv.first == query.m_name) {
                            return check;
                        }
                    }
                }
            } break;

            case SearchContext::COUNT: {
                JACK_INVALID_CODE_PATH;
            } break;
        }
    }

    return {};
}

/// \todo Not sure what to call this method
Variant BeliefContext::get(const std::string& key, const SearchOrder& search) const
{
    ZoneScoped;
    for (size_t index = 0; index < search.size; index++) {
        SearchContext searchContext = search.order[index];
        switch (searchContext) {
            case SearchContext::ACTION_REPLY: {
                for (auto it = m_actionReplyMessages.rbegin(); it != m_actionReplyMessages.rend(); ++it) {

                    std::any field = (*it)->getField(key);
                    if (field.has_value()) {
                        return field;
                    }
                }
            } break;

            case SearchContext::GOAL: {
                if (!m_goal) {
                    break;
                }

                std::any field = m_goal->getField(key);
                if (field.has_value()) {
                    return field;
                }
            } break;

            case SearchContext::AGENT: {
                for (const auto& kv : *m_messages) {
                    std::any field = kv.second->getField(key);
                    if (field.has_value()) {
                        return field;
                    }
                }
            } break;

            case SearchContext::COUNT: {
            } break;
        }
    }
    return {};
}

void BeliefContext::setAgentContext(BeliefContext &context, const AgentHandle &agent)
{
    // point at the original
    m_agent     = agent;
    m_messages  = context.m_messages;
    m_resources = context.m_resources;
}

bool BeliefContext::hasResourceViolation(std::vector<std::string>* violatedResources) const
{
    bool result = false;
    if (m_resources) {
        for (const auto& kv : *m_resources) {
            if (!kv.second->isValid()) {
                result = true;  // Violated
                if (violatedResources) {
                    violatedResources->push_back(kv.first);
                } else {
                    // Early out if vector is not given, we just need to detect at-least one
                    // violation.
                    break;
                }
            }
        }
    }

    return result;
}

void BeliefContext::lockResources(const std::vector<std::string> &resources)
{
    for (std::string_view name : resources) {
        if (std::shared_ptr<jack::Resource> resourcePtr = resource(name)) {
            resourcePtr->lock();
        } else {
            /// \todo unknown resource - log warning
        }
    }
}

void BeliefContext::unlockResources(const std::vector<std::string> &resources)
{
    for (std::string_view name : resources) {
        if (std::shared_ptr<jack::Resource> resourcePtr = resource(name)) {
            resourcePtr->unlock();
        } else {
            /// \todo unknown resource - log warning
        }
    }
}


std::string BeliefContext::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.appendRef("BeliefContext{");
    for (auto it = m_messages->begin(); it != m_messages->end(); it++) {
        if (it != m_messages->begin()) {
            builder.appendRef(", ");
        }
        builder.appendCopy(it->second->toString());
    }

    if (m_goal) {
        builder.appendRef(", ");
        builder.appendCopy(m_goal->toString());
        builder.appendRef(", ");
    }

    builder.appendRef("}");
    std::string result = builder.toString();
    return result;
}

void BeliefContext::print() const
{
    fmt::println(toString());
}
} // namespace aos::jack
