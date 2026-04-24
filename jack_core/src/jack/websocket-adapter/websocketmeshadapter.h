// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_WEBSOCKET_MESH_ADAPTER_H
#define JACK_WEBSOCKET_MESH_ADAPTER_H

/// JACK
#include <jack/event-protocol/busadapter.h>

/// Third Party
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace aos
{
/// \brief Output mode for WebSocket messages
enum class WebSocketOutputMode : uint8_t
{
    BINARY, /// BSON binary format (default)
    TEXT,   /// JSON text format for debugging
};

/// \brief A full mesh WebSocket adapter that supports both server (accepting connections)
/// and client (initiating connections) modes simultaneously.
///
/// This adapter enables true peer-to-peer communication in distributed JACK systems
/// where every node can both listen for incoming connections and connect to other nodes.
///
/// Usage Example:
/// \code{.cpp}
/// // Service node (listens on 8080)
/// aos::WebSocketMeshAdapter meshAdapter("ServiceNode", 8080);
/// meshAdapter.connect();  // Server only
///
/// // Agent node (listens on 8081, connects to service)
/// aos::WebSocketMeshAdapter meshAdapter("AgentNode", 8081);
/// meshAdapter.addPeer("ServiceNode", "localhost:8080");
/// meshAdapter.connect();  // Server + client
/// \endcode
class WebSocketMeshAdapter : public jack::BusAdapter
{
public:
    /// Create a mesh adapter.
    /// @param localNodeId Unique identifier for this node.
    /// @param listenPort Port to listen for incoming connections.
    /// @param outputMode TEXT (JSON) or BINARY (BSON).
    WebSocketMeshAdapter(
        std::string localNodeId,
        uint16_t listenPort,
        WebSocketOutputMode outputMode = WebSocketOutputMode::BINARY
    );

    ~WebSocketMeshAdapter() override;

    /// Add a peer node to connect to.
    /// @param nodeId Unique identifier of remote node.
    /// @param address Host:port to connect to (e.g., "localhost:8081").
    void addPeer(std::string_view nodeId, std::string_view address);

    /// Remove a peer (closes connection, stops reconnection attempts).
    /// @param nodeId Unique identifier of the peer to remove.
    void removePeer(std::string_view nodeId);

    /// Set the node UUID for this adapter.
    /// @param uuid The unique identifier to use in REGISTER messages.
    /// Must be called before connect() to ensure proper node identification.
    void setNodeUUID(const std::string& uuid);

    /// Set the output mode for WebSocket messages.
    /// @param mode TEXT (JSON) or BINARY (BSON).
    void setOutputMode(WebSocketOutputMode mode);

    /// BusAdapter interface: Start server and connect to peers.
    /// @return True if connection was successful.
    bool connect() override;

    /// BusAdapter interface: Close all connections.
    void disconnect() override;

    /// BusAdapter interface: Poll for received events.
    /// @param[out] dest The buffer to write received events into.
    /// @param[in] size The maximum size of dest.
    /// @return The number of events written to dest.
    uint32_t poll(jack::protocol::Event** dest, uint32_t size) override;

    /// BusAdapter interface: Send an event to a specific node.
    /// @param event The event to send (recipient field determines destination).
    /// @return True if the event was successfully sent.
    bool sendEvent(const jack::protocol::Event* event) override;

    /// Check if connected to a specific peer.
    /// @param nodeId The peer to check.
    /// @return True if connected.
    bool isConnectedTo(std::string_view nodeId) const;

    /// Get a list of all connected peer node IDs.
    /// @return Vector of connected peer node IDs.
    std::vector<std::string> getConnectedPeers() const;

    /// Get a list of all pending (not yet connected) peer node IDs.
    /// @return Vector of pending peer node IDs.
    std::vector<std::string> getPendingPeers() const;

    /// Get the local node ID.
    /// @return This node's unique identifier.
    std::string_view getLocalNodeId() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} /// namespace aos

#endif /// JACK_WEBSOCKET_MESH_ADAPTER_H
