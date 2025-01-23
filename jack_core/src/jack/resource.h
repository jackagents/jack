#ifndef JACK_RESOURCE_H
#define JACK_RESOURCE_H

#include <string>
#include <jack/corelib.h>

namespace aos { namespace jack {

class Agent;

/*! ***********************************************************************************************
 * @class Resource
 *
 * JACK's core Resource type. A special belief for handling the agent's usage of shared resources.
 * The main use of the Resources beliefs is to de-conflict Goals
 *
 * Example:
 * A battery, the agent never actually controls the level of the battery
 * so therefore the agent only reserves the resource
 * the resource might be manipulated externally causing a re-plan.
 *
 * @code
 * // Define a battery resource that ranges from 0 to 100
 * auto batteryResource =
 *      bdi.resource("Battery")
 *         .max(99)
 *         .min(0)
 *         .commit();
 * @endcode
 *
 * ************************************************************************************************/

class Resource
{
public:
    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/
    Resource() = default;

    /// Constructs a default Resource with a name
    /// @param name The name of this Resource
    Resource(std::string_view name);

    /// Constructs an consumable Resource with a name
    /// @param name The name of this Resource template
    /// @param max The maximum value of this Resource
    /// @param min The minimum value of this Resource
    Resource(std::string_view name, int max, int min);

    virtual ~Resource() {}

    /// Copy constructs an Resource from another Resource
    /// @param other The Resource to copy
    Resource(const Resource &other);

    /// Assign the contents of another Resource to this one
    /// @param other The Resource to copy from
    Resource& operator=(const Resource &other);

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/

    /// @return The maximum value for this Resource
    int max() const { return m_max; }

    /// @return The minimum value for this Resource
    int min() const { return m_min; }

    /// @return the current value for the Resource
    int count() const { return m_count; }

    /// @return Does this resource have a valid value
    bool isValid() const { return (m_count >= m_min && m_count <= m_max); }

    /// @return The name of the Resource
    const std::string& name() const { return m_name; }

    /// Set the resource count from an external system
    void set(int count);

    /// lock this resource
    /// @return If the lock was successful
    bool lock() { return consume(1) == 1; }

    /// Consume an amount of resource
    /// @return The requested amount or the maximum available
    int consume(int count);

    /// Unlock this resource
    void unlock() { set(m_count + 1); }

    /// Assign the agent that receives events when the resource is updated
    void setAgent(Agent *agent) { m_agent = agent; }

  protected:

    /* ****************************************************************************************
     * Attributes
     * ****************************************************************************************/
    /// The name of the resource
    std::string m_name;

    /// The amount of the resource we have
    int m_count;

    /// The maximum number of resource we can have
    int m_max;

    /// The minimum number of resource we can have
    int m_min;

    /// The minimum amount we must maintain before a replan is triggered
    /// \todo This is not implemented yet
    int m_reserved;

    /// This agent will receive messages via it's event queue when the resource
    /// is updated. When nullptr no event is sent.
    Agent *m_agent;
};

}} // namespace aos::jack

#endif
