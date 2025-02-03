// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/builders/builder.h>
#include <cstdint>   // for uint32_t
#include <jack/corelib.h>

namespace aos::jack
{
Builder::Builder(Engine& engine, std::string_view name)
: m_engine(engine), m_name(name)
{
}

Builder::Builder(Engine& engine)
    : m_engine(engine)
{
    static uint32_t id = 0;
    m_name = JACK_FMT("{}", id++);
}
} // namespace aos::jack
