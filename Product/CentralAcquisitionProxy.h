#ifndef CENTRALACQUISITIONPROXY
#define CENTRALACQUISITIONPROXY
#include <stdbool.h>
#include <stdint.h>
#include "Protocol_PatientAdmin_CentralAcq.h"

bool connectWithCentralAcquisition();
bool disconnectFromCentralAcquisition();
void selectExaminationType(const EXAMINATION_TYPES examination);
bool getDoseDataFromCentralAcquisition(uint32_t * doseData);

#endif