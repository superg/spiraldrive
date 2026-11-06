#ifndef PLEXTOR_CONTROLLER_BUFFER_CHUNK_FILL_H
#define PLEXTOR_CONTROLLER_BUFFER_CHUNK_FILL_H

#include "mcs251.h"

void FillControllerBufferChunkAndWait(fw_addr_t buffer_address, u16 byte_count, u8 value);

#endif
