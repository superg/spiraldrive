#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveRawReadPositionTracker(u8 event, u32 argument0, u32 argument1);
#define TRACKER_EVENT(event, argument0, argument1) ObserveRawReadPositionTracker((event), (argument0), (argument1))
#define TRACKER_READ_SEQUENCE_VALID() ((u8)TRACKER_EVENT(RAW_READ_TRACKER_READ_SEQUENCE_VALID, 0ul, 0ul))
#define TRACKER_READ_SPECIAL_MODE() FirmwareReadBit0013()
#define TRACKER_SET_SEQUENCE_VALID() ((void)TRACKER_EVENT(RAW_READ_TRACKER_SET_SEQUENCE_VALID, 1ul, 0ul))
#define TRACKER_ARM_CAPTURE() ((void)TRACKER_EVENT(RAW_READ_TRACKER_ARM_CAPTURE, 0ul, 0ul))
#define TRACKER_SERVICE_COUNTDOWN() ((void)TRACKER_EVENT(RAW_READ_TRACKER_SERVICE_COUNTDOWN, 0ul, 0ul))
#define TRACKER_CONVERT_BCD(value) ((u8)TRACKER_EVENT(RAW_READ_TRACKER_CONVERT_BCD, (value), 0ul))
#define TRACKER_CONVERT_MSF(address) ((s32)TRACKER_EVENT(RAW_READ_TRACKER_CONVERT_MSF, (address), 0ul))
#define TRACKER_ENQUEUE_TIMING(status, mode) ((void)TRACKER_EVENT(RAW_READ_TRACKER_ENQUEUE_TIMING_STATUS, (status), (mode)))
#else
u8 bdata RawReadPositionTrackerBitByte _at_ 0x22;
sbit RawReadPositionTrackerSequenceValid = RawReadPositionTrackerBitByte ^ 2;
sbit RawReadPositionTrackerSpecialMode = RawReadPositionTrackerBitByte ^ 3;

#define TRACKER_READ_SEQUENCE_VALID() RawReadPositionTrackerSequenceValid
#define TRACKER_READ_SPECIAL_MODE() RawReadPositionTrackerSpecialMode
#define TRACKER_SET_SEQUENCE_VALID() (RawReadPositionTrackerSequenceValid = 1)
#define TRACKER_ARM_CAPTURE() ArmRawSectorBufferCapture()
#define TRACKER_SERVICE_COUNTDOWN() ServiceParsedQDescriptorCountdownAndPositionAdvance()
#define TRACKER_CONVERT_BCD(value) PackedBcdByteToBinary(value)
#define TRACKER_CONVERT_MSF(address) ConvertBinaryMsfToAdjustedFramePosition(address)
#define TRACKER_ENQUEUE_TIMING(status, mode) EnqueueRawSectorDescriptorWithTimingGate((status), (mode))
#endif
#endif
