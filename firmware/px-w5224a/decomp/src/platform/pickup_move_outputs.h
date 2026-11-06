#ifndef PLEXTOR_PICKUP_MOVE_OUTPUTS_H
#define PLEXTOR_PICKUP_MOVE_OUTPUTS_H

#include "mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ConfigureScaledRadialMovePulseOutputs(u8 inward_direction);
void PLEXTOR_NEAR_FUNCTION ConfigureControllerStateForScaledRadialPickupMove(u8 inward_direction);
void PLEXTOR_NEAR_FUNCTION HomePickupAgainstP1Bit2AndResetEstimate(void);

#endif
