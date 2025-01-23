#ifndef JACK_MESSAGESCHEMA_H
#define JACK_MESSAGESCHEMA_H

#include <jack/message.h>
#include <jack/event-protocol/protocol.h>

#include <string>
#include <set>
#include <vector>
#include <cassert>
#include <any>

namespace aos::jack
{
struct MessageSchemaHeader
{
    std::string name;                              ///< Name of the message
    std::vector<struct MessageSchemaField> fields; ///< Children fields of the message. Set only when type is MESSAGE
};

struct MessageSchemaField
{
    std::string                name;         ///< Name of the field
    protocol::AnyType          type;         ///< Underlying type of the field
    bool                       array;        ///< Describes if the field should be an array of this field type
    std::any                   defaultValue; ///< Default value that this field should be instantiated with
    const MessageSchemaHeader* msgHeader;    ///< Schema information for a message. Only valid when type is MESSAGE
};

/******************************************************************************
 * @brief The schema for messages describing the fields that need to be
 * instantiated to make this message
 ******************************************************************************/
class MessageSchema
{
public:
    /* ************************************************************************
     * Public Ctor & Dtor
     * ************************************************************************/
    MessageSchema() = default;

    /// Construct a schema with a specific name
    /// @param name The name of this schema
    MessageSchema(std::string_view name);

    /// Construct a message from the schema
    std::unique_ptr<Message> createMessage() const { return m_factory(); }

    void setFactory(std::function<std::unique_ptr<Message>()> factory) {
        m_factory = factory;
    }

    /**************************************************************************
     * Functions
     **************************************************************************/
    bool valid() const { return m_name.size(); }

    /// Add a field-value pair to the schema of this message.
    ///
    /// The schema is used to construct a concrete message with starting fields
    /// and values.
    ///
    /// This function can return false if the field already exists unless
    /// `overwrite` is set to `true`. It may also return false if the `name` or
    /// `type` are empty strings.
    ///
    /// @param[in] name The name of the field
    /// @param[in] type The type name of the field, this type must match the
    /// name that the concrete type was registered under in the Engine
    /// @param[in] defaultValue The default value the field should have when
    /// instantiated. If not specified, the value will be default initalised
    /// when it is instantiated.
    /// @param[in] overwrite When true, if the field exists, overwrite the entry
    /// @return True if the field was added, false otherwise
    bool addFieldVariant(std::string_view name, std::string_view type, const std::any& defaultValue = {}, bool overwrite = false);

    template <typename T>
    bool addFieldValue(std::string_view name, std::string_view type, const T& defaultValue = {}, bool overwrite = false);

    /// @return The fields that compose this schema
    const std::set<Field>& fields() const { return m_fields; }

    std::string toString() const;

    struct VerifyMessageResult
    {
        bool        success;
        std::string msg;
    };

    VerifyMessageResult verifyMessage(const Message& msg) const;

    const std::string& name() const { return m_name; }

    /* ************************************************************************
     * Member Fields
     * ************************************************************************/
    std::string m_name; ///< The name of the schema \todo Make this protected after 0.5.3

protected:
    std::set<Field> m_fields; ///< The field to value pairs that a message will have when constructed via the schema

    // callback to create the concreate message
    std::function<std::unique_ptr<Message>()> m_factory;
};

template <typename T>
bool MessageSchema::addFieldValue(std::string_view name, std::string_view type, const T& defaultValue, bool overwrite)
{
    bool result = addFieldVariant(name, type, std::any(defaultValue), overwrite);
    return result;
}
} /// namespace aos::jack
#endif /// JACK_MESSAGE_SCHEMA_H
