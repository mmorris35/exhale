# CO2 Fan Controller - Development Plan

## How to Use This Plan

**For Claude Code**: Read this plan, find the subtask ID from the prompt, complete ALL checkboxes, update completion notes, commit.

**For You**: Use this prompt (change only the subtask ID):
```
please re-read CLAUDE.md and DEVELOPMENT_PLAN.md (the entire documents, for context), then continue with [X.Y.Z], following all of the development plan and claude.md rules.
```

---

## Project Overview

**Project Name**: co2-fan-controller
**Goal**: ESP32-based CO2 monitor with PoE that automatically controls a 110V ventilation fan
**Target Users**: Homeowners, grow room operators, office managers
**Timeline**: 1 week

**MVP Scope**:
- [x] CO2 monitoring with SCD-40 sensor via I2C
- [x] Configurable ON/OFF thresholds with hysteresis
- [x] Web dashboard for monitoring and configuration
- [x] 110V relay control for exhaust fan
- [x] WireGuard/Tailscale support for remote access
- [x] Persistent configuration across reboots

---

## Technology Stack

| Component | Technology |
|-----------|------------|
| **MCU** | Olimex ESP32-POE-ISO |
| **Primary Firmware** | ESPHome 2024.x |
| **Alternative Firmware** | Arduino IDE 2.x + ESP32 Core 3.x |
| **CO2 Sensor** | Adafruit SCD-40 (Sensirion I2C library) |
| **Web Server** | ESPHome WebServer / Arduino WebServer |
| **Remote Access** | ESPHome WireGuard component |
| **Config Storage** | ESPHome globals / Arduino Preferences |

---

## Progress Tracking

### Phase 0: Project Setup
- [x] 0.1.1: Initialize Git Repository
- [x] 0.1.2: ESPHome Project Structure
- [x] 0.1.3: Arduino Project Structure

### Phase 1: Hardware Validation
- [x] 1.1.1: Ethernet/PoE Connectivity (test sketch created)
- [x] 1.1.2: I2C Bus and SCD-40 Sensor (test sketch created)
- [x] 1.1.3: Relay GPIO Control (test sketch created)

### Phase 2: Core Firmware (ESPHome)
- [x] 2.1.1: Sensor Integration
- [x] 2.1.2: Hysteresis Control Logic
- [x] 2.1.3: Web Dashboard
- [x] 2.2.1: WireGuard Configuration
- [x] 2.2.2: Persistent Storage

### Phase 3: Arduino Alternative
- [x] 3.1.1: Ethernet and Web Server
- [x] 3.1.2: Sensor and Control Logic
- [x] 3.1.3: Embedded HTML Dashboard

### Phase 4: Documentation & Testing
- [x] 4.1.1: Wiring Documentation (Mermaid diagrams in README)
- [x] 4.1.2: Installation Guide (README complete)
- [ ] 4.1.3: Integration Testing (requires hardware)

**Current**: Phase 4 - Awaiting hardware testing
**Next**: 4.1.3 (Integration Testing)

---

## Phase 0: Project Setup

**Goal**: Initialize repository and project structure for both ESPHome and Arduino variants
**Duration**: 2-4 hours

### Task 0.1: Repository and Structure

**Git**: Create branch `feature/0.1-project-setup` when starting first subtask.

---

**Subtask 0.1.1: Initialize Git Repository (Single Session)**

**Prerequisites**:
- None (first subtask)

**Deliverables**:
- [ ] Run `git init` in project root
- [ ] Create `.gitignore` with ESP32/Arduino ignores
- [ ] Create base `README.md`
- [ ] Initial commit

**Complete Code**:

Create `.gitignore`:
```gitignore
# ESPHome
.esphome/
secrets.yaml

# Arduino
*.elf
*.bin
*.map
.pio/
.vscode/

# Build artifacts
build/
__pycache__/

# OS files
.DS_Store
Thumbs.db

# Secrets
*.key
*.pem
```

**Verification**:
```bash
git status
# Should show clean working tree after commit
```

**Success Criteria**:
- [ ] `git log` shows initial commit
- [ ] `.gitignore` excludes secrets.yaml
- [ ] README.md exists with project title

---

**Completion Notes**:
- **Implementation**: Git initialized, .gitignore created with ESP32/Arduino ignores, initial commit with all project files
- **Files Created**:
  - `.gitignore` - 22 lines
  - `README.md` - 258 lines (with Mermaid diagrams)
- **Branch**: main
- **Date**: 2024-12-28

---

**Subtask 0.1.2: ESPHome Project Structure (Single Session)**

**Prerequisites**:
- [x] 0.1.1: Initialize Git Repository

**Deliverables**:
- [ ] Create `esphome/` directory
- [ ] Copy `co2-fan-controller.yaml` to esphome/
- [ ] Create `esphome/secrets.yaml` from template
- [ ] Verify YAML syntax with `esphome config esphome/co2-fan-controller.yaml`

**Complete Code**:

Create `esphome/secrets.yaml`:
```yaml
# ESPHome Secrets - DO NOT COMMIT
api_encryption_key: "generate-with-esphome-wizard"
ota_password: "change-me"
web_password: "change-me"

# WireGuard (optional)
wg_private_key: "your-esp32-private-key"
wg_peer_public_key: "your-subnet-router-public-key"
wg_peer_endpoint: "100.x.x.x:51820"
```

**Verification**:
```bash
cd esphome
esphome config co2-fan-controller.yaml
# Should output parsed YAML without errors
```

**Success Criteria**:
- [ ] `esphome config` parses without errors
- [ ] secrets.yaml is git-ignored
- [ ] All substitutions resolve correctly

---

**Completion Notes**:
- **Implementation**: (describe what was done)
- **Files Created**: (list with line counts)
- **Branch**: feature/0.1-project-setup

---

**Subtask 0.1.3: Arduino Project Structure (Single Session)**

**Prerequisites**:
- [x] 0.1.2: ESPHome Project Structure

**Deliverables**:
- [ ] Verify `arduino/co2_fan_controller/` directory exists
- [ ] Create `arduino/co2_fan_controller/config.h` for user settings
- [ ] Verify sketch compiles with Arduino CLI or IDE

**Complete Code**:

Create `arduino/co2_fan_controller/config.h`:
```cpp
/*
 * CO2 Fan Controller - Configuration
 * Edit these values before flashing
 */

#ifndef CONFIG_H
#define CONFIG_H

// Network (leave empty for DHCP)
#define STATIC_IP ""
#define GATEWAY ""
#define SUBNET ""
#define DNS ""

// CO2 Thresholds (ppm)
#define DEFAULT_CO2_ON_THRESHOLD  1000
#define DEFAULT_CO2_OFF_THRESHOLD 800

// Measurement interval (milliseconds)
#define MEASUREMENT_INTERVAL_MS 30000

// Web server credentials
#define WEB_USERNAME "admin"
#define WEB_PASSWORD "changeme"

// Pin assignments (ESP32-POE-ISO)
#define I2C_SDA_PIN 13
#define I2C_SCL_PIN 16
#define RELAY_PIN   4

#endif // CONFIG_H
```

**Verification**:
```bash
# Arduino CLI
arduino-cli compile --fqbn esp32:esp32:esp32 arduino/co2_fan_controller/

# Or verify in Arduino IDE: Sketch -> Verify/Compile
```

**Success Criteria**:
- [ ] Sketch compiles without errors
- [ ] config.h is included and parsed
- [ ] All pin definitions match hardware

---

**Completion Notes**:
- **Implementation**: (describe what was done)
- **Files Created**: (list with line counts)
- **Branch**: feature/0.1-project-setup

---

### Task 0.1 Complete - Squash Merge
- [ ] All subtasks complete
- [ ] Both ESPHome and Arduino compile
- [ ] Squash merge: `git checkout main && git merge --squash feature/0.1-project-setup`
- [ ] Delete branch: `git branch -d feature/0.1-project-setup`

---

## Phase 1: Hardware Validation

**Goal**: Verify all hardware components work before writing full firmware
**Duration**: 2-4 hours

### Task 1.1: Component Testing

**Git**: Create branch `feature/1.1-hardware-validation`

---

**Subtask 1.1.1: Ethernet/PoE Connectivity (Single Session)**

**Prerequisites**:
- [x] 0.1.3: Arduino Project Structure

**Deliverables**:
- [ ] Flash minimal Ethernet test sketch
- [ ] Verify DHCP address assignment
- [ ] Verify PoE power delivery
- [ ] Test web server responds on port 80

**Complete Code**:

Create `arduino/tests/ethernet_test/ethernet_test.ino`:
```cpp
/*
 * Ethernet Test - Olimex ESP32-POE-ISO
 * Verifies PoE power and network connectivity
 */

#include <ETH.h>
#include <WebServer.h>

#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   12
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        0
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

WebServer server(80);
bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("[ETH] Started");
            ETH.setHostname("esp32-poe-test");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("[ETH] Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("[ETH] IP: ");
            Serial.println(ETH.localIP());
            eth_connected = true;
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("[ETH] Disconnected");
            eth_connected = false;
            break;
        default:
            break;
    }
}

void handleRoot() {
    String html = "<html><body>";
    html += "<h1>ESP32-POE-ISO Test</h1>";
    html += "<p>Ethernet: OK</p>";
    html += "<p>PoE Power: OK</p>";
    html += "<p>IP: " + ETH.localIP().toString() + "</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Ethernet Test ===");
    
    WiFi.onEvent(WiFiEvent);
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
    
    server.on("/", handleRoot);
    server.begin();
    Serial.println("[WEB] Server started");
}

void loop() {
    server.handleClient();
}
```

**Verification**:
```bash
# Flash and monitor serial output
# Should see IP address assigned

# Test from another machine
curl http://<ESP32_IP>/
# Should return HTML page
```

**Success Criteria**:
- [ ] Serial shows `[ETH] IP: 192.168.x.x`
- [ ] `curl http://<IP>/` returns HTML
- [ ] Device runs stable on PoE power only (no USB)

---

**Completion Notes**:
- **Implementation**: (describe what was done)
- **IP Assigned**: (record the IP)
- **Branch**: feature/1.1-hardware-validation

---

**Subtask 1.1.2: I2C Bus and SCD-40 Sensor (Single Session)**

**Prerequisites**:
- [x] 1.1.1: Ethernet/PoE Connectivity

**Deliverables**:
- [ ] Flash I2C scanner to verify sensor at 0x62
- [ ] Flash SCD-40 test to read CO2/temp/humidity
- [ ] Verify readings are sane (400-2000 ppm, 15-35C, 20-80% RH)

**Complete Code**:

Create `arduino/tests/scd40_test/scd40_test.ino`:
```cpp
/*
 * SCD-40 Sensor Test
 * Reads CO2, temperature, humidity
 */

#include <Wire.h>
#include <SensirionI2cScd4x.h>

#define I2C_SDA 13
#define I2C_SCL 16

SensirionI2cScd4x scd4x;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== SCD-40 Test ===");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    scd4x.begin(Wire, SCD4X_I2C_ADDR_62);
    
    // Stop any existing measurement
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    // Get serial number
    uint64_t serial = 0;
    uint16_t error = scd4x.getSerialNumber(serial);
    if (error) {
        Serial.println("[ERROR] Could not read serial number");
        while(1) delay(1000);
    }
    Serial.print("[SCD40] Serial: 0x");
    Serial.println((unsigned long)serial, HEX);
    
    // Start measurements
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.println("[ERROR] Could not start measurement");
        while(1) delay(1000);
    }
    Serial.println("[SCD40] Measurement started");
}

void loop() {
    delay(5000);  // SCD40 updates every 5 seconds
    
    bool dataReady = false;
    scd4x.getDataReadyStatus(dataReady);
    
    if (!dataReady) {
        Serial.println("[SCD40] Data not ready");
        return;
    }
    
    uint16_t co2;
    float temperature, humidity;
    
    uint16_t error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        Serial.println("[ERROR] Read failed");
        return;
    }
    
    Serial.print("[SCD40] CO2: ");
    Serial.print(co2);
    Serial.print(" ppm | Temp: ");
    Serial.print(temperature, 1);
    Serial.print(" C | RH: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
}
```

**Verification**:
```bash
# Monitor serial output
# Should see CO2 readings every 5 seconds
# Outdoor air ~420 ppm, indoor 600-1500 ppm
```

**Success Criteria**:
- [ ] I2C scanner finds device at 0x62
- [ ] Serial number is read successfully
- [ ] CO2 readings are in valid range (400-5000 ppm)
- [ ] Temperature readings are reasonable (10-40C)
- [ ] Humidity readings are reasonable (10-90%)

---

**Completion Notes**:
- **Implementation**: (describe what was done)
- **Sample Reading**: CO2: XXX ppm, Temp: XX.X C, RH: XX.X %
- **Branch**: feature/1.1-hardware-validation

---

**Subtask 1.1.3: Relay GPIO Control (Single Session)**

**Prerequisites**:
- [x] 1.1.2: I2C Bus and SCD-40 Sensor

**Deliverables**:
- [ ] Flash relay toggle test
- [ ] Verify relay clicks ON/OFF
- [ ] Verify GPIO4 drives relay coil
- [ ] Test with multimeter on relay contacts (NO goes to COM when activated)

**Complete Code**:

Create `arduino/tests/relay_test/relay_test.ino`:
```cpp
/*
 * Relay Test
 * Toggles relay every 2 seconds
 */

#define RELAY_PIN 4

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Relay Test ===");
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("[RELAY] Initialized OFF");
}

void loop() {
    Serial.println("[RELAY] ON");
    digitalWrite(RELAY_PIN, HIGH);
    delay(2000);
    
    Serial.println("[RELAY] OFF");
    digitalWrite(RELAY_PIN, LOW);
    delay(2000);
}
```

**Verification**:
```bash
# Listen for relay click every 2 seconds
# Use multimeter to verify:
#   - COM-NO: open when OFF, closed when ON
#   - COM-NC: closed when OFF, open when ON
```

**Success Criteria**:
- [ ] Relay clicks audibly every 2 seconds
- [ ] LED on relay module toggles
- [ ] Multimeter confirms contact switching
- [ ] No voltage on contacts when MCU powered off (fail-safe)

---

**Completion Notes**:
- **Implementation**: (describe what was done)
- **Relay Model**: (record relay module used)
- **Branch**: feature/1.1-hardware-validation

---

### Task 1.1 Complete - Squash Merge
- [ ] All hardware components verified working
- [ ] Squash merge: `git checkout main && git merge --squash feature/1.1-hardware-validation`
- [ ] Delete branch: `git branch -d feature/1.1-hardware-validation`

---

## Phase 2: Core Firmware (ESPHome)

**Goal**: Complete ESPHome-based firmware with all MVP features
**Duration**: 4-6 hours

### Task 2.1: Sensor and Control

**Git**: Create branch `feature/2.1-esphome-core`

---

**Subtask 2.1.1: Sensor Integration (Single Session)**

**Prerequisites**:
- [x] 1.1.3: Relay GPIO Control

**Deliverables**:
- [ ] Verify `scd4x` platform in ESPHome config
- [ ] Configure 30-second update interval
- [ ] Flash and verify readings in ESPHome logs
- [ ] Verify readings match standalone test

**Verification**:
```bash
esphome logs esphome/co2-fan-controller.yaml
# Should show CO2, temperature, humidity every 30 seconds
```

**Success Criteria**:
- [ ] ESPHome logs show sensor readings
- [ ] Readings update every 30 seconds
- [ ] Values match standalone Arduino test within 5%

---

**Subtask 2.1.2: Hysteresis Control Logic (Single Session)**

**Prerequisites**:
- [x] 2.1.1: Sensor Integration

**Deliverables**:
- [ ] Verify lambda in `on_value` trigger
- [ ] Test with threshold set low (fan should turn ON)
- [ ] Test with threshold set high (fan should turn OFF)
- [ ] Verify fan doesn't cycle rapidly at threshold boundary

**Verification**:
```bash
# Set threshold low via web UI (e.g., ON at 500 ppm)
# Fan should turn ON immediately
# Set threshold high (e.g., OFF at 2000 ppm)
# Fan should turn OFF immediately
```

**Success Criteria**:
- [ ] Fan turns ON when CO2 > ON threshold
- [ ] Fan stays ON until CO2 < OFF threshold
- [ ] Fan does not cycle when CO2 is between thresholds
- [ ] Manual override works via web UI

---

**Subtask 2.1.3: Web Dashboard (Single Session)**

**Prerequisites**:
- [x] 2.1.2: Hysteresis Control Logic

**Deliverables**:
- [ ] Access web UI at device IP
- [ ] Verify sensor readings display
- [ ] Verify threshold sliders work
- [ ] Verify fan toggle switch works
- [ ] Verify authentication prompt appears

**Verification**:
```bash
curl -u admin:password http://<IP>/
# Should return HTML dashboard
```

**Success Criteria**:
- [ ] Dashboard loads in <2 seconds
- [ ] All sensor values display correctly
- [ ] Threshold changes take effect immediately
- [ ] Fan can be toggled manually
- [ ] Basic auth protects the interface

---

### Task 2.1 Complete - Squash Merge
- [ ] All subtasks complete
- [ ] Squash merge: `git checkout main && git merge --squash feature/2.1-esphome-core`
- [ ] Delete branch

---

### Task 2.2: Remote Access and Storage

**Git**: Create branch `feature/2.2-esphome-remote`

---

**Subtask 2.2.1: WireGuard Configuration (Single Session)**

**Prerequisites**:
- [x] 2.1.3: Web Dashboard

**Deliverables**:
- [ ] Generate WireGuard keypair for ESP32
- [ ] Configure subnet router (see docs/TAILNET_SETUP.md)
- [ ] Add keys to secrets.yaml
- [ ] Verify tunnel establishes on boot

**Verification**:
```bash
# On subnet router
sudo wg show
# Should show ESP32 peer with recent handshake

# From any Tailscale device
curl http://10.13.13.10/api/status
# Should return JSON
```

**Success Criteria**:
- [ ] WireGuard handshake succeeds within 30 seconds of boot
- [ ] Device reachable from Tailnet at 10.13.13.10
- [ ] Web UI accessible over WireGuard tunnel

---

**Subtask 2.2.2: Persistent Storage (Single Session)**

**Prerequisites**:
- [x] 2.2.1: WireGuard Configuration

**Deliverables**:
- [ ] Verify `restore_value: true` on number inputs
- [ ] Change thresholds via web UI
- [ ] Reboot device (power cycle)
- [ ] Verify thresholds persist after reboot

**Verification**:
```bash
# Set threshold to non-default value
# Power cycle the device
# Check threshold value after boot
```

**Success Criteria**:
- [ ] Custom thresholds persist across reboot
- [ ] Auto mode setting persists
- [ ] No corruption after multiple power cycles

---

### Task 2.2 Complete - Squash Merge
- [ ] All subtasks complete
- [ ] Squash merge: `git checkout main && git merge --squash feature/2.2-esphome-remote`
- [ ] Delete branch

---

## Phase 3: Arduino Alternative

**Goal**: Complete standalone Arduino firmware for users who prefer Arduino IDE
**Duration**: 2-4 hours

### Task 3.1: Arduino Implementation

**Git**: Create branch `feature/3.1-arduino-firmware`

---

**Subtask 3.1.1: Ethernet and Web Server (Single Session)**

**Prerequisites**:
- [x] 2.2.2: Persistent Storage

**Deliverables**:
- [ ] Verify ethernet initialization in main sketch
- [ ] Verify web server handles all routes
- [ ] Test /api/status returns JSON
- [ ] Test /api/config POST updates settings

**Success Criteria**:
- [ ] Device gets IP via DHCP
- [ ] All API endpoints respond correctly
- [ ] JSON responses are valid

---

**Subtask 3.1.2: Sensor and Control Logic (Single Session)**

**Prerequisites**:
- [x] 3.1.1: Ethernet and Web Server

**Deliverables**:
- [ ] Verify sensor reading in main loop
- [ ] Verify control_fan() hysteresis logic
- [ ] Test threshold changes take effect
- [ ] Verify settings persist via Preferences

**Success Criteria**:
- [ ] Sensor readings match ESPHome variant
- [ ] Fan control matches ESPHome behavior
- [ ] Settings survive reboot

---

**Subtask 3.1.3: Embedded HTML Dashboard (Single Session)**

**Prerequisites**:
- [x] 3.1.2: Sensor and Control Logic

**Deliverables**:
- [ ] Verify HTML dashboard renders correctly
- [ ] Test JavaScript polling updates values
- [ ] Test threshold form submission
- [ ] Test fan toggle button

**Success Criteria**:
- [ ] Dashboard displays in all modern browsers
- [ ] Values update via AJAX without page reload
- [ ] UI is responsive on mobile devices

---

### Task 3.1 Complete - Squash Merge
- [ ] All subtasks complete
- [ ] Squash merge: `git checkout main && git merge --squash feature/3.1-arduino-firmware`
- [ ] Delete branch

---

## Phase 4: Documentation & Testing

**Goal**: Complete documentation and verify all features work end-to-end
**Duration**: 2-4 hours

### Task 4.1: Final Documentation

**Git**: Create branch `feature/4.1-documentation`

---

**Subtask 4.1.1: Wiring Documentation (Single Session)**

**Prerequisites**:
- [x] 3.1.3: Embedded HTML Dashboard

**Deliverables**:
- [ ] Verify QUICK_REFERENCE.txt is accurate
- [ ] Add photos of completed wiring (optional)
- [ ] Review README wiring diagram
- [ ] Add troubleshooting for common wiring mistakes

**Success Criteria**:
- [ ] Pin assignments match actual hardware
- [ ] Wiring diagram is clear and accurate
- [ ] Safety warnings are prominent

---

**Subtask 4.1.2: Installation Guide (Single Session)**

**Prerequisites**:
- [x] 4.1.1: Wiring Documentation

**Deliverables**:
- [ ] Write step-by-step ESPHome installation
- [ ] Write step-by-step Arduino installation
- [ ] Document first-boot configuration
- [ ] Document Tailnet setup reference

**Success Criteria**:
- [ ] New user can follow guide from scratch
- [ ] All commands are copy-paste ready
- [ ] Common errors are documented

---

**Subtask 4.1.3: Integration Testing (Single Session)**

**Prerequisites**:
- [x] 4.1.2: Installation Guide

**Deliverables**:
- [ ] Test ESPHome firmware end-to-end
- [ ] Test Arduino firmware end-to-end
- [ ] Test WireGuard/Tailnet access
- [ ] Document any bugs found

**Test Checklist**:
- [ ] Fresh flash installs correctly
- [ ] Ethernet gets IP within 30 seconds
- [ ] Sensor readings appear within 60 seconds
- [ ] Web dashboard loads and functions
- [ ] Threshold changes take effect
- [ ] Fan toggles on threshold crossing
- [ ] Settings persist after reboot
- [ ] WireGuard tunnel establishes (if configured)

**Success Criteria**:
- [ ] All test checklist items pass
- [ ] No critical bugs remaining
- [ ] Documentation matches actual behavior

---

### Task 4.1 Complete - Squash Merge
- [ ] All subtasks complete
- [ ] Squash merge: `git checkout main && git merge --squash feature/4.1-documentation`
- [ ] Delete branch

---

## Git Workflow

### Branch Strategy
- **ONE branch per TASK** (e.g., `feature/2.1-esphome-core`)
- Create branch when starting first subtask
- Commit after each subtask
- Squash merge when task complete

### Commit Messages
- Format: `type(scope): description`
- Types: `feat`, `fix`, `docs`, `test`, `chore`
- Example: `feat(sensor): add SCD-40 I2C integration`

---

## Ready to Build

Each subtask is detailed enough for Claude Code to execute. Start with:

```
Please read CLAUDE.md and DEVELOPMENT_PLAN.md completely, then implement subtask [0.1.1], following all rules and marking checkboxes as you complete each item.
```

---

*Generated for DevPlan methodology*
