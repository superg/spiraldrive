#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void FirmwareWriteBit0030(u8 value);
extern void FirmwareWriteBit0035(u8 value);
extern void ObserveSyntheticPatternYield(void);
extern fw_addr_t ObserveSyntheticPatternResumeRing(void);
extern u8 ObserveSyntheticPatternWaitForCapacity(void);
extern void ObserveSyntheticPatternReportFailure(u8 failure_code);
extern u32 ObserveSyntheticPatternRemainder(s32 dividend, s32 divisor);
extern void ObserveSyntheticPatternFill(fw_addr_t buffer_address, u16 byte_count, u8 value);
extern void ObserveSyntheticPatternCopyAuxiliary(fw_addr_t destination, fw_addr_t source, u8 byte_count);
extern void ObserveSyntheticPatternEnqueue(void);
extern void ObserveSyntheticPatternAdvanceMsf(u8 reverse_relative);
extern u32 ObserveSyntheticPatternAdvanceRing(u32 cursor);
#define SET_SYNTHETIC_ACTIVE_BITS() (FirmwareWriteBit0030(1u), FirmwareWriteBit0035(1u))
#define CLEAR_SYNTHETIC_ACTIVE_BITS() (FirmwareWriteBit0030(0u), FirmwareWriteBit0035(0u))
#define SYNTHETIC_YIELD() ObserveSyntheticPatternYield()
#define SYNTHETIC_RESUME_RING() ObserveSyntheticPatternResumeRing()
#define SYNTHETIC_WAIT_CAPACITY() ObserveSyntheticPatternWaitForCapacity()
#define SYNTHETIC_REPORT_FAILURE(code) ObserveSyntheticPatternReportFailure(code)
#define SYNTHETIC_REMAINDER(value, divisor) ObserveSyntheticPatternRemainder((value), (divisor))
#define SYNTHETIC_FILL(address, count, value) ObserveSyntheticPatternFill((address), (count), (value))
#define SYNTHETIC_COPY_AUXILIARY(destination, source, count) ObserveSyntheticPatternCopyAuxiliary((destination), (source), (count))
#define SYNTHETIC_ENQUEUE() ObserveSyntheticPatternEnqueue()
#define SYNTHETIC_ADVANCE_MSF(reverse) ObserveSyntheticPatternAdvanceMsf(reverse)
#define SYNTHETIC_ADVANCE_RING(cursor) ObserveSyntheticPatternAdvanceRing(cursor)
#else
extern void SetSyntheticPatternActiveBitsAtomic(void);
extern void ClearSyntheticPatternActiveBitsAtomic(void);
extern void YieldCoroutine0(void);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern u32 DivideS32MagnitudeRemainderNative(s32 dividend, s32 divisor);
#define SET_SYNTHETIC_ACTIVE_BITS() SetSyntheticPatternActiveBitsAtomic()
#define CLEAR_SYNTHETIC_ACTIVE_BITS() ClearSyntheticPatternActiveBitsAtomic()
#define SYNTHETIC_YIELD() YieldCoroutine0()
#define SYNTHETIC_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define SYNTHETIC_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define SYNTHETIC_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#define SYNTHETIC_REMAINDER(value, divisor) DivideS32MagnitudeRemainderNative((value), (divisor))
#define SYNTHETIC_FILL(address, count, value) FillControllerBufferChunkAndWait((address), (count), (value))
#define SYNTHETIC_COPY_AUXILIARY(destination, source, count) CopyBytesToAddressPlus1MiB((destination), (source), (count))
#define SYNTHETIC_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define SYNTHETIC_ADVANCE_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define SYNTHETIC_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[4];
fw_addr_t tuple_address = (fw_addr_t)tuple;
#endif
#endif
