// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#if !defined(JACK_WEBSOCKET_ADAPTER_H)
#define JACK_WEBSOCKET_ADAPTER_H

/// JACK
#include <jack/event-protocol/busadapter.h>

/// Third Party
#include <memory>

namespace aos::jack::protocol { struct Event; }

namespace aos
{
enum class WebSocketOutputMode : uint8_t
{
    BINARY, /// BSON binary format (default)
    TEXT,   /// JSON text format for debugging
};

class WebSocketAdapter : public jack::BusAdapter
{
    /**************************************************************************
     * Functions
     **************************************************************************/
public:
    WebSocketAdapter(uint16_t port = 8080);

    /// Set the output mode for WebSocket messages (binary BSON or text JSON)
    void setOutputMode(WebSocketOutputMode mode);

    ~WebSocketAdapter();

    uint32_t poll([[maybe_unused]] jack::protocol::Event **dest, [[maybe_unused]] uint32_t size) override final { return 0; }

    bool connect() override final;

    void disconnect() override final;

    bool sendEvent(const jack::protocol::Event* event) override final;

    /**************************************************************************
     * Fields
     **************************************************************************/
public:
    struct WebSocketAdapterImpl;

private:
    std::unique_ptr<WebSocketAdapterImpl> m_impl;
};
} /// namespace aos
#endif /// JACK_WEBSOCKET_ADAPTER
