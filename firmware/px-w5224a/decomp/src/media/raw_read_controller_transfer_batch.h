#ifndef PLEXTOR_RAW_READ_CONTROLLER_TRANSFER_BATCH_H
#define PLEXTOR_RAW_READ_CONTROLLER_TRANSFER_BATCH_H

#include "../platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION ConfigureRawReadControllerTransferBatch(fw_addr_t fallback_address, u16 block_count);

#endif
