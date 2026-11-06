#ifndef PLEXTOR_RAW_READ_FRAME_SEQUENCE_H
#define PLEXTOR_RAW_READ_FRAME_SEQUENCE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameSequenceWithQUpdates(s32 frame_count, u8 reverse_relative_msf);
u8 PLEXTOR_NEAR_FUNCTION AcquireFixedRawReadFramePrefixWindow(u8 prefix_mode, u8 capture_source_kind);
u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameSequenceWithCurrentQPacket(s32 frame_count, u8 capture_source_kind);
u8 WaitForRawReadProducerCapacityAndCheckStatus(void);
void AcquireRawReadPrefixAtCurrentPipelineTarget(void);

#endif
