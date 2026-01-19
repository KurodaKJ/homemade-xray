#pragma once

// Hardware pin
#define PIN_BTN_PREPARE   2
#define PIN_BTN_XRAY      3
#define PIN_LED_PREPARED  4
#define PIN_LED_ACQUIRING 5
#define PIN_SAN_ENABLE    6   // Safety Hub

// I2C Slave addresses
#define ADDR_GEOMETRY     0x08
#define ADDR_XRAY         0x09

// Internal commands
#define CMD_IDLE          0
#define CMD_PREPARE       1
#define CMD_PULSE         2
#define CMD_READ_DOSE     3

// States
typedef enum { 
    STATE_DISCONNECTED, 
    STATE_CONNECTED 
} CENTRAL_ACQ_STATES;

typedef enum { 
    SUBSTATE_IDLE, 
    SUBSTATE_PREPARING, 
    SUBSTATE_PREPARED, 
    SUBSTATE_ACQUIRING 
} CONNECTED_SUBSTATES;

// Events
typedef enum { 
    EV_CONNECT, 
    EV_DISCONNECT, 
    EV_EXAM, 
    EV_NONE 
} EVENTS;