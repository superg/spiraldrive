#ifndef PLEXTOR_PLATFORM_RECOVERY_WAIT_H
#define PLEXTOR_PLATFORM_RECOVERY_WAIT_H

#include "mcs251.h"

void PLEXTOR_NEAR_FUNCTION RunRecoveryWaitAndRestartIfNeeded(void);
void PLEXTOR_NEAR_FUNCTION RunControllerStartupStatusHandshake(void);

#endif
