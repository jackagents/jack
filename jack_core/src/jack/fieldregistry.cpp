// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/// JACK
#include <jack/event-protocol/protocol.h>
#include <jack/fieldregistry.h>
#include <jack/message.h>
#include <jack/utils.h>

/// Third Party
#include <nlohmann/json.hpp>

namespace aos::jack
{
/******************************************************************************
 * Field
 ******************************************************************************/
void to_json(nlohmann::json& j, const Field& f)
{
    j["name"]  = f.m_name;
    j["type"]  = f.m_type;
    j["value"] = jackFieldToJSON(f);
}

void from_json(const nlohmann::json& j, Field& f)
{
    j.at("name").get_to(f.m_name);
    j.at("type").get_to(f.m_type);
    j.at("value").get_to(f.m_value);
}

std::string Field::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("Field{{name={}, type={}, value={}"), m_name, m_type, m_value.type().name());

   if      (m_value.type() == typeid(int8_t))      { builder.append(FMT_STRING("{}"), std::any_cast<int8_t>(m_value)); }
   else if (m_value.type() == typeid(int16_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int16_t>(m_value)); }
   else if (m_value.type() == typeid(int32_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int32_t>(m_value)); }
   else if (m_value.type() == typeid(int64_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int64_t>(m_value)); }
   else if (m_value.type() == typeid(uint8_t))     { builder.append(FMT_STRING("{}"), std::any_cast<uint8_t>(m_value)); }
   else if (m_value.type() == typeid(uint16_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint16_t>(m_value)); }
   else if (m_value.type() == typeid(uint32_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint32_t>(m_value)); }
   else if (m_value.type() == typeid(uint64_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint64_t>(m_value)); }
   else if (m_value.type() == typeid(float))       { builder.append(FMT_STRING("{}"), std::any_cast<float>(m_value)); }
   else if (m_value.type() == typeid(double))      { builder.append(FMT_STRING("{}"), std::any_cast<double>(m_value)); }
   else if (m_value.type() == typeid(bool))        { builder.append(FMT_STRING("{}"), std::any_cast<bool>(m_value)); }
   //else if (m_value.type() == typeid(Message))     { builder.append(FMT_STRING("{}"), std::any_cast<Message>(m_value).toString()); }
   else if (m_value.type() == typeid(std::string)) {
       const std::string* value = std::any_cast<std::string>(&m_value);
       builder.append(FMT_STRING("'{}'"), *value);
   } else if (m_value.type() == typeid(V2)) {
       const V2* value = std::any_cast<V2>(&m_value);
       builder.appendCopy(value->toString());
   }

   builder.appendRef("}");
   std::string result = builder.toString();
   return result;
}

std::string Field::toHumanString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder builder          = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("{}: {} = "), m_name, m_type);

    if      (m_value.type() == typeid(int8_t))      { builder.append(FMT_STRING("{}"), std::any_cast<int8_t>(m_value)); }
    else if (m_value.type() == typeid(int16_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int16_t>(m_value)); }
    else if (m_value.type() == typeid(int32_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int32_t>(m_value)); }
    else if (m_value.type() == typeid(int64_t))     { builder.append(FMT_STRING("{}"), std::any_cast<int64_t>(m_value)); }
    else if (m_value.type() == typeid(uint8_t))     { builder.append(FMT_STRING("{}"), std::any_cast<uint8_t>(m_value)); }
    else if (m_value.type() == typeid(uint16_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint16_t>(m_value)); }
    else if (m_value.type() == typeid(uint32_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint32_t>(m_value)); }
    else if (m_value.type() == typeid(uint64_t))    { builder.append(FMT_STRING("{}"), std::any_cast<uint64_t>(m_value)); }
    else if (m_value.type() == typeid(float))       { builder.append(FMT_STRING("{}"), std::any_cast<float>(m_value)); }
    else if (m_value.type() == typeid(double))      { builder.append(FMT_STRING("{}"), std::any_cast<double>(m_value)); }
    else if (m_value.type() == typeid(bool))        { builder.append(FMT_STRING("{}"), std::any_cast<bool>(m_value)); }
    //else if (m_value.type() == typeid(Message))     { builder.append(FMT_STRING("{}"), std::any_cast<Message>(m_value).toString()); }
    else if (m_value.type() == typeid(std::string)) {
       const std::string* value = std::any_cast<std::string>(&m_value);
       builder.append(FMT_STRING("'{}'"), *value);
    } else if (m_value.type() == typeid(V2)) {
       const V2* value = std::any_cast<V2>(&m_value);
       builder.appendCopy(value->toString());
    }

    std::string result = builder.toString();
    return result;
}

std::string format_as(const Field& field)
{
    std::string result = field.toString();
    return result;
}

/******************************************************************************
 * FieldFactory
 ******************************************************************************/
Field FieldRegistry::createField(const std::string& name, const std::string&
type) const
{
    Field result = {};
    auto  it     = m_factory.find(type);
    if (it != m_factory.end()) {
        result = it->second->create(name);
    } else {
        JACK_WARNING("Cannot create field, type does not exist [name={}, type={}]", name, type);
    }
    return result;
}

const FieldFactoryBase* FieldRegistry::queryTypeByString(std::string_view type) const
{
    const FieldFactoryBase *result = nullptr;
    auto it = m_factory.find(type);
    if (it != m_factory.end()) {
        result = it->second;
    }
    return result;
}

const FieldFactoryBase* FieldRegistry::queryTypeByVariant(const std::any& variant) const
{
    const FieldFactoryBase *result = nullptr;
    for (auto it : m_factory) {
        const FieldFactoryBase* factory = it.second;
        if (factory->type() == variant.type()) {
            result = factory;
            break;
        }
    }
    return result;
}

/******************************************************************************
 * FieldRegistry
 ******************************************************************************/
// Meyers Singleton
// https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton/
FieldRegistry& FieldRegistry::getInstance()
{
    static FieldRegistry result = {};
    if (!result.m_init) { /// \note Register the basic built types
        result.registerType<int8_t>                  (protocol::anyTypePropertyString(protocol::AnyType_I8,      /*array*/ false));
        result.registerType<int16_t>                 (protocol::anyTypePropertyString(protocol::AnyType_I16,     /*array*/ false));
        result.registerType<int32_t>                 (protocol::anyTypePropertyString(protocol::AnyType_I32,     /*array*/ false));
        result.registerType<int64_t>                 (protocol::anyTypePropertyString(protocol::AnyType_I64,     /*array*/ false));
        result.registerType<uint8_t>                 (protocol::anyTypePropertyString(protocol::AnyType_U8,      /*array*/ false));
        result.registerType<uint16_t>                (protocol::anyTypePropertyString(protocol::AnyType_U16,     /*array*/ false));
        result.registerType<uint32_t>                (protocol::anyTypePropertyString(protocol::AnyType_U32,     /*array*/ false));
        result.registerType<uint64_t>                (protocol::anyTypePropertyString(protocol::AnyType_U64,     /*array*/ false));
        result.registerType<float>                   (protocol::anyTypePropertyString(protocol::AnyType_F32,     /*array*/ false));
        result.registerType<double>                  (protocol::anyTypePropertyString(protocol::AnyType_F64,     /*array*/ false));
        result.registerType<bool>                    (protocol::anyTypePropertyString(protocol::AnyType_Bool,    /*array*/ false));
        result.registerType<std::string>             (protocol::anyTypePropertyString(protocol::AnyType_String,  /*array*/ false));
        result.registerType<V2>                      (protocol::anyTypePropertyString(protocol::AnyType_V2,      /*array*/ false));
        //result.registerType<Message>                 (protocol::anyTypePropertyString(protocol::AnyType_Message, /*array*/ false));
        result.registerType<protocol::BDILogLevel>   ("BDILogLevel");

        result.registerType<std::vector<int8_t>>     (protocol::anyTypePropertyString(protocol::AnyType_I8,      /*array*/ true));
        result.registerType<std::vector<int16_t>>    (protocol::anyTypePropertyString(protocol::AnyType_I16,     /*array*/ true));
        result.registerType<std::vector<int32_t>>    (protocol::anyTypePropertyString(protocol::AnyType_I32,     /*array*/ true));
        result.registerType<std::vector<int64_t>>    (protocol::anyTypePropertyString(protocol::AnyType_I64,     /*array*/ true));
        result.registerType<std::vector<uint8_t>>    (protocol::anyTypePropertyString(protocol::AnyType_U8,      /*array*/ true));
        result.registerType<std::vector<uint16_t>>   (protocol::anyTypePropertyString(protocol::AnyType_U16,     /*array*/ true));
        result.registerType<std::vector<uint32_t>>   (protocol::anyTypePropertyString(protocol::AnyType_U32,     /*array*/ true));
        result.registerType<std::vector<uint64_t>>   (protocol::anyTypePropertyString(protocol::AnyType_U64,     /*array*/ true));
        result.registerType<std::vector<float>>      (protocol::anyTypePropertyString(protocol::AnyType_F32,     /*array*/ true));
        result.registerType<std::vector<double>>     (protocol::anyTypePropertyString(protocol::AnyType_F64,     /*array*/ true));
        result.registerType<std::vector<bool>>       (protocol::anyTypePropertyString(protocol::AnyType_Bool,    /*array*/ true));
        result.registerType<std::vector<std::string>>(protocol::anyTypePropertyString(protocol::AnyType_String,  /*array*/ true));
        result.registerType<std::vector<V2>>         (protocol::anyTypePropertyString(protocol::AnyType_V2,      /*array*/ true));
        //result.registerType<std::vector<Message>>    (protocol::anyTypePropertyString(protocol::AnyType_Message, /*array*/ true));
        result.m_init = true;
    }
    return result;
}

/******************************************************************************
 * Functions
 ******************************************************************************/
/// \note Forward declaration for recrusive messages embedded in fields
nlohmann::json jackMessageToJSON(const Message& msg);
nlohmann::json jackFieldToJSON(const jack::Field& field)
{
    ZoneScoped;
    const FieldRegistry& registry = FieldRegistry::getInstance();
    nlohmann::json result;
    try {
        if (field.m_value.type() == typeid(int8_t)) {
            result = std::any_cast<int8_t>(field.m_value);
        } else if (field.m_value.type() == typeid(int16_t)) {
            result = std::any_cast<int16_t>(field.m_value);
        } else if (field.m_value.type() == typeid(int32_t)) {
            result = std::any_cast<int32_t>(field.m_value);
        } else if (field.m_value.type() == typeid(int64_t)) {
            result = std::any_cast<int64_t>(field.m_value);
        } else if (field.m_value.type() == typeid(uint8_t)) {
            result = std::any_cast<uint8_t>(field.m_value);
        } else if (field.m_value.type() == typeid(uint16_t)) {
            result = std::any_cast<uint16_t>(field.m_value);
        } else if (field.m_value.type() == typeid(uint32_t)) {
            result = std::any_cast<uint32_t>(field.m_value);
        } else if (field.m_value.type() == typeid(uint64_t)) {
            result = std::any_cast<uint64_t>(field.m_value);
        } else if (field.m_value.type() == typeid(float)) {
            result = std::any_cast<float>(field.m_value);
        } else if (field.m_value.type() == typeid(double)) {
            result = std::any_cast<double>(field.m_value);
        } else if (field.m_value.type() == typeid(aos::jack::V2)) {
            auto v2              = std::any_cast<aos::jack::V2>(field.m_value);
            result = {
                {"m_x", v2.m_x},
                {"m_y", v2.m_y},
            };
        } else if (field.m_value.type() == typeid(bool)) {
            result = std::any_cast<bool>(field.m_value);
        } else if (field.m_value.type() == typeid(std::string)) {
            result = std::any_cast<std::string>(field.m_value);
        } else if (field.m_value.type() == typeid(jack::Message)) {
 //@todo do we even need this?          result = jackMessageToJSON(std::any_cast<Message>(field.m_value));
        } 
        /*
         * Handle Arrays of basic types
         */
        else if (field.m_value.type() == typeid(std::vector<int8_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<int8_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<int16_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<int16_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<int32_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<int32_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<int64_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<int64_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<uint8_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<uint8_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<uint16_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<uint16_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<uint32_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<uint32_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<uint64_t>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<uint64_t>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<float>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<float>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<double>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<double>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<aos::jack::V2>)) {
            
            auto v2array         = std::any_cast<std::vector<aos::jack::V2>>(field.m_value);
            nlohmann::json array = nlohmann::json::array();
            for (const auto& v2 : v2array) {
                array.push_back({
                    {"m_x", v2.m_x},
                    {"m_y", v2.m_y},
                });
            }
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<bool>)) {
            nlohmann::json array = nlohmann::json::array();
            array = std::any_cast<std::vector<bool>>(field.m_value);
            result = std::move(array);
        } else if (field.m_value.type() == typeid(std::vector<std::string>)) {
            nlohmann::json array = nlohmann::json::array();
            array                = std::any_cast<std::vector<std::string>>(field.m_value);
            result = std::move(array);
        }

        /*
         * MW - I was trying to avoid having to include the
         * jack/event-protocol/protocol.h here. Feels unclean and loopy/(chicken
         * and eggy). BDILogLevel should could be put in its own header and
         * elevated into the jack namespace ie similar to aos::jack::V2 which is
         * currently in util.h
         */
        else if (field.m_value.type() == typeid(jack::protocol::BDILogLevel)) {
            jack::protocol::BDILogLevel logLevel =
                std::any_cast<jack::protocol::BDILogLevel>(field.m_value);
            result = static_cast<int32_t>(logLevel);
        }/* else if (field.m_value.type() == typeid(std::vector<jack::Message>)) { ///@ needed?
            const auto& msgArray =
                *std::any_cast<std::vector<Message>>(&field.m_value);
            nlohmann::json array = nlohmann::json::array();
            for (const auto& msg : msgArray) {
                array.push_back(jackMessageToJSON(msg));
            }
            result = std::move(array);
        }*/ else {
            ZoneNamedN(debugTracyCustomMessage, "Custom message serialisation", true);
            const FieldFactoryBase* factory = registry.queryTypeByString(field.m_type);
            if (factory) {
                result = factory->m_customMessageAnyToNlohmannJSONProc(field.m_value);
            }
        }
    } catch (const std::bad_any_cast& e) {
        JACK_WARNING("Failed to convert the field to a protocol field [field={}, reason='{}']", field.m_name, e.what());
        JACK_ASSERT(!"Internal error: invalid code path");
    }
    return result;
}

std::string jackFieldToJSONString(const jack::Field& field)
{
    ZoneScoped;
    std::string result = jackFieldToJSON(field).dump();
    return result;
}

nlohmann::json jackMessageToJSON(const Message& msg)
{
    ZoneScoped;
    nlohmann::json result;

/// @todo fixme - broken 
    result["schemaName"] = msg.schema();
//    result["fields"]     = msg.fields();
    return result;
}

std::string jackMessageToJSONString(const Message& msg)
{
    ZoneScoped;
    std::string result = jackMessageToJSON(msg).dump();
    return result;
}
} /// namespace aos::jack
