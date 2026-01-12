#include <stdio.h>
#include "menu.h"
#include <fcntl.h>
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
	(void) selectedPatient; // remove this line when you are doing something with selectedPatient
	// add here the code that adds John Doe into the admin
	
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
				AddPatient(selectedPatient);
				break;
			case MO_DELETE_PATIENT:
				RemovePatient(selectedPatient);
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
				disconnectFromCentralAcquisition();
				centralAcqConnectionState = NOT_CONNECTED_WITH_CENTRAL_ACQUISITION;
				return 0;
				break;
			default:
				printf("Please, enter a valid number! %d\n", choice);
				break;
			}
			displayMenu();
		}
    }
	return 0;
}
