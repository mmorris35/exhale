# CLAUDE.md - Project Rules for co2-fan-controller

> This document defines HOW Claude Code should work on co2-fan-controller.
> Read at the start of every session to maintain consistency.

## Project Overview

**Type**: Embedded Firmware (ESP32)
**Goal**: CO2 monitor with PoE that controls a 110V ventilation fan
**Hardware**: Olimex ESP32-POE-ISO + Adafruit SCD-40 + 10A Relay

## Quick Reference

| Component | Technology |
|-----------|------------|
| **MCU** | ESP32 (Olimex ESP32-POE-ISO) |
| **Primary Firmware** | ESPHome 2024.x (YAML + lambdas) |
| **Alternative Firmware** | Arduino IDE 2.x + ESP32 Core 3.x |
| **Sensor Library** | Sensirion I2C SCD4x |
| **Network** | Ethernet (LAN8720 PHY) |
| **Remote Access** | WireGuard |

## Directory Structure

```
co2-fan-controller/
├── esphome/
│   ├── co2-fan-controller.yaml   # Main ESPHome config
│   ├── secrets.yaml              # Credentials (git-ignored)
│   └── secrets.yaml.template     # Template for secrets
├── arduino/
│   ├── co2_fan_controller/
│   │   ├── co2_fan_controller.ino  # Main Arduino sketch
│   │   └── config.h                # User configuration
│   └── tests/                      # Hardware test sketches
│       ├── ethernet_test/
│       ├── scd40_test/
│       └── relay_test/
├── docs/
│   ├── QUICK_REFERENCE.txt       # Printable wiring card
│   └── TAILNET_SETUP.md          # WireGuard/Tailscale guide
├── PROJECT_BRIEF.md              # Requirements
├── DEVELOPMENT_PLAN.md           # This development roadmap
├── CLAUDE.md                     # This file
└── README.md                     # User documentation
```

## Commands

| Command | Purpose |
|---------|---------|
| `esphome config esphome/co2-fan-controller.yaml` | Validate ESPHome YAML |
| `esphome run esphome/co2-fan-controller.yaml` | Build and flash ESPHome |
| `esphome logs esphome/co2-fan-controller.yaml` | Monitor ESPHome logs |
| `arduino-cli compile --fqbn esp32:esp32:esp32 arduino/co2_fan_controller/` | Compile Arduino |
| `arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 arduino/co2_fan_controller/` | Flash Arduino |

## Hardware Pin Assignments

| Function | GPIO | Notes |
|----------|------|-------|
| I2C SDA | GPIO13 | UEXT connector |
| I2C SCL | GPIO16 | UEXT connector |
| Relay Control | GPIO4 | Active HIGH |
| ETH MDC | GPIO23 | LAN8720 |
| ETH MDIO | GPIO18 | LAN8720 |
| ETH CLK | GPIO17 | Clock output |
| ETH Power | GPIO12 | PHY enable |

## Core Operating Principles

### 1. Single Session Execution
- Complete the ENTIRE subtask in one session
- End every session with a git commit
- If blocked, document why in completion notes

### 2. Read Before Acting
Every session must begin with:
1. Read DEVELOPMENT_PLAN.md completely
2. Locate the specific subtask ID from the prompt
3. Verify prerequisites are marked `[x]` complete
4. Read completion notes from prerequisites for context

### 3. Firmware Development Rules

**ESPHome:**
- Always validate with `esphome config` before flashing
- Use lambdas sparingly - prefer native ESPHome components
- Put secrets in `secrets.yaml`, never commit them
- Use `!secret` references in main config

**Arduino:**
- Include all required headers at top of file
- Use `#define` for pin assignments in config.h
- Initialize Serial at 115200 baud
- Add error handling for all I2C operations
- Use Preferences library for persistent storage

### 4. Testing Protocol

**Hardware Testing** (no unit tests for firmware):
- Flash test sketches from `arduino/tests/`
- Verify each component works in isolation
- Document readings and observations in completion notes

**Integration Testing:**
- Test full firmware end-to-end
- Verify all features from PROJECT_BRIEF.md
- Document any deviations or issues

### 5. Completion Protocol

When a subtask is complete:

1. **Update DEVELOPMENT_PLAN.md** with completion notes:
```markdown
**Completion Notes:**
- **Implementation**: Brief description of what was done
- **Files Created**:
  - `esphome/co2-fan-controller.yaml` - 150 lines
- **Files Modified**:
  - `README.md` - added installation section
- **Hardware Test Results**: (if applicable)
  - CO2 reading: 847 ppm
  - Temperature: 22.3 C
  - Relay: clicks confirmed
- **Build**: esphome compile: PASS
- **Branch**: feature/X.Y-description
```

2. **Check all checkboxes** in the subtask

3. **Git commit** with semantic message:
```bash
git add .
git commit -m "feat(sensor): add SCD-40 integration

- Configure I2C on GPIO13/16
- Add 30-second measurement interval
- Test verified: 847 ppm @ 22.3C"
```

### 6. Error Handling

If you encounter an error:
1. Check serial monitor for error messages
2. Verify wiring matches pin assignments
3. Check I2C address (SCD-40 is 0x62)
4. If blocked, document in completion notes with:
   - Error message
   - What was attempted
   - Suggested fix

### 7. Safety Standards

**Relay Control:**
- Default to OFF on boot (`restore_mode: ALWAYS_OFF`)
- Add watchdog timer for firmware hangs
- Test fail-safe behavior (relay should be OFF when MCU unpowered)

**110V AC:**
- Never work on AC wiring while powered
- Keep low-voltage and high-voltage separated
- Document all safety considerations

### 8. Code Style

**ESPHome YAML:**
```yaml
# Use 2-space indentation
sensor:
  - platform: scd4x
    co2:
      name: "CO2"
      id: co2_sensor
```

**Arduino C++:**
```cpp
// Use descriptive variable names (no single letters)
uint16_t co2_reading = 0;
float temperature_celsius = 0.0;

// Add comments for non-obvious code
// Hysteresis: ON at 1000, OFF at 800 prevents rapid cycling
if (co2_reading >= threshold_on && !fan_running) {
    activate_fan();
}
```

### 9. Git Conventions

**Branch Naming:**
- `feature/X.Y-description` for task branches
- Example: `feature/2.1-esphome-core`

**Commit Messages:**
- Format: `type(scope): description`
- Types: `feat`, `fix`, `docs`, `test`, `chore`
- Example: `feat(relay): add hysteresis control logic`

### 10. Build Verification

**Before marking subtask complete:**

For ESPHome:
```bash
esphome config esphome/co2-fan-controller.yaml  # Must pass
```

For Arduino:
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 arduino/co2_fan_controller/
# Must compile without errors
```

## Session Checklists

### Starting a Session
- [ ] Read CLAUDE.md (this file)
- [ ] Read DEVELOPMENT_PLAN.md completely
- [ ] Locate subtask ID from prompt
- [ ] Verify prerequisites marked `[x]`
- [ ] Checkout correct branch (or create if first subtask of task)
- [ ] Ready to implement!

### Ending a Session
- [ ] All subtask deliverables complete
- [ ] All checkboxes marked `[x]`
- [ ] Firmware compiles without errors
- [ ] Hardware tested (if applicable)
- [ ] Completion notes written in DEVELOPMENT_PLAN.md
- [ ] Git commit with semantic message
- [ ] Report completion to user

---

**Version**: 1.0
**Last Updated**: 2024-12-29
**Project**: co2-fan-controller

*For Claude Code - Read this file at the start of every session*
