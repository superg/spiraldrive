#ifndef PLEXTOR_SCSI_READ_SUBCHANNEL_ISRC_F1_H
#define PLEXTOR_SCSI_READ_SUBCHANNEL_ISRC_F1_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION BuildReadSubChannelIsrcOrF1PayloadC(u8 response_format, fw_addr_t destination_address);

#endif
