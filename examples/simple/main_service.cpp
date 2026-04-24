#include <simple/meta/simpleproject.h>
#include <simple/meta/messages/fanstatusmeta.h>
#include <simple/impl/services/fanserviceimpl.h>
#include <jack/jack.h>
#include <jack/websocket-adapter/websocketmeshadapter.h>

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
    
    // Set distinct node name and deterministic UUID (must match WebSocket adapter name)
    // This ensures consistent node identity for bus routing
    bdi.setName("ServiceNode");
    aos::jack::UniqueId serviceNodeUuid = aos::jack::UniqueId::initFromString("a1b2c3d4e5f678901234567890123456");
    bdi.setNodeId(serviceNodeUuid);

    // Create mesh adapter: listen on 8080 (no peers - service is server-only)
    // Note: name must match the BDI engine name for consistent node identity
    aos::WebSocketMeshAdapter meshAdapter("ServiceNode", 8080);
    // CRITICAL: Set UUID to match BDI engine node ID - ensures routing table uses UUID not name
    meshAdapter.setNodeUUID(std::string(serviceNodeUuid.toString()));
    meshAdapter.setOutputMode(aos::WebSocketOutputMode::TEXT);
    if (!meshAdapter.connect()) {
        std::cerr << "Failed to start mesh adapter on port 8080" << std::endl;
        return 1;
    }
    bdi.addBusAdapter(&meshAdapter);
    std::cout << "WebSocket mesh adapter listening on port 8080 as 'ServiceNode'" << std::endl;

    // Create and start the concrete (non-proxy) FanService with hardcoded UUID
    // (matching the proxy on the agent node to test UUID synchronization hypothesis)
    // NOTE: UUID must be 32 hex characters without dashes for initFromString
    aos::jack::UniqueId fanServiceUuid = aos::jack::UniqueId::initFromString("f0e1d2c3b4a569788091a2b3c4d5e6f7");
    FanService* fanService = bdi.createFanServiceInstance("FanService", false /*proxy*/, fanServiceUuid);
    fanService->start();
    std::cout << "FanService started (concrete)" << std::endl;

    // Service node just polls and waits for actions
    while (g_running) {
        bdi.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    fanService->stop();
    meshAdapter.disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
