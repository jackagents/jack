#include <simple/meta/simpleproject.h>
#include <simple/meta/messages/fanstatusmeta.h>
#include <simple/impl/services/fanserviceimpl.h>
#include <jack/jack.h>
#include <jack/websocket-adapter/websocketadapter.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>

static std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    if (signal == SIGINT) {
        g_running = false;
    }
}

int main(int /*argc*/, char **/*argv*/)
{
    std::signal(SIGINT, signalHandler);

    std::cout << "========================================" << std::endl;
    std::cout << "  Fan Service Node (Distributed)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Waiting for agent connections..." << std::endl;

    // Create the BDI engine for this node
    simple bdi;

    // Create WebSocket adapter to communicate on the bus
    aos::WebSocketAdapter wsAdapter(8080);
    wsAdapter.setOutputMode(aos::WebSocketOutputMode::TEXT);
    if (!wsAdapter.connect()) {
        std::cerr << "Failed to start WebSocket adapter on port 8080" << std::endl;
        return 1;
    }
    bdi.addBusAdapter(&wsAdapter);
    std::cout << "WebSocket adapter listening on port 8080" << std::endl;

    // Create and start the concrete (non-proxy) FanService
    FanService* fanService = bdi.createFanServiceInstance("FanService", false /*proxy*/);
    fanService->start();
    std::cout << "FanService started (concrete)" << std::endl;

    // Service node just polls and waits for actions
    while (g_running) {
        bdi.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    fanService->stop();
    wsAdapter.disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
