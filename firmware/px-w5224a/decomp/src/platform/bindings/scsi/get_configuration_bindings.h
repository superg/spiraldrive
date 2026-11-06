#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveValidateGetConfigurationCdb(fw_addr_t mask_descriptor);
extern void ObserveRejectGetConfigurationCommand(void);
extern u16 ObserveBuildGetConfigurationFeatureResponse(u8 request_type, u16 starting_feature_number, u16 initial_response_length);
extern void ObserveStageGetConfigurationResponse(fw_addr_t source_address, u32 controller_offset, u32 byte_count);
extern u8 ObserveExecuteGetConfigurationDataIn(u32 transfer_length);
extern void ObserveSetNoSenseForGetConfiguration(void);
#define VALIDATE_GET_CONFIGURATION_CDB(mask) ObserveValidateGetConfigurationCdb(mask)
#define REJECT_GET_CONFIGURATION() ObserveRejectGetConfigurationCommand()
#define BUILD_GET_CONFIGURATION_FEATURES(type, feature, length) ObserveBuildGetConfigurationFeatureResponse((type), (feature), (length))
#define STAGE_GET_CONFIGURATION_RESPONSE(source, offset, count) ObserveStageGetConfigurationResponse((source), (offset), (count))
#define EXECUTE_GET_CONFIGURATION_DATA_IN(length) ObserveExecuteGetConfigurationDataIn(length)
#define SET_GET_CONFIGURATION_NO_SENSE() ObserveSetNoSenseForGetConfiguration()
#else
extern u8 ValidateScsiCdbFieldMasks(fw_addr_t mask_descriptor);
extern void HandleUnsupportedScsiCommand(void);
extern u32 CopyToControllerBufferWindow(fw_addr_t source_address, u32 controller_offset, u32 byte_count);
extern u8 ExecuteControllerDataInAndMapStatus(u32 transfer_length);
extern void SetNoSenseData(void);
#define VALIDATE_GET_CONFIGURATION_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define REJECT_GET_CONFIGURATION() HandleUnsupportedScsiCommand()
#define BUILD_GET_CONFIGURATION_FEATURES(type, feature, length) BuildGetConfigurationFeatureResponse((type), (feature), (length))
#define STAGE_GET_CONFIGURATION_RESPONSE(source, offset, count) ((void)CopyToControllerBufferWindow((source), (offset), (count)))
#define EXECUTE_GET_CONFIGURATION_DATA_IN(length) ExecuteControllerDataInAndMapStatus(length)
#define SET_GET_CONFIGURATION_NO_SENSE() SetNoSenseData()
#endif
#endif
