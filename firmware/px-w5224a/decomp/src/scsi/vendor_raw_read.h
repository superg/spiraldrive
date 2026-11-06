#ifndef PLEXTOR_VENDOR_RAW_READ_H
#define PLEXTOR_VENDOR_RAW_READ_H

#include "../platform/mcs251.h"

void HandleScsiOpcodeD8(void);
void HandleScsiOpcodeD9(void);
void HandleScsiOpcodeDF(void);
void HandleScsiOpcodeF6(void);
void HandleScsiOpcodeF2(void);
u8 PLEXTOR_NEAR_FUNCTION PollVendorDfSubchannelTransfer(void);
u8 PLEXTOR_NEAR_FUNCTION ValidateRawSectorReadFrameRangeAgainstMediaBounds(s32 start_frame_position, s32 end_frame_position);

#endif
