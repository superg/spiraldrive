#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u8 ObserveRawSectorSequenceSelectTrack(void);
extern fw_addr_t ObserveRawSectorSequenceResumeRing(void);
extern void ObserveRawSectorSequenceConvertPosition(s32 frame_position, fw_addr_t tuple_address);
extern void ObserveRawSectorSequencePackTuple(u16 tuple_address);
extern void ObserveRawSectorSequenceCopyTuple(u16 destination, u16 source);
extern u8 ObserveRawSectorSequenceWaitCapacity(void);
extern u8 ObserveRawSectorSequenceCapture(fw_addr_t main_buffer_address, u8 preserve_main_buffer, u8 clear_subchannel_slot);
extern void ObserveRawSectorSequenceEnqueue(void);
extern fw_addr_t ObserveRawSectorSequenceAdvanceRing(fw_addr_t cursor);
extern void ObserveRawSectorSequenceIncrementMsf(u16 tuple_address);
extern void ObserveRawSectorSequenceReportFailure(u8 failure_code);
#define SEQUENCE_SELECT_TRACK() ObserveRawSectorSequenceSelectTrack()
#define SEQUENCE_RESUME_RING() ObserveRawSectorSequenceResumeRing()
#define SEQUENCE_CONVERT_POSITION(frame, address) ObserveRawSectorSequenceConvertPosition((frame), (address))
#define SEQUENCE_PACK_TUPLE(address) ObserveRawSectorSequencePackTuple(address)
#define SEQUENCE_COPY_TUPLE(destination, source) ObserveRawSectorSequenceCopyTuple((destination), (source))
#define SEQUENCE_WAIT_CAPACITY() ObserveRawSectorSequenceWaitCapacity()
#define SEQUENCE_CAPTURE(address, preserve, clear) ObserveRawSectorSequenceCapture((address), (preserve), (clear))
#define SEQUENCE_ENQUEUE() ObserveRawSectorSequenceEnqueue()
#define SEQUENCE_ADVANCE_RING(cursor) ObserveRawSectorSequenceAdvanceRing(cursor)
#define SEQUENCE_INCREMENT_MSF(address) ObserveRawSectorSequenceIncrementMsf(address)
#define SEQUENCE_REPORT_FAILURE(code) ObserveRawSectorSequenceReportFailure(code)
#else
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
#define SEQUENCE_SELECT_TRACK() SelectRawReadQTrackNumberForCurrentOperation()
#define SEQUENCE_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define SEQUENCE_CONVERT_POSITION(frame, address) ConvertFramePositionToBinaryMsfTuple((frame), (address))
#define SEQUENCE_PACK_TUPLE(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define SEQUENCE_COPY_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define SEQUENCE_WAIT_CAPACITY() WaitForRawReadProducerCapacityAndCheckStatus()
#define SEQUENCE_CAPTURE(address, preserve, clear) CompleteRawSectorCaptureAndClearBuffers((address), (preserve), (clear))
#define SEQUENCE_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define SEQUENCE_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define SEQUENCE_INCREMENT_MSF(address) IncrementPackedBcdMsf(address)
#define SEQUENCE_REPORT_FAILURE(code) ReportReadPipelineFailure(code)
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
fw_addr_t tuple_address = FW_NEAR_UNKNOWN_7000;
#else
u8 tuple[3];
fw_addr_t tuple_address = (fw_addr_t)tuple;
#endif
#endif
