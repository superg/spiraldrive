#ifndef PLEXTOR_MEDIA_RAW_READ_RANGE_RESET_H
#define PLEXTOR_MEDIA_RAW_READ_RANGE_RESET_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION ResetAndAcquireRawReadRange(s32 start_frame_position, u32 frame_count, u8 q_metadata_mode);

#endif
