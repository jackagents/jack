// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_ACTION_BUILDER_H
#define JACK_ACTION_BUILDER_H

/// JACK
#include <jack/builders/builder.h>
#include <jack/corelib.h>           // for Span

/// Third Party
#include <string>
#include <string_view>

namespace aos::jack
{
class Engine;
struct Action;

/******************************************************************************
 * @class ActionBuilder
 *
 * @brief A helper class for configuring an agent Action message type
 *
 * An Action builder is requested from the engine instance by calling the action member
 * with the Action name
 *
 * @see Engine
 * @code
 *  bdi.action("WorkTeam")
 * @endcode
 *
 * Once constructed the builder can be used to configure the Action
 *
 * Builders support a fluent style API for easier configuration
 *
 * Example:
 * @code
 * bdi  .action("DeliverItem")
 *      .commit();
 * @endcode
 *
 ******************************************************************************/
class ActionBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// Constructs an unconfigured ActionBuilder with a name
    /// @param engine The JACK engine instance
    /// @param name The name of this Action
    ActionBuilder(Engine& engine, std::string_view name);

    /// Copy constructs an ActionBuilder
    /// @param other The other ActionBuilder
    ActionBuilder(const ActionBuilder &other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    ActionBuilder& request(std::string_view msg) { m_request = msg; return *this; }

    ActionBuilder& reply(std::string_view msg) { m_reply = msg; return *this; }

    ActionBuilder& feedback(std::string_view msg) { m_feedback = msg; return *this; }

    /// Commit this Action template into JACK model
    ActionBuilder& commit();

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    std::string_view m_request  = {}; ///< Request  message name, the parameters the action must have to execute
    std::string_view m_reply    = {}; ///< Reply    message name, the parameters the action will produce on completion
    std::string_view m_feedback = {}; ///< Feedback message name, a progress update that an async action can send during execution
};
} // namespace aos::jack
#endif
