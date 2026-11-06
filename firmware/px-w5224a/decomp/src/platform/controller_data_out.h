#ifndef PLEXTOR_PLATFORM_CONTROLLER_DATA_OUT_H
#define PLEXTOR_PLATFORM_CONTROLLER_DATA_OUT_H

#include "mcs251.h"

void ExecuteControllerDataOutTransfer(fw_addr_t destination_address, u32 transfer_length);
void ExecuteControllerDataInTransfer(fw_addr_t source_address, u32 transfer_length);

#endif
