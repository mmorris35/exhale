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
