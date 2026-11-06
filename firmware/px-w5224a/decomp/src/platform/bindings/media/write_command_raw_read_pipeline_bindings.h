#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveWriteRawPipelineCopyModeBit(void);
extern void ObserveWriteRawPipelineSetCaptureBit(void);
extern void ObserveWriteRawPipelineFill(fw_addr_t address, u32 count, u8 value);
extern void ObserveWriteRawPipelineReset(void);
extern u32 ObserveWriteRawPipelineInitializeRange(void);
extern u8 ObserveWriteRawPipelineInitializeMode(void);
extern u8 ObserveWriteRawPipelineContinuation(void);
extern u8 ObserveWriteRawPipelinePreparePrefix(void);
extern void ObserveWriteRawPipelineFailure(u8 code);
extern void ObserveWriteRawPipelineQueueSense(void);
extern u8 ObserveWriteRawPipelineServiceBatch(void);
extern void ObserveWriteRawPipelineYield(void);
extern u8 ObserveWriteRawPipelineFinalize(void);
extern void ObserveWriteRawPipelineNoSense(void);
#define CopyWriteRawPipelineModeBit() ObserveWriteRawPipelineCopyModeBit()
#define SetWriteRawPipelineCaptureBit() ObserveWriteRawPipelineSetCaptureBit()
#define WriteRawPipelineFill(a, c, v) ObserveWriteRawPipelineFill((a), (c), (v))
#define WriteRawPipelineReset() ObserveWriteRawPipelineReset()
#define WriteRawPipelineInitializeRange() ObserveWriteRawPipelineInitializeRange()
#define WriteRawPipelineInitializeMode() ObserveWriteRawPipelineInitializeMode()
#define WriteRawPipelineContinuation() ObserveWriteRawPipelineContinuation()
#define WriteRawPipelinePreparePrefix() ObserveWriteRawPipelinePreparePrefix()
#define WriteRawPipelineFailure(code) ObserveWriteRawPipelineFailure(code)
#define WriteRawPipelineQueueSense() ObserveWriteRawPipelineQueueSense()
#define WriteRawPipelineServiceBatch() ObserveWriteRawPipelineServiceBatch()
#define WriteRawPipelineYield() ObserveWriteRawPipelineYield()
#define WriteRawPipelineFinalize() ObserveWriteRawPipelineFinalize()
#define WriteRawPipelineNoSense() ObserveWriteRawPipelineNoSense()
#else
extern void PLEXTOR_NEAR_FUNCTION CopyBit002FTo0030Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0035Atomic(void);
extern void ResetRawReadPipelineBuffersAndState(void);
extern void YieldCoroutine0(void);
#define CopyWriteRawPipelineModeBit() CopyBit002FTo0030Atomic()
#define SetWriteRawPipelineCaptureBit() SetBit0035Atomic()
#define WriteRawPipelineFill(a, c, v) FillControllerBufferRange((a), (c), (v))
#define WriteRawPipelineReset() ResetRawReadPipelineBuffersAndState()
#define WriteRawPipelineInitializeRange() InitializeHostTransferRange()
#define WriteRawPipelineInitializeMode() InitializeRawReadPipelineForCurrentMode()
#define WriteRawPipelineContinuation() CheckWriteRawReadPipelineContinuation()
#define WriteRawPipelinePreparePrefix() PrepareWriteCommandRawReadPrefix()
#define WriteRawPipelineFailure(code) ReportReadPipelineFailure(code)
#define WriteRawPipelineQueueSense() ReportSense030C09FromRawReadQueue()
#define WriteRawPipelineServiceBatch() ServiceRawReadTransferBatch()
#define WriteRawPipelineYield() YieldCoroutine0()
#define WriteRawPipelineFinalize() FinalizeWriteCommandRawReadPipelinePreparation()
#define WriteRawPipelineNoSense() SetNoSenseData()
#endif
#endif
