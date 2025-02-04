/// Project
#include <tests/meta/messages/pongmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
Pong::Pong()
{
    // set the default values for this message
    count = int32_t{0};

    m_schemaName = "tests.Pong";
}

Pong::Pong(
    const int32_t& count)
{
    this->count = count;

    m_schemaName = "tests.Pong";

}


std::unique_ptr<Pong> Pong::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const Pong* ptr = dynamic_cast<const Pong*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create Pong from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<Pong>();
    *result = *ptr;

    return result;
}

bool Pong::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const Pong& other = static_cast<const Pong&>(rhs);
    return
           count == other.count;
}

bool Pong::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void Pong::swap(Message& other)
{
    if (Pong* derived = dynamic_cast<Pong*>(&other)) {
        std::swap(count, derived->count);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string Pong::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("tests.Pong{{"
                   "count={}}}")
                   , count);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader Pong::SCHEMA =
{
    /*name*/   "tests.Pong",
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

const aos::jack::MessageSchemaField& Pong::schemaField(Pong::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < Pong::SchemaField_COUNT)) {
        result = &Pong::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* Pong::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const Pong* concreteType = std::any_cast<Pong>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> Pong::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<Pong>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string Pong::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<Pong>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string Pong::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<Pong>>(&any);
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

nlohmann::json Pong::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<Pong>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<Pong>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(Pong)) {
        const auto* concreteType = std::any_cast<Pong>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void Pong::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> Pong::clone() const
{
    std::unique_ptr<Pong> msg = std::make_unique<Pong>();
    msg->count = count;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any Pong::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const Pong&)>> factories = {
            {"count", [](const Pong& msg) { return std::make_any<int32_t>(msg.count); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any Pong::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const Pong&)>> factories = {
            {"count", [](const Pong& msg) { std::any ptr = &msg.count; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool Pong::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(Pong&, std::any value)>> factories = {
            {"count", [](Pong& msg, const std::any& v) { msg.count = std::any_cast<int32_t>(v); }},
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
const aos::jack::MessageSchema& Pong::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "tests.Pong";
        result.addFieldValue<int32_t>("count" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.setFactory([](){ return std::make_unique<Pong>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
Pong::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(Pong::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> Pong::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<Pong>();
    msg->count = count;

    return msg;
}

aos::sim::JsonParsedComponent *Pong::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new Pong::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const Pong& src)
{
    dest["count"] = src.count;
}

void from_json(const nlohmann::json& src, Pong& dest)
{
    Pong defaultValue = {};
    dest.count = src.value("count", defaultValue.count);
}

std::string format_as(const ::Pong& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::Pong> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
