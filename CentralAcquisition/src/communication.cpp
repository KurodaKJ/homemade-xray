#include "communication.h"
#include <Wire.h>
#include <string.h>

bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]) {
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
