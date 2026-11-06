#ifndef PLEXTOR_RADIAL_MODEL_H
#define PLEXTOR_RADIAL_MODEL_H

#include "mcs251.h"

u16 MapSignedFrameSecondsToRadialModelValue(s32 signed_frame_seconds);
void RecalculateRadialOperatingThresholdState(void);
u8 PLEXTOR_NEAR_FUNCTION CalibrateRadialModelSpanAtFrame90000(void);
void PLEXTOR_NEAR_FUNCTION InitializeReadAcquisitionRadialState(void);

#endif
