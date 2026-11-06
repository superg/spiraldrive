#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u16 ObserveWriteRangeSequenceBuild(s32 start_frame_position, u32 block_count);
extern u8 ObserveWriteRangeSequencePrepare(u8 omit_final_prefix_frame, u8 wait_for_completion);
extern void ObserveWriteRangeSequenceFailure(u8 failure_code);
extern void ObserveWriteRangeSequenceYield(void);
extern u8 ObserveWriteRangeSequenceSelect(u8 start_index, u8 record_count);
extern void ObserveWriteRangeSequenceInitializeTrack(void);
extern void ObserveWriteRangeSequenceFill(fw_addr_t buffer_address, u32 byte_count, u8 value);
extern u8 ObserveWriteRangeSequenceAcquire(s32 target_address, u8 mapping_mode);
extern void ObserveWriteRangeSequenceWait(void);
#define RANGE_BUILD(start, count) ObserveWriteRangeSequenceBuild((start), (count))
#define RANGE_PREPARE(omit, wait) ObserveWriteRangeSequencePrepare((omit), (wait))
#define RANGE_FAILURE(code) ObserveWriteRangeSequenceFailure(code)
#define RANGE_YIELD() ObserveWriteRangeSequenceYield()
#define RANGE_SELECT(start, count) ObserveWriteRangeSequenceSelect((start), (count))
#define RANGE_INITIALIZE_TRACK() ObserveWriteRangeSequenceInitializeTrack()
#define RANGE_FILL(address, count, value) ObserveWriteRangeSequenceFill((address), (count), (value))
#define RANGE_ACQUIRE(address, mode) ObserveWriteRangeSequenceAcquire((address), (mode))
#define RANGE_WAIT() ObserveWriteRangeSequenceWait()
#else
extern void YieldCoroutine0(void);
#define RANGE_BUILD(start, count) BuildSortedWriteControllerBufferRanges((start), (count))
#define RANGE_PREPARE(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define RANGE_FAILURE(code) ReportReadPipelineFailure(code)
#define RANGE_YIELD() YieldCoroutine0()
#define RANGE_SELECT(start, count) SelectWriteControllerBufferRangeEndIndex((start), (count))
#define RANGE_INITIALIZE_TRACK() InitializeReadVerifyWorkingTrackState()
#define RANGE_FILL(address, count, value) FillControllerBufferRange((address), (count), (value))
#define RANGE_ACQUIRE(address, mode) AcquireWritePreservationRawReadAtAddress((address), (mode))
#define RANGE_WAIT() WaitForWritePipelineStatusCompletion()
#endif
#endif
