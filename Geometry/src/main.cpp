#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

#define I2C_ADDR        0x08
#define PIN_SERVO       9
#define PIN_SAN_IN      A2
#define PIN_SAN_OUT     A3

Servo myServo;
volatile int command = 0;
volatile bool newData = false; // Flag to print cleanly

void receiveEvent(int howMany) {
    if (Wire.available()) {
        command = Wire.read();
        newData = true; // Tell loop to print this
    }
}

void requestEvent() {
    if (command >= 1) Wire.write(1); else Wire.write(0);
}

void setup() {
    Serial.begin(9600); // Enable Debugging
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    myServo.attach(PIN_SERVO);
    pinMode(PIN_SAN_IN, INPUT);
    pinMode(PIN_SAN_OUT, OUTPUT);
    digitalWrite(PIN_SAN_OUT, LOW);

    Serial.println("=== GEOMETRY STARTED ===");
}

void loop() {
    // PRINT NEW COMMANDS
    if (newData) {
        Serial.print("RX CMD: "); Serial.println(command);
        newData = false;
    }

    // SERVO LOGIC
    if (command >= 1) myServo.write(90);
    else myServo.write(0);

    // SAFETY LOGIC
    if (digitalRead(PIN_SAN_IN) == HIGH) digitalWrite(PIN_SAN_OUT, HIGH);
    else digitalWrite(PIN_SAN_OUT, LOW);

    delay(10);
}