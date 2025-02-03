// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_BUS_ADAPTER_H
#define JACK_BUS_ADAPTER_H

/// Third Party
#include <cstdint>

namespace aos::jack::protocol { struct Event; }

namespace aos::jack
{
class BusAdapter
{
    /**************************************************************************
     * Functions
     **************************************************************************/
public:
    virtual ~BusAdapter() {}

    /// Consume the events that the bus adapter has received
    /// @param[out] dest The buffer to write the received events into
    /// @param[in] size The maximum size of dest that we can write elements into
    /// @return The number of events written to the dest buffer, 0 if the bus
    /// adapter does not have any more events.
    virtual uint32_t poll([[maybe_unused]] protocol::Event **dest, [[maybe_unused]] uint32_t size) { return 0; }

    /// Connect/reconnect the adapter to the transport layer.
    /// @return True if connection was successful, false otherwise
    virtual bool connect() { return false; }

    /// Disconnect the adapter from the transport layer.
    virtual void disconnect() {}

    /// Send the given event onto the transport layer
    /// @return True if the event was successfully sent onto the transport layer.
    virtual bool sendEvent([[maybe_unused]] const protocol::Event* event) { return false; }
};
}  /// namespace aos::jack
#endif /// JACK_BUS_ADAPTER_H
