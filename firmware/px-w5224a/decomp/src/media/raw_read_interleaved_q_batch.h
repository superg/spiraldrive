#ifndef PLEXTOR_RAW_READ_INTERLEAVED_Q_BATCH_H
#define PLEXTOR_RAW_READ_INTERLEAVED_Q_BATCH_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadFrameBatchWithInterleavedQRecords(u32 frame_count);

#endif
