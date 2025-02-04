/// Project
#include <tests/meta/messages/pingpongbeliefsetmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
PingPongBeliefSet::PingPongBeliefSet()
{
    // set the default values for this message
    count = int32_t{0};
    target = std::string{};

    m_schemaName = "tests.PingPong BeliefSet";
}

PingPongBeliefSet::PingPongBeliefSet(
    const int32_t& count,
    const std::string& target)
{
    this->count = count;
    this->target = target;

    m_schemaName = "tests.PingPong BeliefSet";

}


std::unique_ptr<PingPongBeliefSet> PingPongBeliefSet::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const PingPongBeliefSet* ptr = dynamic_cast<const PingPongBeliefSet*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create PingPongBeliefSet from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<PingPongBeliefSet>();
    *result = *ptr;

    return result;
}

bool PingPongBeliefSet::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const PingPongBeliefSet& other = static_cast<const PingPongBeliefSet&>(rhs);
    return
           count == other.count &&
           target == other.target;
}

bool PingPongBeliefSet::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void PingPongBeliefSet::swap(Message& other)
{
    if (PingPongBeliefSet* derived = dynamic_cast<PingPongBeliefSet*>(&other)) {
        std::swap(count, derived->count);
        std::swap(target, derived->target);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string PingPongBeliefSet::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("tests.PingPong BeliefSet{{"
                   "count={}, "
                   "target={}}}")
                   , count
                   , target);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader PingPongBeliefSet::SCHEMA =
{
    /*name*/   "tests.PingPong BeliefSet",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "count",
            /*type*/         aos::jack::protocol::AnyType_I32,
            /*array*/        false,
            /*defaultValue*/ std::any(int32_t{0}),
            /*msgHeader*/    nullptr,
        },
        aos::jack::MessageSchemaField{
            /*name*/         "target",
            /*type*/         aos::jack::protocol::AnyType_String,
            /*array*/        false,
            /*defaultValue*/ std::any(std::string{}),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& PingPongBeliefSet::schemaField(PingPongBeliefSet::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < PingPongBeliefSet::SchemaField_COUNT)) {
        result = &PingPongBeliefSet::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* PingPongBeliefSet::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const PingPongBeliefSet* concreteType = std::any_cast<PingPongBeliefSet>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> PingPongBeliefSet::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<PingPongBeliefSet>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string PingPongBeliefSet::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<PingPongBeliefSet>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string PingPongBeliefSet::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<PingPongBeliefSet>>(&any);
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

nlohmann::json PingPongBeliefSet::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<PingPongBeliefSet>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<PingPongBeliefSet>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(PingPongBeliefSet)) {
        const auto* concreteType = std::any_cast<PingPongBeliefSet>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void PingPongBeliefSet::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> PingPongBeliefSet::clone() const
{
    std::unique_ptr<PingPongBeliefSet> msg = std::make_unique<PingPongBeliefSet>();
    msg->count = count;
    msg->target = target;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any PingPongBeliefSet::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const PingPongBeliefSet&)>> factories = {
            {"count", [](const PingPongBeliefSet& msg) { return std::make_any<int32_t>(msg.count); }},
            {"target", [](const PingPongBeliefSet& msg) { return std::make_any<std::string>(msg.target); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any PingPongBeliefSet::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const PingPongBeliefSet&)>> factories = {
            {"count", [](const PingPongBeliefSet& msg) { std::any ptr = &msg.count; return ptr; }},
            {"target", [](const PingPongBeliefSet& msg) { std::any ptr = &msg.target; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool PingPongBeliefSet::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(PingPongBeliefSet&, std::any value)>> factories = {
            {"count", [](PingPongBeliefSet& msg, const std::any& v) { msg.count = std::any_cast<int32_t>(v); }},
            {"target", [](PingPongBeliefSet& msg, const std::any& v) { msg.target = std::any_cast<std::string>(v); }},
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
const aos::jack::MessageSchema& PingPongBeliefSet::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "tests.PingPong BeliefSet";
        result.addFieldValue<int32_t>("count" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.addFieldValue<std::string>("target" /*name*/, "String" /*type*/, std::string{} /*value*/);
        result.setFactory([](){ return std::make_unique<PingPongBeliefSet>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
PingPongBeliefSet::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(PingPongBeliefSet::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> PingPongBeliefSet::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<PingPongBeliefSet>();
    msg->count = count;
    msg->target = target;

    return msg;
}

aos::sim::JsonParsedComponent *PingPongBeliefSet::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new PingPongBeliefSet::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const PingPongBeliefSet& src)
{
    dest["count"] = src.count;
    dest["target"] = src.target;
}

void from_json(const nlohmann::json& src, PingPongBeliefSet& dest)
{
    PingPongBeliefSet defaultValue = {};
    dest.count = src.value("count", defaultValue.count);
    dest.target = src.value("target", defaultValue.target);
}

std::string format_as(const ::PingPongBeliefSet& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::PingPongBeliefSet> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
