/// Project
#include <simple/meta/messages/fanstatusmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
FanStatus::FanStatus()
{
    // set the default values for this message
    is_on = false;

    m_schemaName = "simple.Fan Status";
}

FanStatus::FanStatus(
    const bool& is_on)
{
    this->is_on = is_on;

    m_schemaName = "simple.Fan Status";

}


std::unique_ptr<FanStatus> FanStatus::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const FanStatus* ptr = dynamic_cast<const FanStatus*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create FanStatus from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<FanStatus>();
    *result = *ptr;

    return result;
}

bool FanStatus::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const FanStatus& other = static_cast<const FanStatus&>(rhs);
    return
           is_on == other.is_on;
}

bool FanStatus::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void FanStatus::swap(Message& other)
{
    if (FanStatus* derived = dynamic_cast<FanStatus*>(&other)) {
        std::swap(is_on, derived->is_on);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string FanStatus::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("simple.Fan Status{{"
                   "is_on={}}}")
                   , is_on);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader FanStatus::SCHEMA =
{
    /*name*/   "simple.Fan Status",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "is_on",
            /*type*/         aos::jack::protocol::AnyType_Bool,
            /*array*/        false,
            /*defaultValue*/ std::any(false),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& FanStatus::schemaField(FanStatus::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < FanStatus::SchemaField_COUNT)) {
        result = &FanStatus::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* FanStatus::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const FanStatus* concreteType = std::any_cast<FanStatus>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> FanStatus::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanStatus>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string FanStatus::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<FanStatus>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string FanStatus::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanStatus>>(&any);
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

nlohmann::json FanStatus::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<FanStatus>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanStatus>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(FanStatus)) {
        const auto* concreteType = std::any_cast<FanStatus>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void FanStatus::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> FanStatus::clone() const
{
    std::unique_ptr<FanStatus> msg = std::make_unique<FanStatus>();
    msg->is_on = is_on;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any FanStatus::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const FanStatus&)>> factories = {
            {"is_on", [](const FanStatus& msg) { return std::make_any<bool>(msg.is_on); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any FanStatus::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const FanStatus&)>> factories = {
            {"is_on", [](const FanStatus& msg) { std::any ptr = &msg.is_on; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool FanStatus::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(FanStatus&, std::any value)>> factories = {
            {"is_on", [](FanStatus& msg, const std::any& v) { msg.is_on = std::any_cast<bool>(v); }},
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
const aos::jack::MessageSchema& FanStatus::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "simple.Fan Status";
        result.addFieldValue<bool>("is_on" /*name*/, "Bool" /*type*/, false /*value*/);
        result.setFactory([](){ return std::make_unique<FanStatus>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
FanStatus::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(FanStatus::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> FanStatus::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<FanStatus>();
    msg->is_on = is_on;

    return msg;
}

aos::sim::JsonParsedComponent *FanStatus::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new FanStatus::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const FanStatus& src)
{
    dest["is_on"] = src.is_on;
}

void from_json(const nlohmann::json& src, FanStatus& dest)
{
    FanStatus defaultValue = {};
    dest.is_on = src.value("is_on", defaultValue.is_on);
}

std::string format_as(const ::FanStatus& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::FanStatus> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
