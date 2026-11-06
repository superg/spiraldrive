#ifndef PLEXTOR_SYNTHETIC_RAW_READ_PATTERN_H
#define PLEXTOR_SYNTHETIC_RAW_READ_PATTERN_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION GenerateSyntheticRawReadPatternRange(s32 start_frame_position, s32 end_frame_position_exclusive);

#endif
