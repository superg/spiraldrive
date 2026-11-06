#ifndef PLEXTOR_READ_MEASUREMENT_WAIT_H
#define PLEXTOR_READ_MEASUREMENT_WAIT_H

#include "mcs251.h"

void InitializeReadMeasurementTargetPacketWait(void);
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementForDescriptorMode(void);
void PLEXTOR_NEAR_FUNCTION PrepareReadMeasurementAtAdjustedTarget(void);

#endif
