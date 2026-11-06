#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateGetEventStatusCdb(fw_addr_t mask_descriptor);
extern void ObserveRejectGetEventStatusCommand(void);
extern void ObserveStageGetEventStatusResponse(fw_addr_t source_address, u32 controller_offset, u32 byte_count);
extern u16 ObserveGetEventStatusMinU16(u16 value, u16 limit);
extern u8 ObserveExecuteGetEventStatusDataIn(u32 transfer_length);
#define VALIDATE_GET_EVENT_STATUS_CDB(mask) ObserveValidateGetEventStatusCdb(mask)
#define REJECT_GET_EVENT_STATUS() ObserveRejectGetEventStatusCommand()
#define STAGE_GET_EVENT_STATUS_RESPONSE(source, offset, count) ObserveStageGetEventStatusResponse((source), (offset), (count))
#define GET_EVENT_STATUS_MIN_U16(value, limit) ObserveGetEventStatusMinU16((value), (limit))
#define EXECUTE_GET_EVENT_STATUS_DATA_IN(length) ObserveExecuteGetEventStatusDataIn(length)
#else
#define VALIDATE_GET_EVENT_STATUS_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define REJECT_GET_EVENT_STATUS() HandleUnsupportedScsiCommand()
#define STAGE_GET_EVENT_STATUS_RESPONSE(source, offset, count) ((void)CopyToControllerBufferWindow((source), (offset), (count)))
#define GET_EVENT_STATUS_MIN_U16(value, limit) MinU16((value), (limit))
#define EXECUTE_GET_EVENT_STATUS_DATA_IN(length) ExecuteControllerDataInAndMapStatus(length)
#endif
#endif
