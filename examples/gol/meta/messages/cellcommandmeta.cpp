/// Project
#include <gol/meta/messages/cellcommandmeta.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
CellCommand::CellCommand()
{
    // set the default values for this message
    cell_index = int32_t{0};

    m_schemaName = "gol.Cell Command";
}

CellCommand::CellCommand(
    const int32_t& cell_index)
{
    this->cell_index = cell_index;

    m_schemaName = "gol.Cell Command";

}


std::unique_ptr<CellCommand> CellCommand::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const CellCommand* ptr = dynamic_cast<const CellCommand*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create CellCommand from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<CellCommand>();
    *result = *ptr;

    return result;
}

bool CellCommand::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const CellCommand& other = static_cast<const CellCommand&>(rhs);
    return
           cell_index == other.cell_index;
}

bool CellCommand::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void CellCommand::swap(Message& other)
{
    if (CellCommand* derived = dynamic_cast<CellCommand*>(&other)) {
        std::swap(cell_index, derived->cell_index);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string CellCommand::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("gol.Cell Command{{"
                   "cell_index={}}}")
                   , cell_index);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader CellCommand::SCHEMA =
{
    /*name*/   "gol.Cell Command",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "cell_index",
            /*type*/         aos::jack::protocol::AnyType_I32,
            /*array*/        false,
            /*defaultValue*/ std::any(int32_t{0}),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& CellCommand::schemaField(CellCommand::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < CellCommand::SchemaField_COUNT)) {
        result = &CellCommand::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* CellCommand::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const CellCommand* concreteType = std::any_cast<CellCommand>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> CellCommand::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellCommand>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string CellCommand::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<CellCommand>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string CellCommand::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellCommand>>(&any);
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

nlohmann::json CellCommand::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<CellCommand>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellCommand>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(CellCommand)) {
        const auto* concreteType = std::any_cast<CellCommand>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void CellCommand::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> CellCommand::clone() const
{
    std::unique_ptr<CellCommand> msg = std::make_unique<CellCommand>();
    msg->cell_index = cell_index;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any CellCommand::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const CellCommand&)>> factories = {
            {"cell_index", [](const CellCommand& msg) { return std::make_any<int32_t>(msg.cell_index); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any CellCommand::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const CellCommand&)>> factories = {
            {"cell_index", [](const CellCommand& msg) { std::any ptr = &msg.cell_index; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool CellCommand::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(CellCommand&, std::any value)>> factories = {
            {"cell_index", [](CellCommand& msg, const std::any& v) { msg.cell_index = std::any_cast<int32_t>(v); }},
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
const aos::jack::MessageSchema& CellCommand::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "gol.Cell Command";
        result.addFieldValue<int32_t>("cell_index" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.setFactory([](){ return std::make_unique<CellCommand>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
CellCommand::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(CellCommand::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> CellCommand::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<CellCommand>();
    msg->cell_index = cell_index;

    return msg;
}

aos::sim::JsonParsedComponent *CellCommand::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new CellCommand::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const CellCommand& src)
{
    dest["cell_index"] = src.cell_index;
}

void from_json(const nlohmann::json& src, CellCommand& dest)
{
    CellCommand defaultValue = {};
    dest.cell_index = src.value("cell_index", defaultValue.cell_index);
}

std::string format_as(const ::CellCommand& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::CellCommand> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
