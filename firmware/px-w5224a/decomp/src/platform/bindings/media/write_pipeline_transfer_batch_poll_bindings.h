#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveWritePipelineBatchOperation(u8 operation, u32 argument0, u32 argument1, u32 argument2);
#define OBSERVE_BATCH(op, a, b, c) ObserveWritePipelineBatchOperation((op), (a), (b), (c))
#define LATEST_DESCRIPTOR() ((u16)OBSERVE_BATCH(WRITE_BATCH_LATEST_DESCRIPTOR, 0ul, 0ul, 0ul))
#define ACQUIRE_PREFIX(mode, source) ((u8)OBSERVE_BATCH(WRITE_BATCH_ACQUIRE_PREFIX, (mode), (source), 0ul))
#define YIELD_BATCH() ((void)OBSERVE_BATCH(WRITE_BATCH_YIELD, 0ul, 0ul, 0ul))
#define SHOULD_WAIT() ((u8)OBSERVE_BATCH(WRITE_BATCH_SHOULD_WAIT, 0ul, 0ul, 0ul))
#define RECOVER_FAILURE() ((u8)OBSERVE_BATCH(WRITE_BATCH_RECOVER_FAILURE, 0ul, 0ul, 0ul))
#define RESTART_ACQUISITION() ((u8)OBSERVE_BATCH(WRITE_BATCH_RESTART_ACQUISITION, 0ul, 0ul, 0ul))
#define FAIL_PREPARATION() ((u8)OBSERVE_BATCH(WRITE_BATCH_FAIL_PREPARATION, 0ul, 0ul, 0ul))
#define RELEASE_WAIT() ((u8)OBSERVE_BATCH(WRITE_BATCH_RELEASE_WAIT, 0ul, 0ul, 0ul))
#define BUILD_Q() ((void)OBSERVE_BATCH(WRITE_BATCH_BUILD_Q, 0ul, 0ul, 0ul))
#define COPY_Q_MSF() ((void)OBSERVE_BATCH(WRITE_BATCH_COPY_Q_MSF, 0ul, 0ul, 0ul))
#define RESUME_RING() ((fw_addr_t)OBSERVE_BATCH(WRITE_BATCH_RESUME_RING, 0ul, 0ul, 0ul))
#define WRITE_REGISTER_BLOCK(address) ((void)OBSERVE_BATCH(WRITE_BATCH_WRITE_REGISTER_BLOCK, (address), 0ul, 0ul))
#define SUBMIT_COMMAND(address, parameter) ((void)OBSERVE_BATCH(WRITE_BATCH_SUBMIT_COMMAND, (address), (parameter), 0ul))
#define UPDATE_Q_CONTROL() ((void)OBSERVE_BATCH(WRITE_BATCH_UPDATE_Q_CONTROL, 0ul, 0ul, 0ul))
#define ENQUEUE_DESCRIPTOR() ((void)OBSERVE_BATCH(WRITE_BATCH_ENQUEUE_DESCRIPTOR, 0ul, 0ul, 0ul))
#define INCREMENT_LOCAL_MSF(tuple) ((void)(tuple), (void)OBSERVE_BATCH(WRITE_BATCH_INCREMENT_LOCAL_MSF, 0ul, 0ul, 0ul))
#define INCREMENT_PACKET_MSF() ((void)OBSERVE_BATCH(WRITE_BATCH_INCREMENT_PACKET_MSF, 0ul, 0ul, 0ul))
#define ADVANCE_RING(address) ((fw_addr_t)OBSERVE_BATCH(WRITE_BATCH_ADVANCE_RING, (address), 0ul, 0ul))
#define ADVANCE_Q_MSF() ((void)OBSERVE_BATCH(WRITE_BATCH_ADVANCE_Q_MSF, 0ul, 0ul, 0ul))
#define TRANSFER_SECTORS(source, destination, count) ((void)OBSERVE_BATCH(WRITE_BATCH_TRANSFER_SECTORS, (source), (destination), (count)))
#define SEED_RANGE() ((void)OBSERVE_BATCH(WRITE_BATCH_SEED_RANGE, 0ul, 0ul, 0ul))
#define WAIT_CAPACITY() ((u8)OBSERVE_BATCH(WRITE_BATCH_WAIT_CAPACITY, 0ul, 0ul, 0ul))
#define LATCH_FAILURE() ((void)OBSERVE_BATCH(WRITE_BATCH_LATCH_FAILURE, 0ul, 0ul, 0ul))
#define COPY_AUXILIARY_PREFIX(destination, source, count) ((void)OBSERVE_BATCH(WRITE_BATCH_COPY_AUXILIARY_PREFIX, (destination), (source), (count)))
#define DIVIDE_S32(dividend, divisor) ((dividend) / (divisor))
#define DIVIDE_U32(dividend, divisor) ((dividend) / (divisor))
#define MULTIPLY_U32(left, right) ((u32)(left) * (u32)(right))
#else
extern void YieldCoroutine0(void);
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
extern u32 DivideU32QuotientNative(u32 dividend, u32 divisor);
#define LATEST_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define ACQUIRE_PREFIX(mode, source) AcquireFixedRawReadFramePrefixWindow((mode), (source))
#define YIELD_BATCH() YieldCoroutine0()
#define SHOULD_WAIT() ShouldWaitForRawReadProducerCapacity()
#define RECOVER_FAILURE() TryRecoverRawReadPipelineFailure()
#define RESTART_ACQUISITION() RestartReadAcquisitionCoroutineForProducerWait()
#define FAIL_PREPARATION() FailIfReadMeasurementPreparationActive()
#define RELEASE_WAIT() TryReleaseRawReadProducerCapacityWait()
#define BUILD_Q() BuildWritePipelineQPacketForTargetAddress()
#define COPY_Q_MSF() CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1279, FW_NEAR_UNKNOWN_1428, 3u)
#define RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define WRITE_REGISTER_BLOCK(address) WriteControllerRegisterBlock25FromMemory(address)
#define SUBMIT_COMMAND(address, parameter) SubmitControllerCommand80AtBufferAddressAndWait((address), (parameter))
#define UPDATE_Q_CONTROL() UpdateRawReadQPacketControlBit()
#define ENQUEUE_DESCRIPTOR() EnqueueCurrentRawSectorDescriptorAtomic()
#define INCREMENT_LOCAL_MSF(tuple) IncrementPackedBcdMsf((u16)(tuple))
#define INCREMENT_PACKET_MSF() IncrementPackedBcdMsf(0x1428u)
#define ADVANCE_RING(address) AdvanceRawReadRingPosition(address)
#define ADVANCE_Q_MSF() AdvanceRawReadQPacketMsfTuples(0u)
#define TRANSFER_SECTORS(source, destination, count) TransferControllerBufferSectorsAndWait((source), (destination), (count))
#define SEED_RANGE() SeedSelectedWriteControllerBufferRangeAnchors()
#define WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define LATCH_FAILURE() LatchFirstReadFailureContext()
#define COPY_AUXILIARY_PREFIX(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define DIVIDE_S32(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#define DIVIDE_U32(dividend, divisor) DivideU32QuotientNative((dividend), (divisor))
#define MULTIPLY_U32(left, right) MultiplyU32Low((left), (right))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
packed_msf[0] = FirmwareReadNear8(FW_NEAR_UNKNOWN_1279);
packed_msf[1] = FirmwareReadNear8(FW_NEAR_UNKNOWN_127A);
packed_msf[2] = FirmwareReadNear8(FW_NEAR_UNKNOWN_127B);
#else
CopyThreeBytes16((u16)packed_msf, 0x1279u);
#endif
#endif
