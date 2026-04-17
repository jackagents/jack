#include <simple/meta/simpleproject.h>
#include <simple/meta/messages/temperaturereadingmeta.h>
#include <simple/impl/agents/thermostatagentimpl.h>
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
    std::cout << "  Thermostat Agent Node (Distributed)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Connecting to service on port 8080..." << std::endl;

    // Create the BDI engine for this node
    simple bdi;

    // Create WebSocket adapter to connect to service
    aos::WebSocketAdapter wsAdapter(8080);
    wsAdapter.setOutputMode(aos::WebSocketOutputMode::TEXT);
    if (!wsAdapter.connect()) {
        std::cerr << "Failed to connect WebSocket adapter to port 8080" << std::endl;
        return 1;
    }
    bdi.addBusAdapter(&wsAdapter);
    std::cout << "WebSocket adapter connected to port 8080" << std::endl;

    // Create a PROXY service - forwards to real service over bus
    FanService* proxyFanService = bdi.createFanServiceInstance("FanService", true /*proxy*/);
    proxyFanService->start();
    std::cout << "FanService proxy started" << std::endl;

    // Create agent and attach to proxy service
    ThermostatAgent* agent = bdi.createThermostatAgentInstance("ThermostatAgent");
    agent->attachService(proxyFanService->handle(), false);
    agent->start();
    std::cout << "ThermostatAgent started" << std::endl;

    // Initialize temperature
    float temperature = 24.0f;
    const float TEMP_CHANGE_RATE = 0.01f;

    // Set initial belief
    {
        auto tempReading = agent->context().getMessageAsPtr<TemperatureReading>();
        tempReading->temp = temperature;
    }

    std::cout << "Initial temperature: " << temperature << "C" << std::endl;
    std::cout << "Physics: Fan ON = -0.01C/tick, Fan OFF = +0.01C/tick" << std::endl;
    std::cout << "Running simulation..." << std::endl;

    // Physics simulation loop
    int tick = 0;
    while (g_running) {
        bdi.poll();

        // Get fan state from proxy service
        bool fanOn = proxyFanService->isFanOn();

        // Update temperature based on fan state
        if (fanOn) {
            temperature -= TEMP_CHANGE_RATE;
        } else {
            temperature += TEMP_CHANGE_RATE;
        }

        // Update agent's temperature belief
        {
            auto tempReading = agent->context().getMessageAsPtr<TemperatureReading>();
            tempReading->temp = temperature;
        }

        // Print status every 10 ticks (1 second)
        if (tick % 10 == 0) {
            std::cout << "[Tick " << tick << "] Temperature: " << temperature << "C, Fan: " 
                      << (fanOn ? "ON" : "OFF") << std::endl;
        }

        tick++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;
    agent->stop();
    proxyFanService->stop();
    wsAdapter.disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
