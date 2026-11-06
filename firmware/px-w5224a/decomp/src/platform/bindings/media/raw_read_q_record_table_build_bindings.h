#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawQTableBit0041(void);
extern void ObserveRawQTableConvert(s32 frame_position, fw_addr_t tuple_address);
extern void ObserveRawQTablePack(u16 tuple_address);
extern void ObserveRawQTableUnpack(u16 tuple_address);
extern void ObserveRawQTableCopyTuple(u16 destination, u16 source);
extern s32 ObserveRawQTableTupleToFrame(fw_addr_t tuple_address);
extern u8 ObserveRawQTableBinaryToBcd(u8 value);
extern u32 ObserveRawQTableMultiply(u32 left, u32 right);
extern u8 ObserveRawQTableWaitCapacity(void);
extern void ObserveRawQTableWriteControllerBlock(fw_addr_t source);
extern u8 ObserveRawQTableCapture(fw_addr_t cursor, u8 source_kind);
extern void ObserveRawQTableBuildRid01(fw_addr_t cursor);
extern void ObserveRawQTableIncrement(u16 tuple_address);
extern void ObserveRawQTableUpdateQ(void);
extern void ObserveRawQTableEnqueue(void);
extern void ObserveRawQTableReportFailure(u8 failure_code);
extern void ObserveRawQTableLatchFailure(void);
extern void ObserveRawQTableSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveRawQTableSetInformation(u32 information);
#define RAW_Q_BIT0041() ObserveRawQTableBit0041()
#define RAW_Q_CONVERT(frame, output) ObserveRawQTableConvert((frame), (output))
#define RAW_Q_PACK(address) ObserveRawQTablePack(address)
#define RAW_Q_UNPACK(address) ObserveRawQTableUnpack(address)
#define RAW_Q_COPY(destination, source) ObserveRawQTableCopyTuple((destination), (source))
#define RAW_Q_TO_FRAME(address) ObserveRawQTableTupleToFrame(address)
#define RAW_Q_TO_BCD(value) ObserveRawQTableBinaryToBcd(value)
#define RAW_Q_MULTIPLY(left, right) ObserveRawQTableMultiply((left), (right))
#define RAW_Q_WAIT() ObserveRawQTableWaitCapacity()
#define RAW_Q_WRITE_BLOCK(source) ObserveRawQTableWriteControllerBlock(source)
#define RAW_Q_CAPTURE(cursor, kind) ObserveRawQTableCapture((cursor), (kind))
#define RAW_Q_BUILD_RID01(cursor) ObserveRawQTableBuildRid01(cursor)
#define RAW_Q_INCREMENT(address) ObserveRawQTableIncrement(address)
#define RAW_Q_UPDATE() ObserveRawQTableUpdateQ()
#define RAW_Q_ENQUEUE() ObserveRawQTableEnqueue()
#define RAW_Q_REPORT(code) ObserveRawQTableReportFailure(code)
#define RAW_Q_LATCH_FAILURE() ObserveRawQTableLatchFailure()
#define RAW_Q_SET_SENSE(key, code) ObserveRawQTableSetSense((key), (code))
#define RAW_Q_SET_INFORMATION(value) ObserveRawQTableSetInformation(value)
#else
u8 bdata RawQTableBitByte28 _at_ 0x28;
sbit RawQTableBit0041 = RawQTableBitByte28 ^ 1;
#define RAW_Q_BIT0041() RawQTableBit0041
#define RAW_Q_CONVERT(frame, output) ConvertFramePositionToBinaryMsfTuple((frame), (output))
#define RAW_Q_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define RAW_Q_UNPACK(address) ConvertThreePackedBcdBytesToBinary(address)
#define RAW_Q_COPY(destination, source) CopyThreeBytes16((destination), (source))
#define RAW_Q_TO_FRAME(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define RAW_Q_TO_BCD(value) BinaryByteToPackedBcd(value)
#define RAW_Q_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define RAW_Q_WAIT() WaitForRawReadProducerCapacityAndCheckStatus()
#define RAW_Q_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define RAW_Q_CAPTURE(cursor, kind) CaptureRawSectorToControllerBufferAndWait((cursor), (kind))
#define RAW_Q_BUILD_RID01(cursor) BuildRid01RecorderIdentificationBlockAtControllerAddress(cursor)
#define RAW_Q_INCREMENT(address) IncrementPackedBcdMsf(address)
#define RAW_Q_UPDATE() UpdateRawReadQPacketControlBit()
#define RAW_Q_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define RAW_Q_REPORT(code) ReportReadPipelineFailure(code)
#define RAW_Q_LATCH_FAILURE() LatchFirstReadFailureContext()
#define RAW_Q_SET_SENSE(key, code) SetSenseData((key), (code))
#define RAW_Q_SET_INFORMATION(value) SetSenseInformation(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = 0x7000ul;
#else
u8 tuple[3];
fw_addr_t tuple_address = (u16)tuple;
#endif
#endif
