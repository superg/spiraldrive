#ifndef PLEXTOR_CONTROLLER_TRANSITION_WAIT_H
#define PLEXTOR_CONTROLLER_TRANSITION_WAIT_H

#include "mcs251.h"

void PLEXTOR_NEAR_FUNCTION WaitForOperatingCodeTransitionSettling(void);
void WaitForControllerMeasurementWithinToleranceOrExpiry(u8 wait_for_initial_advance, u8 lower_bound_only);

#endif
