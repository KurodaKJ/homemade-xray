#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR        0x09

// PINS
#define PIN_XRAY_LED    5
#define PIN_SAN_MASTER  6   // Input from Master
#define PIN_SAN_GEO     4   // Input from Geometry
#define PIN_LDR         A0

volatile int command = 0;

void receiveEvent(int howMany) {
    if (Wire.available()) command = Wire.read();
}

void requestEvent() {
    if (command >= 1) Wire.write(1); else Wire.write(0);
}

void setup() {
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    Serial.begin(9600);

    pinMode(PIN_XRAY_LED, OUTPUT);
    pinMode(PIN_SAN_MASTER, INPUT);
    pinMode(PIN_SAN_GEO, INPUT);
}

void loop() {
    // READ STATUS
    bool masterSafe = (digitalRead(PIN_SAN_MASTER) == HIGH);
    bool geoSafe    = (digitalRead(PIN_SAN_GEO) == HIGH); // Assuming High = Safe
    int doseLevel   = analogRead(PIN_LDR);

    // LOGIC
    if (command == 2 && masterSafe && geoSafe) {
        digitalWrite(PIN_XRAY_LED, HIGH);
        Serial.println("STATUS: FIRING !!!");
    } else {
        digitalWrite(PIN_XRAY_LED, LOW);

        // PRINT DIAGNOSTICS (Every 0.5s)
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 500) {
            lastPrint = millis();
            Serial.print("Cmd: "); Serial.print(command);
            Serial.print(" | MasterSafe(D6): "); Serial.print(masterSafe ? "OK (HIGH)" : "FAIL (LOW)");
            Serial.print(" | GeoSafe(D4): "); Serial.println(geoSafe ? "OK (HIGH)" : "FAIL (LOW)");
        }
    }
}