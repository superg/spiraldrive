#ifndef PLEXTOR_RAW_SECTOR_CAPTURE_H
#define PLEXTOR_RAW_SECTOR_CAPTURE_H

#include "mcs251.h"

u8 CaptureRawSectorToControllerBufferAndWait(fw_addr_t controller_buffer_address, u8 raw_sector_source_kind);
void PLEXTOR_NEAR_FUNCTION SelectRawSectorCaptureModeAndLength(u16 *transfer_length, u8 *capture_mode, u8 raw_sector_source_kind);

#endif
