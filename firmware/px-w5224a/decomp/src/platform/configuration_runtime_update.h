#ifndef PLEXTOR_CONFIGURATION_RUNTIME_UPDATE_H
#define PLEXTOR_CONFIGURATION_RUNTIME_UPDATE_H

#include "mcs251.h"

void PLEXTOR_NEAR_FUNCTION UpdateDriveConfigurationRange6CFromRuntimeState(void);
void PLEXTOR_NEAR_FUNCTION ApplyStartupDriveConfigurationMaintenance(void);
void RunDriveInitializationCoroutine1(void);
void RunStartupSequenceCoroutine1(void);
void RunDriveInitializationCompletionCoroutine1(void);

#endif
