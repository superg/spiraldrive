#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveCaptureInheritedR15ForResetRange(void);
extern u8 ObserveAcquireRawSectorSequenceForResetRange(u32 frame_count, u8 q_metadata_mode, u8 preserve_main_buffer);
extern u8 ObservePollRawReadPipelineForResetRange(u8 wait_for_completion);
extern void ObserveReportReadPipelineFailureForResetRange(u8 failure_code);
extern void ObserveReportSense030C09ForResetRange(void);
extern void ObserveYieldResetRangeCoroutine0(void);
#define CAPTURE_INHERITED_R15() ObserveCaptureInheritedR15ForResetRange()
#define ACQUIRE_RESET_RANGE(count, mode, preserve) ObserveAcquireRawSectorSequenceForResetRange((count), (mode), (preserve))
#define POLL_RESET_RANGE(wait) ObservePollRawReadPipelineForResetRange(wait)
#define REPORT_RESET_RANGE_FAILURE(code) ObserveReportReadPipelineFailureForResetRange(code)
#define REPORT_RESET_RANGE_SENSE_030C09() ObserveReportSense030C09ForResetRange()
#define YIELD_RESET_RANGE() ObserveYieldResetRangeCoroutine0()
#else
extern u8 PLEXTOR_NEAR_FUNCTION CaptureInheritedR15ForResetAndAcquireRawReadRange(void);
extern u8 PLEXTOR_NEAR_FUNCTION AcquireRawSectorSequenceForResetRange(u32 frame_count, u8 q_metadata_mode, u8 preserve_main_buffer);
extern u8 PLEXTOR_NEAR_FUNCTION PollRawReadPipelineProgress(u8 wait_for_completion);
extern void ReportReadPipelineFailure(u8 failure_code);
extern void ReportSense030C09FromRawReadQueue(void);
extern void YieldCoroutine0(void);
#define CAPTURE_INHERITED_R15() CaptureInheritedR15ForResetAndAcquireRawReadRange()
#define ACQUIRE_RESET_RANGE(count, mode, preserve) AcquireRawSectorSequenceForResetRange((count), (mode), (preserve))
#define POLL_RESET_RANGE(wait) PollRawReadPipelineProgress(wait)
#define REPORT_RESET_RANGE_FAILURE(code) ReportReadPipelineFailure(code)
#define REPORT_RESET_RANGE_SENSE_030C09() ReportSense030C09FromRawReadQueue()
#define YIELD_RESET_RANGE() YieldCoroutine0()
#endif
#endif
