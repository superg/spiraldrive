#ifndef PLEXTOR_RAW_READ_OPERATION_MODE_SPEED_H
#define PLEXTOR_RAW_READ_OPERATION_MODE_SPEED_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ExecuteRawReadOperationForModeAndSpeed(u8 operation_mode, u8 requested_speed_x);

#endif
