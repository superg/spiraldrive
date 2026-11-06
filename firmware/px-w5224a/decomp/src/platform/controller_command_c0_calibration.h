#ifndef PLEXTOR_CONTROLLER_COMMAND_C0_CALIBRATION_H
#define PLEXTOR_CONTROLLER_COMMAND_C0_CALIBRATION_H

#include "mcs251.h"

u8 RunControllerCommandC0AndAdjustWord030A(void);
u8 RunControllerCommandD0AndAdjustWord031A(void);
u8 CalibrateControllerWords030A031AAndCaptureReferences(void);

#endif
