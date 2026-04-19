// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#if !defined(JACK_WEBSOCKET_ADAPTER_H)
#define JACK_WEBSOCKET_ADAPTER_H

/// JACK
#include <jack/websocket-adapter/websocketmeshadapter.h>

namespace aos
{
/// \deprecated WebSocketAdapter is deprecated. Use WebSocketMeshAdapter instead.
/// This class is kept for backward compatibility and acts as a simple server-only
/// mesh adapter with no peers configured.
///
/// For new code, use WebSocketMeshAdapter directly:
/// \code{.cpp}
/// // Old (deprecated):
/// aos::WebSocketAdapter adapter(8080);
///
/// // New:
/// aos::WebSocketMeshAdapter adapter("MyNode", 8080);
/// \endcode
class [[deprecated("Use WebSocketMeshAdapter instead")]] WebSocketAdapter : public WebSocketMeshAdapter
{
public:
    WebSocketAdapter(uint16_t port = 8080)
        : WebSocketMeshAdapter("WebSocketAdapterNode", port, WebSocketOutputMode::BINARY)
    {}

    /// Set the output mode for WebSocket messages (binary BSON or text JSON)
    void setOutputMode(WebSocketOutputMode mode)
    {
        WebSocketMeshAdapter::setOutputMode(mode);
    }
};
} /// namespace aos
#endif /// JACK_WEBSOCKET_ADAPTER
