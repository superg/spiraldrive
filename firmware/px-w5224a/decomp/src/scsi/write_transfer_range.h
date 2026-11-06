#ifndef PLEXTOR_SCSI_WRITE_TRANSFER_RANGE_H
#define PLEXTOR_SCSI_WRITE_TRANSFER_RANGE_H

#include "../platform/mcs251.h"

u8 ExecuteWriteTransferBlockRange(u32 block_count);

#endif
