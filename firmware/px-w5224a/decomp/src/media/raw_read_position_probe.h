#ifndef PLEXTOR_RAW_READ_POSITION_PROBE_H
#define PLEXTOR_RAW_READ_POSITION_PROBE_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION InitializeRawReadControllerForPositionProbe(void);
void PLEXTOR_NEAR_FUNCTION ArmRawSectorBufferCapture(void);
u8 PLEXTOR_NEAR_FUNCTION ProcessRawReadPositionProbeStep(void);
u8 PLEXTOR_NEAR_FUNCTION ProcessAlternateRawReadPipelineStep(void);
void RunRawReadPositionProbeCoroutine2(void);
void RunAlternateRawReadPipelineCoroutine2(void);

#endif
