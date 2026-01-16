#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

#define I2C_ADDR        0x08

// --- SHIELD PIN MAPPINGS ---
// The Joystick is typically on A0 (X-Axis) and A1 (Y-Axis)
#define PIN_JOYSTICK    A0  
// The large buttons on the shield usually map to D2-D6. Button 'A' is usually D2.
#define PIN_BTN_FIXED   2   

// --- CUSTOM PIN MAPPINGS ---
// We moved these to D9-D11 to avoid conflicts with the Shield's buttons (D2-D6)
#define PIN_SERVO       9   // Servo Signal Pin
#define PIN_SAN_IN      10  // Safety Input from Master (D6 -> D10)
#define PIN_SAN_OUT     11  // Safety Output to Xray (D11 -> D4)

Servo myServo;
volatile uint8_t command = 0; 

// Function Prototypes
void receiveEvent(int howMany);
void requestEvent();

void setup() {
    // 1. Setup I2C
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    
    // 2. Setup Servo
    myServo.attach(PIN_SERVO);
    
    // 3. Setup Pins
    // Shield buttons are often active LOW (read 0 when pressed).
    pinMode(PIN_BTN_FIXED, INPUT_PULLUP); 
    
    pinMode(PIN_SAN_IN, INPUT);
    pinMode(PIN_SAN_OUT, OUTPUT);
    
    Serial.begin(9600);
    Serial.println("GEOMETRY: Started");
}

void loop() {
    // --- 1. SERVO LOGIC ---
    int joyVal = analogRead(PIN_JOYSTICK); // Reads 0 to 1023
    
    // Map Joystick value to Servo Angle (0 to 180 degrees)
    int angle = map(joyVal, 0, 1023, 0, 180);
    
    // Feature: "Fixed Position" Button
    // If shield button is pressed, force arm to center (90 degrees)
    if (digitalRead(PIN_BTN_FIXED) == LOW) { 
        angle = 90; 
    }
    
    myServo.write(angle);
    
    // --- 2. SAFETY LOGIC (SAN) ---
    // Rule: If the arm is not in a stable position, tell X-ray to STOP.
    // We define "Stable" as being roughly centered (e.g., 85 to 95 degrees).
    // If angle is outside this range, we send HIGH ("I am moving/unsafe").
    
    if (angle < 85 || angle > 95) {
        digitalWrite(PIN_SAN_OUT, HIGH); // Signal: "UNSAFE / MOVING"
    } else {
        digitalWrite(PIN_SAN_OUT, LOW);  // Signal: "SAFE / STABLE"
    }

    delay(15); // Small delay for servo mechanics
}

// --- I2C EVENT HANDLERS ---

// Called when Master sends data (e.g., "Prepare" or "Idle")
void receiveEvent(int howMany) {
    if (Wire.available()) {
        command = Wire.read();
    }
}

// Called when Master asks "Are you ready?"
void requestEvent() {
    // We are ready if we are in the "Prepare" state (Command 1)
    if (command == 1) {
        Wire.write(1); // Yes, Ready
    } else {
        Wire.write(0); // No
    }
}