#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t base = (fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_155A) << 24;

base |= (fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_155B) << 16;
base |= (fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_155C) << 8;
base |= FirmwareReadNear8(FW_NEAR_UNKNOWN_155D);
return base;
#else
return *(volatile u32 huge *)FW_NEAR_UNKNOWN_155A;
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u16 index;

for(index = 0u; index < byte_count; ++index)
{
    WriteControllerBufferWindowByteAtomic(destination, source[index]);
    destination = FirmwareAddressIncrement24(destination, 1u);
}
#else
CopyRamBytesToReadBufferSnapshot(destination, (fw_addr_t)source, byte_count);
#endif
#endif
