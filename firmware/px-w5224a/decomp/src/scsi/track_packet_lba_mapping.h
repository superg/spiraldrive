#ifndef PLEXTOR_TRACK_PACKET_LBA_MAPPING_H
#define PLEXTOR_TRACK_PACKET_LBA_MAPPING_H

#include "../platform/mcs251.h"

s32 MapLogicalBlockAddressThroughTrackPacketLayout(s32 logical_block_address);

#endif
