#ifndef PLEXTOR_WRITE_PIPELINE_Q_H
#define PLEXTOR_WRITE_PIPELINE_Q_H

#include "../platform/mcs251.h"

void BuildWritePipelineQPacketForTargetAddress(void);
void PLEXTOR_NEAR_FUNCTION AdvanceWorkingTrackWriteCursorByCallerFlag(u32 frame_delta, u8 caller_flag);
u8 InitializeWriteCommandRawReadPipelineState(void);

#endif
