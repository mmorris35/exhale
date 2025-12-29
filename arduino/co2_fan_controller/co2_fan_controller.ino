/*
 * CO2 Fan Controller - Arduino Firmware
 * Olimex ESP32-POE-ISO with SCD-40 CO2 Sensor and Relay
 *
 * File Name      : co2_fan_controller.ino
 * Author         : Mike Morris
 * Prerequisite   : Arduino IDE 2.x, ESP32 Board Package
 * Copyright      : (c) 2024 Mike Morris
 * License        : GNU GPL
 *
 * Hardware:
 *   - Olimex ESP32-POE-ISO (or ESP32-POE)
 *   - Adafruit SCD-40 CO2 Sensor (#5187)
 *   - 10A Relay Module (connected to GPIO4)
 *
 * Required Libraries (install via Arduino Library Manager):
 *   - Sensirion I2C SCD4x
 *   - ArduinoJson
 *   - Preferences (built-in)
 *   - ETH (built-in for ESP32)
 *   - WebServer (built-in for ESP32)
 *
 * Board Settings:
 *   - Board: ESP32 Dev Module (or Olimex ESP32-POE-ISO if available)
 *   - Partition Scheme: Default 4MB with spiffs
 */

#include <ETH.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>
#include <ArduinoJson.h>

// ============================================================================
// Pin Definitions for Olimex ESP32-POE-ISO
// ============================================================================
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN   12
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_ADDR        0
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18

#define I2C_SDA_PIN     13
#define I2C_SCL_PIN     16
#define RELAY_PIN       4

// ============================================================================
// Configuration Defaults
// ============================================================================
#define DEFAULT_CO2_ON_THRESHOLD    1000    // PPM to turn fan ON
#define DEFAULT_CO2_OFF_THRESHOLD   800     // PPM to turn fan OFF
#define MEASUREMENT_INTERVAL_MS     30000   // 30 seconds between readings
#define WEB_SERVER_PORT             80

// ============================================================================
// Global Objects
// ============================================================================
SensirionI2cScd4x scd4x_sensor;
WebServer web_server(WEB_SERVER_PORT);
Preferences preferences;

// ============================================================================
// Global State
// ============================================================================
struct ControllerState {
    uint16_t co2_ppm;
    float temperature_celsius;
    float humidity_percent;
    bool fan_running;
    bool auto_mode;
    uint16_t co2_on_threshold;
    uint16_t co2_off_threshold;
    bool eth_connected;
    IPAddress ip_address;
    unsigned long uptime_seconds;
    unsigned long last_measurement_ms;
    String last_error;
} state;

// ============================================================================
// Function Prototypes
// ============================================================================
void setup_ethernet();
void setup_i2c();
void setup_sensor();
void setup_relay();
void setup_web_server();
void load_configuration();
void save_configuration();
void read_sensor();
void control_fan();
void handle_root();
void handle_api_status();
void handle_api_config();
void handle_api_fan();
void handle_not_found();
String generate_html_page();

// ============================================================================
// Ethernet Event Handler
// ============================================================================
void ethernet_event(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("[ETH] Started");
            ETH.setHostname("co2-fan-controller");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("[ETH] Connected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            state.eth_connected = true;
            state.ip_address = ETH.localIP();
            Serial.print("[ETH] Got IP: ");
            Serial.println(state.ip_address);
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("[ETH] Disconnected");
            state.eth_connected = false;
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("[ETH] Stopped");
            state.eth_connected = false;
            break;
        default:
            break;
    }
}

// ============================================================================
// Setup Functions
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========================================");
    Serial.println("CO2 Fan Controller - Starting Up");
    Serial.println("========================================");

    // Initialize state
    memset(&state, 0, sizeof(state));
    state.auto_mode = true;

    load_configuration();
    setup_relay();
    setup_ethernet();
    setup_i2c();
    setup_sensor();
    setup_web_server();

    Serial.println("[INIT] Setup complete");
    Serial.print("[INIT] CO2 ON threshold: ");
    Serial.print(state.co2_on_threshold);
    Serial.println(" ppm");
    Serial.print("[INIT] CO2 OFF threshold: ");
    Serial.print(state.co2_off_threshold);
    Serial.println(" ppm");
}

void setup_ethernet() {
    Serial.println("[ETH] Initializing Ethernet...");
    WiFi.onEvent(ethernet_event);
    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE);
}

void setup_i2c() {
    Serial.println("[I2C] Initializing I2C bus...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
}

void setup_sensor() {
    Serial.println("[SCD4x] Initializing CO2 sensor...");
    
    scd4x_sensor.begin(Wire, SCD4X_I2C_ADDR_62);
    
    // Stop any existing measurement
    uint16_t error = scd4x_sensor.stopPeriodicMeasurement();
    if (error != 0) {
        Serial.print("[SCD4x] Warning: stopPeriodicMeasurement error: ");
        Serial.println(error);
    }
    delay(500);

    // Get serial number to verify sensor is connected
    uint64_t serial_number = 0;
    error = scd4x_sensor.getSerialNumber(serial_number);
    if (error != 0) {
        Serial.println("[SCD4x] ERROR: Could not read serial number!");
        state.last_error = "SCD4x sensor not found";
        return;
    }
    Serial.print("[SCD4x] Serial number: 0x");
    Serial.println((unsigned long)serial_number, HEX);

    // Start periodic measurements
    error = scd4x_sensor.startPeriodicMeasurement();
    if (error != 0) {
        Serial.print("[SCD4x] ERROR: startPeriodicMeasurement failed: ");
        Serial.println(error);
        state.last_error = "Failed to start measurements";
        return;
    }
    
    Serial.println("[SCD4x] Sensor initialized successfully");
}

void setup_relay() {
    Serial.println("[RELAY] Initializing relay...");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Start with fan OFF
    state.fan_running = false;
    Serial.println("[RELAY] Relay initialized (OFF)");
}

void setup_web_server() {
    Serial.println("[WEB] Setting up web server...");
    
    web_server.on("/", HTTP_GET, handle_root);
    web_server.on("/api/status", HTTP_GET, handle_api_status);
    web_server.on("/api/config", HTTP_GET, handle_api_config);
    web_server.on("/api/config", HTTP_POST, handle_api_config);
    web_server.on("/api/fan", HTTP_POST, handle_api_fan);
    web_server.onNotFound(handle_not_found);
    
    web_server.begin();
    Serial.println("[WEB] Web server started on port 80");
}

void load_configuration() {
    Serial.println("[CONFIG] Loading configuration...");
    preferences.begin("co2-ctrl", false);
    
    state.co2_on_threshold = preferences.getUShort("co2_on", DEFAULT_CO2_ON_THRESHOLD);
    state.co2_off_threshold = preferences.getUShort("co2_off", DEFAULT_CO2_OFF_THRESHOLD);
    state.auto_mode = preferences.getBool("auto_mode", true);
    
    preferences.end();
    Serial.println("[CONFIG] Configuration loaded");
}

void save_configuration() {
    Serial.println("[CONFIG] Saving configuration...");
    preferences.begin("co2-ctrl", false);
    
    preferences.putUShort("co2_on", state.co2_on_threshold);
    preferences.putUShort("co2_off", state.co2_off_threshold);
    preferences.putBool("auto_mode", state.auto_mode);
    
    preferences.end();
    Serial.println("[CONFIG] Configuration saved");
}

// ============================================================================
// Main Loop
// ============================================================================
void loop() {
    unsigned long current_time = millis();
    state.uptime_seconds = current_time / 1000;
    
    // Handle web requests
    web_server.handleClient();
    
    // Read sensor at configured interval
    if (current_time - state.last_measurement_ms >= MEASUREMENT_INTERVAL_MS) {
        read_sensor();
        if (state.auto_mode) {
            control_fan();
        }
        state.last_measurement_ms = current_time;
    }
    
    // Small delay to prevent CPU hogging
    delay(10);
}

void read_sensor() {
    bool data_ready = false;
    uint16_t error = scd4x_sensor.getDataReadyStatus(data_ready);
    
    if (error != 0) {
        Serial.print("[SCD4x] getDataReadyStatus error: ");
        Serial.println(error);
        return;
    }
    
    if (!data_ready) {
        Serial.println("[SCD4x] Data not ready yet");
        return;
    }
    
    uint16_t co2_raw;
    float temp_raw, humid_raw;
    
    error = scd4x_sensor.readMeasurement(co2_raw, temp_raw, humid_raw);
    if (error != 0) {
        Serial.print("[SCD4x] readMeasurement error: ");
        Serial.println(error);
        return;
    }
    
    state.co2_ppm = co2_raw;
    state.temperature_celsius = temp_raw;
    state.humidity_percent = humid_raw;
    
    Serial.print("[SCD4x] CO2: ");
    Serial.print(state.co2_ppm);
    Serial.print(" ppm | Temp: ");
    Serial.print(state.temperature_celsius, 1);
    Serial.print(" C | Humidity: ");
    Serial.print(state.humidity_percent, 1);
    Serial.println(" %");
}

void control_fan() {
    if (state.co2_ppm == 0) {
        return;  // No valid reading yet
    }
    
    // Hysteresis logic
    if (state.co2_ppm >= state.co2_on_threshold && !state.fan_running) {
        digitalWrite(RELAY_PIN, HIGH);
        state.fan_running = true;
        Serial.print("[CTRL] Fan ON - CO2 at ");
        Serial.print(state.co2_ppm);
        Serial.print(" ppm (threshold: ");
        Serial.print(state.co2_on_threshold);
        Serial.println(")");
    } else if (state.co2_ppm <= state.co2_off_threshold && state.fan_running) {
        digitalWrite(RELAY_PIN, LOW);
        state.fan_running = false;
        Serial.print("[CTRL] Fan OFF - CO2 at ");
        Serial.print(state.co2_ppm);
        Serial.print(" ppm (threshold: ");
        Serial.print(state.co2_off_threshold);
        Serial.println(")");
    }
}

// ============================================================================
// Web Server Handlers
// ============================================================================
void handle_root() {
    web_server.send(200, "text/html", generate_html_page());
}

void handle_api_status() {
    JsonDocument json_doc;
    
    json_doc["co2_ppm"] = state.co2_ppm;
    json_doc["temperature_c"] = state.temperature_celsius;
    json_doc["humidity_percent"] = state.humidity_percent;
    json_doc["fan_running"] = state.fan_running;
    json_doc["auto_mode"] = state.auto_mode;
    json_doc["co2_on_threshold"] = state.co2_on_threshold;
    json_doc["co2_off_threshold"] = state.co2_off_threshold;
    json_doc["uptime_seconds"] = state.uptime_seconds;
    json_doc["ip_address"] = state.ip_address.toString();
    json_doc["eth_connected"] = state.eth_connected;
    
    String response;
    serializeJson(json_doc, response);
    web_server.send(200, "application/json", response);
}

void handle_api_config() {
    if (web_server.method() == HTTP_POST) {
        if (web_server.hasArg("co2_on")) {
            uint16_t new_val = web_server.arg("co2_on").toInt();
            if (new_val >= 400 && new_val <= 2000) {
                state.co2_on_threshold = new_val;
            }
        }
        if (web_server.hasArg("co2_off")) {
            uint16_t new_val = web_server.arg("co2_off").toInt();
            if (new_val >= 400 && new_val <= 2000) {
                state.co2_off_threshold = new_val;
            }
        }
        if (web_server.hasArg("auto_mode")) {
            state.auto_mode = (web_server.arg("auto_mode") == "true" || 
                              web_server.arg("auto_mode") == "1");
        }
        save_configuration();
    }
    
    JsonDocument json_doc;
    json_doc["co2_on_threshold"] = state.co2_on_threshold;
    json_doc["co2_off_threshold"] = state.co2_off_threshold;
    json_doc["auto_mode"] = state.auto_mode;
    
    String response;
    serializeJson(json_doc, response);
    web_server.send(200, "application/json", response);
}

void handle_api_fan() {
    if (web_server.hasArg("state")) {
        bool requested_state = (web_server.arg("state") == "on" || 
                               web_server.arg("state") == "true" ||
                               web_server.arg("state") == "1");
        
        if (requested_state) {
            digitalWrite(RELAY_PIN, HIGH);
            state.fan_running = true;
        } else {
            digitalWrite(RELAY_PIN, LOW);
            state.fan_running = false;
        }
        
        Serial.print("[API] Fan manually set to: ");
        Serial.println(state.fan_running ? "ON" : "OFF");
    }
    
    JsonDocument json_doc;
    json_doc["fan_running"] = state.fan_running;
    
    String response;
    serializeJson(json_doc, response);
    web_server.send(200, "application/json", response);
}

void handle_not_found() {
    web_server.send(404, "text/plain", "Not found");
}

String generate_html_page() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>CO2 Fan Controller</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #eee;
            min-height: 100vh;
            padding: 20px;
        }
        .container { max-width: 600px; margin: 0 auto; }
        h1 { text-align: center; margin-bottom: 30px; color: #4ecca3; }
        .card {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            backdrop-filter: blur(10px);
        }
        .card h2 { color: #4ecca3; margin-bottom: 15px; font-size: 1.2em; }
        .reading {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        .reading:last-child { border-bottom: none; }
        .reading-label { color: #aaa; }
        .reading-value { font-size: 1.5em; font-weight: bold; }
        .co2-value { color: #4ecca3; }
        .co2-value.warning { color: #f39c12; }
        .co2-value.danger { color: #e74c3c; }
        .fan-status {
            display: inline-block;
            padding: 8px 16px;
            border-radius: 20px;
            font-weight: bold;
        }
        .fan-on { background: #27ae60; color: white; }
        .fan-off { background: #7f8c8d; color: white; }
        .threshold-input {
            display: flex;
            gap: 10px;
            margin: 10px 0;
        }
        .threshold-input label { flex: 1; }
        .threshold-input input {
            width: 100px;
            padding: 8px;
            border: none;
            border-radius: 5px;
            background: rgba(255,255,255,0.2);
            color: white;
            font-size: 1em;
        }
        button {
            background: #4ecca3;
            color: #1a1a2e;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            cursor: pointer;
            font-weight: bold;
            font-size: 1em;
            width: 100%;
            margin-top: 10px;
        }
        button:hover { background: #3db892; }
        button.secondary { background: #7f8c8d; color: white; }
        .toggle {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        .toggle-switch {
            width: 50px;
            height: 26px;
            background: #7f8c8d;
            border-radius: 13px;
            position: relative;
            cursor: pointer;
        }
        .toggle-switch.on { background: #27ae60; }
        .toggle-switch::after {
            content: '';
            position: absolute;
            width: 22px;
            height: 22px;
            background: white;
            border-radius: 50%;
            top: 2px;
            left: 2px;
            transition: 0.2s;
        }
        .toggle-switch.on::after { left: 26px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>CO2 Fan Controller</h1>
        
        <div class="card">
            <h2>Current Readings</h2>
            <div class="reading">
                <span class="reading-label">CO2 Level</span>
                <span class="reading-value co2-value" id="co2-value">-- ppm</span>
            </div>
            <div class="reading">
                <span class="reading-label">Temperature</span>
                <span class="reading-value" id="temp-value">-- C</span>
            </div>
            <div class="reading">
                <span class="reading-label">Humidity</span>
                <span class="reading-value" id="humid-value">-- %</span>
            </div>
        </div>
        
        <div class="card">
            <h2>Fan Control</h2>
            <div class="reading">
                <span class="reading-label">Fan Status</span>
                <span class="fan-status fan-off" id="fan-status">OFF</span>
            </div>
            <div class="reading">
                <span class="reading-label">Auto Mode</span>
                <div class="toggle-switch" id="auto-toggle" onclick="toggleAuto()"></div>
            </div>
            <button onclick="toggleFan()" class="secondary" id="fan-btn">Toggle Fan</button>
        </div>
        
        <div class="card">
            <h2>Thresholds</h2>
            <div class="threshold-input">
                <label>Turn ON at (ppm):</label>
                <input type="number" id="threshold-on" min="400" max="2000" step="50">
            </div>
            <div class="threshold-input">
                <label>Turn OFF at (ppm):</label>
                <input type="number" id="threshold-off" min="400" max="2000" step="50">
            </div>
            <button onclick="saveThresholds()">Save Thresholds</button>
        </div>
        
        <div class="card">
            <h2>System</h2>
            <div class="reading">
                <span class="reading-label">IP Address</span>
                <span id="ip-addr">--</span>
            </div>
            <div class="reading">
                <span class="reading-label">Uptime</span>
                <span id="uptime">--</span>
            </div>
        </div>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(data => {
                    const co2El = document.getElementById('co2-value');
                    co2El.textContent = data.co2_ppm + ' ppm';
                    co2El.className = 'reading-value co2-value';
                    if (data.co2_ppm >= 1000) co2El.classList.add('danger');
                    else if (data.co2_ppm >= 800) co2El.classList.add('warning');
                    
                    document.getElementById('temp-value').textContent = 
                        data.temperature_c.toFixed(1) + ' C';
                    document.getElementById('humid-value').textContent = 
                        data.humidity_percent.toFixed(1) + ' %';
                    
                    const fanStatus = document.getElementById('fan-status');
                    fanStatus.textContent = data.fan_running ? 'ON' : 'OFF';
                    fanStatus.className = 'fan-status ' + (data.fan_running ? 'fan-on' : 'fan-off');
                    
                    const autoToggle = document.getElementById('auto-toggle');
                    autoToggle.className = 'toggle-switch' + (data.auto_mode ? ' on' : '');
                    
                    document.getElementById('threshold-on').value = data.co2_on_threshold;
                    document.getElementById('threshold-off').value = data.co2_off_threshold;
                    document.getElementById('ip-addr').textContent = data.ip_address;
                    
                    const hours = Math.floor(data.uptime_seconds / 3600);
                    const mins = Math.floor((data.uptime_seconds % 3600) / 60);
                    document.getElementById('uptime').textContent = hours + 'h ' + mins + 'm';
                });
        }
        
        function toggleFan() {
            fetch('/api/status').then(r => r.json()).then(data => {
                fetch('/api/fan', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'state=' + (data.fan_running ? 'off' : 'on')
                }).then(() => updateStatus());
            });
        }
        
        function toggleAuto() {
            fetch('/api/status').then(r => r.json()).then(data => {
                fetch('/api/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'auto_mode=' + (!data.auto_mode)
                }).then(() => updateStatus());
            });
        }
        
        function saveThresholds() {
            const onVal = document.getElementById('threshold-on').value;
            const offVal = document.getElementById('threshold-off').value;
            fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'co2_on=' + onVal + '&co2_off=' + offVal
            }).then(() => {
                alert('Thresholds saved!');
                updateStatus();
            });
        }
        
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
)rawliteral";
    return html;
}
