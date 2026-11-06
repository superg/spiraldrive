#ifndef PLEXTOR_RAW_READ_RANGE_FIVE_FRAME_PREROLL_H
#define PLEXTOR_RAW_READ_RANGE_FIVE_FRAME_PREROLL_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireRawReadRangeWithFiveFramePreroll(s32 start_frame_address, u32 frame_count);

#endif
