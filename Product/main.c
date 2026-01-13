#include <stdio.h>
#include "menu.h"
#include <fcntl.h>
#include <string.h>

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
	 
	char selectedPatient[MAX_PATIENTNAME_SIZE] = "JohnDoe";
	AddPatient(selectedPatient);
	
	displayMenu();	
	while (true) {  
        MenuOptions choice = getMenuChoice();
		if (choice == -1) {
			if (centralAcqConnectionState == CONNECTED_WITH_CENTRAL_ACQUISITION) {
				uint32_t doseData;
				if (getDoseDataFromCentralAcquisition(&doseData)) {
					printf("Received dose: %d\n", doseData); // instead of this print call here the function that handles the received dose datawqt
				}
			}
		}
		else {
			switch (choice)
			{
			case MO_ADD_PATIENT:
				printf("Enter the name of the patient to add: ");
				scanf("%79s", selectedPatient);

				if (AddPatient(selectedPatient) == 0) {
					printf("Patient added successfully.\n");
				} else {
					printf("Failed to add patient.\n");
				}

				// Clear the input buffer to remove the newline character
				while (getchar() != '\n');
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
				// add here your select patient code
				break;
			case MO_SELECT_EXAMINATION_TYPE:
				if (centralAcqConnectionState != CONNECTED_WITH_CENTRAL_ACQUISITION) {
					printf("This option is only valid when connected with CentralAcquisition\n");
					break;
				}
				// add here your select examination code
				break;
			case MO_QUIT:
				RemovePatient("JohnDoe");

				if (WriteToFile("patient_data.txt") == 0) {
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
