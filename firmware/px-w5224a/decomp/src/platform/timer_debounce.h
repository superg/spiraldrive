#ifndef PLEXTOR_TIMER_DEBOUNCE_H
#define PLEXTOR_TIMER_DEBOUNCE_H

#include "mcs251.h"

void UpdateP3Bits0And1PatternState(u8 restart_pattern);
void DebounceP0Bit7State(void);
void DebounceControllerTransactionStatusBit0(void);
void ServiceControllerChannel2GateTimeoutTick(void);
void PLEXTOR_NEAR_FUNCTION ServiceP1Bit6DriveStateTransition(void);

#endif
