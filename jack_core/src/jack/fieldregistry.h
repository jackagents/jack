// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#if !defined(JACK_FIELD_REGISTRY_H)
#define JACK_FIELD_REGISTRY_H

/// JACK
#include <jack/corelib.h>

/// Third Party
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <string_view>
#include <map>
#include <any>

namespace aos::jack
{
/******************************************************************************
 * @brief A class that is capable of instantiating the type T at runtime.
 *
 * Each JACK engine stores a list of this class that is registered at run-time.
 * This allows the Engine to instantiate a field at runtime by their registered
 * name to construct arbtirary messages that can be sent between JACK entities.
 *
 * @see Engine::registerType
 ******************************************************************************/
class Field
{
public:
    /// Try update the property value. The underlying value must already be set.
    /// @return True if the underlying value is set and the types match such
    /// that the value was updated. False otherwise.
    template <typename T>
    bool update(const T& newValue);

    /// @return True if the field has values set, false otherwise
    bool valid() const { return m_name.size() && m_type.size() && m_value.has_value(); }

    /// Print the field and its contents
    std::string toString() const;

    /// Print the field in a human friendly representation
    std::string toHumanString() const;

    /// Operator to sort a list of fields by their lexicographic ordering of the
    /// field names.
    bool operator<(const Field& other) const { return m_name < other.m_name; }

    std::string m_name;  ///< The name of the field
    std::string m_type;  ///< The type of the field
    std::any    m_value; ///< The underlying value of the field

    // std::any needs special treatment so spell out conversion,
    // implementation in fieldregistry.cpp
    friend void to_json(nlohmann::json& j, const Field& f);
    friend void from_json(const nlohmann::json& j, Field& f);
};
std::string format_as(const Field& field);

template <typename T>
bool Field::update(const T& newValue) {
    if (!m_value.has_value() || m_value.type() != typeid(T)) {
        return false;
    }
    m_value = newValue;
    return true;
}

/******************************************************************************
 * @brief Base class that is capable of instantiating a registered type at
 * runtime.
 *
 * The registry stores a list of this base class.
 *
 * @see FieldFactory
 ******************************************************************************/
class FieldFactoryBase
{
public:
    using CustomMessageAnyToMessageProc      = const class Message*(const std::any& any);
    using CustomMessageAnyArrayToMessageProc = std::vector<const Message*>(const std::any& any);
    using CustomMessageAnyToJSONProc         = std::string(const std::any& any);
    using CustomMessageAnyArrayToJSONProc    = std::string(const std::any& any);
    using CustomMessageAnyToNlohmannJSONProc = nlohmann::json(const std::any& any);

    virtual Field create(std::string_view fieldName = "") const = 0;
    virtual std::shared_ptr<Message> createMessage() const = 0;
    virtual const std::type_info& type() const = 0;
    virtual ~FieldFactoryBase() {}

    std::string                         m_name; ///< Type name of the field
    CustomMessageAnyToMessageProc*      m_customMessageAnyToMessageProc      = nullptr;
    CustomMessageAnyArrayToMessageProc* m_customMessageAnyArrayToMessageProc = nullptr;
    CustomMessageAnyToJSONProc*         m_customMessageAnyToJSONProc         = nullptr;
    CustomMessageAnyArrayToJSONProc*    m_customMessageAnyArrayToJSONProc    = nullptr;
    CustomMessageAnyToNlohmannJSONProc* m_customMessageAnyToNlohmannJSONProc = nullptr;
};

/******************************************************************************
 * @brief A class that is capable of instantiating the type T at runtime.
 *
 * The register stores a list of this class's base (@see FieldFactoryBase)
 * that is registered at run-time. This allows the Engine to instantiate a field
 * at runtime by their registered name. Messages in JACK are composed of
 * fields.
 *
 * @see Engine::registerType
 ******************************************************************************/
template<typename T>
class FieldFactory : public FieldFactoryBase
{
public:

    template<typename U = T, typename = typename std::enable_if<std::is_base_of<Message, U>::value>::type>
    Field create(std::string_view fieldName = "") const
    {
        Field result   = {};
        result.m_name  = fieldName;
        result.m_type  = m_name;
        result.m_value = std::make_shared<T>();
        return result;
    }

    Field create(std::string_view fieldName = "") const
    {
        Field result   = {};
        result.m_name  = fieldName;
        result.m_type  = m_name;
        result.m_value = T{};
        return result;
    }

    //Field create(std::string_view fieldName = "") const final;
    
    template<typename U = T, typename = typename std::enable_if<std::is_base_of<Message, U>::value>::type>
    std::shared_ptr<Message>  createMessage() const {
        return std::make_shared<T>();
    }

    std::shared_ptr<Message> createMessage() const {
        return {};
    }

    const std::type_info& type() const final { static const auto& result = typeid(T); return result; }
    static std::string m_staticName; ///< @deprecated This makes all Engines globally shared the registered type names
};





template<typename T>
std::string FieldFactory<T>::m_staticName;

/******************************************************************************
 * @brief A class that stores all the globally registered types that can be
 * instantiated at runtime.
 ******************************************************************************/
class FieldRegistry
{
public:
    static FieldRegistry& getInstance();

    /// \note We do not need a destructor because the class's lifetime is
    /// program lifetime, the OS cleans up for us which is faster than chasing 
    /// pointers and deallocating.
    FieldRegistry() = default;

    FieldRegistry(const FieldRegistry&) = delete;

    FieldRegistry& operator=(const FieldRegistry&) = delete;

public:
    /// Create a field from the given type
    /// @param[in] name The name to assign to the field
    /// @param[in] type The name of the type to instantiate for the field
    /// @return The field created and value instantiated, if the field does not
    /// exist Field::valid() returns false.
    Field createField(const std::string& name, const std::string& type) const;

    /// Register a custom type within the engine to allow creation of the type
    /// at runtime.
    /// @param[in] type Name of the type to register
    template <typename T>
    void registerType(std::string_view                                      type,
                      FieldFactoryBase::CustomMessageAnyToMessageProc*      proc             = nullptr,
                      FieldFactoryBase::CustomMessageAnyArrayToMessageProc* arrayProc        = nullptr,
                      FieldFactoryBase::CustomMessageAnyToJSONProc*         jsonProc         = nullptr,
                      FieldFactoryBase::CustomMessageAnyArrayToJSONProc*    jsonArrayProc    = nullptr,
                      FieldFactoryBase::CustomMessageAnyToNlohmannJSONProc* nlohmannJSONProc = nullptr);

    template <typename T>
    const FieldFactoryBase* queryTypeByVariant(const std::any& variant) const;

    /// Query a custom type in the list of registered types in the Engine
    /// @param[in] type Name of the type to query
    /// @return The type if it has been registered before in the Engine,
    /// null pointer otherwise.
    const FieldFactoryBase* queryTypeByString(std::string_view type) const;

    template <typename T>
    const FieldFactory<T>* queryType() const;

    const FieldFactoryBase* queryTypeByVariant(const std::any& variant) const;

private:
    std::atomic<bool>                                     m_init = false;
    std::map<std::string, FieldFactoryBase*, std::less<>> m_factory;
};

template <typename T>
void FieldRegistry::registerType(std::string_view                                      type,
                                 FieldFactoryBase::CustomMessageAnyToMessageProc*      proc,
                                 FieldFactoryBase::CustomMessageAnyArrayToMessageProc* arrayProc,
                                 FieldFactoryBase::CustomMessageAnyToJSONProc*         jsonProc,
                                 FieldFactoryBase::CustomMessageAnyArrayToJSONProc*    jsonArrayProc,
                                 FieldFactoryBase::CustomMessageAnyToNlohmannJSONProc* nlohmannJSONProc)
{
    FieldFactory<T>* entry = JACK_ALLOC(FieldFactory<T>);
    JACK_ASSERT_MSG(entry, "Memory allocation failure");

    if (FieldFactory<T>::m_staticName.size()) {
        JACK_WARNING("Overwriting pre-existing field factory [staticName={}]", FieldFactory<T>::m_staticName);
    }
    FieldFactory<T>::m_staticName = std::string(type);

    if (entry) {
        entry->m_name                               = type;
        entry->m_customMessageAnyToMessageProc      = proc;
        entry->m_customMessageAnyArrayToMessageProc = arrayProc;
        entry->m_customMessageAnyToJSONProc         = jsonProc;
        entry->m_customMessageAnyArrayToJSONProc    = jsonArrayProc;
        entry->m_customMessageAnyToNlohmannJSONProc = nlohmannJSONProc;
        m_factory.insert({std::string(type), entry});
    }
}

template <typename T>
const FieldFactory<T>* FieldRegistry::queryType() const
{
    const FieldFactory<T>* result = nullptr;
    for (auto it = m_factory.cbegin(); !result && it != m_factory.cend(); it++) {
        const FieldFactoryBase* base = it->second;
        result                       = dynamic_cast<const FieldFactory<T>*>(base);
    }
    return result;
}

/******************************************************************************
 * Functions
 ******************************************************************************/
nlohmann::json jackFieldToJSON(const jack::Field& field);

/// Convert a field into its JSON representation. Custom messages can be
/// converted into JSON provided that the schema has been registered to the
/// passed in registry.
std::string jackFieldToJSONString(const Field& field);

/// Convert a message into its JSON representation. Custom messages can be
/// converted into JSON provided that the schema has been registered to the
/// passed in registry.
std::string jackMessageToJSONString(const Message& msg);
}
#endif /// JACK_FIELD_REGISTRY_H
