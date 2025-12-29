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
