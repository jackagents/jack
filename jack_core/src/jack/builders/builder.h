// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_BUILDER_H
#define JACK_BUILDER_H

#include <string>
#include <string_view>

namespace aos::jack
{
class Engine;

/******************************************************************************
 * @class Builder
 * @brief The base class for the all builder classes
 * Builder classes are used to construct JACK concepts within the engine
 ******************************************************************************/
class Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// Construct an named Builder
    /// @param engine The JACK engine instance
    /// @param name The name of this Builder
    Builder(Engine& engine, std::string_view name);

    /// Construct a Builder without a custom name
    /// @param engine The JACK engine instance
    Builder(Engine &engine);

    /// @return The builder's name
    const std::string& name() const { return m_name; }

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    Engine& m_engine;
    std::string m_name;
};
} // namespace aos::jack
#endif /// JACK_BUILDER_H
