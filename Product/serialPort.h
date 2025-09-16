#ifndef SERIALPORT_H
#define SERIALPORT_H
int setupSerialPort(char* ttyName);
int closeSerialPort();
int readSerialPort(char* receivedChar);
int writeSerialPort(char writeChar);

#endif