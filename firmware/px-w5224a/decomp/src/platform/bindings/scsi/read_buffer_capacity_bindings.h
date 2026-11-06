#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReadBufferCapacityCdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedReadBufferCapacityCommand(void);
extern u8 ObserveTransferReadBufferCapacityResponse(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetNoSenseForReadBufferCapacity(void);
#define VALIDATE_READ_BUFFER_CAPACITY(mask) ObserveValidateReadBufferCapacityCdb(mask)
#define HANDLE_UNSUPPORTED_READ_BUFFER_CAPACITY() ObserveUnsupportedReadBufferCapacityCommand()
#define TRANSFER_READ_BUFFER_CAPACITY_RESPONSE(source, count) ObserveTransferReadBufferCapacityResponse((source), (count))
#define SET_NO_SENSE_FOR_READ_BUFFER_CAPACITY() ObserveSetNoSenseForReadBufferCapacity()
#else
#define VALIDATE_READ_BUFFER_CAPACITY(mask) ValidateScsiCdbFieldMasks(mask)
#define HANDLE_UNSUPPORTED_READ_BUFFER_CAPACITY() HandleUnsupportedScsiCommand()
#define TRANSFER_READ_BUFFER_CAPACITY_RESPONSE(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define SET_NO_SENSE_FOR_READ_BUFFER_CAPACITY() SetNoSenseData()
#endif
#endif
