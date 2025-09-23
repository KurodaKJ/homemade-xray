#ifndef PROTOCOL_PATIENTADMIN_CENTRALACQ_H
#define PROTOCOL_PATIENTADMIN_CENTRALACQ_H


typedef enum {
	MSG_START_SYMBOL = '$', 
	MSG_END_SYMBOL = '#',
	MSG_ARGUMENT_SEPARATOR = ':'
} SYMBOLS;

typedef enum {
	EXAM_TYPE_SINGLE_SHOT,
	EXAM_TYPE_SERIES,
	EXAM_TYPE_SERIES_WITH_MOTION, 
	EXAM_TYPE_FLUORO,
	EXAM_TYPE_NONE
} EXAMINATION_TYPES;


#define CONNECT_MSG  "CONNECT"
#define DISCONNECT_MSG  "DISCONNECT"
#define EXAMINATION_MSG  "EXAM"
// Remark that this msg will have an argument.
// For instance the actual msg could be EXAM:0
// The 0 indicates a single shot exam
#define DOSE_MSG "DOSE"
// Will also have an argument


#define MAX_MSG_SIZE 	(100)



#endif