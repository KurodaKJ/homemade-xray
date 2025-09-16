#include "serialPort.h"

#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>   // write(), read(), close()


static int serial_port = -1;

int setupSerialPort(char* ttyName)
{
	serial_port = open(ttyName, O_RDWR);
	if (serial_port < 0) {
		//printf("Failed to open tty %s\n", ttyName);
		//printf("Error %i from open: %s\n", errno, strerror(errno));
		return -1;;
	}
	//printf("Port is opened, now setup communication settings (baudrate 9600, 8 databits, no parity and 1 stopbit\n");
	
	struct termios tty;
	if(tcgetattr(serial_port, &tty) != 0) printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	
	tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity
	tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication
	tty.c_cflag |= CS8; // 8 bits per byte
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; 
	tty.c_lflag &= ~ISIG; 
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
	tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;   //no blocking
	cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    if (tcsetattr(serial_port, TCSANOW  , &tty) != 0) {
		//printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		serial_port = -1;
		return -1;
	}
	else {
		//printf("Succesfully setup port, port is now available for read and write\n");
		return 0;
	}
}

int closeSerialPort(char* receivedChar)
{
	if (serial_port < 0) {
		printf("Close failed, because port was not setup succesfully\n");
		return -1;
	}
	tcflush(serial_port, TCIFLUSH);
	int n = close(serial_port);
	if (n < 0) {
		printf("Error %i from close: %s\n", errno, strerror(errno));
		return -1;
	}
	else return n;
}

int readSerialPort(char* receivedChar)
{
	if (serial_port < 0) {
		printf("Read failed, because port was not setup succesfully\n");
		return -1;
	}
	int n = read(serial_port, receivedChar, 1);
	if (n < 0) {
		printf("Error %i from read: %s\n", errno, strerror(errno));
		return -1;
	}
	else return n;
}


int writeSerialPort(char writeChar)
{
	if (serial_port < 0) {
		printf("Write failed, because port was not setup succesfully\n");
		return -1;
	}
	int n = write(serial_port, &writeChar, 1);  printf("%c", writeChar);
	if (n < 0) {
		printf("Error %i from write: %s\n", errno, strerror(errno));
		return -1;
	}
	else {
		return n;
	}
}