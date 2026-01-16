#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR        0x09

// --- HARDWARE PINS ---
#define PIN_XRAY_LED    5   // Simulates the X-ray tube
#define PIN_LDR         A0  // Simulates the Dose Sensor

// --- SAFETY PINS (SAN) ---
#define PIN_SAN_MASTER  6   // Input from Master (D6 -> D6)
#define PIN_SAN_GEO     4   // Input from Geometry (D11 -> D4)

volatile uint8_t command = 0; // 0=Idle, 1=Prepare, 2=Pulse

// Function Prototypes
void receiveEvent(int howMany);
void requestEvent();

void setup() {
    // 1. Setup I2C
    Wire.begin(I2C_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    
    // 2. Setup Pins
    pinMode(PIN_XRAY_LED, OUTPUT);
    
    // Inputs for Safety Lines
    pinMode(PIN_SAN_MASTER, INPUT);
    pinMode(PIN_SAN_GEO, INPUT);
    
    Serial.begin(9600);
    Serial.println("XRAY: Generator Started");
}

void loop() {
    // --- 1. SAFETY CHECKS ---
    // Read the SAN (Synchronization Area Network) lines
    
    // Master must be sending HIGH ("I authorize X-ray")
    bool masterSaysShoot = (digitalRead(PIN_SAN_MASTER) == HIGH);
    
    // Geometry must be sending LOW ("I am stable/safe")
    // If Geometry sends HIGH, it means it is moving/unsafe.
    bool geoIsUnsafe = (digitalRead(PIN_SAN_GEO) == HIGH);
    
    // --- 2. FIRING LOGIC ---
    // We only fire if ALL conditions are met:
    // A. We received the PULSE command (2) via I2C
    // B. The Master Safety Line is Active (HIGH)
    // C. The Geometry Safety Line is Inactive (LOW)
    
    if (command == 2 && masterSaysShoot && !geoIsUnsafe) {
        
        digitalWrite(PIN_XRAY_LED, HIGH); // Fire X-ray!
        
        // Measure Dose while firing
        int dose = analogRead(PIN_LDR);
        
        // (Optional Debugging - Warning: Slows down loop slightly)
        // Serial.print("Firing! Dose: ");
        // Serial.println(dose);
        
    } else {
        // If ANY condition fails, immediate shutdown
        digitalWrite(PIN_XRAY_LED, LOW);
    }
    
    delay(10); // Small delay for stability
}

// --- I2C EVENT HANDLERS ---

// Called when Master sends data
void receiveEvent(int howMany) {
    if (Wire.available()) {
        command = Wire.read();
    }
}

// Called when Master asks "Are you ready?"
void requestEvent() {
    // We are ready if we are in "Prepare" mode (Command 1) or "Pulse" mode (Command 2)
    if (command >= 1) {
        Wire.write(1); // Ready
    } else {
        Wire.write(0); // Not Ready
    }
}