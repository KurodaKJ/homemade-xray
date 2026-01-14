#include <Arduino.h>
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"
#include <string.h>


// --- Hardware Definitions ---
#define PIN_BTN_PREPARE   2
#define PIN_BTN_XRAY      3
#define PIN_LED_PREPARED  4
#define PIN_LED_ACQUIRING 5

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
	EV_NONE
} EVENTS;

static EVENTS getEvent();
static void handleEvent(EVENTS event);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);

// Helper to simulate checking slaves ( Will be replace with I2C later)
bool areSlavesPrepared() {
    return true;
}

void runConnectedStateMachine() {
    static unsigned long preparingTimer = 0;

    switch (connectedSubState) {
        case SUBSTATE_IDLE:
            // If Prepare button pressed (and we are connected)
            if (digitalRead(PIN_BTN_PREPARE) == HIGH) {
                // TODO: Send "Prepare" to slaves
                preparingTimer = millis();
                connectedSubState = SUBSTATE_PREPARING;
                Serial.println("State: IDLE -> PREPARING");
            }
            break;

        case SUBSTATE_PREPARING:
            // Check if slaves are ready or timeout
            if (areSlavesPrepared()) {
                connectedSubState = SUBSTATE_PREPARED;
                digitalWrite(PIN_LED_PREPARED, HIGH);
                Serial.println("State: PREPARING -> PREPARED");
            }
            else if (millis() - preparingTimer > 1000) {
                connectedSubState = SUBSTATE_IDLE;
                Serial.println("State: PREPARING -> IDLE (Timeout)");
            }
            break;

        case SUBSTATE_PREPARED:
            // If Xray button pressed
            if (digitalRead(PIN_BTN_XRAY) == HIGH) {
                connectedSubState = SUBSTATE_ACQUIRING;
                digitalWrite(PIN_LED_PREPARED, LOW);
                digitalWrite(PIN_LED_ACQUIRING, HIGH);
                Serial.println("State: PREPARED -> ACQUIRING");
            }
            break;

        case SUBSTATE_ACQUIRING:
            // If Xray button released
            if (digitalRead(PIN_BTN_XRAY) == LOW) {
                digitalWrite(PIN_LED_ACQUIRING, LOW);
                connectedSubState = SUBSTATE_IDLE;
                Serial.println("State: ACQUIRING -> IDLE");
            }
            break;
    }
}

void setup() {
  	Serial.begin(9600);
	pinMode(PIN_BTN_PREPARE, INPUT); // or INPUT_PULLUP if using internal resistors
  	pinMode(PIN_BTN_XRAY, INPUT);
  	pinMode(PIN_LED_PREPARED, OUTPUT);
  	pinMode(PIN_LED_ACQUIRING, OUTPUT);
}

void loop() {
    handleEvent(getEvent());

    // 2. Run Application Logic if Connected
    if (centralAcqState == STATE_CONNECTED) {
        runConnectedStateMachine();
    }
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

