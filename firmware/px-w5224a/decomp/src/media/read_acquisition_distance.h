#ifndef PLEXTOR_READ_ACQUISITION_DISTANCE_H
#define PLEXTOR_READ_ACQUISITION_DISTANCE_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionMoveDistance(void);
u8 PLEXTOR_NEAR_FUNCTION IssueReadAcquisitionPickupMove(void);
u8 PLEXTOR_NEAR_FUNCTION PositionPickupForAcquisition(void);
u8 AcquireAndValidateReadPositionPacket(void);
u8 ReduceReadSpeedForAcquisitionRecovery(u8 force_minimum_speed);

#endif
