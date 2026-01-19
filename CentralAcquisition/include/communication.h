#pragma once
#include <Arduino.h>
#include "Protocol_PatientAdmin_CentralAcq.h"

bool writeMsgToSerialPort(const char msg[MAX_MSG_SIZE]);
bool checkForMsgOnSerialPort(char msgArg[MAX_MSG_SIZE]);
void sendCommand(uint8_t address, uint8_t cmd);
bool isSlaveReady(uint8_t address);
