#ifndef PLEXTOR_MEDIA_CALIBRATION_OFFSET_H
#define PLEXTOR_MEDIA_CALIBRATION_OFFSET_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION AdjustEncodedMediaCalibrationOffsetForCurrentWriteSpeed(void);
u8 PLEXTOR_NEAR_FUNCTION EvaluateMediaState6CalibrationResultAndRetry(void);

#endif
