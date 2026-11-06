#ifndef PLEXTOR_RAW_SECTOR_SEQUENCE_H
#define PLEXTOR_RAW_SECTOR_SEQUENCE_H

#include "../platform/mcs251.h"

u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorSequence(u32 frame_count, u8 q_metadata_mode, u8 preserve_main_buffer, u8 clear_subchannel_slot);

#endif
