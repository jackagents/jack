# Simple Example - Thermostat Agent

A minimal distributed agent example demonstrating a thermostat that controls a fan based on temperature readings.

## Overview

This example showcases the JACK BDI (Beliefs, Desires, Intentions) framework with:
- **One Agent**: `ThermostatAgent` - monitors temperature and decides when to turn the fan on/off
- **One Service**: `FanService` - handles the physical fan actions
- **One Goal**: `MaintainComfort` - keeps temperature within a comfortable range
- **Two Plans**: 
  - `TurnFanOnPlan` - activates when temperature >= 25°C
  - `TurnFanOffPlan` - activates when temperature <= 23°C

## Two Versions

This example includes **two versions**:

1. **Standalone** (`simpleapp`) - Single executable with everything in one process
2. **Distributed** (`simple_service` + `simple_agent`) - Split across two nodes communicating via WebSocket

---

## Standalone Version

Single executable with agent, service, and simulation loop all in one process.

### Building

```bash
cd /path/to/jack
cmake -B build
cmake --build build --target simpleapp
```

### Running

```bash
./build/bin/simpleapp
```

Press `Ctrl+C` to exit.

---

## Distributed Version ✨

**The service and agent run as separate processes communicating via WebSocket.**

This demonstrates distributed BDI agents where:
- `simple_service` runs the `FanService` (the "hardware" node)
- `simple_agent` runs the `ThermostatAgent` (the "control" node)
- They communicate via WebSocket using the JACK bus protocol

### Architecture

```
┌─────────────────────┐         WebSocket         ┌─────────────────────┐
│   simple_service    │ ◄──────────────────────► │    simple_agent     │
│   (ServiceNode)     │    Port 8080 (default)   │    (AgentNode)      │
│                     │                          │                     │
│  ┌───────────────┐  │                          │  ┌───────────────┐  │
│  │  FanService   │  │  ←── ACTION_BEGIN ───    │  │ThermostatAgent│  │
│  │  (concrete)   │  │                          │  │               │  │
│  │               │  │  ─── ACTION_UPDATE ──→   │  │   + Proxy to  │  │
│  │ • Turn Fan On │  │                          │  │    FanService │  │
│  │ • Turn Fan Off│  │                          │  │               │  │
│  └───────────────┘  │                          │  └───────────────┘  │
└─────────────────────┘                          └─────────────────────┘
```

### Building

```bash
cd /path/to/jack
cmake -B build
cmake --build build --target simple_service simple_agent
```

### Running Manually

Terminal 1 - Start the service first:
```bash
./build/bin/simple_service
```

Terminal 2 - Then start the agent:
```bash
./build/bin/simple_agent
```

**Note:** The service must be running before the agent starts, as the agent connects to the service's WebSocket port.

### Running the Automated Test

A test script is included that builds, runs, and verifies the distributed setup:

```bash
cd examples/simple
./test_simple.sh
```

**Expected output:**
```
========================================
  Distributed Fan Control Test Rig
========================================

ℹ️  INFO: Build directory: /home/james/dev/jack/build
ℹ️  INFO: Repo root: /home/james/dev/jack

🔨 Step 1: Building...
✅ PASS: Build completed successfully

🧹 Step 2: Cleaning up old test artifacts...
🚀 Step 3: Starting ServiceNode...
✅ PASS: ServiceNode is running

🚀 Step 4: Starting AgentNode...
✅ PASS: AgentNode is running

⏱️  Step 5: Running simulation for 20 seconds...
   Progress:
   [Tick 60] Temperature: 24.61C, Fan: OFF
   [Tick 110] Temperature: 24.97C, Fan: ON  ← Fan turns on when temp > 25°C!

🛑 Step 6: Stopping processes...
✅ PASS: Processes stopped cleanly

🔍 Step 7: Analyzing Results...
========================================

Test 1: WebSocket Connection           ✅ PASS
Test 2: Peer Registration                ✅ PASS
Test 3: Bus Directory Population           ✅ PASS
Test 4: ACTION_BEGIN Transmission        ✅ PASS
Test 5: ACTION_BEGIN Reception           ✅ PASS
Test 6: Service Action Execution         ✅ PASS
Test 7: Fan State Change                 ✅ PASS
Test 8: Temperature Physics              ✅ PASS

========================================
  TEST SUMMARY
========================================
Tests Passed: 12
Tests Failed: 0

🎉 ALL TESTS PASSED!
```

---

## Behavior

The system uses hysteresis to maintain temperature:
- **Fan ON threshold**: 25°C (cooling begins)
- **Fan OFF threshold**: 23°C (heating begins)
- **Physics**:
  - Fan ON: Temperature decreases by 0.5°C per tick
  - Fan OFF: Temperature increases by 0.5°C per tick

The thermostat maintains temperature in a stable oscillation between approximately 23.5°C and 24.5°C.

## Expected Output

```
========================================
  Simple Thermostat Example
========================================
[Tick 0] Temperature: 24.5C, Fan: OFF
[FanService] Turning fan ON
[Tick 10] Temperature: 23.5C, Fan: ON
[FanService] Turning fan OFF
[Tick 20] Temperature: 24.5C, Fan: OFF
...
```

## Architecture

### Model-Driven Code Generation

The example uses JACK's model-driven approach:
1. **Model**: `simple.mod.jack.json` defines the BDI components
2. **Generation**: `jack-make` generates C++ code from the model
3. **Implementation**: Custom logic in `impl/` files
4. **Application**: `main.cpp` (standalone) or `main_agent.cpp` + `main_service.cpp` (distributed)

### Key Files

| File | Purpose |
|------|---------|
| `simple.mod.jack.json` | JACK model definition |
| `impl/services/fanserviceimpl.cpp` | Fan control action handlers |
| `impl/plans/turnfanonplanimpl.cpp` | "Turn on" precondition logic |
| `impl/plans/turnfanoffplanimpl.cpp` | "Turn off" precondition logic |
| `main.cpp` | Standalone simulation entry point |
| `main_service.cpp` | Distributed service entry point |
| `main_agent.cpp` | Distributed agent entry point |
| `test_simple.sh` | Automated integration test |

### Distributed Key Implementation Details

The distributed version demonstrates several important JACK concepts:

1. **Service Proxy Pattern**: The agent has a proxy `FanService` that forwards actions to the real service
2. **Bus Directory**: The agent discovers the `FanService` through the bus directory
3. **ACTION_BEGIN/ACTION_UPDATE Protocol**: Actions are sent as protocol events and replies routed back
4. **WebSocket Mesh Adapter**: Nodes communicate via WebSocket with automatic peer discovery

See the source files for detailed comments on the implementation.

## Code Structure

```
examples/simple/
├── simple.mod.jack.json      # Model definition
├── CMakeLists.txt            # Build configuration
├── main.cpp                  # Standalone entry point
├── main_service.cpp          # Distributed service entry point
├── main_agent.cpp            # Distributed agent entry point
├── test_simple.sh            # Automated test script ⭐
├── IMPLEMENTATION.md         # Detailed implementation notes
├── meta/                     # Generated code (auto-generated)
│   ├── simpleproject.h/cpp   # Project class
│   ├── agents/               # Agent meta classes
│   ├── services/             # Service meta classes
│   ├── plans/                # Plan meta classes
│   ├── goals/                # Goal meta classes
│   └── messages/             # Message meta classes
└── impl/                     # User implementation
    ├── services/             # Service implementations
    ├── agents/               # Agent implementations
    └── plans/                # Plan implementations
```

## Notes

- The simulation runs at 10Hz (100ms ticks)
- The agent pursues a persistent goal that continuously monitors temperature
- Plan preconditions determine which plan is selected based on current temperature
- The service decouples the agent's decisions from the actual fan hardware
- In the distributed version, the bus protocol handles transparent routing of actions between nodes

---

**⭐ Try it:** `cd examples/simple && ./test_simple.sh`
