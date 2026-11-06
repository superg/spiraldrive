#ifndef PLEXTOR_SCSI_READ_CD_TRACK_RANGE_MATCH_H
#define PLEXTOR_SCSI_READ_CD_TRACK_RANGE_MATCH_H

#include "../platform/mcs251.h"

u8 MatchReadCdFramePositionToTrackRange(s32 target_frame_position, u8 first_track_number, u8 last_track_number, u16 frame_bias);

#endif
