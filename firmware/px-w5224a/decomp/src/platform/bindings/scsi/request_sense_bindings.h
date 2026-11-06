#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateRequestSenseCdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedRequestSenseCommand(void);
extern void ObserveSetRequestSenseData(u8 sense_key, u16 asc_ascq);
extern void ObserveSetRequestSenseDataFromStatusCode(u8 status_code);
extern u8 ObserveTransferRequestSenseResponse(fw_addr_t source_address, u32 byte_count);
extern void ObserveClearRequestSenseData(void);
#define VALIDATE_REQUEST_SENSE(mask) ObserveValidateRequestSenseCdb(mask)
#define HANDLE_UNSUPPORTED_REQUEST_SENSE() ObserveUnsupportedRequestSenseCommand()
#define SET_REQUEST_SENSE_DATA(key, code) ObserveSetRequestSenseData((key), (code))
#define SET_REQUEST_SENSE_FROM_STATUS(status) ObserveSetRequestSenseDataFromStatusCode(status)
#define TRANSFER_REQUEST_SENSE(source, count) ObserveTransferRequestSenseResponse((source), (count))
#define CLEAR_REQUEST_SENSE_DATA() ObserveClearRequestSenseData()
#else
extern void SetSenseDataFromStatusCode(u8 status_code);
#define VALIDATE_REQUEST_SENSE(mask) ValidateScsiCdbFieldMasks(mask)
#define HANDLE_UNSUPPORTED_REQUEST_SENSE() HandleUnsupportedScsiCommand()
#define SET_REQUEST_SENSE_DATA(key, code) SetSenseData((key), (code))
#define SET_REQUEST_SENSE_FROM_STATUS(status) SetSenseDataFromStatusCode(status)
#define TRANSFER_REQUEST_SENSE(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define CLEAR_REQUEST_SENSE_DATA() SetNoSenseData()
#endif
#endif
