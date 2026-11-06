#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveLeadInSequenceReadBit0028(void);
extern void ObserveLeadInSequenceClearBit0028(void);
extern u8 ObserveLeadInSequenceReadBit0041(void);
extern void ObserveLeadInSequenceWriteBit0041(u8 value);
extern void ObserveLeadInSequenceInitializeToc(void);
extern void ObserveLeadInSequenceBuildSyntheticQ(void);
extern void ObserveLeadInSequenceMergeStagedQ(void);
extern u16 ObserveLeadInSequenceFindTrack(u8 track_number);
extern u8 ObserveLeadInSequenceDecodeType(u16 record_index);
extern u8 ObserveLeadInSequenceFindAdjacent(u16 record_index, u8 forward);
extern void ObserveLeadInSequenceConvert(s32 frame_position, fw_addr_t output_tuple);
extern void ObserveLeadInSequencePack(u16 tuple_address);
extern void ObserveLeadInSequenceCopyTuple(u16 destination, u16 source);
extern u8 ObserveLeadInSequenceBinaryToBcd(u8 value);
extern u8 ObserveLeadInSequenceFixedPrefix(u8 mode, u8 source_kind);
extern u8 ObserveLeadInSequenceQPrefix(s32 frame_count, u8 reverse);
extern fw_addr_t ObserveLeadInSequenceResumeRing(void);
extern u8 ObserveLeadInSequenceCaptureWithQ(fw_addr_t tuple, fw_addr_t cursor, u8 source_kind);
extern u8 ObserveLeadInSequenceWaitCapacity(void);
extern void ObserveLeadInSequenceLatchFailure(void);
extern void ObserveLeadInSequencePopulateSubheader(fw_addr_t cursor, u8 source_kind);
extern void ObserveLeadInSequenceUpdateQ(void);
extern void ObserveLeadInSequenceEnqueue(void);
extern void ObserveLeadInSequenceIncrement(u16 tuple_address);
extern fw_addr_t ObserveLeadInSequenceAdvanceRing(fw_addr_t cursor);
extern void ObserveLeadInSequenceYield(void);
extern void ObserveLeadInSequenceWriteControllerBlock(fw_addr_t source);
extern u8 ObserveLeadInSequenceCapture(fw_addr_t cursor, u8 source_kind);
extern void ObserveLeadInSequenceBuildRid01(fw_addr_t cursor);
extern u32 ObserveLeadInSequenceMultiply(u32 left, u32 right);
extern void ObserveLeadInSequenceSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveLeadInSequenceSetInformation(u32 information);
#define LEADIN_READ_BIT0028() ObserveLeadInSequenceReadBit0028()
#define LEADIN_CLEAR_BIT0028() ObserveLeadInSequenceClearBit0028()
#define LEADIN_READ_BIT0041() ObserveLeadInSequenceReadBit0041()
#define LEADIN_WRITE_BIT0041(v) ObserveLeadInSequenceWriteBit0041(v)
#define LEADIN_INITIALIZE_TOC() ObserveLeadInSequenceInitializeToc()
#define LEADIN_BUILD_SYNTHETIC_Q() ObserveLeadInSequenceBuildSyntheticQ()
#define LEADIN_MERGE_STAGED_Q() ObserveLeadInSequenceMergeStagedQ()
#define LEADIN_FIND_TRACK(track) ObserveLeadInSequenceFindTrack(track)
#define LEADIN_DECODE_TYPE(index) ObserveLeadInSequenceDecodeType(index)
#define LEADIN_FIND_ADJACENT(index, forward) ObserveLeadInSequenceFindAdjacent((index), (forward))
#define LEADIN_CONVERT(frame, output) ObserveLeadInSequenceConvert((frame), (output))
#define LEADIN_PACK(address) ObserveLeadInSequencePack(address)
#define LEADIN_COPY(destination, source) ObserveLeadInSequenceCopyTuple((destination), (source))
#define LEADIN_TO_BCD(value) ObserveLeadInSequenceBinaryToBcd(value)
#define LEADIN_FIXED_PREFIX(mode, kind) ObserveLeadInSequenceFixedPrefix((mode), (kind))
#define LEADIN_Q_PREFIX(count, reverse) ObserveLeadInSequenceQPrefix((count), (reverse))
#define LEADIN_RESUME_RING() ObserveLeadInSequenceResumeRing()
#define LEADIN_CAPTURE_WITH_Q(tuple, cursor, kind) ObserveLeadInSequenceCaptureWithQ((tuple), (cursor), (kind))
#define LEADIN_WAIT_CAPACITY() ObserveLeadInSequenceWaitCapacity()
#define LEADIN_LATCH_FAILURE() ObserveLeadInSequenceLatchFailure()
#define LEADIN_POPULATE_SUBHEADER(cursor, kind) ObserveLeadInSequencePopulateSubheader((cursor), (kind))
#define LEADIN_UPDATE_Q() ObserveLeadInSequenceUpdateQ()
#define LEADIN_ENQUEUE() ObserveLeadInSequenceEnqueue()
#define LEADIN_INCREMENT(address) ObserveLeadInSequenceIncrement(address)
#define LEADIN_ADVANCE_RING(cursor) ObserveLeadInSequenceAdvanceRing(cursor)
#define LEADIN_YIELD() ObserveLeadInSequenceYield()
#define LEADIN_WRITE_BLOCK(source) ObserveLeadInSequenceWriteControllerBlock(source)
#define LEADIN_CAPTURE(cursor, kind) ObserveLeadInSequenceCapture((cursor), (kind))
#define LEADIN_BUILD_RID01(cursor) ObserveLeadInSequenceBuildRid01(cursor)
#define LEADIN_MULTIPLY(left, right) ObserveLeadInSequenceMultiply((left), (right))
#define LEADIN_SET_SENSE(key, code) ObserveLeadInSequenceSetSense((key), (code))
#define LEADIN_SET_INFORMATION(value) ObserveLeadInSequenceSetInformation(value)
#else
u8 bdata LeadInSequenceBitByte28 _at_ 0x28;
sbit LeadInSequenceBit0041 = LeadInSequenceBitByte28 ^ 1;
extern void PLEXTOR_NEAR_FUNCTION ClearReadMeasurementBit0028Atomic(void);
extern void YieldCoroutine0(void);
#define LEADIN_READ_BIT0028() FirmwareReadBit0028()
#define LEADIN_CLEAR_BIT0028() ClearReadMeasurementBit0028Atomic()
#define LEADIN_READ_BIT0041() LeadInSequenceBit0041
#define LEADIN_WRITE_BIT0041(v) (LeadInSequenceBit0041 = (v))
#define LEADIN_INITIALIZE_TOC() InitializeRawReadTocSummaryFromTrackTable()
#define LEADIN_BUILD_SYNTHETIC_Q() BuildSyntheticRawReadQRecordTableFromTrackState()
#define LEADIN_MERGE_STAGED_Q() MergeStagedRawReadQRecordTablesIntoRawReadTable()
#define LEADIN_FIND_TRACK(track) FindType1RawTrackRecordIndexByTrackNumberOrLeadOut(track)
#define LEADIN_DECODE_TYPE(index) DecodeRawReadQRecordTypeCode(index)
#define LEADIN_FIND_ADJACENT(index, forward) FindAdjacentRawReadQRecordClass((index), (forward))
#define LEADIN_CONVERT(frame, output) ConvertFramePositionToBinaryMsfTuple((frame), (output))
#define LEADIN_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define LEADIN_COPY(destination, source) CopyThreeBytes16((destination), (source))
#define LEADIN_TO_BCD(value) BinaryByteToPackedBcd(value)
#define LEADIN_FIXED_PREFIX(mode, kind) AcquireFixedRawReadFramePrefixWindow((mode), (kind))
#define LEADIN_Q_PREFIX(count, reverse) AcquireRawReadFrameSequenceWithQUpdates((count), (reverse))
#define LEADIN_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define LEADIN_CAPTURE_WITH_Q(tuple, cursor, kind) CaptureRawSectorUsingQMsfAndPopulateSubheader((tuple), (cursor), (kind))
#define LEADIN_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define LEADIN_LATCH_FAILURE() LatchFirstReadFailureContext()
#define LEADIN_POPULATE_SUBHEADER(cursor, kind) PopulateConfiguredMode2SubheaderWhenRequired((cursor), (kind))
#define LEADIN_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define LEADIN_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define LEADIN_INCREMENT(address) IncrementPackedBcdMsf(address)
#define LEADIN_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define LEADIN_YIELD() YieldCoroutine0()
#define LEADIN_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define LEADIN_CAPTURE(cursor, kind) CaptureRawSectorToControllerBufferAndWait((cursor), (kind))
#define LEADIN_BUILD_RID01(cursor) BuildRid01RecorderIdentificationBlockAtControllerAddress(cursor)
#define LEADIN_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define LEADIN_SET_SENSE(key, code) SetSenseData((key), (code))
#define LEADIN_SET_INFORMATION(value) SetSenseInformation(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = 0x7000ul;
#else
u8 tuple[7];
fw_addr_t tuple_address = (u16)tuple;
#endif
#endif
