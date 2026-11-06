#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveBlankFormatPlanReset(void);
extern u8 ObserveBlankFormatPlanAcquire(s32 start_frame_position, u32 frame_count, u8 q_metadata_mode);
extern u8 ObserveBlankFormatPlanQSequence(u8 first_record_index);
#define PLAN_RESET() ObserveBlankFormatPlanReset()
#define PLAN_ACQUIRE(start, count, mode) ObserveBlankFormatPlanAcquire((start), (count), (mode))
#define PLAN_Q_SEQUENCE(first) ObserveBlankFormatPlanQSequence(first)
#else
#include "media/raw_read_pipeline_reset.h"
#include "platform/leaf_workers.h"
extern u8 PLEXTOR_NEAR_FUNCTION ResetAndAcquireRawReadRangeForBlankFormat(s32 start_frame_position, u32 frame_count, u8 q_metadata_mode);
#define PLAN_RESET() ResetRawReadPipelineBuffersAndState()
#define PLAN_ACQUIRE(start, count, mode) ResetAndAcquireRawReadRangeForBlankFormat((start), (count), (mode))
#define PLAN_Q_SEQUENCE(first) ExecuteRawReadQRecordSequence(first)
#endif
#endif
