#ifndef PLEXTOR_SCSI_WRITE_COMMAND_TRANSFER_H
#define PLEXTOR_SCSI_WRITE_COMMAND_TRANSFER_H

#include "../platform/mcs251.h"

u8 StartOrContinueWriteCommandTransfer(u32 start_address, u32 block_count);

#endif
