#ifndef PLEXTOR_BOOT_IMAGE_H
#define PLEXTOR_BOOT_IMAGE_H

#include "../platform/mcs251.h"

u8 VerifyAdditiveChecksumRange(fw_addr_t end_address, fw_addr_t start_address);
u8 IsMemoryRangeErased(fw_addr_t end_address, fw_addr_t start_address);
u8 ValidateBootAndConfigurationMemory(void);
void InitializeInquiryResponseBuffer(void);
void SelectMainFirmwareInterruptHandlers(void);

#endif
