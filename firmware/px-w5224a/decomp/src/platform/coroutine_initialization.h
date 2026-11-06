#ifndef PLEXTOR_COROUTINE_INITIALIZATION_H
#define PLEXTOR_COROUTINE_INITIALIZATION_H

#include "mcs251.h"

void ConfigureInitialCoroutines(void);
void FirmwareMainLoop(void);
void ServiceDriveInitializationStateTransition(void);
void InitializeCoroutine0(u32 entry_point);
void InitializeCoroutine1(u32 entry_point);
void InitializeCoroutine2(u32 entry_point);
void InitializeCoroutine3(u32 entry_point);

#endif
