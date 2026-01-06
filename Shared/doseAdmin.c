#include "doseAdmin.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>


Patient* patientList[HASHTABLE_SIZE]; // This will guaranteed to be all NULL already, no?

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

// Initialize the hash table
void CreateHashTable() {
    // Do we really need this?
}

// Remove all patient data from the hash table
void RemoveAllDataFromHashTable() {

    for (int i = 0; i < HASHTABLE_SIZE; i++)
    {
        Patient *current = patientList[i];

        while (current != NULL)
        {
            Patient *tempNext = current->next;

            RemoveAllDoseData(&current->dose);

            free(current);

            current = tempNext;
        }

        patientList[i] = NULL;
    }
}

// Add a patient to the hash table
// Brice: make unit tests!
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE]) {

    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
    {
        // Brice: using string library is fine
        // Most important = make your own choices + justify your choicesS
        return -3;
    }

    if (IsPatientPresent(patientName) == 0)
    {
        return -1;
    }

    uint8_t index = hashFunction(patientName);

    Patient *p = malloc(sizeof(Patient));
    if (p == NULL)
    {
        return -2;
    }

    strcpy(p->name, patientName);

    p->dose.next = NULL;

    p->next = patientList[index];

    patientList[index] = p;

    return 0;
}

int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], Date* date, uint16_t dose) {

    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE) {
        return -3;
    }

    if (IsPatientPresent(patientName) == -1)
    {
        return -1;
    }

    uint8_t index = hashFunction(patientName);

    Patient *p = patientList[index];

    while (true)
    {
        if (strcmp(p->name, patientName) == 0)
        {
            break;
        }
        p = p->next;
    }

    DoseData *newDoseData = malloc(sizeof(DoseData));
    if (newDoseData == NULL)
    {
        return -2;
    }

    newDoseData->amount = dose;
    newDoseData->date = *date;
    newDoseData->next = p->dose.next;
    p->dose.next = newDoseData;

    return 0;
}

void RemoveAllDoseData(DoseData *dose)
{
    while (dose != NULL)
    {
        DoseData *nextNode = dose->next;
        free(dose);
        dose = nextNode;
    }
}

// TODO: Still need to implement this
// Brice: First discard the date -> then you test -> Then you check the date
// For the date => think of incremental logic (year, Then Month, then day)
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE],
                           Date* startDate, Date* endDate, uint32_t* totalDose) {

    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
    {
        return -2;
    }

    if (IsPatientPresent(patientName) == -1)
    {
        return -1;
    }

    uint8_t index = hashFunction(patientName);

    Patient *p = patientList[index];

    while (true)
    {
        if (strcmp(p->name, patientName) == 0)
        {
            break;
        }
        p = p->next;
    }

    *totalDose = 0;
    DoseData *currentDose = &p->dose;

    if (currentDose == NULL) return 0;

    // Stuck at here

    return 0;
}

int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE]) {
    
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
    {
        return -2;
    }

    if (IsPatientPresent(patientName) == -1)
    {
        return -1;
    }

    // Brice: solve warnings
    uint8_t index = hashFunction(patientName);
    Patient *current = patientList[index];
    Patient *previous = NULL;

    while (true) // Brice: make a unit test
    {
        if (strcmp(current->name, patientName) == 0)
        {
            if (previous == NULL)
            {
                patientList[index] = current->next;
                RemoveAllDoseData(current->dose.next); // Brice: use array for Dose may be easier
                free(current);
                return 0;
            }

            previous->next = current->next;
            RemoveAllDoseData(current->dose.next);
            free(current);
            return 0;
        }
        previous = current;
        current = current->next;
    }

}

int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE]) {
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
    {
        return -2; // Max length exceeded
    }

    uint8_t index = hashFunction(patientName);

    Patient *current = patientList[index];

    while (current != NULL)
    {
        if (strcmp(current->name, patientName) == 0)
        {
            return 0;
        }

        current = current->next;
    }

    return -1;
}

int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], size_t* numberOfMeasurements) {
    if (strlen(patientName) >= MAX_PATIENTNAME_SIZE)
    {
        return -2;
    }

    if (IsPatientPresent(patientName) == -1)
    {
        return -1;
    }

    uint8_t index = hashFunction(patientName);
    Patient *p = patientList[index];

    while (true)
    {
        if (strcmp(p->name, patientName) == 0)
        {
            break;
        }
        p = p->next;
    }

    size_t count = 0;

    DoseData *currentDose = &p->dose;

    while (currentDose != NULL)
    {
        count++;
        currentDose = currentDose->next;
    }

    *numberOfMeasurements = count;

    return 0;
}

// Calculates basic stats over the hash table (algorithm basics)
// Brice: start with totalNumberOfPatients, then averagenumber, then standardDeviation
void GetHashPerformance(size_t* totalNumberOfPatients, double* averageNumberOfPatients,
                        double* standardDeviation) {
    // TODO: Implement this!
}

int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}

int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}

static bool IsDateInRange(Date date, Date startDate, Date endDate)
{
    // Make dates comparable as integers YYYYMMDD
    int dateValue = date.year * 10000 + date.month * 100 + date.day;
    int startDateValue = startDate.year * 10000 + startDate.month * 100 + startDate.day;
    int endDateValue = endDate.year * 10000 + endDate.month * 100 + endDate.day;

    // Check if date is within range
    return (dateValue >= startDateValue) && (dateValue <= endDateValue);
}
