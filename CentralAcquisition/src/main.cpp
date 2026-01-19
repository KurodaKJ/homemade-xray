#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include "Protocol_PatientAdmin_CentralAcq.h"
#include "config.h"

// Global variable
static CENTRAL_ACQ_STATES centralAcqState = STATE_DISCONNECTED;
static CONNECTED_SUBSTATES connectedSubState = SUBSTATE_IDLE;

// Functions prototypes
static EVENTS getEvent();
static void handleEvent(EVENTS event);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);
void sendCommand(uint8_t address, uint8_t cmd);
bool isSlaveReady(uint8_t address);

void runConnectedStateMachine(EVENTS event) {
    static unsigned long preparingTimer = 0;

    switch (connectedSubState) {
        case SUBSTATE_IDLE:
            digitalWrite(PIN_SAN_ENABLE, LOW);

            if (digitalRead(PIN_BTN_PREPARE) == LOW || event == EV_EXAM) {
                Serial.println(">>> LOGIC: Trigger Received (Button or PC)");

                sendCommand(ADDR_GEOMETRY, CMD_PREPARE);
                sendCommand(ADDR_XRAY, CMD_PREPARE);

                preparingTimer = millis();
                connectedSubState = SUBSTATE_PREPARING;
                Serial.println(">>> STATE: IDLE -> PREPARING");
            }
            break;

        case SUBSTATE_PREPARING:
            if (isSlaveReady(ADDR_GEOMETRY) && isSlaveReady(ADDR_XRAY)) {
                connectedSubState = SUBSTATE_PREPARED;
                digitalWrite(PIN_LED_PREPARED, HIGH);
                Serial.println(">>> STATE: PREPARING -> PREPARED");
            } else if (millis() - preparingTimer > 5000) {
                Serial.println("!!! ERROR: Prepare Timeout");
                sendCommand(ADDR_GEOMETRY, CMD_IDLE);
                sendCommand(ADDR_XRAY, CMD_IDLE);
                connectedSubState = SUBSTATE_IDLE;
                Serial.println(">>> STATE: PREPARING -> IDLE");
            }
            break;

        case SUBSTATE_PREPARED:
            if (digitalRead(PIN_BTN_XRAY) == LOW) {
                connectedSubState = SUBSTATE_ACQUIRING;
                digitalWrite(PIN_LED_PREPARED, LOW);
                digitalWrite(PIN_LED_ACQUIRING, HIGH);
                digitalWrite(PIN_SAN_ENABLE, HIGH);

                Serial.println(">>> LOGIC: X-Ray Button Pressed");
                sendCommand(ADDR_XRAY, CMD_PULSE);
                Serial.println(">>> STATE: PREPARED -> ACQUIRING");
            }
            break;

        case SUBSTATE_ACQUIRING:
            if (digitalRead(PIN_BTN_XRAY) == HIGH) {
                digitalWrite(PIN_LED_ACQUIRING, LOW);
                digitalWrite(PIN_SAN_ENABLE, LOW);

                Serial.println(">>> LOGIC: X-Ray Button Released");
                sendCommand(ADDR_GEOMETRY, CMD_IDLE);
                sendCommand(ADDR_XRAY, CMD_IDLE);

                Serial.println(">>> LOGIC: Retrieving Dose...");
                sendCommand(ADDR_XRAY, CMD_READ_DOSE);
                Wire.requestFrom(ADDR_XRAY, 4);
                unsigned long dose = 0;
                if (Wire.available() == 4) {
                    Wire.readBytes((char*)&dose, 4);
                }

                char doseMsg[32];
                sprintf(doseMsg, "DOSE:%lu", dose);
                writeMsgToSerialPort(doseMsg);

                Serial.print(">>> TX PC: "); Serial.println(doseMsg);

                connectedSubState = SUBSTATE_IDLE;
                Serial.println(">>> STATE: ACQUIRING -> IDLE");
            }
            break;
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin();
    pinMode(PIN_BTN_PREPARE, INPUT_PULLUP);
    pinMode(PIN_BTN_XRAY, INPUT_PULLUP);
    pinMode(PIN_LED_PREPARED, OUTPUT);
    pinMode(PIN_LED_ACQUIRING, OUTPUT);
    pinMode(PIN_SAN_ENABLE, OUTPUT);
    digitalWrite(PIN_SAN_ENABLE, LOW);

    Serial.println("=== MASTER STARTED ===");
}

void loop() {
    EVENTS event = getEvent();
    handleEvent(event);
    if (centralAcqState == STATE_CONNECTED) {
        runConnectedStateMachine(event);
    }
    delay(10);
}

void handleEvent(EVENTS event) {
    switch (centralAcqState) {
    case STATE_DISCONNECTED:
        if (event == EV_CONNECT) {
            centralAcqState = STATE_CONNECTED;
            connectedSubState = SUBSTATE_IDLE;
            writeMsgToSerialPort(CONNECT_MSG);
            Serial.println(">>> STATE: DISCONNECTED -> CONNECTED");
        }
        break;
    case STATE_CONNECTED:
        if (event == EV_DISCONNECT) {
            centralAcqState = STATE_DISCONNECTED;
            digitalWrite(PIN_LED_PREPARED, LOW);
            digitalWrite(PIN_LED_ACQUIRING, LOW);
            writeMsgToSerialPort(DISCONNECT_MSG);
            Serial.println(">>> STATE: CONNECTED -> DISCONNECTED");
        }
        else if (event == EV_CONNECT) {
             writeMsgToSerialPort(CONNECT_MSG);
             Serial.println(">>> INFO: PC Reconnected");
        }
        break;
    }
}

EVENTS getEvent() {
    char msg[MAX_MSG_SIZE];
    if (checkForMsgOnSerialPort(msg)) {
        // For debugging
        Serial.print("RX PC MSG: "); Serial.println(msg);

        if (strcmp(msg, CONNECT_MSG) == 0) {
			return EV_CONNECT;
		}

		if (strcmp(msg, DISCONNECT_MSG) == 0) {
			return EV_DISCONNECT;
		}

        if (strncmp(msg, EXAMINATION_MSG, 4) == 0) {
			return EV_EXAM;
		}
    }
    return EV_NONE;
}

void sendCommand(uint8_t address, uint8_t cmd) {
    // For debugging
    Serial.print("TX I2C [Addr "); Serial.print(address, HEX);
    Serial.print("] Cmd: "); Serial.println(cmd);

    Wire.beginTransmission(address);
    Wire.write(cmd);
    Wire.endTransmission();
}

bool isSlaveReady(uint8_t address) {
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
		return Wire.read() == 1;
	}
    return false;
}

static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]) {
    Serial.write(MSG_START_SYMBOL);
    int i = 0;
    while (i < MAX_MSG_SIZE && msg[i] != '\0') {
		Serial.write(msg[i++]);
	}
    Serial.write(MSG_END_SYMBOL);
    return true;
}

bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]) {
    static int state = 0; // 0=WaitStart, 1=WaitEnd
    static int idx = 0;
    static char msg[MAX_MSG_SIZE];

    if (Serial.available() > 0) {
        char c = Serial.read();
        if (state == 0 && c == MSG_START_SYMBOL) {
            idx=0; state=1;
        } else if (state == 1) {
            if (c == MSG_END_SYMBOL) {
                msg[idx] = '\0';
                strncpy(msgArg, msg, MAX_MSG_SIZE);
                state = 0;
                return true;
            } else if (idx < MAX_MSG_SIZE-1) {
                msg[idx++] = c;
            }
        }
    }
    return false;
}