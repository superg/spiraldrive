#ifndef PLEXTOR_PMA_ACQUISITION_H
#define PLEXTOR_PMA_ACQUISITION_H

#include "platform/mcs251.h"

u8 AcquirePmaQRecordState(void);
u8 PLEXTOR_NEAR_FUNCTION AccumulateCurrentQPacketIntoPmaState(void);

#endif
