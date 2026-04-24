/// Project
#include <simple/meta/messages/fancontrolmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
FanControl::FanControl()
{
    // set the default values for this message
    turn_on = false;

    m_schemaName = "simple.Fan Control";
}

FanControl::FanControl(
    const bool& turn_on)
{
    this->turn_on = turn_on;

    m_schemaName = "simple.Fan Control";

}


std::unique_ptr<FanControl> FanControl::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const FanControl* ptr = dynamic_cast<const FanControl*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create FanControl from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<FanControl>();
    *result = *ptr;

    return result;
}

bool FanControl::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const FanControl& other = static_cast<const FanControl&>(rhs);
    return
           turn_on == other.turn_on;
}

bool FanControl::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void FanControl::swap(Message& other)
{
    if (FanControl* derived = dynamic_cast<FanControl*>(&other)) {
        std::swap(turn_on, derived->turn_on);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string FanControl::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("simple.Fan Control{{"
                   "turn_on={}}}")
                   , turn_on);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader FanControl::SCHEMA =
{
    /*name*/   "simple.Fan Control",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "turn_on",
            /*type*/         aos::jack::protocol::AnyType_Bool,
            /*array*/        false,
            /*defaultValue*/ std::any(false),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& FanControl::schemaField(FanControl::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < FanControl::SchemaField_COUNT)) {
        result = &FanControl::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* FanControl::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const FanControl* concreteType = std::any_cast<FanControl>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> FanControl::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanControl>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string FanControl::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<FanControl>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string FanControl::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanControl>>(&any);
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

nlohmann::json FanControl::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<FanControl>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<FanControl>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(FanControl)) {
        const auto* concreteType = std::any_cast<FanControl>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void FanControl::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> FanControl::clone() const
{
    std::unique_ptr<FanControl> msg = std::make_unique<FanControl>();
    msg->turn_on = turn_on;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any FanControl::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const FanControl&)>> factories = {
            {"turn_on", [](const FanControl& msg) { return std::make_any<bool>(msg.turn_on); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any FanControl::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const FanControl&)>> factories = {
            {"turn_on", [](const FanControl& msg) { std::any ptr = &msg.turn_on; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool FanControl::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(FanControl&, std::any value)>> factories = {
            {"turn_on", [](FanControl& msg, const std::any& v) { msg.turn_on = std::any_cast<bool>(v); }},
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
const aos::jack::MessageSchema& FanControl::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "simple.Fan Control";
        result.addFieldValue<bool>("turn_on" /*name*/, "Bool" /*type*/, false /*value*/);
        result.setFactory([](){ return std::make_unique<FanControl>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
FanControl::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(FanControl::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> FanControl::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<FanControl>();
    msg->turn_on = turn_on;

    return msg;
}

aos::sim::JsonParsedComponent *FanControl::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new FanControl::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const FanControl& src)
{
    dest["turn_on"] = src.turn_on;
}

void from_json(const nlohmann::json& src, FanControl& dest)
{
    FanControl defaultValue = {};
    dest.turn_on = src.value("turn_on", defaultValue.turn_on);
}

std::string format_as(const ::FanControl& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::FanControl> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
