#include <Arduino.h>
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"
#include <string.h>

typedef enum {
	EV_CONNECT_MSG_RECEIVED, 
	EV_DISCONNECT_MSG_RECEIVED, 
	EV_NONE
} EVENTS;

static EVENTS getEvent();
static void handleEvent(EVENTS event);
static bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);

void setup() {
  Serial.begin(9600);
}

void loop() {
    handleEvent(getEvent());

    //below some dummy code that sends dose data to the patient admin. Remove this dummy code asap.
    static unsigned long timeOut = millis();
    static int doseCnt = 0;
    unsigned long curTime = millis();
    if (curTime > timeOut) {
        Serial.print("$dose:"); Serial.print(doseCnt); Serial.println("#");
        timeOut = curTime + 5000;
        doseCnt++;
    }
}

typedef enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED    // probably in the future this state will have substates!!
} CENTRAL_ACQ_STATES;

void handleEvent(EVENTS event)
{
    static CENTRAL_ACQ_STATES centralAcqState = STATE_DISCONNECTED;

    switch (centralAcqState) {
    case STATE_DISCONNECTED:
        if (event == EV_CONNECT_MSG_RECEIVED) {
            centralAcqState = STATE_CONNECTED;
            writeMsgToSerialPort(CONNECT_MSG);
        }
        break;
    case STATE_CONNECTED:
        if (event == EV_DISCONNECT_MSG_RECEIVED) {
            centralAcqState = STATE_DISCONNECTED;
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

