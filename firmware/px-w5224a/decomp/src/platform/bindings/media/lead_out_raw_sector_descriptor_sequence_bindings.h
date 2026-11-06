#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveLeadOutWriteBit0030(u8 value);
extern u8 ObserveLeadOutReadBit0032(void);
extern void ObserveLeadOutClearBit0035(void);
extern u16 ObserveLeadOutLatestDescriptor(void);
extern u32 ObserveLeadOutMultiply(u32 left, u32 right);
extern void ObserveLeadOutCopyFirstTrackStart(u16 destination);
extern u8 ObserveLeadOutBinaryToBcd(u8 value);
extern s32 ObserveLeadOutTupleToFrame(fw_addr_t tuple);
extern void ObserveLeadOutFrameToTuple(s32 frame, fw_addr_t tuple);
extern void ObserveLeadOutPackTuple(u16 tuple);
extern u16 ObserveLeadOutFindTrack(u8 track_number);
extern u8 ObserveLeadOutDecodeType(u16 record_index);
extern u8 ObserveLeadOutFindAdjacent(u16 record_index, u8 forward);
extern u8 ObserveLeadOutAcquireCurrent(s32 count, u8 source_kind);
extern u8 ObserveLeadOutAcquireUpdating(s32 count, u8 reverse);
extern u8 ObserveLeadOutCapture(fw_addr_t tuple, fw_addr_t cursor, u8 source_kind);
extern u8 ObserveLeadOutWaitCapacity(void);
extern void ObserveLeadOutLatchFailure(void);
extern void ObserveLeadOutPopulate(fw_addr_t cursor, u8 source_kind);
extern void ObserveLeadOutUpdateQ(void);
extern void ObserveLeadOutEnqueue(void);
extern void ObserveLeadOutIncrement(u16 tuple);
extern fw_addr_t ObserveLeadOutAdvance(fw_addr_t cursor);
extern void ObserveLeadOutYield(void);
extern void ObserveLeadOutRestart(void);
extern void ObserveLeadOutCopyBytes(fw_addr_t source, fw_addr_t destination, u16 count);
#define LEADOUT_WRITE_BIT30(v) ObserveLeadOutWriteBit0030(v)
#define LEADOUT_READ_BIT32() ObserveLeadOutReadBit0032()
#define LEADOUT_CLEAR_BIT35() ObserveLeadOutClearBit0035()
#define LEADOUT_LATEST_DESCRIPTOR() ObserveLeadOutLatestDescriptor()
#define LEADOUT_MULTIPLY(a, b) ObserveLeadOutMultiply((a), (b))
#define LEADOUT_COPY_FIRST(destination) ObserveLeadOutCopyFirstTrackStart(destination)
#define LEADOUT_TO_BCD(value) ObserveLeadOutBinaryToBcd(value)
#define LEADOUT_TO_FRAME(tuple) ObserveLeadOutTupleToFrame(tuple)
#define LEADOUT_TO_TUPLE(frame, tuple) ObserveLeadOutFrameToTuple((frame), (tuple))
#define LEADOUT_PACK(tuple) ObserveLeadOutPackTuple(tuple)
#define LEADOUT_FIND_TRACK(track) ObserveLeadOutFindTrack(track)
#define LEADOUT_DECODE_TYPE(index) ObserveLeadOutDecodeType(index)
#define LEADOUT_FIND_ADJACENT(index, forward) ObserveLeadOutFindAdjacent((index), (forward))
#define LEADOUT_ACQUIRE_CURRENT(count, kind) ObserveLeadOutAcquireCurrent((count), (kind))
#define LEADOUT_ACQUIRE_UPDATING(count, reverse) ObserveLeadOutAcquireUpdating((count), (reverse))
#define LEADOUT_CAPTURE(tuple, cursor, kind) ObserveLeadOutCapture((tuple), (cursor), (kind))
#define LEADOUT_WAIT_CAPACITY() ObserveLeadOutWaitCapacity()
#define LEADOUT_LATCH_FAILURE() ObserveLeadOutLatchFailure()
#define LEADOUT_POPULATE(cursor, kind) ObserveLeadOutPopulate((cursor), (kind))
#define LEADOUT_UPDATE_Q() ObserveLeadOutUpdateQ()
#define LEADOUT_ENQUEUE() ObserveLeadOutEnqueue()
#define LEADOUT_INCREMENT(tuple) ObserveLeadOutIncrement(tuple)
#define LEADOUT_ADVANCE(cursor) ObserveLeadOutAdvance(cursor)
#define LEADOUT_YIELD() ObserveLeadOutYield()
#define LEADOUT_RESTART() ObserveLeadOutRestart()
#define LEADOUT_COPY_BYTES(source, destination, count) ObserveLeadOutCopyBytes((source), (destination), (count))
#else
u8 bdata LeadOutSequenceBitByte26 _at_ 0x26;
sbit LeadOutSequenceBit0030 = LeadOutSequenceBitByte26 ^ 0;
sbit LeadOutSequenceBit0032 = LeadOutSequenceBitByte26 ^ 2;
sbit LeadOutSequenceBit0035 = LeadOutSequenceBitByte26 ^ 5;
extern void YieldCoroutine0(void);
#define LEADOUT_WRITE_BIT30(v) (LeadOutSequenceBit0030 = (v))
#define LEADOUT_READ_BIT32() LeadOutSequenceBit0032
#define LEADOUT_CLEAR_BIT35() (LeadOutSequenceBit0035 = 0)
#define LEADOUT_LATEST_DESCRIPTOR() GetLatestQueuedRawSectorDescriptorAddress()
#define LEADOUT_MULTIPLY(a, b) MultiplyU32Low((a), (b))
#define LEADOUT_COPY_FIRST(destination) CopyFirstTrackStartMsfFromRawReadQRecordTable(destination)
#define LEADOUT_TO_BCD(value) BinaryByteToPackedBcd(value)
#define LEADOUT_TO_FRAME(tuple) ConvertBinaryMsfToAdjustedFramePosition(tuple)
#define LEADOUT_TO_TUPLE(frame, tuple) ConvertFramePositionToBinaryMsfTuple((frame), (tuple))
#define LEADOUT_PACK(tuple) ConvertThreeBinaryBytesToPackedBcd(tuple)
#define LEADOUT_FIND_TRACK(track) FindType1RawTrackRecordIndexByTrackNumberOrLeadOut(track)
#define LEADOUT_DECODE_TYPE(index) DecodeRawReadQRecordTypeCode(index)
#define LEADOUT_FIND_ADJACENT(index, forward) FindAdjacentRawReadQRecordClass((index), (forward))
#define LEADOUT_ACQUIRE_CURRENT(count, kind) AcquireRawReadFrameSequenceWithCurrentQPacket((count), (kind))
#define LEADOUT_ACQUIRE_UPDATING(count, reverse) AcquireRawReadFrameSequenceWithQUpdates((count), (reverse))
#define LEADOUT_CAPTURE(tuple, cursor, kind) CaptureRawSectorUsingQMsfAndPopulateSubheader((tuple), (cursor), (kind))
#define LEADOUT_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define LEADOUT_LATCH_FAILURE() LatchFirstReadFailureContext()
#define LEADOUT_POPULATE(cursor, kind) PopulateConfiguredMode2SubheaderWhenRequired((cursor), (kind))
#define LEADOUT_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define LEADOUT_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define LEADOUT_INCREMENT(tuple) IncrementPackedBcdMsf(tuple)
#define LEADOUT_ADVANCE(cursor) AdvanceRawReadRingPosition(cursor)
#define LEADOUT_YIELD() YieldCoroutine0()
#define LEADOUT_RESTART() RestartReadAcquisitionCoroutineForProducerWait()
#define LEADOUT_COPY_BYTES(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t absolute_tuple = 0x7000ul;
fw_addr_t relative_tuple = 0x7010ul;
#else
u8 tuple_storage[8];
u8 relative_storage[3];
fw_addr_t absolute_tuple = (u16)tuple_storage;
fw_addr_t relative_tuple = (u16)relative_storage;
#endif
#endif
