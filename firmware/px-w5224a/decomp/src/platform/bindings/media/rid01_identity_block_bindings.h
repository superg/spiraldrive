#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveRid01CopyFirmwareBytes(fw_addr_t destination_base, fw_addr_t source, u8 byte_count);
extern void ObserveRid01CopyPatchBytes(fw_addr_t destination_base, const u8 *source, u8 byte_count);
extern void ObserveRid01ReadConfiguration(fw_addr_t offset, u8 *destination, u16 byte_count);
#define COPY_RID01_FIRMWARE_BYTES(destination, source, count) ObserveRid01CopyFirmwareBytes((destination), (source), (count))
#define COPY_RID01_PATCH_BYTES(destination, source, count) ObserveRid01CopyPatchBytes((destination), (source), (count))
#define READ_RID01_CONFIGURATION(offset, destination, count) ObserveRid01ReadConfiguration((offset), (destination), (count))
#else
#define COPY_RID01_FIRMWARE_BYTES(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define COPY_RID01_PATCH_BYTES(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (fw_addr_t)(source), (count))
#define READ_RID01_CONFIGURATION(offset, destination, count) ReadDriveConfigurationBytes((offset), (fw_addr_t)(destination), (count))
#endif
#endif
