#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 DispatchTransferVendorDeStatusResponse(fw_addr_t source_address, u32 byte_count);
#define TransferVendorDeStatusResponse(source_address, byte_count) DispatchTransferVendorDeStatusResponse((source_address), (byte_count))
#else
#define TransferVendorDeStatusResponse(source_address, byte_count) CopyResponseToControllerBufferAndExecuteDataIn((source_address), (byte_count))
#endif
#endif
