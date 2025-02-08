/// Project
#include <tests/meta/messages/agentplanswitchmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
AgentPlanSwitch::AgentPlanSwitch()
{
    // set the default values for this message
    switchPlans = false;

    m_schemaName = "tests.AgentPlanSwitch";
}

AgentPlanSwitch::AgentPlanSwitch(
    const bool& switchPlans)
{
    this->switchPlans = switchPlans;

    m_schemaName = "tests.AgentPlanSwitch";

}


std::unique_ptr<AgentPlanSwitch> AgentPlanSwitch::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const AgentPlanSwitch* ptr = dynamic_cast<const AgentPlanSwitch*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create AgentPlanSwitch from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<AgentPlanSwitch>();
    *result = *ptr;

    return result;
}

bool AgentPlanSwitch::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const AgentPlanSwitch& other = static_cast<const AgentPlanSwitch&>(rhs);
    return
           switchPlans == other.switchPlans;
}

bool AgentPlanSwitch::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void AgentPlanSwitch::swap(Message& other)
{
    if (AgentPlanSwitch* derived = dynamic_cast<AgentPlanSwitch*>(&other)) {
        std::swap(switchPlans, derived->switchPlans);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string AgentPlanSwitch::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("tests.AgentPlanSwitch{{"
                   "switchPlans={}}}")
                   , switchPlans);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader AgentPlanSwitch::SCHEMA =
{
    /*name*/   "tests.AgentPlanSwitch",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "switchPlans",
            /*type*/         aos::jack::protocol::AnyType_Bool,
            /*array*/        false,
            /*defaultValue*/ std::any(false),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& AgentPlanSwitch::schemaField(AgentPlanSwitch::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < AgentPlanSwitch::SchemaField_COUNT)) {
        result = &AgentPlanSwitch::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* AgentPlanSwitch::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const AgentPlanSwitch* concreteType = std::any_cast<AgentPlanSwitch>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> AgentPlanSwitch::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<AgentPlanSwitch>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string AgentPlanSwitch::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<AgentPlanSwitch>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string AgentPlanSwitch::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<AgentPlanSwitch>>(&any);
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

nlohmann::json AgentPlanSwitch::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<AgentPlanSwitch>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<AgentPlanSwitch>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(AgentPlanSwitch)) {
        const auto* concreteType = std::any_cast<AgentPlanSwitch>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void AgentPlanSwitch::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> AgentPlanSwitch::clone() const
{
    std::unique_ptr<AgentPlanSwitch> msg = std::make_unique<AgentPlanSwitch>();
    msg->switchPlans = switchPlans;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any AgentPlanSwitch::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const AgentPlanSwitch&)>> factories = {
            {"switchPlans", [](const AgentPlanSwitch& msg) { return std::make_any<bool>(msg.switchPlans); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any AgentPlanSwitch::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const AgentPlanSwitch&)>> factories = {
            {"switchPlans", [](const AgentPlanSwitch& msg) { std::any ptr = &msg.switchPlans; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool AgentPlanSwitch::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(AgentPlanSwitch&, std::any value)>> factories = {
            {"switchPlans", [](AgentPlanSwitch& msg, const std::any& v) { msg.switchPlans = std::any_cast<bool>(v); }},
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
const aos::jack::MessageSchema& AgentPlanSwitch::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "tests.AgentPlanSwitch";
        result.addFieldValue<bool>("switchPlans" /*name*/, "Bool" /*type*/, false /*value*/);
        result.setFactory([](){ return std::make_unique<AgentPlanSwitch>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
AgentPlanSwitch::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(AgentPlanSwitch::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> AgentPlanSwitch::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<AgentPlanSwitch>();
    msg->switchPlans = switchPlans;

    return msg;
}

aos::sim::JsonParsedComponent *AgentPlanSwitch::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new AgentPlanSwitch::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const AgentPlanSwitch& src)
{
    dest["switchPlans"] = src.switchPlans;
}

void from_json(const nlohmann::json& src, AgentPlanSwitch& dest)
{
    AgentPlanSwitch defaultValue = {};
    dest.switchPlans = src.value("switchPlans", defaultValue.switchPlans);
}

std::string format_as(const ::AgentPlanSwitch& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::AgentPlanSwitch> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
