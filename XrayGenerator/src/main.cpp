#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR        0x09
#define PIN_XRAY_LED    5
#define PIN_SAN_MASTER  6
#define PIN_SAN_GEO     4
#define PIN_LDR         A0

volatile int command = 0;
volatile bool newData = false;

void receiveEvent(int howMany) {
    if (Wire.available()) {
        command = Wire.read();
        newData = true;
    }
}

void requestEvent() {
    if (command >= 1) {
        Wire.write(1);
    } else {
         Wire.write(0);
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    pinMode(PIN_XRAY_LED, OUTPUT);
    pinMode(PIN_SAN_MASTER, INPUT);
    pinMode(PIN_SAN_GEO, INPUT);

    Serial.println("=== XRAY STARTED ===");
}

void loop() {
    bool masterSafe = (digitalRead(PIN_SAN_MASTER) == HIGH);
    bool geoSafe    = (digitalRead(PIN_SAN_GEO) == HIGH);
    int doseLevel   = analogRead(PIN_LDR);

    // PRINT NEW COMMANDS
    if (newData) {
        Serial.print("RX CMD: "); Serial.print(command);
        Serial.print(" | SafeM: "); Serial.print(masterSafe);
        Serial.print(" | SafeG: "); Serial.println(geoSafe);
        newData = false;
    }

    if (command == 2 && masterSafe && geoSafe) {
        digitalWrite(PIN_XRAY_LED, HIGH);
        Serial.print("FIRING! Dose: "); Serial.println(doseLevel);
    } else {
        digitalWrite(PIN_XRAY_LED, LOW);
    }
}