#ifndef PLEXTOR_SESSION_BOUNDARY_H
#define PLEXTOR_SESSION_BOUNDARY_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireNextSessionBoundaryPosition(void);
void PLEXTOR_NEAR_FUNCTION ServiceMediaQPacketCaptureWithOperatingCodeBudget(void);

#endif
