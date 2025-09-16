#include "doseAdmin.h"
/*
static uint8_t hashFunction(char patientName[MAX_PATIENTNAME_SIZE]){
	// add here the code of your hash function
	return 0;
}
*/

void CreateHashTable()
{
	 return;
}
 
void RemoveAllDataFromHashTable()
{
	 return;
}
 
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	 return -1;
}
 
int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], 
			          Date* date, uint16_t dose)
{
	 return -1;
}
 
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE], 
                           Date* startDate, Date* endDate, uint32_t* totalDose)
{
	 return -1;
}
 
int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE])
{
	 return -1;
}
 
int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE])
{
	 return -1;
}

int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], 
                               size_t * nrOfMeasurements)
{
	 return -1;
}

void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation)
{
	 return;
}
				
int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}

int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH])
{
	 return -1;
}
