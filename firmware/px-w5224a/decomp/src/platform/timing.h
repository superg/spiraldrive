#ifndef PLEXTOR_TIMING_H
#define PLEXTOR_TIMING_H

#include "mcs251.h"

void StopAndClearTimer1(void);
void StartTimer1AndClearMeasurementStateBit4(void);
void BusyWaitAccumulatorTimes2300(u8 scale);
void DelayTimerUnits(u16 delay_units);
void DelayTimerUnits59(void);
void Timer0TickHandler(void);

#endif
