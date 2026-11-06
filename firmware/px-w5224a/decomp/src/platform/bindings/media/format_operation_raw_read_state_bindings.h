#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFormatRawResetPipeline(void);
extern void ObserveFormatRawInitializeSectorControl(void);
extern void ObserveFormatRawClearPipelineStatus(void);
extern u8 ObserveFormatRawPreparePlan(u8 alternate);
extern u8 ObserveFormatRawPoll(u8 count);
extern void ObserveFormatRawReportFailure(u8 code);
extern void ObserveFormatRawReportQueueSense(void);
extern u32 ObserveFormatRawDivide(u32 dividend, u32 divisor);
extern u32 ObserveFormatRawMultiply(u32 left, u32 right);
extern u8 ObserveFormatRawAcquirePreroll(s32 start, u32 count);
extern u8 ObserveFormatRawExecuteQSequence(u8 first_index);
extern u8 ObserveFormatRawInitializeQQueue(void);
extern u8 ObserveFormatRawAcquireLeadSequences(void);
extern u8 ObserveFormatRawAcquireMode2Range(s32 start, u32 count);
#define FORMAT_RAW_RESET() ObserveFormatRawResetPipeline()
#define FORMAT_RAW_INIT_CONTROL() ObserveFormatRawInitializeSectorControl()
#define FORMAT_RAW_CLEAR_STATUS() ObserveFormatRawClearPipelineStatus()
#define FORMAT_RAW_PREPARE_PLAN(value) ObserveFormatRawPreparePlan(value)
#define FORMAT_RAW_POLL(value) ObserveFormatRawPoll(value)
#define FORMAT_RAW_REPORT(value) ObserveFormatRawReportFailure(value)
#define FORMAT_RAW_REPORT_SENSE() ObserveFormatRawReportQueueSense()
#define FORMAT_RAW_DIVIDE(left, right) ObserveFormatRawDivide((left), (right))
#define FORMAT_RAW_MULTIPLY(left, right) ObserveFormatRawMultiply((left), (right))
#define FORMAT_RAW_ACQUIRE_PREROLL(start, count) ObserveFormatRawAcquirePreroll((start), (count))
#define FORMAT_RAW_EXECUTE_Q(first) ObserveFormatRawExecuteQSequence(first)
#define FORMAT_RAW_INIT_Q_QUEUE() ObserveFormatRawInitializeQQueue()
#define FORMAT_RAW_ACQUIRE_LEAD() ObserveFormatRawAcquireLeadSequences()
#define FORMAT_RAW_ACQUIRE_MODE2(start, count) ObserveFormatRawAcquireMode2Range((start), (count))
#else
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
extern u8 AcquireLeadInAndLeadOutRawSectorDescriptorSequences(void);
#define FORMAT_RAW_RESET() ResetRawReadPipelineBuffersAndState()
#define FORMAT_RAW_INIT_CONTROL() InitializeRawReadSectorControlState()
#define FORMAT_RAW_CLEAR_STATUS() ClearRawReadPipelineStatusState()
#define FORMAT_RAW_PREPARE_PLAN(value) PrepareRawReadAcquisitionPlan(value)
#define FORMAT_RAW_POLL(value) PollRawReadPipelineProgress(value)
#define FORMAT_RAW_REPORT(value) ReportReadPipelineFailure(value)
#define FORMAT_RAW_REPORT_SENSE() ReportSense030C09FromRawReadQueue()
#define FORMAT_RAW_DIVIDE(left, right) DivideU32Quotient((left), (right))
#define FORMAT_RAW_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define FORMAT_RAW_ACQUIRE_PREROLL(start, count) AcquireRawReadRangeWithFiveFramePreroll((start), (count))
#define FORMAT_RAW_EXECUTE_Q(first) ExecuteRawReadQRecordSequence(first)
#define FORMAT_RAW_INIT_Q_QUEUE() InitializeAndBuildRawReadQRecordQueue()
#define FORMAT_RAW_ACQUIRE_LEAD() AcquireLeadInAndLeadOutRawSectorDescriptorSequences()
#define FORMAT_RAW_ACQUIRE_MODE2(start, count) AcquireRawReadRangeMode2WithTwoFrameAdjustment((start), (count))
#endif
#endif
