/// Project
#include <tests/meta/messages/pingmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
Ping::Ping()
{
    // set the default values for this message
    count = int32_t{0};

    m_schemaName = "tests.Ping";
}

Ping::Ping(
    const int32_t& count)
{
    this->count = count;

    m_schemaName = "tests.Ping";

}


std::unique_ptr<Ping> Ping::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const Ping* ptr = dynamic_cast<const Ping*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create Ping from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<Ping>();
    *result = *ptr;

    return result;
}

bool Ping::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const Ping& other = static_cast<const Ping&>(rhs);
    return
           count == other.count;
}

bool Ping::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void Ping::swap(Message& other)
{
    if (Ping* derived = dynamic_cast<Ping*>(&other)) {
        std::swap(count, derived->count);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string Ping::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("tests.Ping{{"
                   "count={}}}")
                   , count);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader Ping::SCHEMA =
{
    /*name*/   "tests.Ping",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "count",
            /*type*/         aos::jack::protocol::AnyType_I32,
            /*array*/        false,
            /*defaultValue*/ std::any(int32_t{0}),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& Ping::schemaField(Ping::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < Ping::SchemaField_COUNT)) {
        result = &Ping::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* Ping::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const Ping* concreteType = std::any_cast<Ping>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> Ping::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<Ping>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string Ping::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<Ping>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string Ping::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<Ping>>(&any);
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

nlohmann::json Ping::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<Ping>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<Ping>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(Ping)) {
        const auto* concreteType = std::any_cast<Ping>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void Ping::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> Ping::clone() const
{
    std::unique_ptr<Ping> msg = std::make_unique<Ping>();
    msg->count = count;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any Ping::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const Ping&)>> factories = {
            {"count", [](const Ping& msg) { return std::make_any<int32_t>(msg.count); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any Ping::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const Ping&)>> factories = {
            {"count", [](const Ping& msg) { std::any ptr = &msg.count; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool Ping::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(Ping&, std::any value)>> factories = {
            {"count", [](Ping& msg, const std::any& v) { msg.count = std::any_cast<int32_t>(v); }},
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
const aos::jack::MessageSchema& Ping::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "tests.Ping";
        result.addFieldValue<int32_t>("count" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.setFactory([](){ return std::make_unique<Ping>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
Ping::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(Ping::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> Ping::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<Ping>();
    msg->count = count;

    return msg;
}

aos::sim::JsonParsedComponent *Ping::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new Ping::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const Ping& src)
{
    dest["count"] = src.count;
}

void from_json(const nlohmann::json& src, Ping& dest)
{
    Ping defaultValue = {};
    dest.count = src.value("count", defaultValue.count);
}

std::string format_as(const ::Ping& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::Ping> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
