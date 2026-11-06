#ifndef PLEXTOR_SCSI_RAW_SECTOR_RANGE_H
#define PLEXTOR_SCSI_RAW_SECTOR_RANGE_H

#include "../platform/mcs251.h"

u8 ExecuteRawSectorReadRange(s32 start_frame_position, s32 end_frame_position_exclusive, u8 format_code);

#endif
