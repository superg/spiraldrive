#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern s32 ObserveMapWritePreservationTrackPacketAddress(s32 address);
extern s32 ObserveMapWritePreservationAlternateAddress(s32 address);
extern u32 ObserveResetWritePreservationRawReadTransfer(void);
extern void ObservePrepareWritePreservationHandshakeLow(void);
extern void ObserveSetWritePreservationActiveBits(void);
extern void ObserveClearWritePreservationActiveBitsIfSet(void);
extern void ObserveSetWritePreservationHandshakeHigh(void);
extern void ObserveResetWritePreservationQueue(void);
extern void ObserveInitializeWritePreservationCoroutine2(u32 entry_point);
extern void ObserveYieldWritePreservationCoroutine0(void);
extern u8 ObservePollWritePreservationDescriptorBatch(void);
extern void ObserveResetWritePreservationPipeline(s32 target, u8 mode);
extern void ObserveYieldWritePreservationBatchPoll(void);
extern void ObserveEnqueueWritePreservationDescriptorRange(u32 main_data_cursor, s32 frame_position, u8 descriptor_count);
extern u8 ObserveGetWritePreservationFixedPacketOverhead(s32 frame_position);
extern void ObserveWritePreservationControllerBlock25(fw_addr_t source);
extern void ObserveSubmitWritePreservationDescriptor(u32 cursor, u8 mode);
extern void ObserveUpdateWritePreservationQControl(void);
extern void ObserveEnqueueCurrentWritePreservationDescriptor(void);
extern u32 ObserveAdvanceWritePreservationRingPosition(u32 cursor);
#define MAP_TRACK_PACKET(address) ObserveMapWritePreservationTrackPacketAddress(address)
#define MAP_ALTERNATE(address) ObserveMapWritePreservationAlternateAddress(address)
#define RESET_RAW_TRANSFER() ObserveResetWritePreservationRawReadTransfer()
#define PREPARE_HANDSHAKE_LOW() ObservePrepareWritePreservationHandshakeLow()
#define SET_ACTIVE_BITS() ObserveSetWritePreservationActiveBits()
#define CLEAR_ACTIVE_BITS_IF_SET() ObserveClearWritePreservationActiveBitsIfSet()
#define SET_HANDSHAKE_HIGH() ObserveSetWritePreservationHandshakeHigh()
#define RESET_RAW_QUEUE() ObserveResetWritePreservationQueue()
#define INITIALIZE_COROUTINE2(entry) ObserveInitializeWritePreservationCoroutine2(entry)
#define YIELD_COROUTINE0() ObserveYieldWritePreservationCoroutine0()
#define POLL_DESCRIPTOR_BATCH() ObservePollWritePreservationDescriptorBatch()
#define RESET_PIPELINE(target, mode) ObserveResetWritePreservationPipeline((target), (mode))
#define POLL_BATCH_YIELD() ObserveYieldWritePreservationBatchPoll()
#define ENQUEUE_PRESERVATION_RANGE(cursor, position, count) ObserveEnqueueWritePreservationDescriptorRange((cursor), (position), (count))
#define FIXED_PACKET_OVERHEAD(position) ObserveGetWritePreservationFixedPacketOverhead(position)
#define WRITE_PRESERVATION_BLOCK25(source) ObserveWritePreservationControllerBlock25(source)
#define SUBMIT_PRESERVATION_DESCRIPTOR(cursor, mode) ObserveSubmitWritePreservationDescriptor((cursor), (mode))
#define UPDATE_PRESERVATION_Q_CONTROL() ObserveUpdateWritePreservationQControl()
#define ENQUEUE_CURRENT_PRESERVATION_DESCRIPTOR() ObserveEnqueueCurrentWritePreservationDescriptor()
#define ADVANCE_PRESERVATION_RING(cursor) ObserveAdvanceWritePreservationRingPosition(cursor)
#else
extern s32 MapLogicalBlockAddressThroughAlternateRecordLayout(s32 address);
extern u32 ResetRawReadTransferState(void);
extern void PrepareWritePreservationHandshakeLowAtomic(void);
extern void SetWritePreservationActiveBitsAtomic(void);
extern void ClearWritePreservationActiveBitsIfSetAtomic(void);
extern void SetWritePreservationHandshakeHighAtomic(void);
extern void ResetRawReadQueueAndTransferState(void);
extern void PLEXTOR_NEAR_FUNCTION InitializeCoroutine2AtF90000ForC(void);
extern void YieldCoroutine0(void);
extern void ResetRawReadPipelineForOptionalTarget(s32 target, u8 mode);
extern void PLEXTOR_NEAR_FUNCTION EnqueueWritePreservationDescriptorRange(u32 main_data_cursor, s32 frame_position, u8 descriptor_count);
extern void SubmitControllerCommand80AtBufferAddressAndWait(u32 controller_buffer_address, u8 mode);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern u32 AdvanceRawReadRingPosition(u32 main_data_cursor);
#define MAP_TRACK_PACKET(address) MapLogicalBlockAddressThroughTrackPacketLayout(address)
#define MAP_ALTERNATE(address) MapLogicalBlockAddressThroughAlternateRecordLayout(address)
#define RESET_RAW_TRANSFER() ResetRawReadTransferState()
#define PREPARE_HANDSHAKE_LOW() PrepareWritePreservationHandshakeLowAtomic()
#define SET_ACTIVE_BITS() SetWritePreservationActiveBitsAtomic()
#define CLEAR_ACTIVE_BITS_IF_SET() ClearWritePreservationActiveBitsIfSetAtomic()
#define SET_HANDSHAKE_HIGH() SetWritePreservationHandshakeHighAtomic()
#define RESET_RAW_QUEUE() ResetRawReadQueueAndTransferState()
#define INITIALIZE_COROUTINE2(entry) InitializeCoroutine2AtF90000ForC()
#define YIELD_COROUTINE0() YieldCoroutine0()
#define POLL_DESCRIPTOR_BATCH() PollWritePreservationDescriptorBatch()
#define RESET_PIPELINE(target, mode) ResetRawReadPipelineForOptionalTarget((target), (mode))
#define POLL_BATCH_YIELD() YieldCoroutine0()
#define ENQUEUE_PRESERVATION_RANGE(cursor, position, count) EnqueueWritePreservationDescriptorRange((cursor), (position), (count))
#define FIXED_PACKET_OVERHEAD(position) GetFixedPacketOverheadFramesRemaining(position)
#define WRITE_PRESERVATION_BLOCK25(source) WriteControllerRegisterBlock25FromMemory(source)
#define SUBMIT_PRESERVATION_DESCRIPTOR(cursor, mode) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (mode))
#define UPDATE_PRESERVATION_Q_CONTROL() UpdateRawReadQPacketControlBit()
#define ENQUEUE_CURRENT_PRESERVATION_DESCRIPTOR() EnqueueCurrentRawSectorDescriptorAtomic()
#define ADVANCE_PRESERVATION_RING(cursor) AdvanceRawReadRingPosition(cursor)
#endif
#endif
