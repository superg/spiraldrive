#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveSendDiagnostic(u8 event, u32 argument0, u32 argument1);
#define DIAGNOSTIC_EVENT(event, argument0, argument1) ObserveSendDiagnostic((event), (argument0), (argument1))
#define DIAGNOSTIC_VALIDATE_CDB(mask) ((u8)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_VALIDATE_CDB, (mask), 0ul))
#define DIAGNOSTIC_REJECT() ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_REJECT, 0ul, 0ul))
#define DIAGNOSTIC_RESET_RAW_READ() ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_RESET_RAW_READ, 0ul, 0ul))
#define DIAGNOSTIC_INITIALIZE_COROUTINE2(entry) ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_INITIALIZE_COROUTINE2, (entry), 0ul))
#define DIAGNOSTIC_DISPATCH_REQUEST5() ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_DISPATCH_REQUEST5, 0ul, 0ul))
#define DIAGNOSTIC_YIELD_COROUTINE0() ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_YIELD_COROUTINE0, 0ul, 0ul))
#define DIAGNOSTIC_ZERO_WORKSPACE(address, count) ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_ZERO_WORKSPACE, (address), (count)))
#define DIAGNOSTIC_VALIDATE_BOOT() ((u8)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_VALIDATE_BOOT, 0ul, 0ul))
#define DIAGNOSTIC_TEST_CONTROLLER_BUFFER() ((u8)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_TEST_CONTROLLER_BUFFER, 0ul, 0ul))
#define DIAGNOSTIC_DATA_OUT(address, count) ((u8)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_DATA_OUT, (address), (count)))
#define DIAGNOSTIC_SET_SENSE(key, code) ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_SET_SENSE, (key), (code)))
#define DIAGNOSTIC_SET_NO_SENSE() ((void)DIAGNOSTIC_EVENT(SEND_DIAGNOSTIC_SET_NO_SENSE, 0ul, 0ul))
#else
extern void YieldCoroutine0(void);
#define DIAGNOSTIC_VALIDATE_CDB(mask) ValidateScsiCdbFieldMasks(mask)
#define DIAGNOSTIC_REJECT() HandleUnsupportedScsiCommand()
#define DIAGNOSTIC_RESET_RAW_READ() ResetRawReadQueueAndTransferState()
#define DIAGNOSTIC_INITIALIZE_COROUTINE2(entry) InitializeCoroutine2(entry)
#define DIAGNOSTIC_DISPATCH_REQUEST5() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define DIAGNOSTIC_YIELD_COROUTINE0() YieldCoroutine0()
#define DIAGNOSTIC_ZERO_WORKSPACE(address, count) ZeroMemoryWords((address), (count))
#define DIAGNOSTIC_VALIDATE_BOOT() ValidateBootAndConfigurationMemory()
#define DIAGNOSTIC_TEST_CONTROLLER_BUFFER() TestControllerBufferMemoryAddressing()
#define DIAGNOSTIC_DATA_OUT(address, count) ExecuteDataOutAndCopyFromControllerBuffer((address), (count))
#define DIAGNOSTIC_SET_SENSE(key, code) SetSenseData((key), (code))
#define DIAGNOSTIC_SET_NO_SENSE() SetNoSenseData()
#endif
#endif
