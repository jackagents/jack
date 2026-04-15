#include <simple/meta/simpleproject.h>
#include <simple/meta/messages/temperaturereadingmeta.h>
#include <simple/impl/services/fanserviceimpl.h>
#include <simple/impl/agents/thermostatagentimpl.h>

#include <jack/jack.h>

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
    // Setup signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    std::cout << "========================================" << std::endl;
    std::cout << "  Simple Thermostat Example" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Temperature thresholds: Fan ON at 25C, Fan OFF at 23C" << std::endl;
    std::cout << "Physics: Fan ON = -0.5C/tick, Fan OFF = +0.5C/tick" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "========================================" << std::endl;

    // Create the BDI engine
    simple bdi;

    // Create the Fan Service
    FanService* fanService = bdi.createFanServiceInstance("FanService", false);
    fanService->start();

    // Create the Thermostat Agent
    ThermostatAgent* agent = bdi.createThermostatAgentInstance("ThermostatAgent");
    agent->attachService(fanService->handle(), false);
    agent->start();

    // Initialize temperature
    float temperature = 24.0f;
    const float TEMP_CHANGE_RATE = 0.5f;
    const float FAN_ON_THRESHOLD = 25.0f;
    const float FAN_OFF_THRESHOLD = 23.0f;

    // Set initial belief
    {
        auto tempReading = agent->context().getMessageAsPtr<TemperatureReading>();
        tempReading->temp = temperature;
    }

    std::cout << "Initial temperature: " << temperature << "C" << std::endl;

    // Main simulation loop at 10Hz
    int tick = 0;
    while (g_running) {
        // Poll the BDI engine
        bdi.poll();

        // Update temperature based on fan state
        if (fanService->isFanOn()) {
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
                      << (fanService->isFanOn() ? "ON" : "OFF") << std::endl;
        }

        tick++;

        // Sleep for 100ms (10Hz)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl << "Shutting down..." << std::endl;

    // Cleanup
    agent->stop();
    fanService->stop();

    std::cout << "Done!" << std::endl;

    return 0;
}
