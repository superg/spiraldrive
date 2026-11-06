#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForVendorF3MeasurementResponse(void);
extern void ObserveUnsupportedVendorF3MeasurementResponse(void);
extern u8 ObserveTransferVendorF3MeasurementResponse(fw_addr_t source, u32 byte_count);
extern void ObserveVendorF3MeasurementResponseNoSense(void);
#define CAPTURE_VENDOR_F3_RESPONSE_R15() ObserveCaptureInheritedR15ForVendorF3MeasurementResponse()
#define VENDOR_F3_RESPONSE_UNSUPPORTED() ObserveUnsupportedVendorF3MeasurementResponse()
#define VENDOR_F3_RESPONSE_MIN(value, limit) ((u8)((value) < (limit) ? (value) : (limit)))
#define VENDOR_F3_RESPONSE_TRANSFER(source, count) ObserveTransferVendorF3MeasurementResponse((source), (count))
#define VENDOR_F3_RESPONSE_NO_SENSE() ObserveVendorF3MeasurementResponseNoSense()
#else
extern u8 CaptureInheritedR15ForVendorF3MeasurementResponse(void);
#define CAPTURE_VENDOR_F3_RESPONSE_R15() CaptureInheritedR15ForVendorF3MeasurementResponse()
#define VENDOR_F3_RESPONSE_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define VENDOR_F3_RESPONSE_MIN(value, limit) MinU8((value), (limit))
#define VENDOR_F3_RESPONSE_TRANSFER(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define VENDOR_F3_RESPONSE_NO_SENSE() SetNoSenseData()
#endif
#endif
