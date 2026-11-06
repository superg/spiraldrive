#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s8 ObserveIndexedState04AcquirePatternRange(s32 start_position, u32 frame_count);
extern void ObserveIndexedState04Yield(void);
extern void ObserveMediaReadControlYield(void);
extern u8 ObserveMediaReadControlSnapshotValid(void);
extern void ObserveMediaReadControlAction(u8 action);
extern void ObserveMediaReadControlCopy(u16 destination, u16 source, u8 byte_count);
extern void ObserveResetRawReadQueueForCoroutine2Entry(void);
extern void ObserveYieldCoroutine2ForIdleEntry(void);
extern void ObserveYieldCoroutine3ForIdleEntry(void);
extern void ObserveLoadControllerBufferSetupFromCurrentRawDescriptor(void);
extern void ObserveInitializeControllerBufferCursorForRawReadFinalizer(void);
extern void ObserveDispatchReadAcquisitionRequest5ForRawReadFinalizer(void);
extern u8 ObserveReadBit0005ForRawReadFinalizer(void);
extern void ObserveClearRawReadFinalizerBitsAtomic(void);

#define ResetRawReadQueueForCoroutine2Entry() ObserveResetRawReadQueueForCoroutine2Entry()
#define YieldCoroutine2ForIdleEntry() ObserveYieldCoroutine2ForIdleEntry()
#define YieldCoroutine3ForIdleEntry() ObserveYieldCoroutine3ForIdleEntry()
#define LOAD_CONTROLLER_BUFFER_SETUP_FROM_CURRENT_DESCRIPTOR() ObserveLoadControllerBufferSetupFromCurrentRawDescriptor()
#define InitializeControllerBufferCursorForRawReadFinalizer() ObserveInitializeControllerBufferCursorForRawReadFinalizer()
#define DispatchReadAcquisitionRequest5ForRawReadFinalizer() ObserveDispatchReadAcquisitionRequest5ForRawReadFinalizer()
#define ReadBit0005ForRawReadFinalizer() ObserveReadBit0005ForRawReadFinalizer()
#define ClearRawReadFinalizerBitsAtomic() ObserveClearRawReadFinalizerBitsAtomic()
#define AcquireIndexedState04PatternRange(start_position, frame_count) ObserveIndexedState04AcquirePatternRange((start_position), (frame_count))
#define YieldIndexedState04Coroutine0() ObserveIndexedState04Yield()
#define SetIndexedState04P3Bit0() FirmwareWriteBit0580(1u)
#define ClearIndexedState04P3Bit0() FirmwareWriteBit0580(0u)
#define SetIndexedState04P3Bit1() FirmwareWriteBit0581(1u)
#define ClearIndexedState04P3Bit1() FirmwareWriteBit0581(0u)
#define YieldMediaReadControlCoroutine3() ObserveMediaReadControlYield()
#define IsMediaReadControlSnapshotValid() ObserveMediaReadControlSnapshotValid()
#define ClearMediaReadControlSnapshot() ObserveMediaReadControlAction(1u)
#define ResetMediaReadControlType2Pipeline() ObserveMediaReadControlAction(2u)
#define InitializeMediaReadControlTrackModel() ObserveMediaReadControlAction(3u)
#define UpdateMediaReadControlTrackFlag() ObserveMediaReadControlAction(4u)
#define InitializeMediaReadControlAcquisition() ObserveMediaReadControlAction(5u)
#define CopyMediaReadControlBytes(destination, source, count) ObserveMediaReadControlCopy((destination), (source), (count))
#else
extern s8 PLEXTOR_NEAR_FUNCTION AcquireAndValidateRawReadPatternRange(s32 start_position, u32 frame_count);
extern void SetP3Bit0Atomic(void);
extern void ClearP3Bit0Atomic(void);
extern void SetP3Bit1Atomic(void);
extern void ClearP3Bit1Atomic(void);
extern void YieldCoroutine3(void);
extern u8 IsWriteBufferSnapshotValid(void);
extern void ClearWriteBufferSnapshotValid(void);
extern void ResetType2ControllerRecordPipelineState(void);
extern void InitializeMediaReadTrackModelAndPipelineState(void);
extern void UpdateTrackRecordControlFlag000A(void);
extern void InitializeMediaReadControlAcquisitionState(void);
extern void CopyBytes16(u16 destination, u16 source, u8 byte_count);
extern void ResetRawReadQueueAndTransferState(void);
extern void YieldCoroutine2(void);
extern void YieldCoroutine3(void);
extern void InitializeControllerBufferCursorRegistersIfNeeded(void);
extern void DispatchReadAcquisitionRequest5ForPipelineTarget(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0005ForRawReadFinalizer(void);
extern void PLEXTOR_NEAR_FUNCTION ClearRawReadFinalizerBitsAtomic(void);
extern void PLEXTOR_NEAR_FUNCTION LoadControllerBufferSetupFromCurrentRawDescriptor(void);

#define ResetRawReadQueueForCoroutine2Entry() ResetRawReadQueueAndTransferState()
#define YieldCoroutine2ForIdleEntry() YieldCoroutine2()
#define YieldCoroutine3ForIdleEntry() YieldCoroutine3()
#define LOAD_CONTROLLER_BUFFER_SETUP_FROM_CURRENT_DESCRIPTOR() LoadControllerBufferSetupFromCurrentRawDescriptor()
#define InitializeControllerBufferCursorForRawReadFinalizer() InitializeControllerBufferCursorRegistersIfNeeded()
#define DispatchReadAcquisitionRequest5ForRawReadFinalizer() DispatchReadAcquisitionRequest5ForPipelineTarget()
#define AcquireIndexedState04PatternRange(start_position, frame_count) AcquireAndValidateRawReadPatternRange((start_position), (frame_count))
#define YieldIndexedState04Coroutine0() YieldCoroutine0()
#define SetIndexedState04P3Bit0() SetP3Bit0Atomic()
#define ClearIndexedState04P3Bit0() ClearP3Bit0Atomic()
#define SetIndexedState04P3Bit1() SetP3Bit1Atomic()
#define ClearIndexedState04P3Bit1() ClearP3Bit1Atomic()
#define YieldMediaReadControlCoroutine3() YieldCoroutine3()
#define IsMediaReadControlSnapshotValid() IsWriteBufferSnapshotValid()
#define ClearMediaReadControlSnapshot() ClearWriteBufferSnapshotValid()
#define ResetMediaReadControlType2Pipeline() ResetType2ControllerRecordPipelineState()
#define InitializeMediaReadControlTrackModel() InitializeMediaReadTrackModelAndPipelineState()
#define UpdateMediaReadControlTrackFlag() UpdateTrackRecordControlFlag000A()
#define InitializeMediaReadControlAcquisition() InitializeMediaReadControlAcquisitionState()
#define CopyMediaReadControlBytes(destination, source, count) CopyBytes16((destination), (source), (count))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if !defined(PLEXTOR_HOST_TEST)
extern void RestartCoroutine1AndYieldImpl(u32 entry_point);

/* RAM:fee555 */
/* The native target is
 * nonreturning because it replaces SPX and returns to the scheduler. */
void RestartCoroutine1AndYield(u32 entry_point)
{
    RestartCoroutine1AndYieldImpl(entry_point);
}
#endif
#endif
