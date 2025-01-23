/// Project
#include <gol/meta/messages/cellinfometa.h>

/// JACK
#include <jack/corelib.h>
#include <jack/messageschema.h>
#include <jack/utils.h>


/******************************************************************************
 * Constructor/Destructors
 ******************************************************************************/
CellInfo::CellInfo()
{
    // set the default values for this message
    cell_index = int32_t{0};
    population = int32_t{0};
    is_alive = false;

    m_schemaName = "gol.Cell Info";
}

CellInfo::CellInfo(
    const int32_t& cell_index,
    const int32_t& population,
    const bool& is_alive)
{
    this->cell_index = cell_index;
    this->population = population;
    this->is_alive = is_alive;

    m_schemaName = "gol.Cell Info";

}


std::unique_ptr<CellInfo> CellInfo::createFromPointer(const aos::jack::Message* msg)
{
    if (!msg) {
        return {};
    }

    const CellInfo* ptr = dynamic_cast<const CellInfo*>(msg);

    if (!ptr) {
        JACK_WARNING("Failed to create CellInfo from {} message", msg->schema());
        return {};
    }

    auto result = std::make_unique<CellInfo>();
    *result = *ptr;

    return result;
}

bool CellInfo::operator==(const Message& rhs) const
{
    if (typeid(*this) != typeid(rhs)) {
            return false;
    }

    const CellInfo& other = static_cast<const CellInfo&>(rhs);
    return
           cell_index == other.cell_index &&
           population == other.population &&
           is_alive == other.is_alive;
}

bool CellInfo::operator!=(const Message& rhs) const
{
    return !(*this == rhs);
}

void CellInfo::swap(Message& other)
{
    if (CellInfo* derived = dynamic_cast<CellInfo*>(&other)) {
        std::swap(cell_index, derived->cell_index);
        std::swap(population, derived->population);
        std::swap(is_alive, derived->is_alive);
    } else {
        /// ignore mismatch
    }
}

/******************************************************************************
 * Functions
 ******************************************************************************/
std::string CellInfo::toString() const
{
    aos::jack::ThreadScratchAllocator scratch = aos::jack::getThreadScratchAllocator(nullptr);
    auto builder                              = aos::jack::StringBuilder(scratch.arena);
    builder.append(FMT_STRING("gol.Cell Info{{"
                   "cell_index={}, "
                   "population={}, "
                   "is_alive={}}}")
                   , cell_index
                   , population
                   , is_alive);

    std::string result = builder.toString();
    return result;
}

/******************************************************************************
 * Static Functions
 ******************************************************************************/
const aos::jack::MessageSchemaHeader CellInfo::SCHEMA =
{
    /*name*/   "gol.Cell Info",
    /*fields*/ {
        aos::jack::MessageSchemaField{
            /*name*/         "cell_index",
            /*type*/         aos::jack::protocol::AnyType_I32,
            /*array*/        false,
            /*defaultValue*/ std::any(int32_t{0}),
            /*msgHeader*/    nullptr,
        },
        aos::jack::MessageSchemaField{
            /*name*/         "population",
            /*type*/         aos::jack::protocol::AnyType_I32,
            /*array*/        false,
            /*defaultValue*/ std::any(int32_t{0}),
            /*msgHeader*/    nullptr,
        },
        aos::jack::MessageSchemaField{
            /*name*/         "is_alive",
            /*type*/         aos::jack::protocol::AnyType_Bool,
            /*array*/        false,
            /*defaultValue*/ std::any(false),
            /*msgHeader*/    nullptr,
        },
    }
};

const aos::jack::MessageSchemaField& CellInfo::schemaField(CellInfo::SchemaField field)
{
    const aos::jack::MessageSchemaField* result = nullptr;
    if (JACK_CHECK(field >= 0 && field < CellInfo::SchemaField_COUNT)) {
        result = &CellInfo::SCHEMA.fields[field];
    } else {
        static const aos::jack::MessageSchemaField NIL = {};
        result = &NIL;
    }
    return *result;
}

const aos::jack::Message* CellInfo::anyToMessage(const std::any& any)
{
    const aos::jack::Message* result = nullptr;
    try {
        const CellInfo* concreteType = std::any_cast<CellInfo>(&any);
        if (concreteType) {
            result = concreteType;
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::vector<const aos::jack::Message*> CellInfo::anyArrayToMessage(const std::any& any)
{
    std::vector<const aos::jack::Message*> result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellInfo>>(&any);
        result.reserve(concreteTypeArray->size());
        for (const auto& concreteType : *concreteTypeArray) {
            result.push_back(&concreteType);
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string CellInfo::anyToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteType = std::any_cast<CellInfo>(&any);
        if (concreteType) {
            result = nlohmann::json(*concreteType).dump();
        }
    } catch (const std::bad_any_cast&) {
        /// \note Type did not match, we will return nullptr
    }
    return result;
}

std::string CellInfo::anyArrayToJSON(const std::any& any)
{
    std::string result;
    try {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellInfo>>(&any);
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

nlohmann::json CellInfo::anyToNlohmannJSON(const std::any& any)
{
    nlohmann::json result;
    if (any.type() == typeid(std::vector<CellInfo>)) {
        const auto* concreteTypeArray = std::any_cast<std::vector<CellInfo>>(&any);
        result = *concreteTypeArray;
    } else if (any.type() == typeid(CellInfo)) {
        const auto* concreteType = std::any_cast<CellInfo>(&any);
        result = *concreteType;
    }

    return result;
}

/// Serialise this message into json
void CellInfo::serialise(nlohmann::json& json) const
{
    json = *this;
}

std::unique_ptr<aos::jack::Message> CellInfo::clone() const
{
    std::unique_ptr<CellInfo> msg = std::make_unique<CellInfo>();
    msg->cell_index = cell_index;
    msg->population = population;
    msg->is_alive = is_alive;

    auto basePtr = std::unique_ptr<aos::jack::Message>(std::move(msg));

    return basePtr;
}

std::any CellInfo::getField(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const CellInfo&)>> factories = {
            {"cell_index", [](const CellInfo& msg) { return std::make_any<int32_t>(msg.cell_index); }},
            {"population", [](const CellInfo& msg) { return std::make_any<int32_t>(msg.population); }},
            {"is_alive", [](const CellInfo& msg) { return std::make_any<bool>(msg.is_alive); }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

std::any CellInfo::getFieldPtr(const std::string& fieldName) const
{
    static const std::unordered_map<std::string, std::function<std::any(const CellInfo&)>> factories = {
            {"cell_index", [](const CellInfo& msg) { std::any ptr = &msg.cell_index; return ptr; }},
            {"population", [](const CellInfo& msg) { std::any ptr = &msg.population; return ptr; }},
            {"is_alive", [](const CellInfo& msg) { std::any ptr = &msg.is_alive; return ptr; }},
        };

    auto it = factories.find(fieldName);
    if (it != factories.end()) {
        return it->second(*this);
    }

    return {};
}

bool CellInfo::setField(const std::string& fieldName, const std::any& value)
{
    static const std::unordered_map<std::string, std::function<void(CellInfo&, std::any value)>> factories = {
            {"cell_index", [](CellInfo& msg, const std::any& v) { msg.cell_index = std::any_cast<int32_t>(v); }},
            {"population", [](CellInfo& msg, const std::any& v) { msg.population = std::any_cast<int32_t>(v); }},
            {"is_alive", [](CellInfo& msg, const std::any& v) { msg.is_alive = std::any_cast<bool>(v); }},
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
const aos::jack::MessageSchema& CellInfo::schema()
{
    static aos::jack::MessageSchema result = {};
    for (static bool once = true; once; once = false) {
        result.m_name = "gol.Cell Info";
        result.addFieldValue<int32_t>("cell_index" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.addFieldValue<int32_t>("population" /*name*/, "I32" /*type*/, int32_t{0} /*value*/);
        result.addFieldValue<bool>("is_alive" /*name*/, "Bool" /*type*/, false /*value*/);
        result.setFactory([](){ return std::make_unique<CellInfo>(); });
    }
    return result;
}

/******************************************************************************
 * JSON
 ******************************************************************************/
#if defined(JACK_WITH_SIM)
CellInfo::JsonConfig::JsonConfig()
: aos::sim::JsonParsedComponent(CellInfo::MODEL_NAME)
{
}

std::unique_ptr<aos::jack::Message> CellInfo::JsonConfig::asMessage() const
{
    auto msg = std::make_unique<CellInfo>();
    msg->cell_index = cell_index;
    msg->population = population;
    msg->is_alive = is_alive;

    return msg;
}

aos::sim::JsonParsedComponent *CellInfo::JsonConfig::parseJson(const nlohmann::json &params)
{
    return new CellInfo::JsonConfig(params.get<JsonConfig>());
}
#endif /// JACK_WITH_SIM

void to_json(nlohmann::json& dest, const CellInfo& src)
{
    dest["cell_index"] = src.cell_index;
    dest["population"] = src.population;
    dest["is_alive"] = src.is_alive;
}

void from_json(const nlohmann::json& src, CellInfo& dest)
{
    CellInfo defaultValue = {};
    dest.cell_index = src.value("cell_index", defaultValue.cell_index);
    dest.population = src.value("population", defaultValue.population);
    dest.is_alive = src.value("is_alive", defaultValue.is_alive);
}

std::string format_as(const ::CellInfo& val)
{
    std::string result = val.toString();
    return result;
}

std::string format_as(aos::jack::Span<::CellInfo> val)
{
    std::string result = aos::jack::toStringArray(val);
    return result;
}
