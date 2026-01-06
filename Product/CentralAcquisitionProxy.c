#include "CentralAcquisitionProxy.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h> // sleep
#include "serialPort.h"

static bool setupSerialConnection();
static bool connect();
static bool writeMsgToSerialPort(const char *msg);
static bool getMsgFromCentralAcquisition(char msg[MAX_MSG_SIZE]);

/*
const char * CONNECT_MSG = "CONNECT";
const char * DISCONNECT_MSG = "DISCONNECT";
const char * EXAMINATION_MSG = "EXAM";  // Remark that this msg will have an argument.
                                        // For instance the actual msg could be EXAM:0
										// The 0 indicates a single shot exam
const char * DOSE_MSG = "DOSE";			// Will also have an argument
*/

bool connectWithCentralAcquisition()
{
	bool trySucceeded = false;
	if (setupSerialConnection()) {
		trySucceeded = connect(); 
	}
	return trySucceeded;
}

bool disconnectFromCentralAcquisition()
{
	char receivedMsg[MAX_MSG_SIZE];
	int tryCount = 0;
	const int maxTryCount = 10;
	bool disConnected = false;
	
	printf("Try to disconnect from CentralAquisition...  ");
	do {
		writeMsgToSerialPort(DISCONNECT_MSG);
		if (getMsgFromCentralAcquisition(receivedMsg)) {
			if (strcmp(receivedMsg, DISCONNECT_MSG) == 0) disConnected = true;
			else 									      tryCount++;			
		}
		else tryCount++;
		sleep(1);
	} while (!disConnected && tryCount < maxTryCount);
	
	if (disConnected) closeSerialPort();
		
	if (disConnected) printf("  ...succeeded\n");
	else              printf("  ...failed, tried it %d times\n", maxTryCount);
	return disConnected;
}

void selectExaminationType(const EXAMINATION_TYPES examination) 
{
	(void) examination; // remove this line as soon as you are doing something with the argument
	return;
}

bool getDoseDataFromCentralAcquisition(uint32_t * doseData)
{
	char msg[MAX_MSG_SIZE];
	if (getMsgFromCentralAcquisition(msg)) {
		*doseData = 666;// remove this line as soon as you are really doing something with msg
		return true;
	}
	return false;
}


typedef enum {
	WAITING_FOR_MSG_START_SYMBOL, 
	WAITING_FOR_MSG_END_SYMBOL
} MSG_RECEIVE_STATE;

static bool getMsgFromCentralAcquisition(char msg[MAX_MSG_SIZE])
{
    char receivedChar;
	int receiveIndex = 0;
	MSG_RECEIVE_STATE state = WAITING_FOR_MSG_START_SYMBOL;
	while (1) {
		if (readSerialPort(&receivedChar) == 1) {
			switch (state) {
			case WAITING_FOR_MSG_START_SYMBOL:
				if (receivedChar == MSG_START_SYMBOL) {
					receiveIndex = 0;
					state = WAITING_FOR_MSG_END_SYMBOL;
				}
				break;
			case WAITING_FOR_MSG_END_SYMBOL:
				if (receivedChar == MSG_END_SYMBOL) {
					msg[receiveIndex] = '\0';
					return true;
				}
				else msg[receiveIndex++] = receivedChar;   // oeps, oeps. 
				                                           // What will happen when receiveIndex equals MAX_MSG_SIZE.  FIX THIS
				break;
			default:
				break;
			}
		}
		else return false;
	}
}

static bool setupSerialConnection()
{
	int ACMNumber = 0;
	int tryCount = 0;
	const int maxTryCount = 50;
	
	printf("Setting up serial connection...  ");
	do {	
		char ttyName[30];
		sprintf(ttyName, "/dev/ttyUSB0");
		if (setupSerialPort(ttyName) == 0) {	
			printf("  ...connected with %s\n", ttyName);
			return true;
		}
		else {
			ACMNumber++;
			tryCount++;
		}
	} while (tryCount < maxTryCount);
	printf("  ...failed %d times to connect with an TTY\n", maxTryCount);
	printf("    	Did you connect an Arduino???\n");
	printf("    	In case of wsl, did you start the executable as admin???\n");
	printf("    	In case of wsl, did you set the Arduino USB connection as shared (via usbipd in power shell)???\n");
	return false;
}

static bool connect()
{
	char receivedMsg[MAX_MSG_SIZE];
	int tryCount = 0;
	const int maxTryCount = 10;
	bool connected = false;
	
	printf("Try to connect with CentralAquisition...  ");
	do {
		writeMsgToSerialPort(CONNECT_MSG);
		if (getMsgFromCentralAcquisition(receivedMsg)) {
			if (strcmp(receivedMsg, CONNECT_MSG) == 0) connected = true;
			else 									   tryCount++;			
		}
		else tryCount++;
		sleep(1);
	} while (!connected && tryCount < maxTryCount);
		
	if (connected) printf("  ...succeeded\n");
	else           printf("  ...failed, tried it %d times\n", maxTryCount);
	return connected;
}

static bool writeMsgToSerialPort(const char *msg)
{
	writeSerialPort(MSG_START_SYMBOL);
	int i = 0;
	while (i < MAX_MSG_SIZE && msg[i] != '\0') {
		writeSerialPort(msg[i++]);
	}
	writeSerialPort(MSG_END_SYMBOL);
	return true;
}