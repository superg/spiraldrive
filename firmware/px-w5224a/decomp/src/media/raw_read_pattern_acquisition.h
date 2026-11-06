#ifndef PLEXTOR_RAW_READ_PATTERN_ACQUISITION_H
#define PLEXTOR_RAW_READ_PATTERN_ACQUISITION_H

#include "../platform/mcs251.h"

s8 PLEXTOR_NEAR_FUNCTION AcquireAndValidateRawReadPatternRange(s32 start_frame_position, u32 frame_count);

#endif
