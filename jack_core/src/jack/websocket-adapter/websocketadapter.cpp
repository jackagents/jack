/// JACK
#include <jack/websocket-adapter/websocketadapter.h>
#include <jack/event-protocol/protocol.h>
#include <jack/corelib.h>

/// Third Party
#include <App.h> /// uWebSockets
#include <thread>
#include <string_view>
#include <tracy/Tracy.hpp>

namespace aos
{
struct PerSocketData {}; /// \note Unused, we are only publishing data for now

enum class WebSocketConnectStatus : uint8_t
{
    NIL,
    FAILED,
    SUCCESS,
};

struct WebSocketAdapter::WebSocketAdapterImpl
{
    uWS::App*                           uws                           = nullptr;
    uWS::Loop*                          uwsEventLoop                  = nullptr;
    uint16_t                            uwsListenPort                 = 0;

    /// \note Hold on-to the socket to shutdown UWS from a thread external to the one running it
    struct us_listen_socket_t*          uwsListenSocket               = nullptr;

    /// \note UWS is single threaded so we have 1 backing buffer to store the
    /// serialised BSON before we publish it onto the bus. After each message
    /// is published, this vector is cleared and re-used for subsequent BSON
    /// messages.
    std::vector<uint8_t>                uwsMessageBuffer;

    /// Tracks the highest number of bytes needed for the message buffer
    size_t                              uwsMessageBufferHighWaterMark = 0;
    size_t                              uwsBytesSent                  = 0;

    /// Tracks the number of vector resizes throughout lifetime of the application
    size_t                              uwsMessageBufferResizes       = 0;

    std::thread                         thread;
    std::atomic<WebSocketConnectStatus> connectStatus                 = {};
    bool                                connectErrorLogged            = false;
};

WebSocketAdapter::WebSocketAdapter(uint16_t port)
: m_impl(new WebSocketAdapter::WebSocketAdapterImpl)
{
    /// \note Reserve upfront 4k for BSON
    m_impl->uwsMessageBuffer.reserve(4 * 1024);
    m_impl->uwsListenPort = port;
}

WebSocketAdapter::~WebSocketAdapter()
{
    disconnect();
}

static constexpr inline std::string_view BUS_TOPIC = "jack";

static void cleanUpUWSState(WebSocketAdapter::WebSocketAdapterImpl *impl)
{
    if (!impl) {
        return;
    }

    /// \note Mark socket as closed and reject any further publishing
    impl->connectStatus.store(WebSocketConnectStatus::NIL);
    impl->connectErrorLogged = false;

    /// \note Terminate the socket which will allow the server to fallthrough
    /// the blocking `run()` call.
    if (impl->uwsListenSocket) {
        us_listen_socket_close(0, impl->uwsListenSocket);
        impl->uwsListenSocket = nullptr;
    }

}

bool WebSocketAdapter::connect()
{
    /// \note Thread has already been started and is running, it must be
    /// disconnected first before being reconnected.
    if (m_impl->thread.joinable()) {
        return false;
    }

    /// \note uWS works in a single threaded context and running the UWS app is
    /// a blocking call. We run uWS in a separate thread initialising all the
    /// classes and state on that thread as none of the APIs are thread-safe
    /// except for the event loop.
    ///
    /// Each thread that a uWS app's are run on is assigned their own
    /// `thread_local` event loop which we snap a copy of in the following
    /// lambda. JACK's main thread can then publish to the websocket in a
    /// thread-safe manner by writing to this event loop.
    m_impl->thread = std::thread([this]() {
        uWS::App uws; /// \note App must be initialised on the same thread

        m_impl->uws          = &uws;             /// \note Snap a copy to the app belonging to this thread
        m_impl->uwsEventLoop = uWS::Loop::get(); /// \note Get this threads event loop to allow publishing from external threads

        /**********************************************************************
         * Setup websocket behaviour for server route '/bus'
         **********************************************************************/
        uWS::App::WebSocketBehavior<PerSocketData> webSocketBehaviour = {};
        webSocketBehaviour.compression                                = uWS::SHARED_COMPRESSOR;
        webSocketBehaviour.maxPayloadLength                           = 16 * 1024; // Maximum message size we can receive
        webSocketBehaviour.idleTimeout                                = 120;       // 2 minutes timeout is good
        webSocketBehaviour.maxBackpressure                            = 64 * 1024; // 64kb backpressure is probably good
        webSocketBehaviour.closeOnBackpressureLimit                   = false;
        webSocketBehaviour.resetIdleTimeoutOnSend                     = false;     // This one depends on kernel timeouts and is a bad default
        webSocketBehaviour.sendPingsAutomatically                     = true;
        webSocketBehaviour.maxLifetime                                = 0;         // Maximum socket lifetime in minutes before forced closure (defaults to disabled)

        /// \note A websocket connection has been made to our server. We will
        /// globally subscribe them to us.
        webSocketBehaviour.open = [](uWS::WebSocket<false /*SSL*/, true, PerSocketData>* webSocket) {
            /// \todo This is an ideal location to add some code that pushes a
            /// summary of the JACK runtime model (like agent hierarchies,
            /// current teams and intentions) to the newly connected node so
            /// that they can catch up.
            [[maybe_unused]] PerSocketData *perSocketData = webSocket->getUserData(); /// \todo Sample code to show how to grab the user context
            webSocket->subscribe(BUS_TOPIC);
        };

        /// \note A subscription handler must be set, otherwise, publishing
        /// fails as no topic are registered to the main app.
        webSocketBehaviour.subscription = [](auto */*ws*/, std::string_view /*message*/, int a, int b) { };

        /// \note Client has sent a message to us. Right now we do not do
        /// anything with the message.
        webSocketBehaviour.message      = [](auto */*ws*/, std::string_view message, uWS::OpCode /*opCode*/) { JACK_INFO("{}", message); };

        /**********************************************************************
         * Configure the UWS app
         **********************************************************************/
        /// \note Apply the websocket behaviour for handling bus connections
        uws.ws<PerSocketData>("/bus", std::move(webSocketBehaviour));

        /// \note Setup the port for incoming connections to the websocket server
        uws.listen(m_impl->uwsListenPort, [this](us_listen_socket_t *socket) {
            if (socket) {
                m_impl->uwsListenSocket = socket;
                m_impl->connectStatus.store(WebSocketConnectStatus::SUCCESS);
                JACK_INFO("Websocket server started and listening on port {}", m_impl->uwsListenPort);
            } else {
                m_impl->connectStatus.store(WebSocketConnectStatus::FAILED);
                JACK_INFO("Websocket server failed to create a listening socket for port {} (e.g. check the port is available, permissions...)", m_impl->uwsListenPort);
            }
        });

        uws.run();

        /// \note Whilst disconnect sets these values for us, if, the app enters
        /// a terminating condition for whatever reason without the user calling
        /// disconnect we cleanup ourselves so that other code that interacts
        /// with the server is aware.
        m_impl->uwsListenSocket = nullptr; /// \note Exiting run means the socket was cleaned up already
        cleanUpUWSState(m_impl.get());

        double highWaterMarkKiB = m_impl->uwsMessageBufferHighWaterMark / 1024.0;
        double bytesSentMiB     = m_impl->uwsBytesSent                  / (1024.0 * 1024.0);
        JACK_INFO("Websocket server on port {} shutting down (HWM was {:.3f}KiB, resized {} times, sent {}MiB)",
                      m_impl->uwsListenPort,
                      highWaterMarkKiB,
                      m_impl->uwsMessageBufferResizes,
                      bytesSentMiB);
    });

    /// \note Busy loop until connected
    while (m_impl->connectStatus.load() == WebSocketConnectStatus::NIL) {
    }

    bool result = m_impl->connectStatus.load() == WebSocketConnectStatus::SUCCESS;
    return result;
}

void WebSocketAdapter::disconnect()
{
    cleanUpUWSState(m_impl.get());

    /// \note Server fall's through and hence makes the thread joinable
    if (m_impl->thread.joinable()) {
        m_impl->thread.join();
    }
}

bool WebSocketAdapter::sendEvent(const jack::protocol::Event *event)
{
    ZoneScoped;
    if (!event->isOk()) {
        return false;
    }

    if (!JACK_CHECK(event)) {
        return false;
    }

    WebSocketConnectStatus connectStatus = m_impl->connectStatus.load();
    if (connectStatus != WebSocketConnectStatus::SUCCESS) {
        if (!m_impl->connectErrorLogged) {
            m_impl->connectErrorLogged = true;
            std::string_view reason = connectStatus == WebSocketConnectStatus::NIL ? "hasn't been started yet by calling `connect()`" : "failed to `connect()` successfully";
            JACK_WARNING_ONCE("Websocket adapter received an event but the server {}. This and subsequent events will be dropped silently until connected.", reason);
        }
        return false;
    }

    bool result = true;
    nlohmann::json j;
    switch (event->type) {
        /// JACK
        case jack::protocol::EventType_CONTROL:          j = *static_cast<const jack::protocol::Control*>(event); break;
        case jack::protocol::EventType_PERCEPT:          j = *static_cast<const jack::protocol::Percept*>(event); break;
        case jack::protocol::EventType_PURSUE:           j = *static_cast<const jack::protocol::Pursue*>(event); break;
        case jack::protocol::EventType_DROP:             j = *static_cast<const jack::protocol::Drop*>(event); break;
        case jack::protocol::EventType_DELEGATION:       j = *static_cast<const jack::protocol::Delegation*>(event); break;
        case jack::protocol::EventType_MESSAGE:          j = *static_cast<const jack::protocol::Message*>(event); break;
        /// Protocol
        case jack::protocol::EventType_REGISTER:         j = *static_cast<const jack::protocol::Register*>(event); break;
        case jack::protocol::EventType_DEREGISTER:       j = *static_cast<const jack::protocol::Deregister*>(event); break;
        case jack::protocol::EventType_AGENT_JOIN_TEAM:  j = *static_cast<const jack::protocol::AgentJoinTeam*>(event); break;
        case jack::protocol::EventType_AGENT_LEAVE_TEAM: j = *static_cast<const jack::protocol::AgentLeaveTeam*>(event); break;
        case jack::protocol::EventType_ACTION_BEGIN:     j = *static_cast<const jack::protocol::ActionBegin*>(event); break;
        case jack::protocol::EventType_ACTION_UPDATE:    j = *static_cast<const jack::protocol::ActionUpdate*>(event); break;
        case jack::protocol::EventType_BDI_LOG:          j = *static_cast<const jack::protocol::BDILog*>(event); break;
        case jack::protocol::EventType_NONE:             result = false; break;
        case jack::protocol::EventType_COUNT:            result = false; break;
    }

    if (j.is_null()) {
        JACK_ASSERT_FMT(!j.is_null(), "A event has been serialised to the null object, event serialisation has failed [event={}]", event->toString());
        result = false;
    }

    if (!result) {
        return result;
    }

    /// \note Grab the event loop that belongs to the uWS app thread and
    /// broadcast the JSON over websockets. Note that the defer will queue the
    /// broadcast of the JSON until the event loop on the uWS app thread is
    /// ready to process data.
    uWS::Loop* uwsEventLoop = m_impl->uwsEventLoop;
    uwsEventLoop->defer([this, json = std::move(j)]() {
        /// \note Record for performance metrics
        size_t capacityBefore = m_impl->uwsMessageBuffer.capacity();

        /// \note Serialise and publish the message
        static constexpr bool DEBUG_SEND_JSON = false;
        if (DEBUG_SEND_JSON) {
            std::string buffer = json.dump();
            m_impl->uws->publish(BUS_TOPIC, buffer, uWS::OpCode::TEXT);

            /// \note Calculate some performance counters
            m_impl->uwsMessageBufferHighWaterMark  = std::max(m_impl->uwsMessageBufferHighWaterMark, buffer.size());
            m_impl->uwsBytesSent                  += buffer.size();
        } else {
            nlohmann::json::to_bson(json, m_impl->uwsMessageBuffer);
            auto buffer = std::string_view(reinterpret_cast<const char *>(m_impl->uwsMessageBuffer.data()), m_impl->uwsMessageBuffer.size());
            m_impl->uws->publish(BUS_TOPIC, buffer, uWS::OpCode::BINARY);

            /// \note Calculate some performance counters
            m_impl->uwsMessageBufferHighWaterMark  = std::max(m_impl->uwsMessageBufferHighWaterMark, buffer.size());
            m_impl->uwsBytesSent                  += buffer.size();
            if (m_impl->uwsMessageBuffer.capacity() != capacityBefore) {
                m_impl->uwsMessageBufferResizes++;
            }

            /// \note Clear the buffer for subsequent messages
            m_impl->uwsMessageBuffer.clear();
        }
    });

    return result;
}
} ///  namespace aos
