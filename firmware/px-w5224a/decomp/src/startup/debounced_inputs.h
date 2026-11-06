#ifndef PLEXTOR_STARTUP_DEBOUNCED_INPUTS_H
#define PLEXTOR_STARTUP_DEBOUNCED_INPUTS_H

#include "../platform/mcs251.h"

void RunStartupDebouncedInputSequence(void);
void PLEXTOR_NEAR_FUNCTION ProbeControllerSignalAndInitializeDriveControl(void);
u8 PLEXTOR_NEAR_FUNCTION RunDriveControlState9Sequence(void);

#endif
