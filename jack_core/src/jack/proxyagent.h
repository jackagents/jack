#ifndef JACK_PROXYAGENT_H
#define JACK_PROXYAGENT_H

#include <jack/agent.h>        // for Agent
#include <jack/engine/uuid.h>  // for UniqueId

#include <cstdint>        // for uint8_t
#include <functional>     // for function
#include <map>            // for map
#include <vector>         // for vector
#include <string>         // for string

namespace aos::jack
{
class Engine;
struct Event;

/*! ***********************************************************************************************
 * \class ProxyAgent
 *
 * JACK's proxy agent type - derived from agent it mocks an agent and allows
 * its events to be intercepted.
 *
 * This is an advanced agent used to for integrating JACK into a distribution layer
 *
 * A ProxyAgent is created locally for each remote Agent. Events should be routed between the two via
 * the distribution layer
 *
 * ************************************************************************************************/

class ProxyAgent : public Agent
{
public:

    /* ****************************************************************************************
     * Public Ctor & Dtor
     * ****************************************************************************************/

    /// Constructs a ProxyAgent with a name
    /// @param engine The JACK engine instance
    /// @param name The name of this ProxyAgent instance
    ProxyAgent(Engine& engine, std::string_view name)
        : Agent(engine, name)
    {
    }

    /// Create a ProxyAgent by cloning an instance from another agent with a new
    /// name.
    /// @param other The JACK agent to clone from.
    /// @param newName The name of the cloned ProxyAgent instance
    ProxyAgent(const Agent* other, std::string_view newName)
        : Agent(other, newName)
    {
    }

    virtual ~ProxyAgent() {}

    ProxyAgent(const ProxyAgent &other) = delete;
    ProxyAgent& operator=(const ProxyAgent &other) = delete;

    /// \todo I refactored the event sending code onto the bus for ProxyAgents
    /// into a function because we want to reuse largely the same code for
    /// Agents. This is useful because we want to unconditionally send the
    /// events that agents do onto the bus for explanability.
    ///
    /// We have the same issue in Services, but services were designed with
    /// a isProxy boolean flag instead which meant both proxy agent and normal
    /// agent bus transmission code can be done in the same code path.
    ///
    /// Here we can't do that because ProxyAgents are a derived class from
    /// agents.
    ///
    /// We should just implement ProxyAgents like we do proxy Services so that
    /// we can write the bus code in the same unified code path.
    static void ensureThatEventsThatMightOnlyBeRoutedLocallyGetRoutedOntoTheBus(Agent& agent, const Event *event, bool proxy);

    /* ****************************************************************************************
     * Public Accessors & Mutators
     * ****************************************************************************************/
protected:
    void eventDispatch(Event *event) override;

    /// \todo Currently the proxy agent does not run anything. In future we may
    /// want to simulate the agent via this API
    void run() override { }
};
} /// namespace aos::jack
#endif /// JACK_PROXY_AGENT_H
