#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "menu.h"
#include "doseAdmin.h"
#include "CentralAcquisitionProxy.h"


typedef enum {
	NOT_CONNECTED_WITH_CENTRAL_ACQUISITION, 
	CONNECTED_WITH_CENTRAL_ACQUISITION
} CENTRAL_ACQUISITION_CONNECTION_STATE;

/*---------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	static CENTRAL_ACQUISITION_CONNECTION_STATE centralAcqConnectionState = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
	if (connectWithCentralAcquisition()) {	
		centralAcqConnectionState = CONNECTED_WITH_CENTRAL_ACQUISITION;
	}
	else {
		printf("\n\nConnecting with CentralAcquisition Failed. No problem, you can continue with \n");
		printf("the functionality that does not depend on that connection!\n");
	}

	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);   //non blocking standard input

	if (ReadFromFile((char[MAX_FILEPATH_LEGTH]){"patient_data.txt"}) == 0) {
		printf("Restored patient data from 'patient_data.txt'.\n");
	} else {
		printf("No previous data found (or failed to read). Starting fresh.\n");
	}
	 
	char selectedPatient[MAX_PATIENTNAME_SIZE] = "JohnDoe";
	AddPatient(selectedPatient);

	// Display database statistics
	size_t totalPatients;
	double averagePerBucket;
	double stdDeviation;

	GetHashPerformance(&totalPatients, &averagePerBucket, &stdDeviation);

	printf("\n=== Database Statistics ===\n");
	printf("Total Patients:   %zu\n", totalPatients);
	printf("Avg per Bucket:   %.4f\n", averagePerBucket);
	printf("Std Deviation:    %.4f (Lower is better)\n", stdDeviation);
	printf("===========================\n\n");
	
	displayMenu();	
	while (true) {  
        MenuOptions choice = getMenuChoice();
		if (choice == -1) {
			if (centralAcqConnectionState == CONNECTED_WITH_CENTRAL_ACQUISITION) {
				uint32_t doseData;
				if (getDoseDataFromCentralAcquisition(&doseData)) {
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);

					Date currentDate;
					currentDate.day = tm.tm_mday;
					currentDate.month = tm.tm_mon + 1;    // tm_mon is 0-11
					currentDate.year = tm.tm_year + 1900; // tm_year is years since 1900

					int8_t result = AddPatientDose(selectedPatient, &currentDate, (uint16_t)doseData);

					if (result == 0) {
						printf("Received dose: %d. Successfully added to patient '%s'.\n", doseData, selectedPatient);
					} else {
						printf("Received dose: %d, but failed to add to patient (Error Code: %d).\n", doseData, result);
					}
				}
			}
		}
		else {
			switch (choice)
			{
			case MO_ADD_PATIENT:
			{
				// Wait for user input
				int flags = fcntl(0, F_GETFL);
				fcntl(0, F_SETFL, flags & ~O_NONBLOCK);

				printf("Enter the name of the patient to add: ");
				scanf("%79s", selectedPatient);

				if (AddPatient(selectedPatient) == 0) {
					printf("Patient added successfully.\n");
				} else {
					printf("Failed to add patient.\n");
				}

				// Clear the input buffer to remove the newline character
				while (getchar() != '\n');

				// Turn Non-Blocking BACK ON (For the main menu loop)
				fcntl(0, F_SETFL, flags | O_NONBLOCK);
			}
			break;
			case MO_DELETE_PATIENT:
				if (strlen(selectedPatient) > 0) {
					if (RemovePatient(selectedPatient) == 0) {
						printf("Patient removed successfully.\n");
					} else {
						printf("Failed to remove patient.\n");
					}
				} else {
					printf("No patient selected.\n");
				}
				break;
			case MO_SELECT_PATIENT:
				int flags = fcntl(0, F_GETFL);
				fcntl(0, F_SETFL, flags & ~O_NONBLOCK);

				char inputName[MAX_PATIENTNAME_SIZE];
				printf("Enter the name of the patient to select: ");
				if (scanf("%79s", inputName) == 1) {
					// 2. Check if patient exists in the Hash Table
					if (IsPatientPresent(inputName) == 0) {
						strcpy(selectedPatient, inputName);
						printf("Selected patient: '%s'\n", selectedPatient);
					} else {
						printf("Patient '%s' not found!\n", inputName);
					}
				}

				// Clear buffer
				while (getchar() != '\n');

				// 3. Restore Non-Blocking mode
				fcntl(0, F_SETFL, flags | O_NONBLOCK);
				break;
			case MO_SELECT_EXAMINATION_TYPE:
			{
				if (centralAcqConnectionState != CONNECTED_WITH_CENTRAL_ACQUISITION) {
					printf("This option is only valid when connected with CentralAcquisition\n");
					break;
				}

				printf("\nSelect Examination Type:\n");
				printf("  [0] Single Shot\n");
				printf("  [1] Series\n");
				printf("  [2] Series with Motion\n");
				printf("  [3] Fluoro\n");
				printf("Choice: ");

				int flags = fcntl(0, F_GETFL);
				fcntl(0, F_SETFL, flags & ~O_NONBLOCK);

				int examChoice = getInt();

				fcntl(0, F_SETFL, flags | O_NONBLOCK);

				if (examChoice >= 0 && examChoice <= 3) {
					selectExaminationType((EXAMINATION_TYPES)examChoice);
					printf("Command sent! Check the Master Arduino...\n");
				} else {
					printf("Invalid selection.\n");
				}
			}
			break;
			case MO_QUIT:
				RemovePatient("JohnDoe");

				if (WriteToFile((char[MAX_FILEPATH_LEGTH]){"patient_data.txt"}) == 0) {
					printf("Patient data successfully saved to 'patient_data.txt'.\n");
				} else {
					printf("Error: Failed to save patient data.\n");
				}

				disconnectFromCentralAcquisition();
				centralAcqConnectionState = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
				return 0;
			default:
				printf("Please, enter a valid number! %d\n", choice);
				break;
			}
			displayMenu();
		}
    }
	return 0;
}
