#include <jack/messageschema.h>

/// Third Party
#include <tracy/Tracy.hpp>

namespace aos::jack
{
MessageSchema::MessageSchema(std::string_view name) : m_name(name) { }

bool MessageSchema::addFieldVariant(std::string_view name, std::string_view type, const std::any& defaultValue, bool overwrite)
{
    if (name.empty() || type.empty()) {
        assert(name.size());
        assert(type.size());
        return false;
    }

    Field item   = {};
    item.m_type  = type;
    item.m_name  = name;
    item.m_value = defaultValue;

    bool result = overwrite || m_fields.count(item) == 0;
    if (result) {
        m_fields.insert(item);
    }
    return result;
}

std::string MessageSchema::toString() const
{
    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder          builder = StringBuilder(scratch.arena);
    builder.append(FMT_STRING("MessageSchema{{name={}, fields[]=["), m_name);

    for (auto it = m_fields.begin(); it != m_fields.end(); it++) {
        if (it != m_fields.begin()) {
            builder.appendRef(", ");
        }
        builder.appendCopy(it->toString());
    }

    std::string result = builder.toString();
    return result;
}

MessageSchema::VerifyMessageResult MessageSchema::verifyMessage(const Message& msg) const
{
    ZoneScoped;
    VerifyMessageResult result = {};
    if (msg.schema() != m_name) {
        result.msg = JACK_FMT("Message does not have matching schema names [msg={}, schema={}]", msg.toString(), toString());
        return result;
    }

    #if 0
    // @todo re-enable
    const auto& fieldMap       = msg.fields();
    result.success             = fieldMap.size() == m_fields.size();

    ThreadScratchAllocator scratch = getThreadScratchAllocator(nullptr);
    StringBuilder          builder  = StringBuilder(scratch.arena);
    if (result.success) {
        StringBuilder          missingFieldsBuilder    = StringBuilder(scratch.arena);
        StringBuilder          wrongStringTypeBuilder  = StringBuilder(scratch.arena);
        StringBuilder          wrongRunTimeTypeBuilder = StringBuilder(scratch.arena);

        for (auto fieldIt = m_fields.begin(); fieldIt != m_fields.end(); fieldIt++) {
            /// \note Pull field from message
            const Field& field    = *fieldIt;
            auto mapIt            = fieldMap.find(field.m_name);

            bool missingField     = false;
            bool wrongStringType  = false;
            bool wrongRunTimeType = false;

            /// \note Verify message
            if (mapIt == fieldMap.end()) {
                missingField = true;
            } else {
                const Field& msgField = mapIt->second;
                wrongStringType  = msgField.m_type != field.m_type;
                wrongRunTimeType = msgField.m_value.type() != field.m_value.type();
            }

            /// \note Accumulate errors for actually helpful error messages
            if (missingField) {
                missingFieldsBuilder.append(FMT_STRING(" '{}'"), field.m_name);
            }

            if (wrongStringType) {
                const Field& msgField = mapIt->second;
                wrongStringTypeBuilder.append(FMT_STRING("{{name={}, type={}, expected={}}}, "),
                                              msgField.m_name,
                                              msgField.m_type,
                                              field.m_type);
            }

            if (wrongRunTimeType) {
                const Field& msgField = mapIt->second;
                wrongRunTimeTypeBuilder.append(FMT_STRING("{{name={}, type={}, expected={}}}, "),
                                              msgField.m_name,
                                              msgField.m_value.type().name(),
                                              field.m_value.type().name());
            }
        }

        std::string_view wrongRunTimeTypeMsg = wrongRunTimeTypeBuilder.toStringArena(scratch.arena);
        std::string_view wrongStringTypeMsg  = wrongStringTypeBuilder.toStringArena(scratch.arena);
        std::string_view missingFieldsMsg    = missingFieldsBuilder.toStringArena(scratch.arena);

        result.success = wrongRunTimeTypeMsg.empty() && wrongStringTypeMsg.empty() && missingFieldsMsg.empty();

        if (!result.success) {
            builder.append(FMT_STRING("Fields in the message do not match the schema {}\n"),
                           msg.m_schemaName);
            if (wrongRunTimeTypeMsg.size()) {
                builder.append(FMT_STRING("  wrongRunTimeTypes[]={{{}}}\n"), wrongRunTimeTypeMsg);
            }

            if (wrongStringTypeMsg.size()) {
                builder.append(FMT_STRING("  wrongStringTypes[]={{{}}}\n"), wrongStringTypeMsg);
            }

            if (missingFieldsMsg.size()) {
                builder.append(FMT_STRING("  missingFields[]={{{}}}\n"), missingFieldsMsg);
            }
        }
    } else {
        /// \todo Dump the schema to logs
        /// \note The fields don't match, dump the message layout as an error
        builder.append(FMT_STRING("Number of fields in the message does not match the schema, expected {}, received {}.\n - The message was {}\n - The schema was {}"),
                       fieldMap.size(),
                       m_fields.size(),
                       msg.toString(),
                       toString());
    }

    if (!result.success) {
        result.msg = builder.toString();
    }
    #else
    result.success = true;
    #endif
    return result;
}
} // namespace aos::jack
