#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

#define I2C_ADDR        0x08

// --- PIN MAPPING (MATCHING YOUR CHECKLIST) ---
#define PIN_SERVO       9   // Servo Motor
#define PIN_SAN_IN      A2  // Safety Input (From Master D6)
#define PIN_SAN_OUT     A3  // Safety Output (To Xray D4)

Servo myServo;
volatile int command = 0;

void receiveEvent(int howMany) {
    if (Wire.available()) command = Wire.read();
}

void requestEvent() {
    if (command >= 1) Wire.write(1); // Tell Master we are Ready
    else Wire.write(0);
}

void setup() {
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    myServo.attach(PIN_SERVO);

    // IMPORTANT: Configuring Analog Pins as Digital
    pinMode(PIN_SAN_IN, INPUT);
    pinMode(PIN_SAN_OUT, OUTPUT);
    digitalWrite(PIN_SAN_OUT, LOW); // Default to Safe
}

void loop() {
    // 1. SERVO LOGIC
    if (command >= 1) {
        myServo.write(90); // Move to 90 if Prepared or Firing
    } else {
        myServo.write(0);  // Return to 0 if Idle
    }

    // 2. SAFETY LOGIC (Pass-through)
    // Here we read the signal from Master (A2) and decide what to send to Xray (A3)
    // For now, we just pass it through. Later you can add Joystick logic here.
    if (digitalRead(PIN_SAN_IN) == HIGH) {
        // If Master says "Safety ON", we enable our output (unless joystick is unsafe)
        digitalWrite(PIN_SAN_OUT, HIGH);
    } else {
        digitalWrite(PIN_SAN_OUT, LOW);
    }

    delay(10);
}