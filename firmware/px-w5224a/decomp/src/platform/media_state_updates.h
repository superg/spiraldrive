#ifndef PLEXTOR_MEDIA_STATE_UPDATES_H
#define PLEXTOR_MEDIA_STATE_UPDATES_H

#include "mcs251.h"

void InitializeMediaState5PositionAndControllerState(void);
void BuildMediaState5AdjacentPackedBcdMsfTuples(void);
void InitializeDiscControlAndCalibrationState(void);
void PLEXTOR_NEAR_FUNCTION ServiceMediaState5ControllerAndOperatingUpdates(void);
void PLEXTOR_NEAR_FUNCTION ConfigureControllerForReadAcquisitionTarget(s32 target_frame_position);
#if defined(PLEXTOR_HOST_TEST)
void SelectAndConfigureOperatingCodeForThresholdForTest(u32 threshold_value);
#else
void SelectAndConfigureOperatingCodeForThreshold(u32 threshold_value);
#endif

#endif
