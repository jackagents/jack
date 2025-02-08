/// Project
#include <tests/meta/messages/dummyflagmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
DummyFlag::DummyFlag()
{
    // set the default values for this message
    flag = false;

    m_schemaName = "tests.DummyFlag";
}

DummyFlag::DummyFlag(
    const bool& flag)
{
    this->flag = flag;

    m_schemaName = "tests.DummyFlag";

}


std::unique_ptr<DummyFlag> DummyFlag::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const DummyFlag* ptr = dynamic_cast<const DummyFlag*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create DummyFlag from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<DummyFlag>();
    *result = *ptr;

    return result;
}

bool DummyFlag::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const DummyFlag& other = static_cast<const DummyFlag&>(rhs);
    return
           flag == other.flag;
}

bool DummyFlag::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void DummyFlag::swap(Message& other)
{
    if (DummyFlag* derived = dynamic_cast<DummyFlag*>(&other)) {
        std::swap(flag, derived->flag);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string DummyFlag::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("tests.DummyFlag{{"
                   "flag={}}}")
                   , flag);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader DummyFlag::SCHEMA =
{
    /*name*/   "tests.DummyFlag",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "flag",
            /*type*/         aos::jack::protocol::AnyType_Bool,
            /*array*/        false,
            /*defaultValue*/ std::any(false),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& DummyFlag::schemaField(DummyFlag::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < DummyFlag::SchemaField_COUNT)) {
        result = &DummyFlag::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* DummyFlag::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const DummyFlag* concreteType = std::any_cast<DummyFlag>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> DummyFlag::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<DummyFlag>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string DummyFlag::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<DummyFlag>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string DummyFlag::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<DummyFlag>>(&any);
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

nlohmann::json DummyFlag::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<DummyFlag>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<DummyFlag>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(DummyFlag)) {
        const auto* concreteType = std::any_cast<DummyFlag>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void DummyFlag::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> DummyFlag::clone() const
{
    std::unique_ptr<DummyFlag> msg = std::make_unique<DummyFlag>();
    msg->flag = flag;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any DummyFlag::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const DummyFlag&)>> factories = {
            {"flag", [](const DummyFlag& msg) { return std::make_any<bool>(msg.flag); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any DummyFlag::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const DummyFlag&)>> factories = {
            {"flag", [](const DummyFlag& msg) { std::any ptr = &msg.flag; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool DummyFlag::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(DummyFlag&, std::any value)>> factories = {
            {"flag", [](DummyFlag& msg, const std::any& v) { msg.flag = std::any_cast<bool>(v); }},
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
const aos::jack::MessageSchema& DummyFlag::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "tests.DummyFlag";
        result.addFieldValue<bool>("flag" /*name*/, "Bool" /*type*/, false /*value*/);
        result.setFactory([](){ return std::make_unique<DummyFlag>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
DummyFlag::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(DummyFlag::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> DummyFlag::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<DummyFlag>();
    msg->flag = flag;

    return msg;
}

aos::sim::JsonParsedComponent *DummyFlag::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new DummyFlag::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const DummyFlag& src)
{
    dest["flag"] = src.flag;
}

void from_json(const nlohmann::json& src, DummyFlag& dest)
{
    DummyFlag defaultValue = {};
    dest.flag = src.value("flag", defaultValue.flag);
}

std::string format_as(const ::DummyFlag& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::DummyFlag> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
