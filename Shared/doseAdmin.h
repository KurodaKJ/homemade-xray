#ifndef DOSEADMIN_H
#define DOSEADMIN_H
#include <stdint.h>
#include <stddef.h>

#define MAX_PATIENTNAME_SIZE	(80)
#define HASHTABLE_SIZE			(256)

typedef struct {
	uint8_t   day;    // value in range [1, 31]
	uint8_t   month;  // value in range [1, 12]
	uint16_t  year;   // value in range [1900, 2500]
} Date;

typedef struct doseData {
	uint8_t					   amount;
	Date						 date;
    struct    doseData          *next;
} DoseData;

typedef struct patient {
	char        name[MAX_PATIENTNAME_SIZE];
	DoseData						  dose;
    struct      patient              *next;
} Patient;


/*************************************************************************************** 
 * Creates and initializes a hash table. No patient data will be present after creation
 *  
 */
void CreateHashTable(); 
					   

/*********************************************************:******************************
 * Removes all patient data from the hash table:
 * 
 */
void RemoveAllDataFromHashTable();


/***************************************************************************************
 * Adds the patient in the hash table
 *
 * Returns -1 when the passed patientName is already present 
 * Returns -2 when allocation of memory failed
 * Returns -3 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the patient is successfully put in the hash table
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t AddPatient(char patientName[MAX_PATIENTNAME_SIZE]);


/***************************************************************************************
 * Adds the dose a patient received during an examination at a particular date in 
 * the hash table
 * 
 * Returns -1 when the passed patientName is unknown
 * Returns -2 when allocation of memory failed
 * Returns -3 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the data is successfully copied into the hash table
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 * It is also a precondition that date is not NULL
 * It is also a precondition that dose is not 0
 */
int8_t AddPatientDose(char patientName[MAX_PATIENTNAME_SIZE], Date* date, 
                      uint16_t dose);

/***************************************************************************************
 * Remove dose data linked list
 */
void RemoveAllDoseData(DoseData *dose);

/***************************************************************************************
 * Returns the total dose a patient received in passed period.
 * 
 * Returns -1 when the passed patientName is unknown
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the totalDose is  updated successfully
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 * It is also a precondition that both dates and totalDose are not NULL
 */
int8_t PatientDoseInPeriod(char patientName[MAX_PATIENTNAME_SIZE], 
                           Date* startDate, Date* endDate, uint32_t* totalDose);


/***************************************************************************************
 * Removes the patient from the hash table
 * 
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns  0 when the patient data is successfully removed from the hash table
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t RemovePatient(char patientName[MAX_PATIENTNAME_SIZE]);


/***************************************************************************************
 * Checks if the passed patientName is present in the hash table
 * 
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns 0 when the patientName is present the hash
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t IsPatientPresent(char patientName[MAX_PATIENTNAME_SIZE]);


/***************************************************************************************
 * Returns the number of dose measurements done for a given patient
 *
 * Returns -1 when the passed patientName is not present 
 * Returns -2 when string length of patientName exceeds MAX_PATIENTNAME_SIZE
 * Returns 0 otherwise, the number of measurements is put in nrOfMeasurements
 * 
 * It is a precondition that patientName is not NULL and is \0 terminated
 */
int8_t GetNumberOfMeasurements(char patientName[MAX_PATIENTNAME_SIZE], 
                               size_t * numberOfMeasurements);


/***************************************************************************************
 * Returns the total number of patients in the table, the average number of patients in 
 * a table entry and standard deviation of an table entry. 
 * This function is used to check if the hash function is good enough (i.e. distributes 
 * the patients equally over the entries (i.e. small standard deviation)).
 * 
 */
void GetHashPerformance(size_t *totalNumberOfPatients, double *averageNumberOfPatients,
                        double *standardDeviation);
				
				

#define MAX_FILEPATH_LEGTH (250)

/***************************************************************************************
 * Writes all patient data to a text file in the table
 * 
 * Returns 0 on success
 * Returns -1 on faillure
 */
int8_t WriteToFile(char filePath[MAX_FILEPATH_LEGTH]);



/***************************************************************************************
 * Reads all patient data from a text file, and put the data in an empty table.
 * 
 * Returns 0 on success
 * Returns -1 on faillure
 */
int8_t ReadFromFile(char filePath[MAX_FILEPATH_LEGTH]);

#endif
