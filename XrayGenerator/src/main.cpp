#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR        0x09

// --- PIN MAPPING ---
#define PIN_XRAY_LED    5   // The "X-ray"
#define PIN_SAN_MASTER  6   // Safety Wire 1 (From Master)
#define PIN_SAN_GEO     4   // Safety Wire 2 (From Geometry)
#define PIN_LDR         A0  // Light Sensor

volatile int command = 0;

void receiveEvent(int howMany) {
    if (Wire.available()) command = Wire.read();
}

void requestEvent() {
    if (command >= 1) Wire.write(1); // Ready
    else Wire.write(0);
}

void setup() {
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    Serial.begin(9600); // For viewing LDR data

    pinMode(PIN_XRAY_LED, OUTPUT);
    pinMode(PIN_SAN_MASTER, INPUT);
    pinMode(PIN_SAN_GEO, INPUT);
}

void loop() {
    // 1. READ SAFETY SWITCHES
    bool masterSafe = (digitalRead(PIN_SAN_MASTER) == HIGH); // High = Safe
    bool geoSafe    = (digitalRead(PIN_SAN_GEO) == HIGH);    // High = Safe (Active)

    // 2. READ DOSE SENSOR (LDR)
    int doseLevel = analogRead(PIN_LDR);

    // 3. FIRE LOGIC
    // Only fire if Command is PULSE (2) AND both safety lines are Active
    if (command == 2 && masterSafe && geoSafe) {
        digitalWrite(PIN_XRAY_LED, HIGH);

        // Print "Dose" data while firing
        Serial.print("FIRING! Dose Rate: ");
        Serial.println(doseLevel);
    } else {
        digitalWrite(PIN_XRAY_LED, LOW);

        // Just print sensor data occasionally to test connection
        if (millis() % 500 == 0) {
             Serial.print("Idle Sensor: ");
             Serial.println(doseLevel);
        }
    }
}