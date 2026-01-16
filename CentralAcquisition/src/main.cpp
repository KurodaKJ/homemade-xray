#include <Arduino.h>
#include <Wire.h>
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"
#include <string.h>


// --- Hardware Definitions ---
#define PIN_BTN_PREPARE   2
#define PIN_BTN_XRAY      3
#define PIN_LED_PREPARED  4
#define PIN_LED_ACQUIRING 5
#define PIN_SAN_ENABLE    6

// --- I2C Addresses & Commands ---
#define ADDR_GEOMETRY     0x08
#define ADDR_XRAY         0x09

#define CMD_IDLE          0
#define CMD_PREPARE       1
#define CMD_PULSE         2

// --- State Definitions ---
typedef enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED    // probably in the future this state will have substates!!
} CENTRAL_ACQ_STATES;

typedef enum {
    SUBSTATE_IDLE,
    SUBSTATE_PREPARING,
    SUBSTATE_PREPARED,
    SUBSTATE_ACQUIRING
} CONNECTED_SUBSTATES;

// --- Global State Variables ---
static CENTRAL_ACQ_STATES centralAcqState = STATE_DISCONNECTED;
static CONNECTED_SUBSTATES connectedSubState = SUBSTATE_IDLE;

// --- Protocol Events ---
typedef enum {
	EV_CONNECT_MSG_RECEIVED,
	EV_DISCONNECT_MSG_RECEIVED,
	EV_EXAM_MSG_RECEIVED,
	EV_NONE
} EVENTS;

static EVENTS getEvent();
static void handleEvent(EVENTS event);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);

// Send a command to a specific slave
void sendCommand(uint8_t address, uint8_t cmd) {
    Wire.beginTransmission(address);
    Wire.write(cmd);
    Wire.endTransmission();
}

// Check if a slave is ready (Read 1 byte)
bool isSlaveReady(uint8_t address) {
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read() == 1; // 1 = Ready
    }
    return false;
}

void runConnectedStateMachine(EVENTS event) {
    static unsigned long preparingTimer = 0;

    switch (connectedSubState) {
        case SUBSTATE_IDLE:
			digitalWrite(PIN_SAN_ENABLE, LOW); // Ensure Safety is OFF

            // If Prepare button pressed (and we are connected)
            if (digitalRead(PIN_BTN_PREPARE) == LOW || event == EV_EXAM_MSG_RECEIVED) {
                // Send prepare command to slaves
                sendCommand(ADDR_GEOMETRY, CMD_PREPARE);
				sendCommand(ADDR_XRAY, CMD_PREPARE);

                preparingTimer = millis();
                connectedSubState = SUBSTATE_PREPARING;
                Serial.println("State: IDLE -> PREPARING");
            }
            break;

        case SUBSTATE_PREPARING:
            // Check if slaves are ready or timeout
            if (isSlaveReady(ADDR_GEOMETRY) && isSlaveReady(ADDR_XRAY)) {
                connectedSubState = SUBSTATE_PREPARED;
                digitalWrite(PIN_LED_PREPARED, HIGH);
                Serial.println("State: PREPARING -> PREPARED");
            }
            else if (millis() - preparingTimer > 5000) {
				sendCommand(ADDR_GEOMETRY, CMD_IDLE);
                sendCommand(ADDR_XRAY, CMD_IDLE);
                connectedSubState = SUBSTATE_IDLE;
                Serial.println("State: PREPARING -> IDLE (Timeout)");
            }
            break;

        case SUBSTATE_PREPARED:
            // If Xray button pressed
            if (digitalRead(PIN_BTN_XRAY) == LOW) {
                connectedSubState = SUBSTATE_ACQUIRING;
                digitalWrite(PIN_LED_PREPARED, LOW);
                digitalWrite(PIN_LED_ACQUIRING, HIGH);

                digitalWrite(PIN_SAN_ENABLE, HIGH); // Enable Safety (Allow Xray)
				sendCommand(ADDR_XRAY, CMD_PULSE);
                Serial.println("State: PREPARED -> ACQUIRING");
            }
            break;

        case SUBSTATE_ACQUIRING:
            // If Xray button released
            if (digitalRead(PIN_BTN_XRAY) == HIGH) {
                digitalWrite(PIN_LED_ACQUIRING, LOW);
				digitalWrite(PIN_SAN_ENABLE, LOW); // Disable Safety (Stop Xray)

				sendCommand(ADDR_GEOMETRY, CMD_IDLE);
                sendCommand(ADDR_XRAY, CMD_IDLE);

				connectedSubState = SUBSTATE_IDLE;
                Serial.println("State: ACQUIRING -> IDLE");
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

    digitalWrite(PIN_SAN_ENABLE, LOW); // Disable SAN by default
}

void loop() {
	EVENTS event = getEvent();
    handleEvent(event);

    // 2. Run Application Logic if Connected
    if (centralAcqState == STATE_CONNECTED) {
        runConnectedStateMachine(event);
    }

	delay(10); // Small delay to keep loop from running too fast
}

void handleEvent(EVENTS event)
{

    switch (centralAcqState) {
    case STATE_DISCONNECTED:
        if (event == EV_CONNECT_MSG_RECEIVED) {
            centralAcqState = STATE_CONNECTED;
			connectedSubState = SUBSTATE_IDLE;
            writeMsgToSerialPort(CONNECT_MSG);
        }
        break;
    case STATE_CONNECTED:
        if (event == EV_DISCONNECT_MSG_RECEIVED) {
            centralAcqState = STATE_DISCONNECTED;
			digitalWrite(PIN_LED_PREPARED, LOW);
            digitalWrite(PIN_LED_ACQUIRING, LOW);
            writeMsgToSerialPort(DISCONNECT_MSG);
        }
        break;
    default:
        break;
    }
}

EVENTS getEvent()
{
    char msg[MAX_MSG_SIZE];
    if (checkForMsgOnSerialPort(msg)) {
        if      (strcmp(msg, CONNECT_MSG) == 0)     return EV_CONNECT_MSG_RECEIVED;
        else if (strcmp(msg, DISCONNECT_MSG) == 0)  return EV_DISCONNECT_MSG_RECEIVED;
    }
    return EV_NONE;
}

static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE])
{
	Serial.write(MSG_START_SYMBOL);
	int i = 0;
	while (i < MAX_MSG_SIZE && msg[i] != '\0') {
		Serial.write(msg[i++]);
	}
	Serial.write(MSG_END_SYMBOL);
	return true;
}

typedef enum {
	WAITING_FOR_MSG_START_SYMBOL,
	WAITING_FOR_MSG_END_SYMBOL
} MSG_RECEIVE_STATE;

bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE])
{
    static MSG_RECEIVE_STATE msgRcvState = WAITING_FOR_MSG_START_SYMBOL;
    static int receiveIndex = 0;
    static char msg[MAX_MSG_SIZE] {0};

    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
		switch (msgRcvState) {
			case WAITING_FOR_MSG_START_SYMBOL:
				if (receivedChar == MSG_START_SYMBOL) {
					receiveIndex = 0;
					msgRcvState = WAITING_FOR_MSG_END_SYMBOL;
				}
				break;
			case WAITING_FOR_MSG_END_SYMBOL:
				if (receivedChar == MSG_END_SYMBOL) {
					msg[receiveIndex] = '\0';
                    receiveIndex = 0;
                    strncpy(msgArg, msg, MAX_MSG_SIZE);
					msgRcvState = WAITING_FOR_MSG_START_SYMBOL;
					return true;
				}
				else msg[receiveIndex++] = receivedChar;
				break;
			default:
				break;
		}
	}
    return false;
}

