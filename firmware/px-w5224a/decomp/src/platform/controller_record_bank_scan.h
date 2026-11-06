#ifndef PLEXTOR_CONTROLLER_RECORD_BANK_SCAN_H
#define PLEXTOR_CONTROLLER_RECORD_BANK_SCAN_H

#include "mcs251.h"

u16 ScanControllerRecordBanksForFrameRange(u8 force_bank_scan, s32 start_frame_position, u32 frame_count);

#endif
