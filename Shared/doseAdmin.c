#include "doseAdmin.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>


// Brice: you think you have used 80% AI to complete this work.
// It is good to be transparent BUT this is too much
// My advice : use AI as a supportive tool not as tool
// doing the whole job for you. Reason: using too much AI discard learning

// HasEntry does not hurt. It is a bulldoser to kill a mostiko.
// My advise : use an array of pointers derived from (Patient* patientPtr[HASHTABLE_SIZE];)

typedef struct {
    Patient* patientPtr;   // dynamically allocated patient (NULL = not used)
} HashEntry;

static HashEntry hashTable[HASHTABLE_SIZE];
// Brice: it is important to be able to explain
// working solutions are important but explaining the process is
// even more important


// Simple hash: sum of ASCII values mod table size
static uint8_t hashFunction(char patientName[MAX_PATIENTNAME_SIZE]) {
    unsigned int sum = 0;

    // Calculate the sum of ASCII values
    for (int i = 0; patientName[i] != '\0'; i++)
    { // Brice use specifically a scope
        sum += (unsigned char)patientName[i];
    }

    // Return the hash value
    return (uint8_t)(sum % HASHTABLE_SIZE); // Brice: it is important to be able to explain
                                            // working solutions are important but explaining the process is
                                            // even more important
}


// 0 % 4 =
// 1 % 4 =
// 2 % 4 =
// 3 % 4 =
// 4 % 4 =
// 5 % 4 =


// Initialize the hash table
void CreateHashTable() {
    for (int i = 0; i < HASHTABLE_SIZE; i++)
        hashTable[i].patientPtr = NULL;
}

// Remove all patient data from the hash table
void RemoveAllDataFromHashTable() {
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        if (hashTable[i].patientPtr != NULL) {
            free(hashTable[i].patientPtr);
            hashTable[i].patientPtr = NULL;
        }
    }
}

// Add a patient to the hash table
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE]) {
    // Check if the patient name is too long
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) // Brice:
        // Brice: using string library is fine
        // Most important = make your own choices + justify your choicesS
        return -3;

    // Find the hash index in the hash table
    uint8_t index = hashFunction(patientName);

    // Check if the patient is already in the hash table
    if (hashTable[index].patientPtr != NULL)
    {
        // Check if the patient name is already in the hash table
        if (strcmp(hashTable[index].patientPtr->name, patientName) == 0)
            return -1; // already present
        else
            return -2; // failed to allocate memory
    }

    // Allocate memory for the patient
    Patient* p = malloc(sizeof(Patient));
    if (p == NULL)
        return -2; // failed to allocate memory

    strcpy(p->name, patientName);

    // Initialize the doses array
    for (int i = 0; i < MAX_DOSE_MEASUREMENT; i++)
    {
        p->doses[i] = 0; // Brice always use scopes
    }

    hashTable[index].patientPtr = p;
    return 0;
}

int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], Date* date, uint16_t dose) {
    // Check if the patient name is too long
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
        return -3;

    // Find the hash index in the hash table
    uint8_t index = hashFunction(patientName);

    Patient* p = hashTable[index].patientPtr;
    if (p == NULL)
        return -1; // unknown patient

    for (int i = 0; i < MAX_DOSE_MEASUREMENT; i++) {
        if (p->doses[i] == 0) {
            p->doses[i] = dose;
            return 0;
        }
    }
    return -2;
}

// This is still needs to be implement with dates
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE],
                           Date* startDate, Date* endDate, uint32_t* totalDose) {
    // Check if the patient name is too long
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
        return -2;

    // Find the hash index in the hash table
    uint8_t index = hashFunction(patientName);
    Patient* p = hashTable[index].patientPtr;
    if (p == NULL)
        return -1; // unknown patient

    uint32_t sum = 0;
    // Brice: you need to mnake use of the dates
    for (int i = 0; i < MAX_DOSE_MEASUREMENT; i++)
        sum += p->doses[i];
    *totalDose = sum;
    return 0;
}

int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE]) {
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
        return -2;

    uint8_t index = hashFunction(patientName);
    Patient* p = hashTable[index].patientPtr;
    if (p == NULL)
        return -1;

    if (strcmp(p->name, patientName) == 0) {
        free(p);
        hashTable[index].patientPtr = NULL;
        return 0;
    }
    return -1;
}

int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE]) {
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
        return -2;

    uint8_t index = hashFunction(patientName);
    Patient* p = hashTable[index].patientPtr;
    if (p != NULL && strcmp(p->name, patientName) == 0)
        return 0;
    return -1;
}

int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], size_t* numberOfMeasurements) {
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
        return -2;

    uint8_t index = hashFunction(patientName);
    Patient* p = hashTable[index].patientPtr;
    if (p == NULL)
        return -1;

    size_t count = 0;
    for (int i = 0; i < MAX_DOSE_MEASUREMENT; i++) {
        if (p->doses[i] != 0) count++;
    }
    *numberOfMeasurements = count;
    return 0;
}

// Calculates basic stats over the hash table (algorithm basics)
void GetHashPerformance(size_t* totalNumberOfPatients, double* averageNumberOfPatients,
                        double* standardDeviation) {
    size_t total = 0;
    double sum = 0.0;
    double sumSquare = 0.0;

    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        int count = (hashTable[i].patientPtr != NULL) ? 1 : 0;
        total += count;
        sum += count;
        sumSquare += count * count;
    }

    *totalNumberOfPatients = total;
    *averageNumberOfPatients = sum / HASHTABLE_SIZE;
    *standardDeviation = sqrt((sumSquare / HASHTABLE_SIZE) -
                              ((*averageNumberOfPatients) * (*averageNumberOfPatients)));
}

int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}

int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}
