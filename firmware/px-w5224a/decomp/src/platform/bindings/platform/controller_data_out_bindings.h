#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveConfigureControllerDataOutTransferStatus(void);
extern u8 ObserveReadControllerDataOutTimer(void);
extern void ObserveSetControllerDataOutSense(u8 sense_key, u16 asc_ascq);
extern void ObserveYieldControllerDataOut(void);
extern void ObserveConfigureControllerDataInTransferStatus(void);
extern u8 ObserveReadControllerDataInTimer(void);
extern void ObserveSetControllerDataInSense(u8 sense_key, u16 asc_ascq);
extern void ObserveYieldControllerDataIn(void);
#define CONFIGURE_DATA_OUT_STATUS() ObserveConfigureControllerDataOutTransferStatus()
#define READ_DATA_OUT_TIMER() ObserveReadControllerDataOutTimer()
#define SET_DATA_OUT_SENSE(key, code) ObserveSetControllerDataOutSense((key), (code))
#define YIELD_DATA_OUT() ObserveYieldControllerDataOut()
#define CONFIGURE_DATA_IN_STATUS() ObserveConfigureControllerDataInTransferStatus()
#define READ_DATA_IN_TIMER() ObserveReadControllerDataInTimer()
#define SET_DATA_IN_SENSE(key, code) ObserveSetControllerDataInSense((key), (code))
#define YIELD_DATA_IN() ObserveYieldControllerDataIn()
#else
extern void YieldCoroutine0(void);
#define CONFIGURE_DATA_OUT_STATUS() ConfigureAtaDataTransferStatus()
#define READ_DATA_OUT_TIMER() FirmwareReadNear8(FW_NEAR_UNKNOWN_120D)
#define SET_DATA_OUT_SENSE(key, code) SetSenseData((key), (code))
#define YIELD_DATA_OUT() YieldCoroutine0()
#define CONFIGURE_DATA_IN_STATUS() ConfigureAtaDataTransferStatus()
#define READ_DATA_IN_TIMER() FirmwareReadNear8(FW_NEAR_UNKNOWN_120D)
#define SET_DATA_IN_SENSE(key, code) SetSenseData((key), (code))
#define YIELD_DATA_IN() YieldCoroutine0()
#endif
#endif
