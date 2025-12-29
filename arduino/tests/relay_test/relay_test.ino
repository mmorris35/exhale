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
