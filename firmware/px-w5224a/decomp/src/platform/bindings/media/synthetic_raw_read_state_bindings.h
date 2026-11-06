#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveSyntheticOperationPrepareRange(u8 use_prepared_plan, s32 start_frame, s32 end_frame_exclusive);
extern u8 ObserveSyntheticOperationPreparePrefix(u8 omit_final_prefix_frame, u8 wait_for_completion);
extern void ObserveClearSyntheticRawReadPipelineBits(void);
extern u8 ObserveSyntheticOperationBuildRawQQueue(void);
extern void ObserveSyntheticOperationBuildTrackTable(void);

#define PREPARE_SYNTHETIC_RANGE(plan, start, end) ObserveSyntheticOperationPrepareRange((plan), (start), (end))
#define PREPARE_SYNTHETIC_PREFIX(omit, wait) ObserveSyntheticOperationPreparePrefix((omit), (wait))
#define CLEAR_SYNTHETIC_PIPELINE_BITS() ObserveClearSyntheticRawReadPipelineBits()
#define BUILD_SYNTHETIC_RAW_Q_QUEUE() ObserveSyntheticOperationBuildRawQQueue()
#define BUILD_SYNTHETIC_TRACK_TABLE() ObserveSyntheticOperationBuildTrackTable()
#else
extern void ClearSyntheticRawReadPipelineBitsAtomic(void);
#define PREPARE_SYNTHETIC_RANGE(plan, start, end) PrepareSyntheticRawReadPatternRange((plan), (start), (end))
#define PREPARE_SYNTHETIC_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define CLEAR_SYNTHETIC_PIPELINE_BITS() ClearSyntheticRawReadPipelineBitsAtomic()
#define BUILD_SYNTHETIC_RAW_Q_QUEUE() InitializeAndBuildRawReadQRecordQueue()
#define BUILD_SYNTHETIC_TRACK_TABLE() BuildSyntheticTrackTableAndAcquireLeadInLeadOutSequences()
#endif
#endif
