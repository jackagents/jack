// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#ifndef JACK_ROUTING_TABLE_H
#define JACK_ROUTING_TABLE_H

/// Third Party
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace uWS {
template <bool SSL, bool isServer, typename UserData>
struct WebSocket;
}

namespace aos
{
struct PerSocketData;
using WebSocket = uWS::WebSocket<false /*SSL*/, true, PerSocketData>;

/// rief Manages the mapping from node IDs to WebSocket connections for message routing.
/// 
/// In a mesh topology, each node can have multiple connections (both incoming and outgoing).
/// The routing table maintains the mapping so that when an event needs to be sent to a
/// specific node, the adapter knows which WebSocket connection to use.
///
/// Thread-safe for concurrent reads and writes.
class RoutingTable
{
public:
    /// Register a WebSocket connection for a given node ID.
    /// @param nodeId The unique identifier of the remote node.
    /// @param ws The WebSocket connection to that node.
    void registerConnection(const std::string& nodeId, WebSocket* ws);

    /// Unregister a WebSocket connection for a given node ID.
    /// @param nodeId The unique identifier of the remote node.
    void unregisterConnection(const std::string& nodeId);

    /// Find the WebSocket connection for a given node ID.
    /// @param nodeId The unique identifier of the remote node.
    /// @return The WebSocket connection if found, nullptr otherwise.
    WebSocket* findRoute(const std::string& nodeId);

    /// Get a list of all currently connected node IDs.
    /// @return A vector of connected node IDs.
    std::vector<std::string> getConnectedNodes() const;

    /// Clear all registered connections.
    void clear();

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, WebSocket*> m_routes;
};

} /// namespace aos

#endif /// JACK_ROUTING_TABLE_H
