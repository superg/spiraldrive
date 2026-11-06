#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveReadAcquisitionWaitBit001B(void);
extern u8 ObserveReadAcquisitionWaitBit0021(void);
extern void ObserveDispatchReadAcquisitionWaitRequest(void);
extern void ObserveYieldCoroutine2ForReadAcquisitionWait(void);
#define READ_BIT_001B() ObserveReadAcquisitionWaitBit001B()
#define READ_BIT_0021() ObserveReadAcquisitionWaitBit0021()
#define DISPATCH_READ_ACQUISITION_REQUEST() ObserveDispatchReadAcquisitionWaitRequest()
#define YIELD_COROUTINE2() ObserveYieldCoroutine2ForReadAcquisitionWait()
#else
extern u8 FirmwareReadBit0021(void);
extern void DispatchPendingReadAcquisitionRequest(void);
extern void YieldCoroutine2(void);
#define READ_BIT_001B() FirmwareReadBit001B()
#define READ_BIT_0021() FirmwareReadBit0021()
#define DISPATCH_READ_ACQUISITION_REQUEST() DispatchPendingReadAcquisitionRequest()
#define YIELD_COROUTINE2() YieldCoroutine2()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveReadOrSeekSetSenseData(u8 sense_key, u16 asc_ascq);
extern void ObserveReadOrSeekSetSenseInformation(u32 information);
extern void ObserveReadOrSeekResetRawReadQueue(void);
extern void ObserveReadOrSeekInitializeCoroutine2(u32 entry_address);
extern void ObserveReadOrSeekSetNoSense(void);
extern void ObserveReadOrSeekDispatchAndWait(void);
#define READ_OR_SEEK_SET_SENSE(key, asc) ObserveReadOrSeekSetSenseData((key), (asc))
#define READ_OR_SEEK_SET_INFORMATION(value) ObserveReadOrSeekSetSenseInformation(value)
#define READ_OR_SEEK_RESET_QUEUE() ObserveReadOrSeekResetRawReadQueue()
#define READ_OR_SEEK_INITIALIZE_COROUTINE2(entry) ObserveReadOrSeekInitializeCoroutine2(entry)
#define READ_OR_SEEK_SET_NO_SENSE() ObserveReadOrSeekSetNoSense()
#define READ_OR_SEEK_DISPATCH_AND_WAIT() ObserveReadOrSeekDispatchAndWait()
#else
extern void SetSenseData(u8 sense_key, u16 asc_ascq);
extern void SetSenseInformation(u32 information);
extern void SetNoSenseData(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(u32 entry_address);
#define READ_OR_SEEK_SET_SENSE(key, asc) SetSenseData((key), (asc))
#define READ_OR_SEEK_SET_INFORMATION(value) SetSenseInformation(value)
#define READ_OR_SEEK_RESET_QUEUE() ResetRawReadQueueAndTransferState()
#define READ_OR_SEEK_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define READ_OR_SEEK_SET_NO_SENSE() SetNoSenseData()
#define READ_OR_SEEK_DISPATCH_AND_WAIT() ((void)DispatchAndWaitForReadAcquisitionResult())
#endif
#endif
