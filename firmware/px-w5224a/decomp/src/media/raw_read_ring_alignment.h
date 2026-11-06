#ifndef PLEXTOR_MEDIA_RAW_READ_RING_ALIGNMENT_H
#define PLEXTOR_MEDIA_RAW_READ_RING_ALIGNMENT_H

#include "../platform/mcs251.h"

u8 DecodeControllerPositionFields(u8 check_controller_status);

void PLEXTOR_NEAR_FUNCTION AdvanceMainDataRingAndCheckControllerBufferAlignment(u32 expected_controller_buffer_address);

#endif
