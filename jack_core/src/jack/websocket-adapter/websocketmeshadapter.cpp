// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

/// JACK
#include <jack/websocket-adapter/websocketmeshadapter.h>
#include <jack/websocket-adapter/websocketadapter.h>
#include <jack/websocket-adapter/routingtable.h>
#include <jack/websocket-adapter/websocketclient.h>
#include <jack/event-protocol/protocol.h>
#include <jack/event-protocol/eventqueue.h>
#include <jack/corelib.h>

/// Third Party
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <tracy/Tracy.hpp>

namespace aos
{

/// Parse a JSON object into the correct concrete protocol::Event subclass.
/// Returns nullptr if the type field is missing, unrecognised, or deserialization fails.
static std::unique_ptr<jack::protocol::Event> deserializeEvent(const nlohmann::json& j)
{
    using namespace jack::protocol;

    // Read the type discriminator from the envelope first.
    if (!j.contains("type")) {
        JACK_WARNING("Received event JSON with no 'type' field [json={}]", j.dump());
        return nullptr;
    }

    EventType type = EventType_NONE;
    j.at("type").get_to(type);

    std::unique_ptr<Event> event;
    try {
        switch (type) {
            case EventType_CONTROL:          { auto e = std::make_unique<Control>();          j.get_to(*e); event = std::move(e); } break;
            case EventType_PERCEPT:          { auto e = std::make_unique<Percept>();          j.get_to(*e); event = std::move(e); } break;
            case EventType_DROP:             { auto e = std::make_unique<Drop>();             j.get_to(*e); event = std::move(e); } break;
            case EventType_MESSAGE:          { auto e = std::make_unique<Message>();          j.get_to(*e); event = std::move(e); } break;
            case EventType_REGISTER:         { auto e = std::make_unique<Register>();         j.get_to(*e); event = std::move(e); } break;
            case EventType_DEREGISTER:       { auto e = std::make_unique<Deregister>();       j.get_to(*e); event = std::move(e); } break;
            case EventType_AGENT_JOIN_TEAM:  { auto e = std::make_unique<AgentJoinTeam>();    j.get_to(*e); event = std::move(e); } break;
            case EventType_AGENT_LEAVE_TEAM: { auto e = std::make_unique<AgentLeaveTeam>();   j.get_to(*e); event = std::move(e); } break;
            case EventType_ACTION_BEGIN:     { auto e = std::make_unique<ActionBegin>();      j.get_to(*e); event = std::move(e); } break;
            case EventType_ACTION_UPDATE:    { auto e = std::make_unique<ActionUpdate>();     j.get_to(*e); event = std::move(e); } break;
            case EventType_BDI_LOG:          { auto e = std::make_unique<BDILog>();           j.get_to(*e); event = std::move(e); } break;
            // Pursue and Delegation have unimplemented from_json (assert(0)) - not expected inbound
            case EventType_PURSUE:
            case EventType_DELEGATION:
                JACK_WARNING("Received inbound event type that has no inbound deserializer [type={}]", static_cast<int>(type));
                return nullptr;
            default:
                JACK_WARNING("Received unknown event type [type={}]", static_cast<int>(type));
                return nullptr;
        }
    } catch (const std::exception& ex) {
        JACK_ERROR("Failed to deserialize event [type={}, error={}]", static_cast<int>(type), ex.what());
        return nullptr;
    }

    return event;
}

struct PerSocketData {
    std::string nodeId;  /// The node ID of the connected peer (set after REGISTER message)
};

enum class ConnectionState : uint8_t {
    NIL,
    CONNECTING,
    CONNECTED,
    DISCONNECTED,
};

struct MeshConnection {
    std::string nodeId;
    std::string address;
    bool isIncoming;
    ConnectionState state;
    std::chrono::steady_clock::time_point lastActivity;
    WebSocket* websocket;
};

struct WebSocketMeshAdapter::Impl {
    /**************************************************************************
     * Server Component
     **************************************************************************/
    uWS::App*                           uwsServer                     = nullptr;
    uWS::Loop*                          uwsServerLoop                 = nullptr;
    struct us_listen_socket_t*          uwsListenSocket               = nullptr;
    uint16_t                            uwsListenPort                 = 0;
    std::thread                         serverThread;
    std::atomic<bool>                   serverRunning                 = false;
    std::atomic<bool>                   serverConnected               = false;
    /// Set to true while uwsServer points at a live uWS::App object.
    /// Cleared (under the uWS event loop via defer) before the App destructs,
    /// so that any concurrently-queued deferred lambda can bail out safely.
    std::atomic<bool>                   serverValid                   = false;

    /**************************************************************************
     * Client Component
     **************************************************************************/
    std::unique_ptr<WebSocketClientPool> clientPool;

    /**************************************************************************
     * Shared State
     **************************************************************************/
    RoutingTable                        routingTable;
    std::string                         localNodeId;
    WebSocketOutputMode                 outputMode                    = WebSocketOutputMode::BINARY;

    /**************************************************************************
     * Message Handling
     **************************************************************************/
    std::queue<std::unique_ptr<jack::protocol::Event>> eventQueue;
    std::mutex                          eventQueueMutex;
    std::vector<uint8_t>                messageBuffer;
    size_t                              messageBufferHighWaterMark  = 0;
    size_t                              bytesSent                   = 0;
    size_t                              messageBufferResizes        = 0;
};

WebSocketMeshAdapter::WebSocketMeshAdapter(
    std::string localNodeId,
    uint16_t listenPort,
    WebSocketOutputMode outputMode
)
    : m_impl(new Impl)
{
    m_impl->localNodeId = std::move(localNodeId);
    m_impl->uwsListenPort = listenPort;
    m_impl->outputMode = outputMode;
    m_impl->messageBuffer.reserve(4 * 1024);  /// 4KB initial capacity
    m_impl->clientPool = std::make_unique<WebSocketClientPool>();
}

WebSocketMeshAdapter::~WebSocketMeshAdapter()
{
    disconnect();
}

void WebSocketMeshAdapter::addPeer(std::string_view nodeId, std::string_view address)
{
    // Pass both the peer's node ID and our local node ID
    m_impl->clientPool->addPeer(std::string(nodeId), std::string(address), m_impl->localNodeId);
}

void WebSocketMeshAdapter::removePeer(std::string_view nodeId)
{
    m_impl->clientPool->removePeer(std::string(nodeId));
}

void WebSocketMeshAdapter::setOutputMode(WebSocketOutputMode mode)
{
    m_impl->outputMode = mode;
}

bool WebSocketMeshAdapter::connect()
{
    if (m_impl->serverRunning.exchange(true)) {
        JACK_WARNING("WebSocketMeshAdapter for node '{}' is already running", m_impl->localNodeId);
        return false;
    }

    /**************************************************************************
     * Set local node ID for client connections (used in REGISTER messages)
     **************************************************************************/
    m_impl->clientPool->setLocalNodeId(m_impl->localNodeId);

    /**************************************************************************
     * Setup callbacks for client connections
     **************************************************************************/
    m_impl->clientPool->setOnConnectedCallback(
        [this](const std::string& nodeId, void* /*ws*/) {
            // Note: IXWebSocket doesn't expose a void* handle like uWebSockets
            // The client pool manages sending internally
            // We don't register in routing table since client connections are managed separately
            JACK_INFO("Node '{}' connected to peer '{}'", m_impl->localNodeId, nodeId);

            // Note: The REGISTER message is sent by the client pool itself
            // when the connection opens (see WebSocketClientPool::maintainConnection)
        }
    );

    m_impl->clientPool->setOnDisconnectedCallback(
        [this](const std::string& nodeId) {
            m_impl->routingTable.unregisterConnection(nodeId);
            JACK_INFO("Node '{}' disconnected from peer '{}'", m_impl->localNodeId, nodeId);
        }
    );

    m_impl->clientPool->setOnMessageCallback(
        [this](const std::string& nodeId, std::string_view message) {
            /// Deserialize and queue the event
            try {
                nlohmann::json j;
                if (message.size() > 0 && message[0] == '{') {
                    /// JSON text
                    j = nlohmann::json::parse(message);
                } else {
                    /// BSON binary
                    j = nlohmann::json::from_bson(message);
                }

                auto event = deserializeEvent(j);
                if (!event) {
                    JACK_WARNING("Dropping undeserializable message from peer '{}'", nodeId);
                    return;
                }

                std::lock_guard<std::mutex> lock(m_impl->eventQueueMutex);
                m_impl->eventQueue.push(std::move(event));
            } catch (const std::exception& e) {
                JACK_ERROR("Failed to parse message from peer '{}': {}", nodeId, e.what());
            }
        }
    );

    /**************************************************************************
     * Start the server thread
     **************************************************************************/
    m_impl->serverThread = std::thread([this]() {
        uWS::App uws;
        m_impl->uwsServer = &uws;
        m_impl->uwsServerLoop = uWS::Loop::get();
        m_impl->serverValid.store(true);

        /**********************************************************************
         * Setup WebSocket behavior for server connections
         **********************************************************************/
        uWS::App::WebSocketBehavior<PerSocketData> wsBehavior = {};
        wsBehavior.compression = uWS::SHARED_COMPRESSOR;
        wsBehavior.maxPayloadLength = 16 * 1024;
        wsBehavior.idleTimeout = 120;
        wsBehavior.maxBackpressure = 64 * 1024;
        wsBehavior.closeOnBackpressureLimit = false;
        wsBehavior.resetIdleTimeoutOnSend = false;
        wsBehavior.sendPingsAutomatically = true;

        wsBehavior.open = [this](WebSocket* ws) {
            JACK_DEBUG("Incoming WebSocket connection to node '{}'", m_impl->localNodeId);
            ws->subscribe("jack");
        };

        wsBehavior.message = [this](WebSocket* ws, std::string_view message, uWS::OpCode opCode) {
            try {
                nlohmann::json j;
                if (opCode == uWS::OpCode::TEXT) {
                    j = nlohmann::json::parse(message);
                } else {
                    j = nlohmann::json::from_bson(message);
                }

                /// Check if this is a REGISTER message
                if (j.contains("type") && j["type"] == "REGISTER") {
                    jack::protocol::Register reg;
                    j.get_to(reg);

                    /// Store the node ID in the socket's user data
                    PerSocketData* data = ws->getUserData();
                    data->nodeId = reg.senderNode.id;

                    /// Register in routing table
                    m_impl->routingTable.registerConnection(reg.senderNode.id, ws);

                    JACK_INFO("Peer '{}' registered with node '{}'", reg.senderNode.id, m_impl->localNodeId);
                    return;
                }

                /// Regular event - queue it using the typed factory
                auto event = deserializeEvent(j);
                if (!event) {
                    JACK_WARNING("Dropping undeserializable message on node '{}'", m_impl->localNodeId);
                    return;
                }

                std::lock_guard<std::mutex> lock(m_impl->eventQueueMutex);
                m_impl->eventQueue.push(std::move(event));

            } catch (const std::exception& e) {
                JACK_ERROR("Failed to parse message on node '{}': {}", m_impl->localNodeId, e.what());
            }
        };

        wsBehavior.close = [this](WebSocket* ws, int code, std::string_view message) {
            PerSocketData* data = ws->getUserData();
            if (!data->nodeId.empty()) {
                m_impl->routingTable.unregisterConnection(data->nodeId);
                JACK_INFO("Peer '{}' disconnected from node '{}' (code: {}, message: {})",
                          data->nodeId, m_impl->localNodeId, code, message);
            }
        };

        uws.ws<PerSocketData>("/bus", std::move(wsBehavior));

        uws.listen(m_impl->uwsListenPort, [this](us_listen_socket_t* socket) {
            if (socket) {
                m_impl->uwsListenSocket = socket;
                m_impl->serverConnected.store(true);
                JACK_INFO("WebSocketMeshAdapter for node '{}' listening on port {}",
                          m_impl->localNodeId, m_impl->uwsListenPort);
            } else {
                m_impl->serverConnected.store(false);
                JACK_ERROR("WebSocketMeshAdapter for node '{}' failed to listen on port {}",
                           m_impl->localNodeId, m_impl->uwsListenPort);
            }
        });

        uws.run();

        /// Cleanup after run() returns.
        /// Clear serverValid first so any deferred lambdas queued after this point
        /// will not attempt to dereference the about-to-destruct uWS::App.
        m_impl->serverValid.store(false);
        m_impl->uwsServer = nullptr;
        m_impl->uwsServerLoop = nullptr;
        m_impl->uwsListenSocket = nullptr;
        m_impl->serverConnected.store(false);
        m_impl->serverRunning.store(false);
    });

    /// Wait for server to start
    while (m_impl->serverRunning.load() && !m_impl->serverConnected.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!m_impl->serverConnected.load()) {
        JACK_ERROR("WebSocketMeshAdapter failed to start server for node '{}'", m_impl->localNodeId);
        return false;
    }

    /**************************************************************************
     * Start client connections to peers
     **************************************************************************/
    m_impl->clientPool->startConnecting();

    return true;
}

void WebSocketMeshAdapter::disconnect()
{
    /**************************************************************************
     * Stop client connections
     **************************************************************************/
    if (m_impl->clientPool) {
        m_impl->clientPool->stop();
    }

    /**************************************************************************
     * Stop the server
     **************************************************************************/
    if (m_impl->serverRunning.exchange(false)) {
        if (m_impl->uwsListenSocket) {
            us_listen_socket_close(0, m_impl->uwsListenSocket);
            m_impl->uwsListenSocket = nullptr;
        }

        if (m_impl->serverThread.joinable()) {
            m_impl->serverThread.join();
        }
    }

    m_impl->routingTable.clear();

    JACK_INFO("WebSocketMeshAdapter for node '{}' disconnected", m_impl->localNodeId);
}

uint32_t WebSocketMeshAdapter::poll(jack::protocol::Event** dest, uint32_t size)
{
    ZoneScoped;

    std::lock_guard<std::mutex> lock(m_impl->eventQueueMutex);

    uint32_t count = 0;
    while (count < size && !m_impl->eventQueue.empty()) {
        auto& event = m_impl->eventQueue.front();
        /// Note: This is a simplification - in production we'd need proper memory management
        /// For now, we assume the caller manages the lifetime
        dest[count] = event.release();
        m_impl->eventQueue.pop();
        count++;
    }

    return count;
}

bool WebSocketMeshAdapter::sendEvent(const jack::protocol::Event* event)
{
    ZoneScoped;

    if (!event || !event->isOk()) {
        return false;
    }

    /**************************************************************************
     * Serialize the event
     **************************************************************************/
    nlohmann::json j;
    switch (event->type) {
        case jack::protocol::EventType_CONTROL:          j = *static_cast<const jack::protocol::Control*>(event); break;
        case jack::protocol::EventType_PERCEPT:          j = *static_cast<const jack::protocol::Percept*>(event); break;
        case jack::protocol::EventType_PURSUE:           j = *static_cast<const jack::protocol::Pursue*>(event); break;
        case jack::protocol::EventType_DROP:             j = *static_cast<const jack::protocol::Drop*>(event); break;
        case jack::protocol::EventType_DELEGATION:       j = *static_cast<const jack::protocol::Delegation*>(event); break;
        case jack::protocol::EventType_MESSAGE:          j = *static_cast<const jack::protocol::Message*>(event); break;
        case jack::protocol::EventType_REGISTER:         j = *static_cast<const jack::protocol::Register*>(event); break;
        case jack::protocol::EventType_DEREGISTER:       j = *static_cast<const jack::protocol::Deregister*>(event); break;
        case jack::protocol::EventType_AGENT_JOIN_TEAM:  j = *static_cast<const jack::protocol::AgentJoinTeam*>(event); break;
        case jack::protocol::EventType_AGENT_LEAVE_TEAM: j = *static_cast<const jack::protocol::AgentLeaveTeam*>(event); break;
        case jack::protocol::EventType_ACTION_BEGIN:     j = *static_cast<const jack::protocol::ActionBegin*>(event); break;
        case jack::protocol::EventType_ACTION_UPDATE:    j = *static_cast<const jack::protocol::ActionUpdate*>(event); break;
        case jack::protocol::EventType_BDI_LOG:          j = *static_cast<const jack::protocol::BDILog*>(event); break;
        default:
            JACK_WARNING("Unknown event type: {}", static_cast<int>(event->type));
            return false;
    }

    if (j.is_null()) {
        JACK_ERROR("Event serialization failed for type: {}", static_cast<int>(event->type));
        return false;
    }

    /**************************************************************************
     * Route the message
     **************************************************************************/
    std::string destNode;
    if (event->recipient.isSet()) {
        destNode = event->recipient.id;
    }

    /// If no specific recipient, broadcast to all connected peers
    if (destNode.empty()) {
        if (m_impl->outputMode == WebSocketOutputMode::TEXT) {
            std::string msg = j.dump();
            /// Broadcast via server to all connected clients
            if (m_impl->uwsServer && m_impl->uwsServerLoop) {
                // Copy msg into the lambda (do NOT move) so the original string
                // remains valid for the clientPool->broadcast() call below.
                m_impl->uwsServerLoop->defer([this, msg]() {
                    // Guard against the server thread having exited since this
                    // lambda was queued (uwsServer would be a dangling pointer).
                    if (m_impl->serverValid.load()) {
                        m_impl->uwsServer->publish("jack", msg, uWS::OpCode::TEXT);
                    }
                });
            }
            /// Broadcast via client pool to all connected peers
            m_impl->clientPool->broadcast(msg, WebSocketOutputMode::TEXT);
        } else {
            nlohmann::json::to_bson(j, m_impl->messageBuffer);

            m_impl->messageBufferHighWaterMark = std::max(m_impl->messageBufferHighWaterMark, m_impl->messageBuffer.size());
            m_impl->bytesSent += m_impl->messageBuffer.size();

            if (m_impl->uwsServer && m_impl->uwsServerLoop) {
                // Capture an owned copy so the deferred lambda does not hold a
                // dangling string_view into messageBuffer (which is cleared below).
                std::string ownedMsg(reinterpret_cast<const char*>(m_impl->messageBuffer.data()),
                                     m_impl->messageBuffer.size());
                m_impl->uwsServerLoop->defer([this, msg = std::move(ownedMsg)]() {
                    // Guard against the server thread having exited since this
                    // lambda was queued (uwsServer would be a dangling pointer).
                    if (m_impl->serverValid.load()) {
                        m_impl->uwsServer->publish("jack", msg, uWS::OpCode::BINARY);
                    }
                });
            }

            // broadcast() copies the view into each send() call before we clear,
            // so passing a string_view here is safe.
            std::string_view buffer(reinterpret_cast<const char*>(m_impl->messageBuffer.data()),
                                    m_impl->messageBuffer.size());
            m_impl->clientPool->broadcast(buffer, WebSocketOutputMode::BINARY);

            m_impl->messageBuffer.clear();
        }
        return true;
    }

    /// Route to specific node
    WebSocket* ws = m_impl->routingTable.findRoute(destNode);
    if (!ws) {
        /// Try via client pool
        if (m_impl->outputMode == WebSocketOutputMode::TEXT) {
            return m_impl->clientPool->sendTo(destNode, j.dump(), WebSocketOutputMode::TEXT);
        } else {
            nlohmann::json::to_bson(j, m_impl->messageBuffer);
            auto buffer = std::string_view(reinterpret_cast<const char*>(m_impl->messageBuffer.data()),
                                           m_impl->messageBuffer.size());
            bool result = m_impl->clientPool->sendTo(destNode, buffer, WebSocketOutputMode::BINARY);
            m_impl->messageBuffer.clear();
            return result;
        }
    }

    /// Send via the routing table connection
    if (m_impl->outputMode == WebSocketOutputMode::TEXT) {
        std::string msg = j.dump();
        ws->send(msg, uWS::OpCode::TEXT);
    } else {
        nlohmann::json::to_bson(j, m_impl->messageBuffer);
        auto buffer = std::string_view(reinterpret_cast<const char*>(m_impl->messageBuffer.data()),
                                       m_impl->messageBuffer.size());
        ws->send(buffer, uWS::OpCode::BINARY);
        m_impl->messageBuffer.clear();
    }

    return true;
}

bool WebSocketMeshAdapter::isConnectedTo(std::string_view nodeId) const
{
    return m_impl->clientPool->isConnectedTo(std::string(nodeId));
}

std::vector<std::string> WebSocketMeshAdapter::getConnectedPeers() const
{
    return m_impl->clientPool->getConnectedPeers();
}

std::vector<std::string> WebSocketMeshAdapter::getPendingPeers() const
{
    /// For now, return empty list (would need to track pending connections in client pool)
    return {};
}

std::string_view WebSocketMeshAdapter::getLocalNodeId() const
{
    return m_impl->localNodeId;
}

} /// namespace aos
