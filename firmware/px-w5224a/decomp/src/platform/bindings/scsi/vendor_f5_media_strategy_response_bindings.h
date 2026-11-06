#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveCaptureInheritedDR12ForVendorF5MediaStrategyResponse(void);
extern void ObserveVendorF5MediaStrategySense(u8 sense_key, u16 asc_ascq);
extern u8 ObserveTransferVendorF5MediaStrategyResponse(fw_addr_t source, u32 byte_count);
extern void ObserveVendorF5MediaStrategyNoSense(void);
#define CAPTURE_VENDOR_F5_RESPONSE_DR12() ObserveCaptureInheritedDR12ForVendorF5MediaStrategyResponse()
#define VENDOR_F5_SET_SENSE(key, code) ObserveVendorF5MediaStrategySense((key), (code))
#define VENDOR_F5_MULTIPLY(a, b) ((u32)(a) * (u32)(b))
#define VENDOR_F5_TRANSFER(source, count) ObserveTransferVendorF5MediaStrategyResponse((source), (count))
#define VENDOR_F5_NO_SENSE() ObserveVendorF5MediaStrategyNoSense()
#else
extern u32 CaptureInheritedDR12ForVendorF5MediaStrategyResponse(void);
#define CAPTURE_VENDOR_F5_RESPONSE_DR12() CaptureInheritedDR12ForVendorF5MediaStrategyResponse()
#define VENDOR_F5_SET_SENSE(key, code) SetSenseData((key), (code))
#define VENDOR_F5_MULTIPLY(a, b) MultiplyU32Low((a), (b))
#define VENDOR_F5_TRANSFER(source, count) CopyResponseToControllerBufferAndExecuteDataIn((source), (count))
#define VENDOR_F5_NO_SENSE() SetNoSenseData()
#endif
#endif
