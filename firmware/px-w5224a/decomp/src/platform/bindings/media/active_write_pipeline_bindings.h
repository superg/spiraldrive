#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveActiveWriteReadBit35(void);
extern void ObserveActiveWriteClearBit35(void);
extern void ObserveActiveWriteClearBit30(void);
extern u8 ObserveActiveWriteReadBit32(void);
extern void ObserveActiveWriteYield(void);
extern void ObserveActiveWriteRestartProducer(void);
extern u8 ObserveActiveWriteAcquireTail(s32 frame_count);
extern void ObserveActiveWriteFailure(void);
extern u8 ObserveActiveWriteAcquirePadding(void);
extern u8 ObserveActiveWritePreparePrefix(u8 omit_final_frame, u8 wait_for_completion);
extern u8 ObserveActiveWriteExecuteRange(u32 block_count);
extern void ObserveActiveWriteVerifyReadback(void);
extern void ObserveActiveWriteClearControllerWords(void);
extern void ObserveActiveWriteBuildAdr1(void);
extern u8 ObserveActiveWriteBuildRawQ(void);
extern u8 ObserveActiveWriteRunOpc(void);
extern void ObserveActiveWriteSetSense(u8 sense_key, u16 sense_code);
extern void ObserveActiveWriteAdvancePacket(void);
extern void ObserveActiveWriteFinalizeType1(void);
extern void ObserveActiveWriteFinalizeType2(void);
extern void ObserveActiveWriteInitializeCoroutine1(u32 entry_point);
extern void ObserveActiveWriteFinalizeOpcode(u8 opcode);
#define ACTIVE_READ_BIT35() ObserveActiveWriteReadBit35()
#define ACTIVE_CLEAR_BIT35() ObserveActiveWriteClearBit35()
#define ACTIVE_CLEAR_BIT30() ObserveActiveWriteClearBit30()
#define ACTIVE_READ_BIT32() ObserveActiveWriteReadBit32()
#define ACTIVE_YIELD() ObserveActiveWriteYield()
#define ACTIVE_RESTART_PRODUCER() ObserveActiveWriteRestartProducer()
#define ACTIVE_ACQUIRE_TAIL(count) ObserveActiveWriteAcquireTail(count)
#define ACTIVE_UPDATE_FAILURE() ObserveActiveWriteFailure()
#define ACTIVE_ACQUIRE_PADDING() ObserveActiveWriteAcquirePadding()
#define ACTIVE_PREPARE_PREFIX(omit, wait) ObserveActiveWritePreparePrefix((omit), (wait))
#define ACTIVE_EXECUTE_RANGE(count) ObserveActiveWriteExecuteRange(count)
#define ACTIVE_VERIFY_READBACK() ObserveActiveWriteVerifyReadback()
#define ACTIVE_CLEAR_CONTROLLER_WORDS() ObserveActiveWriteClearControllerWords()
#define ACTIVE_BUILD_ADR1() ObserveActiveWriteBuildAdr1()
#define ACTIVE_BUILD_RAW_Q() ObserveActiveWriteBuildRawQ()
#define ACTIVE_RUN_OPC() ObserveActiveWriteRunOpc()
#define ACTIVE_SET_SENSE(key, code) ObserveActiveWriteSetSense((key), (code))
#define ACTIVE_ADVANCE_PACKET() ObserveActiveWriteAdvancePacket()
#define ACTIVE_FINALIZE_TYPE1() ObserveActiveWriteFinalizeType1()
#define ACTIVE_FINALIZE_TYPE2() ObserveActiveWriteFinalizeType2()
#define ACTIVE_INITIALIZE_COROUTINE1(entry) ObserveActiveWriteInitializeCoroutine1(entry)
#define ACTIVE_FINALIZE_OPCODE(opcode) ObserveActiveWriteFinalizeOpcode(opcode)
#else
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0035ForActiveWriteFinalizer(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0035ForReadAcquisitionCoroutine3(void);
extern void PLEXTOR_NEAR_FUNCTION ClearBit0030ForReadAcquisitionCoroutine3(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0032ForReadAcquisitionCoroutine3(void);
extern void YieldCoroutine0(void);
extern u8 ExecuteWriteTransferBlockRange(u32 block_count);
extern u8 InitializeAndBuildRawReadQRecordQueue(void);
#define ACTIVE_READ_BIT35() ReadBit0035ForActiveWriteFinalizer()
#define ACTIVE_CLEAR_BIT35() ClearBit0035ForReadAcquisitionCoroutine3()
#define ACTIVE_CLEAR_BIT30() ClearBit0030ForReadAcquisitionCoroutine3()
#define ACTIVE_READ_BIT32() ReadBit0032ForReadAcquisitionCoroutine3()
#define ACTIVE_YIELD() YieldCoroutine0()
#define ACTIVE_RESTART_PRODUCER() RestartReadAcquisitionCoroutineForProducerWait()
#define ACTIVE_ACQUIRE_TAIL(count) AcquireRawReadFramesForWorkingTrack(count)
#define ACTIVE_UPDATE_FAILURE() UpdateWorkingTrackAfterWriteFailure()
#define ACTIVE_ACQUIRE_PADDING() AcquireFixedPacketPaddingRawSectors()
#define ACTIVE_PREPARE_PREFIX(omit, wait) PrepareRawReadFramePrefixAndOptionallyWait((omit), (wait))
#define ACTIVE_EXECUTE_RANGE(count) ExecuteWriteTransferBlockRange(count)
#define ACTIVE_VERIFY_READBACK() ExecuteWriteAndVerifyReadback()
#define ACTIVE_CLEAR_CONTROLLER_WORDS() ClearControllerBufferWordsC119AndC517()
#define ACTIVE_BUILD_ADR1() BuildWorkingTrackAdr1RawQRecord()
#define ACTIVE_BUILD_RAW_Q() InitializeAndBuildRawReadQRecordQueue()
#define ACTIVE_RUN_OPC() RunOpcForCurrentWriteSpeed()
#define ACTIVE_SET_SENSE(key, code) SetSenseData((key), (code))
#define ACTIVE_ADVANCE_PACKET() AdvanceWorkingTrackPacketBoundaryFromProgress()
#define ACTIVE_FINALIZE_TYPE1() FinalizeWriteType1WorkingTrackState()
#define ACTIVE_FINALIZE_TYPE2() FinalizeWriteType2TrackTableState()
#define ACTIVE_INITIALIZE_COROUTINE1(entry) InitializeCoroutine1(entry)
#define ACTIVE_FINALIZE_OPCODE(opcode) FinalizeTrackStateForScsiOpcode(opcode)
#endif
#endif
