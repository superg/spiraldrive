#ifndef PLEXTOR_MEDIA_INTERRUPT_LEAVES_H
#define PLEXTOR_MEDIA_INTERRUPT_LEAVES_H

#include "controller_word0319_offset_calibration.h"
#include "mcs251.h"

u8 LoadParameterFCAndCalibrateWord0319ForState(u8 parameter_state);
void SelectAndConfigureOperatingCodeForThreshold(u32 threshold_value);
void ResetControllerOperatingState(void);
u16 PLEXTOR_NEAR_FUNCTION CompleteControllerCommand0EAndReadPorts80C7C6(void);
void CalibrateControllerPort8097WithCommand08(void);
/* RAM:fb9838 returns its signed scaled adjustment in WR6 through ERET. */
s16 CaptureControllerD8D9WindowAndUpdateAdjustment(void);

#endif
