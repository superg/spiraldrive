#ifndef PLEXTOR_CONTROLLER_PROGRESS_H
#define PLEXTOR_CONTROLLER_PROGRESS_H

#include "mcs251.h"

void UpdateControllerProfileFromReadAcquisitionTarget(void);
void UpdateControllerWord035FFromProgressBucket(void);
u8 NormalizeControllerMeasurementPollStatus(void);

#endif
