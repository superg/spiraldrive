#ifndef PLEXTOR_SCSI_LBA_MAPPING_H
#define PLEXTOR_SCSI_LBA_MAPPING_H

#include "../platform/mcs251.h"

#if defined(PLEXTOR_HOST_TEST)
s32 MapLogicalBlockAddressToControllerFramePositionForTest(s32 logical_block_address);
#else
s32 MapLogicalBlockAddressToControllerFramePosition(s32 logical_block_address);
#endif

#endif
