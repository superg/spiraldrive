#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 DispatchReadControllerWordForVendorF5Selector09(u16 request_word);
extern u8 DispatchTransferVendorF5Selector09Response(fw_addr_t source_address, u32 byte_count);
#define ReadVendorF5Selector09ControllerWord(request_word) DispatchReadControllerWordForVendorF5Selector09(request_word)
#define TransferVendorF5Selector09Response(source_address, byte_count) DispatchTransferVendorF5Selector09Response(source_address, byte_count)
#else
#define ReadVendorF5Selector09ControllerWord(request_word) ReadControllerWord(request_word)
#define TransferVendorF5Selector09Response(source_address, byte_count) CopyResponseToControllerBufferAndExecuteDataIn(source_address, byte_count)
#endif
#endif
