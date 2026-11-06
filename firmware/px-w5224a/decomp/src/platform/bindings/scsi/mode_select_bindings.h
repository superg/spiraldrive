#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveModeSelectDataOut(u32 transfer_length);
extern u32 ObserveModeSelectControllerCopy(fw_addr_t destination, u32 controller_offset, u16 byte_count);
extern u8 ObserveModeSelectLengthMatch(u8 page_code, u8 payload_length);
extern u8 ObserveModeSelectValidate(u8 page_code);
extern void ObserveModeSelectRuntimeCopy(fw_addr_t destination, fw_addr_t source, u8 byte_count);
extern void ObserveModeSelectApply(u8 page_code);
extern void ObserveModeSelectSense(u8 sense_key, u16 asc_ascq);
extern void ObserveModeSelectNoSense(void);
extern void ObserveModeSelectResetRawRead(void);
extern void ObserveModeSelectInitializeCoroutine2(u32 entry_point);
extern void ObserveModeSelectDispatchRequest5(void);
extern void ObserveModeSelectYield(void);
#define MODE_SELECT_DATA_OUT(length) ObserveModeSelectDataOut(length)
#define MODE_SELECT_CONTROLLER_COPY(destination, offset, count) ObserveModeSelectControllerCopy((destination), (offset), (count))
#define MODE_SELECT_LENGTH_MATCH(page, length) ObserveModeSelectLengthMatch((page), (length))
#define MODE_SELECT_VALIDATE(page) ObserveModeSelectValidate(page)
#define MODE_SELECT_RUNTIME_COPY(destination, source, count) ObserveModeSelectRuntimeCopy((destination), (source), (count))
#define MODE_SELECT_APPLY(page, call) ObserveModeSelectApply(page)
#define MODE_SELECT_SENSE(key, code) ObserveModeSelectSense((key), (code))
#define MODE_SELECT_NO_SENSE() ObserveModeSelectNoSense()
#define MODE_SELECT_RESET_RAW_READ() ObserveModeSelectResetRawRead()
#define MODE_SELECT_INITIALIZE_COROUTINE2(entry) ObserveModeSelectInitializeCoroutine2(entry)
#define MODE_SELECT_DISPATCH_REQUEST5() ObserveModeSelectDispatchRequest5()
#define MODE_SELECT_YIELD() ObserveModeSelectYield()
#else
#include "platform/coroutine_initialization.h"
#include "platform/leaf_workers.h"
#include "platform/memory.h"
#include "platform/status_mapping.h"
extern void YieldCoroutine0(void);
#define MODE_SELECT_DATA_OUT(length) ExecuteControllerDataOutAndMapStatus(length)
#define MODE_SELECT_CONTROLLER_COPY(destination, offset, count) CopyFromControllerBufferWindow((offset), (destination), (count))
#define MODE_SELECT_LENGTH_MATCH(page, length) DoesModePagePayloadLengthMatch((page), (length))
#define MODE_SELECT_RUNTIME_COPY(destination, source, count) CopyBytes8((destination), (source), (count))
#define MODE_SELECT_APPLY(page, call) call
#define MODE_SELECT_SENSE(key, code) SetSenseData((key), (code))
#define MODE_SELECT_NO_SENSE() SetNoSenseData()
#define MODE_SELECT_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define MODE_SELECT_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define MODE_SELECT_DISPATCH_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define MODE_SELECT_YIELD() YieldCoroutine0()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
static u8 ValidateModeSelectPage(u8 page_code)
{
    switch(page_code)
    {
    case 0x01u:
        return ValidateModeSelectPage01ParameterList();
    case 0x02u:
        return ValidateModeSelectPage0EWithTimingValue();
    case 0x03u:
        return ValidateModeSelectPage03ParameterList();
    case 0x05u:
        return ValidateModeSelectPage05ParameterList();
    case 0x07u:
        return ValidateModeSelectPage06AllowedCode();
    case 0x08u:
        return ValidateModeSelectPage07ParameterList();
    case 0x0du:
        return ValidateModeSelectPage06ParameterList();
    case 0x0eu:
        return ValidateModeSelectPage0EParameterList();
    case 0x1au:
        return ValidateModeSelectPage1AParameterList();
    default:
        return ValidateModeSelectPage1DParameterList();
    }
}
#define MODE_SELECT_VALIDATE(page) ValidateModeSelectPage(page)
#endif
#endif
