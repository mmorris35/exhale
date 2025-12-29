/*
 * CO2 Fan Controller - Configuration
 * Edit these values before flashing
 *
 * File Name      : config.h
 * Author         : Mike Morris
 * Prerequisite   : Arduino IDE 2.x, ESP32 Board Package
 * Copyright      : (c) 2024 Mike Morris
 * License        : GNU GPL
 */

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// Network Configuration
// =============================================================================
// Leave empty for DHCP (recommended)
// Or set static IP for fixed address

#define STATIC_IP       ""          // e.g., "192.168.1.100"
#define GATEWAY         ""          // e.g., "192.168.1.1"
#define SUBNET          ""          // e.g., "255.255.255.0"
#define DNS             ""          // e.g., "8.8.8.8"

// =============================================================================
// CO2 Thresholds (ppm)
// =============================================================================
// Hysteresis: Fan turns ON above ON_THRESHOLD, OFF below OFF_THRESHOLD
// The gap prevents rapid cycling when CO2 hovers near the threshold

#define DEFAULT_CO2_ON_THRESHOLD    1000    // Fan turns ON above this
#define DEFAULT_CO2_OFF_THRESHOLD   800     // Fan turns OFF below this

// =============================================================================
// Timing
// =============================================================================

#define MEASUREMENT_INTERVAL_MS     30000   // 30 seconds between readings
#define WEB_SERVER_PORT             80      // HTTP port

// =============================================================================
// Web Server Authentication
// =============================================================================
// Basic HTTP auth for web dashboard
// CHANGE THESE before deploying!

#define WEB_USERNAME    "admin"
#define WEB_PASSWORD    "changeme"

// =============================================================================
// Pin Assignments (Olimex ESP32-POE-ISO)
// =============================================================================
// Do not change unless using different hardware

// I2C for SCD-40 sensor (UEXT connector)
#define I2C_SDA_PIN     13
#define I2C_SCL_PIN     16

// Relay control
#define RELAY_PIN       4

// Ethernet (LAN8720 PHY) - Do not change
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   12
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        0
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

#endif // CONFIG_H
