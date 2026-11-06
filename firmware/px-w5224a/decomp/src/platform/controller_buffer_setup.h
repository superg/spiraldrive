#ifndef PLEXTOR_CONTROLLER_BUFFER_SETUP_H
#define PLEXTOR_CONTROLLER_BUFFER_SETUP_H

#include "mcs251.h"

void PLEXTOR_NEAR_FUNCTION ServiceControllerBufferSetupStateMachine(void);
void PLEXTOR_NEAR_FUNCTION AdvanceControllerBufferSetupPhase(void);

#endif
