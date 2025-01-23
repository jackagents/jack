#include <jack/event-protocol/protocolhelpers.h>
#include <jack/messageschema.h>
#include <jack/engine.h>
#include <nlohmann/json.hpp>
#include <string.h>

namespace aos::jack
{
#if 0 // I've disabled all this for now - not sure it's needed

Field jackFieldToProtocolField(const jack::Field& field)
{
    ZoneScoped;
    const FieldRegistry& registry = FieldRegistry::getInstance();
    Field result = {};

    const FieldFactoryBase* factory = registry.queryTypeByVariant(field.m_value);
    if (factory) {
        /// @todo: This recursive converts the field, we should use a stack for safety.
        /// @todo: We should accept pointers for performance, probably.
        if (factory->m_customMessageAnyToMessageProc) {
            jack::Message msg = *factory->m_customMessageAnyToMessageProc(field.m_value);

            //Are there const opportunities here?? Don't understand what is going on here
            //some recursive magic ??
            for (auto& it : msg.fields()) {
                Field& subField = it.second;
                subField = jackFieldToProtocolField(subField);
            }

            result.m_type  = protocol::anyTypePropertyString(protocol::AnyType_Message, /*array*/ false);
            result.m_value = msg;
        } else if (factory->m_customMessageAnyArrayToMessageProc) {
            std::vector<const Message*> array = factory->m_customMessageAnyArrayToMessageProc(field.m_value);
            std::vector<Message> copy;
            copy.reserve(array.size());
            for (const Message* item : array) {
                Message itemCopy = *item;
                for (auto& it : itemCopy.fields()) {
                    Field& subField = it.second;
                    subField        = jackFieldToProtocolField(subField);
                }
                copy.push_back(itemCopy);
            }
            result.m_type  = protocol::anyTypePropertyString(protocol::AnyType_Message, /*array*/ true);
            result.m_value = copy;
        } else {
            result = field;
        }
    } else {
        JACK_WARNING(
            "Cannot convert field for the bus protocol because the field is a "
            "type that was not registered to the engine [field={}]", field.toString());
    }

    return result;
}

std::vector<Field> jackMessageToProtocolFieldVector(const Message* src)
{
    std::vector<Field> result = {};
    if (!src) {
        return result;
    }

    const std::map<std::string, Field, std::less<>>& fields = src->fields();
    result.reserve(fields.size());
    for (const auto& it : fields) {
        result.push_back(jackFieldToProtocolField(it.second));
    }
    return result;
}

bool protocolFieldVectorToJACKMessage(const std::vector<Field>& src, Message* msg)
{
    bool result = msg;
    for (auto it = src.begin(); result && it != src.end(); it++) {
        result = msg->setField(*it, Message::SetFlags_ALL);
    }
    return result;
}

#endif

}  // namespace aos::jack
