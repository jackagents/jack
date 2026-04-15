/// Project
#include <simple/meta/messages/temperaturereadingmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
TemperatureReading::TemperatureReading()
{
    // set the default values for this message
    temp = 24.0f;

    m_schemaName = "simple.Temperature Reading";
}

TemperatureReading::TemperatureReading(
    const float& temp)
{
    this->temp = temp;

    m_schemaName = "simple.Temperature Reading";

}


std::unique_ptr<TemperatureReading> TemperatureReading::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const TemperatureReading* ptr = dynamic_cast<const TemperatureReading*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create TemperatureReading from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<TemperatureReading>();
    *result = *ptr;

    return result;
}

bool TemperatureReading::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const TemperatureReading& other = static_cast<const TemperatureReading&>(rhs);
    return
           temp == other.temp;
}

bool TemperatureReading::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void TemperatureReading::swap(Message& other)
{
    if (TemperatureReading* derived = dynamic_cast<TemperatureReading*>(&other)) {
        std::swap(temp, derived->temp);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string TemperatureReading::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("simple.Temperature Reading{{"
                   "temp={}}}")
                   , temp);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader TemperatureReading::SCHEMA =
{
    /*name*/   "simple.Temperature Reading",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "temp",
            /*type*/         aos::jack::protocol::AnyType_F32,
            /*array*/        false,
            /*defaultValue*/ std::any(24.0f),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& TemperatureReading::schemaField(TemperatureReading::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < TemperatureReading::SchemaField_COUNT)) {
        result = &TemperatureReading::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* TemperatureReading::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const TemperatureReading* concreteType = std::any_cast<TemperatureReading>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> TemperatureReading::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<TemperatureReading>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string TemperatureReading::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<TemperatureReading>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string TemperatureReading::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<TemperatureReading>>(&any);
        result.reserve(concreteTypeArray->size());

        nlohmann::json array = nlohmann::json::array();
        for (const auto& concreteType : *concreteTypeArray) {
            array.push_back(concreteType);
        }
        result = array.dump();
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

nlohmann::json TemperatureReading::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<TemperatureReading>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<TemperatureReading>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(TemperatureReading)) {
        const auto* concreteType = std::any_cast<TemperatureReading>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void TemperatureReading::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> TemperatureReading::clone() const
{
    std::unique_ptr<TemperatureReading> msg = std::make_unique<TemperatureReading>();
    msg->temp = temp;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any TemperatureReading::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const TemperatureReading&)>> factories = {
            {"temp", [](const TemperatureReading& msg) { return std::make_any<float>(msg.temp); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any TemperatureReading::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const TemperatureReading&)>> factories = {
            {"temp", [](const TemperatureReading& msg) { std::any ptr = &msg.temp; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool TemperatureReading::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(TemperatureReading&, std::any value)>> factories = {
            {"temp", [](TemperatureReading& msg, const std::any& v) { msg.temp = std::any_cast<float>(v); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {

        try {
            it->second(*this, value);
            return true;
        } catch (const std::bad_any_cast& e) {
            return false;
        }
    }
    return false;
}

/// @todo: Deprecate, this method of storing schemas is not great because
/// it requires multiple lookups to resolve nested messages. Our new approach
/// is able to inline the entire data structure without requiring any lookups
/// from the engine.
const aos::jack::MessageSchema& TemperatureReading::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "simple.Temperature Reading";
        result.addFieldValue<float>("temp" /*name*/, "F32" /*type*/, 24.0f /*value*/);
        result.setFactory([](){ return std::make_unique<TemperatureReading>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
TemperatureReading::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(TemperatureReading::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> TemperatureReading::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<TemperatureReading>();
    msg->temp = temp;

    return msg;
}

aos::sim::JsonParsedComponent *TemperatureReading::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new TemperatureReading::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const TemperatureReading& src)
{
    dest["temp"] = src.temp;
}

void from_json(const nlohmann::json& src, TemperatureReading& dest)
{
    TemperatureReading defaultValue = {};
    dest.temp = src.value("temp", defaultValue.temp);
}

std::string format_as(const ::TemperatureReading& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::TemperatureReading> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
