#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveQRecordSectorConvert(s32 frame_position, fw_addr_t output_tuple_address);
extern void ObserveQRecordSectorPack(u16 tuple_address);
extern void ObserveQRecordSectorCopyTuple(u16 destination, u16 source);
extern u32 ObserveQRecordSectorDivide(u32 dividend, u32 divisor);
extern u32 ObserveQRecordSectorMultiply(u32 left, u32 right);
extern u8 ObserveQRecordSectorWaitCapacity(void);
extern void ObserveQRecordSectorWriteControllerBlock(fw_addr_t source);
extern u8 ObserveQRecordSectorCapture(fw_addr_t cursor, u8 source_kind);
extern void ObserveQRecordSectorBuildRid01(fw_addr_t cursor);
extern void ObserveQRecordSectorIncrement(u16 tuple_address);
extern void ObserveQRecordSectorUpdateQ(void);
extern void ObserveQRecordSectorEnqueue(void);
extern void ObserveQRecordSectorReportFailure(u8 failure_code);
extern void ObserveQRecordSectorSetSense(u8 sense_key, u16 asc_ascq);
extern void ObserveQRecordSectorSetInformation(u32 information);
#define Q_RECORD_CONVERT(frame, output) ObserveQRecordSectorConvert((frame), (output))
#define Q_RECORD_PACK(address) ObserveQRecordSectorPack(address)
#define Q_RECORD_COPY_TUPLE(destination, source) ObserveQRecordSectorCopyTuple((destination), (source))
#define Q_RECORD_DIVIDE(dividend, divisor) ObserveQRecordSectorDivide((dividend), (divisor))
#define Q_RECORD_MULTIPLY(left, right) ObserveQRecordSectorMultiply((left), (right))
#define Q_RECORD_WAIT_CAPACITY() ObserveQRecordSectorWaitCapacity()
#define Q_RECORD_WRITE_BLOCK(source) ObserveQRecordSectorWriteControllerBlock(source)
#define Q_RECORD_CAPTURE(cursor, kind) ObserveQRecordSectorCapture((cursor), (kind))
#define Q_RECORD_BUILD_RID01(cursor) ObserveQRecordSectorBuildRid01(cursor)
#define Q_RECORD_INCREMENT(address) ObserveQRecordSectorIncrement(address)
#define Q_RECORD_UPDATE_Q() ObserveQRecordSectorUpdateQ()
#define Q_RECORD_ENQUEUE() ObserveQRecordSectorEnqueue()
#define Q_RECORD_REPORT_FAILURE(code) ObserveQRecordSectorReportFailure(code)
#define Q_RECORD_SET_SENSE(key, code) ObserveQRecordSectorSetSense((key), (code))
#define Q_RECORD_SET_INFORMATION(value) ObserveQRecordSectorSetInformation(value)
#else
extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
#define Q_RECORD_CONVERT(frame, output) ConvertFramePositionToBinaryMsfTuple((frame), (output))
#define Q_RECORD_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define Q_RECORD_COPY_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define Q_RECORD_DIVIDE(dividend, divisor) DivideU32Quotient((dividend), (divisor))
#define Q_RECORD_MULTIPLY(left, right) MultiplyU32Low((left), (right))
#define Q_RECORD_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define Q_RECORD_WRITE_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define Q_RECORD_CAPTURE(cursor, kind) CaptureRawSectorToControllerBufferAndWait((cursor), (kind))
#define Q_RECORD_BUILD_RID01(cursor) BuildRid01RecorderIdentificationBlockAtControllerAddress(cursor)
#define Q_RECORD_INCREMENT(address) IncrementPackedBcdMsf(address)
#define Q_RECORD_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define Q_RECORD_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define Q_RECORD_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define Q_RECORD_SET_SENSE(key, code) SetSenseData((key), (code))
#define Q_RECORD_SET_INFORMATION(value) SetSenseInformation(value)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
u16 tuple_address = 0x7000u;
#else
u8 tuple[3];
u16 tuple_address = (u16)tuple;
#endif
#endif
