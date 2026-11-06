#ifndef PLEXTOR_SCSI_READ_CD_TRACK_COVERAGE_H
#define PLEXTOR_SCSI_READ_CD_TRACK_COVERAGE_H

#include "../platform/mcs251.h"

u8 ClassifyReadCdFrameRangeTrackCoverage(s32 start_frame_position, s32 end_frame_position);

#endif
