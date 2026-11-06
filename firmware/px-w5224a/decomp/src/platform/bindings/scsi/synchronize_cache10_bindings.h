#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveSynchronizeCacheValidate(fw_addr_t mask);
extern void ObserveSynchronizeCacheUnsupported(void);
extern void ObserveSynchronizeCacheNoSense(void);
extern void ObserveSynchronizeCacheComplete(void);
extern void ObserveSynchronizeCacheClearBit30(void);
extern void ObserveSynchronizeCacheClearBit35(void);
extern void ObserveSynchronizeCacheCloseOperation(void);
extern void ObserveSynchronizeCacheSequenceError(void);
extern void ObserveSynchronizeCacheFinalizeWorkingTrack(void);
extern void ObserveSynchronizeCacheFinalizeActiveWrite(void);
#define SYNC_VALIDATE(mask) ObserveSynchronizeCacheValidate(mask)
#define SYNC_UNSUPPORTED() ObserveSynchronizeCacheUnsupported()
#define SYNC_NO_SENSE() ObserveSynchronizeCacheNoSense()
#define SYNC_COMPLETE() ObserveSynchronizeCacheComplete()
#define SYNC_CLEAR_BIT30() ObserveSynchronizeCacheClearBit30()
#define SYNC_CLEAR_BIT35() ObserveSynchronizeCacheClearBit35()
#define SYNC_CLOSE_OPERATION() ObserveSynchronizeCacheCloseOperation()
#define SYNC_SEQUENCE_ERROR() ObserveSynchronizeCacheSequenceError()
#define SYNC_FINALIZE_WORKING() ObserveSynchronizeCacheFinalizeWorkingTrack()
#define SYNC_FINALIZE_ACTIVE() ObserveSynchronizeCacheFinalizeActiveWrite()
#else
#include "media/close_track_sync_operation.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
extern void PLEXTOR_NEAR_FUNCTION ClearBit0030ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0035ForReadAcquisitionCoroutine3(void);
#define SYNC_VALIDATE(mask) ValidateScsiCdbFieldMasks(mask)
#define SYNC_UNSUPPORTED() HandleUnsupportedScsiCommand()
#define SYNC_NO_SENSE() SetNoSenseData()
#define SYNC_COMPLETE() CompleteScsiCommandWithNoSense()
#define SYNC_CLEAR_BIT30() ClearBit0030ForReadAcquisitionCoroutine3()
#define SYNC_CLEAR_BIT35() ClearBit0035ForReadAcquisitionCoroutine3()
#define SYNC_CLOSE_OPERATION() ExecuteCloseTrackOrSynchronizeCacheOperation()
#define SYNC_SEQUENCE_ERROR() SetCommandSequenceErrorSense()
#define SYNC_FINALIZE_WORKING() FinalizeWorkingTrackAfterRepairOrCacheSync()
#define SYNC_FINALIZE_ACTIVE() FinalizeActiveWritePipelineAndTrackState()
#endif
#endif
