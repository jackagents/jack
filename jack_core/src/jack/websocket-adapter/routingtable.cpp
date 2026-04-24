// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/// JACK
#include <jack/websocket-adapter/routingtable.h>

/// Third Party
#include <uwebsockets/App.h>

namespace aos
{

void RoutingTable::registerConnection(const std::string& nodeId, WebSocket* ws)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_routes[nodeId] = ws;
}

void RoutingTable::unregisterConnection(const std::string& nodeId)
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_routes.find(nodeId);
    if (it != m_routes.end()) {
        m_routes.erase(it);
    }
}

WebSocket* RoutingTable::findRoute(const std::string& nodeId)
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_routes.find(nodeId);
    if (it != m_routes.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> RoutingTable::getConnectedNodes() const
{
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<std::string> nodes;
    nodes.reserve(m_routes.size());
    for (const auto& [nodeId, _] : m_routes) {
        nodes.push_back(nodeId);
    }
    return nodes;
}

void RoutingTable::clear()
{
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_routes.clear();
}

} /// namespace aos
