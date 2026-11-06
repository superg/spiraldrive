#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveResetRawReadQueueForBufferTransfer(void);
extern void ObserveInitializeCoroutine2ForBufferTransfer(fw_addr_t entry_address);
extern void ObserveDispatchReadAcquisitionForBufferTransfer(void);
extern void ObserveYieldCoroutine0ForBufferTransfer(void);
#define RESET_BUFFER_TRANSFER_QUEUE() ObserveResetRawReadQueueForBufferTransfer()
#define INITIALIZE_BUFFER_TRANSFER_COROUTINE2(address) ObserveInitializeCoroutine2ForBufferTransfer(address)
#define DISPATCH_BUFFER_TRANSFER_ACQUISITION() ObserveDispatchReadAcquisitionForBufferTransfer()
#define YIELD_BUFFER_TRANSFER_COROUTINE0() ObserveYieldCoroutine0ForBufferTransfer()
#else
extern void ResetRawReadQueueAndTransferState(void);
extern void InitializeCoroutine2(fw_addr_t entry_address);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern void YieldCoroutine0(void);
#define RESET_BUFFER_TRANSFER_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_BUFFER_TRANSFER_COROUTINE2(address) InitializeCoroutine2(address)
#define DISPATCH_BUFFER_TRANSFER_ACQUISITION() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define YIELD_BUFFER_TRANSFER_COROUTINE0() YieldCoroutine0()
#endif
#endif
