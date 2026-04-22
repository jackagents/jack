// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/// JACK
#include <jack/websocket-adapter/websocketclient.h>
#include <jack/websocket-adapter/websocketadapter.h>
#include <jack/event-protocol/protocol.h>
#include <jack/corelib.h>

/// Third Party
#include <ixwebsocket/IXWebSocket.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>
#include <atomic>

namespace aos
{

WebSocketClientPool::WebSocketClientPool() = default;

WebSocketClientPool::~WebSocketClientPool()
{
    stop();
}

void WebSocketClientPool::setLocalNodeId(std::string localNodeId)
{
    m_localNodeId = std::move(localNodeId);
}

void WebSocketClientPool::setLocalNodeUUID(std::string localNodeUUID)
{
    m_localNodeUUID = std::move(localNodeUUID);
}

void WebSocketClientPool::addPeer(const std::string& nodeId, const std::string& address, const std::string& localNodeId)
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    if (m_peers.find(nodeId) != m_peers.end()) {
        JACK_WARNING("Peer with nodeId '{}' already exists, ignoring addPeer call", nodeId);
        return;
    }

    auto peer = std::make_unique<PeerConnection>();
    peer->nodeId = nodeId;
    peer->address = address;
    peer->localNodeId = localNodeId;  // Store our local node ID (name) for REGISTER messages
    peer->localNodeUUID = m_localNodeUUID.empty() ? localNodeId : m_localNodeUUID;  // Use UUID if set, otherwise fallback to name
    peer->ixwebsocket.store(nullptr);
    peer->isConnected.store(false);
    peer->shouldStop.store(false);
    peer->reconnectDelay = std::chrono::milliseconds(100);
    peer->lastAttempt = std::chrono::steady_clock::now();

    m_peers[nodeId] = std::move(peer);
}

void WebSocketClientPool::removePeer(const std::string& nodeId)
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    auto it = m_peers.find(nodeId);
    if (it == m_peers.end()) {
        return;
    }

    // Signal the peer to stop
    it->second->shouldStop.store(true);

    // Close the WebSocket if connected
    ix::WebSocket* ws = it->second->ixwebsocket.load();
    if (ws) {
        ws->stop();
    }

    m_peers.erase(it);
}

void WebSocketClientPool::startConnecting()
{
    if (m_running.exchange(true)) {
        return; // Already running
    }

    std::lock_guard<std::mutex> lock(m_peersMutex);
    for (auto& [nodeId, peer] : m_peers) {
        auto threadState = std::make_unique<ThreadState>();
        threadState->thread = std::thread([this, peerPtr = peer.get()]() {
            maintainConnection(peerPtr);
        });
        m_threads.push_back(std::move(threadState));
    }
}

void WebSocketClientPool::stop()
{
    if (!m_running.exchange(false)) {
        return; // Already stopped
    }

    // Signal all peers to stop
    {
        std::lock_guard<std::mutex> lock(m_peersMutex);
        for (auto& [_, peer] : m_peers) {
            peer->shouldStop.store(true);
            ix::WebSocket* ws = peer->ixwebsocket.load();
            if (ws) {
                ws->stop();
            }
        }
    }

    // Wait for all threads to finish
    for (auto& threadState : m_threads) {
        if (threadState->thread.joinable()) {
            threadState->thread.join();
        }
    }
    m_threads.clear();
}

bool WebSocketClientPool::sendTo(const std::string& nodeId, std::string_view message, WebSocketOutputMode mode)
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    auto it = m_peers.find(nodeId);
    if (it == m_peers.end()) {
        return false;
    }

    ix::WebSocket* ws = it->second->ixwebsocket.load();
    if (!ws || !it->second->isConnected.load()) {
        return false;
    }

    // IXWebSocket uses bool binary parameter: false for text, true for binary
    bool binary = (mode == WebSocketOutputMode::BINARY);
    ws->send(std::string(message), binary);
    return true;
}

void WebSocketClientPool::broadcast(std::string_view message, WebSocketOutputMode mode)
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    bool binary = (mode == WebSocketOutputMode::BINARY);

    for (auto& [_, peer] : m_peers) {
        ix::WebSocket* ws = peer->ixwebsocket.load();
        if (ws && peer->isConnected.load()) {
            ws->send(std::string(message), binary);
        }
    }
}

bool WebSocketClientPool::isConnectedTo(const std::string& nodeId) const
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    auto it = m_peers.find(nodeId);
    if (it == m_peers.end()) {
        return false;
    }
    return it->second->isConnected.load();
}

std::vector<std::string> WebSocketClientPool::getConnectedPeers() const
{
    std::lock_guard<std::mutex> lock(m_peersMutex);
    std::vector<std::string> connected;
    for (auto& [nodeId, peer] : m_peers) {
        if (peer->isConnected.load()) {
            connected.push_back(nodeId);
        }
    }
    return connected;
}

void WebSocketClientPool::setOnConnectedCallback(OnConnectedCallback callback)
{
    m_onConnected = std::move(callback);
}

void WebSocketClientPool::setOnDisconnectedCallback(OnDisconnectedCallback callback)
{
    m_onDisconnected = std::move(callback);
}

void WebSocketClientPool::setOnMessageCallback(OnMessageCallback callback)
{
    m_onMessage = std::move(callback);
}

std::chrono::milliseconds WebSocketClientPool::calculateReconnectDelay(std::chrono::milliseconds current)
{
    // Exponential backoff: 100ms, 200ms, 400ms, 800ms, ... max 5 seconds
    constexpr std::chrono::milliseconds MAX_DELAY(5000);
    auto next = current * 2;
    return (next > MAX_DELAY) ? MAX_DELAY : next;
}

void WebSocketClientPool::maintainConnection(PeerConnection* peer)
{
    using namespace std::chrono_literals;

    while (!peer->shouldStop.load() && m_running.load()) {
        if (peer->isConnected.load()) {
            // Connection is healthy, just wait a bit before checking again
            std::this_thread::sleep_for(100ms);
            continue;
        }

        // Check if it's time to attempt reconnection
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastAttempt = std::chrono::duration_cast<std::chrono::milliseconds>(now - peer->lastAttempt);

        if (timeSinceLastAttempt < peer->reconnectDelay) {
            std::this_thread::sleep_for(10ms);
            continue;
        }

        // Time to attempt connection
        peer->lastAttempt = now;

        // Parse address (format: "host:port")
        size_t colonPos = peer->address.find(':');
        if (colonPos == std::string::npos) {
            JACK_WARNING("Invalid peer address format '{}', expected 'host:port'", peer->address);
            peer->reconnectDelay = calculateReconnectDelay(peer->reconnectDelay);
            continue;
        }

        std::string host = peer->address.substr(0, colonPos);
        int port = std::stoi(peer->address.substr(colonPos + 1));

        // Build WebSocket URL
        std::string url = "ws://" + host + ":" + std::to_string(port) + "/bus";

        JACK_INFO("Attempting connection to {} at {}...", peer->nodeId, url);

        // Create IXWebSocket client
        auto ws = std::make_unique<ix::WebSocket>();
        ws->setUrl(url);
        ws->disableAutomaticReconnection();

        // Use a shared_ptr so both the maintenance loop and the callback lambda share
        // ownership of the WebSocket.  The socket is destroyed only when *both* sides
        // release their reference, which prevents the use-after-free that occurs when
        // the maintenance loop calls delete while a callback is still in flight.
        auto wsStorage = std::make_shared<std::unique_ptr<ix::WebSocket>>(std::move(ws));

        // Store the raw pointer in peer for send() callers (valid for the shared lifetime)
        peer->ixwebsocket.store(wsStorage->get());

        JACK_DEBUG("WebSocket storage allocated for peer {}", peer->nodeId);

        // Set up message callback – captures wsStorage by value (shared ownership)
        wsStorage->get()->setOnMessageCallback([peer, wsStorage, this](const ix::WebSocketMessagePtr& msg) {
            // Safety check: don't process if peer is being destroyed
            if (peer->shouldStop.load()) {
                JACK_DEBUG("Ignoring message for {} - peer is stopping", peer->nodeId);
                return;
            }
            
            // Lock to prevent concurrent access to peer data
            std::lock_guard<std::mutex> lock(peer->mutex);
            
            if (msg->type == ix::WebSocketMessageType::Message) {
                JACK_DEBUG("Received message from {} ({} bytes)", peer->nodeId, msg->str.size());
                if (m_onMessage) {
                    m_onMessage(peer->nodeId, msg->str);
                }
            } else if (msg->type == ix::WebSocketMessageType::Open) {
                JACK_INFO("WebSocket OPEN event for peer {} at {}", peer->nodeId, peer->address);
                peer->isConnected.store(true);
                peer->reconnectDelay = std::chrono::milliseconds(100); // Reset on success
                
                // Send REGISTER message to identify ourselves
                // Use UUID for id and name for the name field
                jack::protocol::Register reg;
                reg.senderNode = jack::protocol::BusAddress(
                    jack::protocol::NodeType_NODE,
                    peer->localNodeUUID,  // UUID goes in the 'id' field
                    peer->localNodeId     // Name goes in the 'name' field
                );
                reg.address = reg.senderNode;
                reg.start = false;

                nlohmann::json j = reg;
                std::string msgStr = j.dump();
                
                JACK_DEBUG("Sending REGISTER message for {}: {}", peer->nodeId, msgStr);
                
                // Send using wsStorage directly (it's captured in the lambda)
                // This avoids the race condition with peer->ixwebsocket
                if (!peer->shouldStop.load() && wsStorage && *wsStorage) {
                    JACK_DEBUG("Calling send() on WebSocket for peer {}...", peer->nodeId);
                    (*wsStorage)->send(msgStr, false); // false = text
                    JACK_DEBUG("REGISTER message sent successfully for peer {}", peer->nodeId);
                } else {
                    JACK_WARNING("Cannot send REGISTER for {} - shouldStop={}", 
                                 peer->nodeId, peer->shouldStop.load());
                }

                if (m_onConnected) {
                    JACK_DEBUG("Calling onConnected callback for peer {}...", peer->nodeId);
                    m_onConnected(peer->nodeId, nullptr);
                    JACK_DEBUG("onConnected callback completed for peer {}", peer->nodeId);
                }
            } else if (msg->type == ix::WebSocketMessageType::Close) {
                JACK_INFO("WebSocket CLOSE event for peer {} at {} (code: {})",
                          peer->nodeId, peer->address, msg->closeInfo.code);
                peer->isConnected.store(false);
                peer->ixwebsocket.store(nullptr);

                if (m_onDisconnected) {
                    m_onDisconnected(peer->nodeId);
                }
            } else if (msg->type == ix::WebSocketMessageType::Error) {
                JACK_WARNING("WebSocket ERROR for peer {}: {} (retries: {})", 
                             peer->nodeId, msg->errorInfo.reason, msg->errorInfo.retries);
            }
        });
        
        // Start the connection (non-blocking)
        JACK_DEBUG("Starting WebSocket connection for peer {}...", peer->nodeId);
        (*wsStorage)->start();

        // Wait for connection to establish or fail
        int waitMs = 0;
        const int MAX_WAIT_MS = 5000; // 5 seconds max wait
        while (!peer->isConnected.load() && waitMs < MAX_WAIT_MS && !peer->shouldStop.load()) {
            std::this_thread::sleep_for(10ms);
            waitMs += 10;
        }

        if (!peer->isConnected.load()) {
            // Connection failed
            JACK_DEBUG("Connection attempt to {} failed after {}ms, will retry in {}ms",
                       peer->nodeId, waitMs, peer->reconnectDelay.count());
            
            JACK_DEBUG("Stopping WebSocket for peer {}...", peer->nodeId);
            (*wsStorage)->stop();

            JACK_DEBUG("Clearing peer ixwebsocket pointer for {}...", peer->nodeId);
            peer->ixwebsocket.store(nullptr);

            // Release our shared_ptr reference.  The WebSocket is destroyed here if
            // no callback is still holding the other reference.
            wsStorage.reset();

            peer->reconnectDelay = calculateReconnectDelay(peer->reconnectDelay);
            
            JACK_DEBUG("Connection attempt cleanup complete for peer {}", peer->nodeId);
        } else {
            // Connection successful
            JACK_INFO("Connection to {} established, entering connection maintenance loop", 
                      peer->nodeId);
            
            // Wait for the connection to close or stop signal
            int maintenanceTicks = 0;
            while (peer->isConnected.load() && !peer->shouldStop.load()) {
                std::this_thread::sleep_for(100ms);
                maintenanceTicks++;
                if (maintenanceTicks % 100 == 0) {  // Log every 10 seconds
                    JACK_DEBUG("Connection to {} still active ({}s)", 
                               peer->nodeId, maintenanceTicks / 10);
                }
            }
            
            JACK_INFO("Connection to {} ending (connected={}, shouldStop={})", 
                      peer->nodeId, peer->isConnected.load(), peer->shouldStop.load());
            
            // Clean up - stop the websocket first.
            // ix::WebSocket::stop() joins the internal receive thread, so by the
            // time it returns no further callbacks will fire.  Releasing wsStorage
            // after stop() is therefore safe regardless of which shared_ptr holder
            // goes away last.
            JACK_DEBUG("Stopping WebSocket for peer {}...", peer->nodeId);
            (*wsStorage)->stop();

            JACK_DEBUG("Clearing peer ixwebsocket pointer for {}...", peer->nodeId);
            peer->ixwebsocket.store(nullptr);

            JACK_DEBUG("Setting peer connected=false for {}...", peer->nodeId);
            peer->isConnected.store(false);

            // Release our shared_ptr reference.  The WebSocket is destroyed here
            // (stop() already joined the IXWebSocket thread, so no callbacks remain).
            wsStorage.reset();

            JACK_INFO("Connection cleanup complete for peer {}", peer->nodeId);
        }

        // Wait before retry
        JACK_DEBUG("Waiting 100ms before next connection attempt for peer {}...", peer->nodeId);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    JACK_INFO("Connection maintenance thread ending for peer {}", peer->nodeId);
}

} /// namespace aos
