# Project Brief: CO2 Fan Controller

## Overview

| Field | Value |
|-------|-------|
| **Project Name** | co2-fan-controller |
| **Project Type** | Embedded Firmware (ESP32) |
| **Goal** | ESP32-based CO2 monitor with PoE that automatically controls a 110V ventilation fan when air quality drops below configurable thresholds |
| **Timeline** | 1 week |
| **Team Size** | 1 |

## Target Users

- Homeowners monitoring indoor air quality
- Grow room / greenhouse operators
- Office managers
- Home automation enthusiasts

## Features

### Must-Have (MVP)

1. **CO2 Monitoring** - Read CO2, temperature, humidity from SCD-40 sensor via I2C every 30 seconds
2. **Hysteresis Control** - Configurable ON/OFF thresholds (default 1000/800 ppm) to prevent relay cycling
3. **Web Dashboard** - Real-time readings, threshold configuration, manual fan override
4. **Relay Control** - GPIO-driven relay for 110V AC fan (10A capacity)
5. **WireGuard/Tailscale** - Secure remote access via subnet routing
6. **Persistent Config** - Thresholds and settings survive reboots (NVS/Preferences)

### Nice-to-Have (v2)

- MQTT publishing for Home Assistant autodiscovery
- Historical data logging to SD card
- Email/SMS alerts for sustained high CO2
- Multi-zone support with multiple sensors

## Technical Requirements

### Hardware

| Component | Specification |
|-----------|---------------|
| MCU | Olimex ESP32-POE-ISO |
| CO2 Sensor | Adafruit SCD-40 (I2C, 0x62) |
| Relay | 10A @ 120VAC, 3.3V logic |
| Power | IEEE 802.3af PoE |

### Firmware Stack

| Component | Technology |
|-----------|------------|
| Primary | ESPHome (YAML + C++) |
| Alternative | Arduino (C++) |
| Network | Ethernet (LAN8720 PHY) |
| Remote Access | WireGuard |

### Pin Assignments (ESP32-POE-ISO)

| Function | GPIO |
|----------|------|
| I2C SDA | GPIO13 |
| I2C SCL | GPIO16 |
| Relay Control | GPIO4 |
| ETH MDC | GPIO23 |
| ETH MDIO | GPIO18 |
| ETH CLK | GPIO17 |
| ETH Power | GPIO12 |

### Constraints

- Must work offline (no cloud dependency)
- Web UI must function without internet
- Single PoE cable deployment (no separate power)
- Galvanic isolation required (110V AC in same enclosure)

## Success Criteria

1. CO2 readings update every 30 seconds with <5% error
2. Fan activates within 5 seconds of crossing ON threshold
3. Fan deactivates within 5 seconds of crossing OFF threshold
4. Web dashboard loads in <2 seconds on LAN
5. Configuration persists across power cycles
6. WireGuard tunnel establishes within 30 seconds of boot
7. Device accessible from any Tailscale node

## Out of Scope

- WiFi connectivity (Ethernet/PoE only for MVP)
- Battery operation
- Multiple relay channels
- Cloud data storage
- Mobile app

## Safety Considerations

- Relay must default to OFF on boot
- Watchdog timer must reset device if firmware hangs
- 110V wiring must be physically separated from low-voltage
- Enclosure must be rated for AC voltage

---

*Generated for DevPlan methodology*
