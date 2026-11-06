#ifndef PLEXTOR_SCSI_READ_SUBCHANNEL_CURRENT_POSITION_H
#define PLEXTOR_SCSI_READ_SUBCHANNEL_CURRENT_POSITION_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION BuildReadSubChannelCurrentPositionPayloadC(fw_addr_t destination_address);

#endif
