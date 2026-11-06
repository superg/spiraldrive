#ifndef PLEXTOR_CONTROLLER_STATE6_CALIBRATION_POSITIONING_H
#define PLEXTOR_CONTROLLER_STATE6_CALIBRATION_POSITIONING_H

#include "mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION PositionReadAcquisitionForCalibrationMode(u8 target_mode, u8 require_tolerance);

#endif
