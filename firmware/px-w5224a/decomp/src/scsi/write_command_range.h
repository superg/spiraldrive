#ifndef PLEXTOR_WRITE_COMMAND_RANGE_H
#define PLEXTOR_WRITE_COMMAND_RANGE_H

#include "platform/mcs251.h"

void PLEXTOR_NEAR_FUNCTION ExecuteWriteCommandRange(s32 start_logical_block_address, u32 transfer_block_count);

#endif
