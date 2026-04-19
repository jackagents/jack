// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_WEBSOCKET_CLIENT_H
#define JACK_WEBSOCKET_CLIENT_H

/// JACK
#include <jack/websocket-adapter/routingtable.h>

/// Third Party
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ix {
class WebSocket;
}

namespace aos
{
/// Forward declaration
enum class WebSocketOutputMode : uint8_t;

/// \brief Represents a connection to a peer node that we initiated (outgoing connection).
struct PeerConnection {
    std::string nodeId;           /// Remote peer's node ID
    std::string address;          /// Remote peer's address (host:port)
    std::string localNodeId;      /// Our local node ID (sent in REGISTER messages)
    std::atomic<ix::WebSocket*> ixwebsocket{nullptr};
    std::atomic<bool> isConnected{false};
    std::atomic<bool> shouldStop{false};
    std::chrono::milliseconds reconnectDelay{100};
    std::chrono::steady_clock::time_point lastAttempt;
    mutable std::mutex mutex;     /// Protects data accessed by both callback and main thread
};

/// \brief Manages outgoing WebSocket connections to peer nodes with automatic reconnection.
///
/// Each peer connection runs in its own thread with exponential backoff retry logic:
/// Attempt 1: 100ms, Attempt 2: 200ms, Attempt 3: 400ms, ... max 5 seconds
///
/// Uses IXWebSocket for client connections (uWebSockets client API is not implemented).
class WebSocketClientPool
{
public:
    /// Callbacks for connection lifecycle events
    using OnConnectedCallback = std::function<void(const std::string& nodeId, void* ws)>;
    using OnDisconnectedCallback = std::function<void(const std::string& nodeId)>;
    using OnMessageCallback = std::function<void(const std::string& nodeId, std::string_view message)>;

    WebSocketClientPool();
    ~WebSocketClientPool();

    /// Set the local node ID (our identifier, used in REGISTER messages).
    /// @param localNodeId The unique identifier of this node.
    void setLocalNodeId(std::string localNodeId);

    /// Add a peer to connect to.
    /// @param nodeId Unique identifier for the remote node.
    /// @param address Host:port to connect to (e.g., "localhost:8081").
    /// @param localNodeId Unique identifier for this local node (used in REGISTER messages).
    void addPeer(const std::string& nodeId, const std::string& address, const std::string& localNodeId);

    /// Remove a peer (closes connection and stops reconnection attempts).
    /// @param nodeId Unique identifier of the peer to remove.
    void removePeer(const std::string& nodeId);

    /// Start connecting to all configured peers (non-blocking, spawns background threads).
    void startConnecting();

    /// Stop all connections and background threads.
    void stop();

    /// Send a message to a specific peer.
    /// @param nodeId The peer to send to.
    /// @param message The message to send.
    /// @param mode TEXT (JSON) or BINARY (BSON).
    /// @return True if the message was queued for sending.
    bool sendTo(const std::string& nodeId, std::string_view message, WebSocketOutputMode mode);

    /// Broadcast a message to all connected peers.
    /// @param message The message to broadcast.
    /// @param mode TEXT (JSON) or BINARY (BSON).
    void broadcast(std::string_view message, WebSocketOutputMode mode);

    /// Check if connected to a specific peer.
    /// @param nodeId The peer to check.
    /// @return True if connected.
    bool isConnectedTo(const std::string& nodeId) const;

    /// Get a list of all connected peer node IDs.
    /// @return Vector of connected peer node IDs.
    std::vector<std::string> getConnectedPeers() const;

    /// Set the callbacks for connection events.
    void setOnConnectedCallback(OnConnectedCallback callback);
    void setOnDisconnectedCallback(OnDisconnectedCallback callback);
    void setOnMessageCallback(OnMessageCallback callback);

private:
    /// Background thread function that maintains a connection to a peer.
    void maintainConnection(PeerConnection* peer);

    /// Calculate the next reconnect delay with exponential backoff.
    std::chrono::milliseconds calculateReconnectDelay(std::chrono::milliseconds current);

    /// Thread-local storage for per-thread state.
    struct ThreadState {
        std::thread thread;
    };

    std::unordered_map<std::string, std::unique_ptr<PeerConnection>> m_peers;
    mutable std::mutex m_peersMutex;

    OnConnectedCallback m_onConnected;
    OnDisconnectedCallback m_onDisconnected;
    OnMessageCallback m_onMessage;

    std::atomic<bool> m_running{false};
    std::vector<std::unique_ptr<ThreadState>> m_threads;
    
    std::string m_localNodeId;  /// Our node identifier, sent in REGISTER messages
};

} /// namespace aos

#endif /// JACK_WEBSOCKET_CLIENT_H
