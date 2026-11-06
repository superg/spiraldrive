#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
u16 index;

for(index = 0u; index < byte_count; ++index)
{
    destination[index] = ReadControllerBufferByteAtomic(controller_offset);
    controller_offset = FirmwareAddressIncrement24(controller_offset, 1u);
}
#else
CopyReadBufferSnapshotBytesToRam(controller_offset, (fw_addr_t)destination, byte_count);
#endif
#endif
