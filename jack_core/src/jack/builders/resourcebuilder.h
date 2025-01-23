#ifndef JACK_RESOURCE_BUILDER_H
#define JACK_RESOURCE_BUILDER_H

/// JACK
#include <jack/builders/builder.h>

/// Third Party
#include <string>

namespace aos::jack
{
class Engine;
class Resource;

/******************************************************************************
 * @class ResourceBuilder
 *
 * A helper class for configuring a resource template
 *
 * Example:
 * @code
 * // create a battery resource with min 0 and max 100
 * bdi.resource("Battery")
 *         .max(100)
 *         .min(0)
 *         .commit();
 * @endcode
 ******************************************************************************/
class ResourceBuilder : public Builder
{
public:
    /**************************************************************************
     * Constructor & Destructor
     **************************************************************************/
    /// @param engine The engine instance
    /// @param name The name of the resource
    ResourceBuilder(Engine& engine, std::string_view name);

    /// @param other The builder to copy from
    ResourceBuilder(const ResourceBuilder &other);

    /**************************************************************************
     * Functions
     **************************************************************************/
    /// Set the maximum resource allocation allowed for this resource
    /// @param max The maximum value
    ResourceBuilder& max(int max);

    /// Set the minimum resource allocation allowed for this resource
    /// @param min The minimum value
    ResourceBuilder& min(int min);

    /// Save this resource to the BDI engine
    ResourceBuilder& commit();

protected:
    /**************************************************************************
     * Fields
     **************************************************************************/
    /// The minimum value to the resource can be represented at
    int m_min;

    /// The maximum value to the resource can be represented at
    int m_max;
};
} // namespace aos::jack
#endif
