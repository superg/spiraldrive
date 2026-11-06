#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveWritePrefixRecoverPipeline(void);
extern u8 ObserveWritePrefixPrepareFrames(u8 omit_final_frame, u8 wait_for_completion);
extern u8 ObserveWritePrefixPreparePlan(u8 alternate_plan);
extern void ObserveWritePrefixAdvanceTrackRecord(void);
extern void ObserveWritePrefixRefreshIsrc(void);
#define DispatchWritePrefixRecoverPipeline() ObserveWritePrefixRecoverPipeline()
#define DispatchWritePrefixPrepareFrames(omit, wait) ObserveWritePrefixPrepareFrames((omit), (wait))
#define DispatchWritePrefixPreparePlan(alternate) ObserveWritePrefixPreparePlan(alternate)
#define DispatchWritePrefixAdvanceTrackRecord() ObserveWritePrefixAdvanceTrackRecord()
#define DispatchWritePrefixRefreshIsrc() ObserveWritePrefixRefreshIsrc()
#else
#define DispatchWritePrefixRecoverPipeline() TryRecoverRawReadPipelineFailure()
#define DispatchWritePrefixPrepareFrames(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define DispatchWritePrefixPreparePlan(alternate) PrepareRawReadAcquisitionPlan(alternate)
#define DispatchWritePrefixAdvanceTrackRecord() AdvanceRawReadTrackRecordAndUpdateCurrentQPacket()
#define DispatchWritePrefixRefreshIsrc() RefreshIsrcForCurrentRawTrack()
#endif
#endif
