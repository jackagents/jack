// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/message.h>
#include <jack/engine.h>         // for Engine
#include <jack/messageschema.h>  // for SchemaField, MessageSchema

namespace aos::jack
{

void Message::setReasoning(const std::string& text, int level)
{
    m_reasoningText  =  text;
    m_reasoningLevel =  level;
}

std::string format_as(const Message& message)
{
    std::string result = message.toString();
    return result;
}
} // namespace aos::jack
