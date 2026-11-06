#ifndef PLEXTOR_WRITE_PRESERVATION_READ_H
#define PLEXTOR_WRITE_PRESERVATION_READ_H

#include "../platform/mcs251.h"

u8 AcquireWritePreservationRawReadAtAddress(s32 target_address, u8 mapping_mode);
u8 PollWritePreservationDescriptorBatch(void);
void PLEXTOR_NEAR_FUNCTION EnqueueWritePreservationDescriptorRange(u32 main_data_cursor, s32 frame_position, u8 descriptor_count);

#endif
