#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateReadDiscInformationCdb(fw_addr_t mask_descriptor);
extern void ObserveUnsupportedReadDiscInformationCommand(void);
extern void ObserveBuildBasicReadDiscInformationResponse(void);
extern u8 ObserveBuildRecordableReadDiscInformationResponse(void);
extern u8 ObserveTransferReadDiscInformationResponse(fw_addr_t source_address, u32 byte_count);
extern void ObserveSetNoSenseForReadDiscInformation(void);
#define VALIDATE_READ_DISC_INFORMATION(mask) ObserveValidateReadDiscInformationCdb(mask)
#define HANDLE_UNSUPPORTED_READ_DISC_INFORMATION() ObserveUnsupportedReadDiscInformationCommand()
#define BUILD_BASIC_READ_DISC_INFORMATION() ObserveBuildBasicReadDiscInformationResponse()
#define BUILD_RECORDABLE_READ_DISC_INFORMATION() ObserveBuildRecordableReadDiscInformationResponse()
#define TRANSFER_READ_DISC_INFORMATION(source, count) ObserveTransferReadDiscInformationResponse((source), (count))
#define SET_NO_SENSE_FOR_READ_DISC_INFORMATION() ObserveSetNoSenseForReadDiscInformation()
#else
#define VALIDATE_READ_DISC_INFORMATION(mask) ValidateScsiCdbFieldMasks(mask)
#define HANDLE_UNSUPPORTED_READ_DISC_INFORMATION() HandleUnsupportedScsiCommand()
#define BUILD_BASIC_READ_DISC_INFORMATION() BuildBasicReadDiscInformationResponse()
#define BUILD_RECORDABLE_READ_DISC_INFORMATION() BuildRecordableMediaReadDiscInformationResponse()
#define TRANSFER_READ_DISC_INFORMATION(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define SET_NO_SENSE_FOR_READ_DISC_INFORMATION() SetNoSenseData()
#endif
#endif
