#ifndef PLEXTOR_VECTOR_ENTRIES_H
#define PLEXTOR_VECTOR_ENTRIES_H

#include "../platform/mcs251.h"

void ResetVector(void);
void ControllerEventInterruptEntry(void);
void HostInterfaceInterruptEntry(void);
void ControllerPacketInterruptEntry(void);
void Timer0InterruptEntry(void);
void SfrE1Bit7InterruptEntry(void);

#if defined(PLEXTOR_HOST_TEST)
extern u8 VectorEntryPsw;
extern u8 VectorEntryPsw1;
#endif

#endif
