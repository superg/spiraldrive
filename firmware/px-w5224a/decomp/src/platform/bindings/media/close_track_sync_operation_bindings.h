#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveCloseSyncResetRawRead(void);
extern void ObserveCloseSyncInitializeCoroutine2(u32 entry_point);
extern void ObserveCloseSyncYieldCoroutine0(void);
extern u8 ObserveCloseSyncEnsureOpc(void);
extern u8 ObserveCloseSyncPrepareRawRead(void);
extern void ObserveCloseSyncWriteFailure(void);
extern void ObserveCloseSyncCommitCloseState(void);
extern void ObserveCloseSyncFinalizeTrack(u8 scsi_opcode);
extern void ObserveCloseSyncCommitTail(void);
#define RESET_CLOSE_SYNC_RAW_READ() ObserveCloseSyncResetRawRead()
#define INITIALIZE_CLOSE_SYNC_COROUTINE2(entry) ObserveCloseSyncInitializeCoroutine2(entry)
#define YIELD_CLOSE_SYNC_COROUTINE0() ObserveCloseSyncYieldCoroutine0()
#define ENSURE_CLOSE_SYNC_OPC() ObserveCloseSyncEnsureOpc()
#define PREPARE_CLOSE_SYNC_RAW_READ() ObserveCloseSyncPrepareRawRead()
#define UPDATE_CLOSE_SYNC_WRITE_FAILURE() ObserveCloseSyncWriteFailure()
#define COMMIT_CLOSE_SYNC_STATE() ObserveCloseSyncCommitCloseState()
#define FINALIZE_CLOSE_SYNC_TRACK(opcode) ObserveCloseSyncFinalizeTrack(opcode)
#define COMMIT_CLOSE_SYNC_TAIL() ObserveCloseSyncCommitTail()
#else
extern void YieldCoroutine0(void);
#define RESET_CLOSE_SYNC_RAW_READ() ResetRawReadQueueAndTransferState()
#define INITIALIZE_CLOSE_SYNC_COROUTINE2(entry) InitializeCoroutine2(entry)
#define YIELD_CLOSE_SYNC_COROUTINE0() YieldCoroutine0()
#define ENSURE_CLOSE_SYNC_OPC() EnsureOpcForSelectedWriteSpeed()
#define PREPARE_CLOSE_SYNC_RAW_READ() PrepareCloseTrackOrSyncRawReadState()
#define UPDATE_CLOSE_SYNC_WRITE_FAILURE() UpdateWorkingTrackAfterWriteFailure()
#define COMMIT_CLOSE_SYNC_STATE() CommitTrackCloseStateForCurrentWriteSpeed()
#define FINALIZE_CLOSE_SYNC_TRACK(opcode) FinalizeTrackStateForScsiOpcode(opcode)
#define COMMIT_CLOSE_SYNC_TAIL() CommitTrackTailState()
#endif
#endif
