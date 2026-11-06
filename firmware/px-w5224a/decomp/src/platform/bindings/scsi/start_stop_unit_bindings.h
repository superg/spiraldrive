#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveStartStopValidateCdb(fw_addr_t mask_address);
extern void ObserveStartStopUnsupported(void);
extern void ObserveStartStopSetSense(u8 key, u16 code);
extern void ObserveStartStopSetSenseFromStatus(u8 status);
extern void ObserveStartStopSetNoSense(void);
extern void ObserveStartStopSetP3Bit(u8 bit_index, u8 value);
extern void ObserveStartStopResetMediaCoroutines(void);
extern void ObserveStartStopYieldCoroutine0(void);
extern void ObserveStartStopDispatchAndWait(void);
extern void ObserveStartStopDispatchPending(void);
extern void ObserveStartStopPrepareAtaCompletion(void);
extern void ObserveStartStopPublishAtaCompletion(void);
extern void ObserveStartStopPrepareMediaWrite(void);
extern void ObserveStartStopAction2Worker(void);
extern void ObserveStartStopAction3Worker(void);
#define START_STOP_VALIDATE(mask) ObserveStartStopValidateCdb(mask)
#define START_STOP_UNSUPPORTED() ObserveStartStopUnsupported()
#define START_STOP_SET_SENSE(key, code) ObserveStartStopSetSense((key), (code))
#define START_STOP_STATUS_SENSE(status) ObserveStartStopSetSenseFromStatus(status)
#define START_STOP_NO_SENSE() ObserveStartStopSetNoSense()
#define START_STOP_P3(bit, value) ObserveStartStopSetP3Bit((bit), (value))
#define START_STOP_RESET_COROUTINES() ObserveStartStopResetMediaCoroutines()
#define START_STOP_YIELD() ObserveStartStopYieldCoroutine0()
#define START_STOP_DISPATCH_AND_WAIT() ObserveStartStopDispatchAndWait()
#define START_STOP_DISPATCH_PENDING() ObserveStartStopDispatchPending()
#define START_STOP_PREPARE_ATA() ObserveStartStopPrepareAtaCompletion()
#define START_STOP_PUBLISH_ATA() ObserveStartStopPublishAtaCompletion()
#define START_STOP_PREPARE_WRITE() ObserveStartStopPrepareMediaWrite()
#define START_STOP_ACTION2_WORKER() ObserveStartStopAction2Worker()
#define START_STOP_ACTION3_WORKER() ObserveStartStopAction3Worker()
#else
#include "platform/format_unit_type2_pipeline.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine0(void);
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit0Atomic(void);
extern void SetP3Bit1Atomic(void);
extern void ClearP3Bit1Atomic(void);
#define START_STOP_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define START_STOP_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define START_STOP_SET_SENSE(key, code) SetSenseData((key), (code))
#define START_STOP_STATUS_SENSE(status) SetSenseDataFromStatusCode(status)
#define START_STOP_NO_SENSE() SetNoSenseData()
#define START_STOP_P3(bit, value)                                                                                                                                                                      \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if((bit) == 0u)                                                                                                                                                                                \
        {                                                                                                                                                                                              \
            if((value) != 0u)                                                                                                                                                                          \
                SetP3Bit0Atomic();                                                                                                                                                                     \
            else                                                                                                                                                                                       \
                ClearP3Bit0Atomic();                                                                                                                                                                   \
        }                                                                                                                                                                                              \
        else                                                                                                                                                                                           \
        {                                                                                                                                                                                              \
            if((value) != 0u)                                                                                                                                                                          \
                SetP3Bit1Atomic();                                                                                                                                                                     \
            else                                                                                                                                                                                       \
                ClearP3Bit1Atomic();                                                                                                                                                                   \
        }                                                                                                                                                                                              \
    } while(0)
#define START_STOP_RESET_COROUTINES() ResetMediaProcessingCoroutines()
#define START_STOP_YIELD() YieldCoroutine0()
#define START_STOP_DISPATCH_AND_WAIT() ((void)DispatchAndWaitForReadAcquisitionResult())
#define START_STOP_DISPATCH_PENDING() DispatchPendingReadAcquisitionRequest()
#define START_STOP_PREPARE_ATA() PrepareAtaCompletionStatusFromSense()
#define START_STOP_PUBLISH_ATA() PublishAtaCommandCompletion()
#define START_STOP_PREPARE_WRITE() PrepareMediaWritePipelineState()
#define START_STOP_ACTION2_WORKER() FUN_RAM_fdcb8e()
#define START_STOP_ACTION3_WORKER() FUN_RAM_fdcbdf()
#endif
#endif
