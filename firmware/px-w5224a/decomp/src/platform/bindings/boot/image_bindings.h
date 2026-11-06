#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
static u16 FirmwareReadBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}
#else
#define FirmwareReadBigEndianU16(address) (*((volatile u16 huge *)(fw_addr_t)(address)))
#endif
#endif
