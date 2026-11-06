#ifndef PLEXTOR_CONTROLLER_BUFFER_ATOMIC_H
#define PLEXTOR_CONTROLLER_BUFFER_ATOMIC_H

#include "mcs251.h"

u8 ReadControllerBufferByteAtomic(fw_addr_t buffer_offset);
u8 PLEXTOR_NEAR_FUNCTION TestControllerBufferBankStridePattern(u8 starting_pattern);
void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
u8 AcquireZeroStateControllerRecordIndexRuns(u16 last_index, u8 operation_mode);

#endif
