#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR        0x09

// --- PIN MAPPING ---
#define PIN_XRAY_LED    5   // The "X-ray"
#define PIN_SAN_MASTER  6   // Safety Wire 1 (From Master D6/Hub)
#define PIN_SAN_GEO     4   // Safety Wire 2 (From Geometry A3)
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
    Serial.begin(9600);

    pinMode(PIN_XRAY_LED, OUTPUT);
    pinMode(PIN_SAN_MASTER, INPUT);
    pinMode(PIN_SAN_GEO, INPUT);
}

void loop() {
    // 1. READ SAFETY SWITCHES
    bool masterSafe = (digitalRead(PIN_SAN_MASTER) == HIGH);
    bool geoSafe    = (digitalRead(PIN_SAN_GEO) == HIGH);

    // 2. READ DOSE SENSOR
    int doseLevel = analogRead(PIN_LDR);

    // 3. FIRE LOGIC
    if (command == 2 && masterSafe && geoSafe) {
        digitalWrite(PIN_XRAY_LED, HIGH);

        // Print "Dose" data only while firing
        Serial.print("FIRING! Dose Rate: ");
        Serial.println(doseLevel);
    } else {
        digitalWrite(PIN_XRAY_LED, LOW);
        // Silence is golden (No debug spam)
    }
}