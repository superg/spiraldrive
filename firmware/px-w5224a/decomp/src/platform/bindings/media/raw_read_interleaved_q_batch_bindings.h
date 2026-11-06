#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern fw_addr_t ObserveInterleavedQResumeRing(void);
extern void ObserveInterleavedQConvert(s32 frame_position, fw_addr_t output_tuple_address);
extern void ObserveInterleavedQPack(u16 tuple_address);
extern u8 ObserveInterleavedQCapture(fw_addr_t controller_buffer_address, u8 source_kind);
extern void ObserveInterleavedQCopyTuple(u16 destination, u16 source);
extern void ObserveInterleavedQUpdate(void);
extern void ObserveInterleavedQEnqueue(void);
extern void ObserveInterleavedQIncrement(u16 tuple_address);
extern fw_addr_t ObserveInterleavedQAdvanceRing(fw_addr_t cursor);
#define INTERLEAVED_RESUME_RING() ObserveInterleavedQResumeRing()
#define INTERLEAVED_CONVERT(frame, output) ObserveInterleavedQConvert((frame), (output))
#define INTERLEAVED_PACK(address) ObserveInterleavedQPack(address)
#define INTERLEAVED_CAPTURE(address, kind) ObserveInterleavedQCapture((address), (kind))
#define INTERLEAVED_COPY_TUPLE(destination, source) ObserveInterleavedQCopyTuple((destination), (source))
#define INTERLEAVED_UPDATE_Q() ObserveInterleavedQUpdate()
#define INTERLEAVED_ENQUEUE() ObserveInterleavedQEnqueue()
#define INTERLEAVED_INCREMENT(address) ObserveInterleavedQIncrement(address)
#define INTERLEAVED_ADVANCE_RING(cursor) ObserveInterleavedQAdvanceRing(cursor)
#else
#define INTERLEAVED_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define INTERLEAVED_CONVERT(frame, output) ConvertFramePositionToBinaryMsfTuple((frame), (output))
#define INTERLEAVED_PACK(address) ConvertThreeBinaryBytesToPackedBcd(address)
#define INTERLEAVED_CAPTURE(address, kind) CaptureRawSectorToControllerBufferAndWait((address), (kind))
#define INTERLEAVED_COPY_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define INTERLEAVED_UPDATE_Q() UpdateRawReadQPacketControlBit()
#define INTERLEAVED_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define INTERLEAVED_INCREMENT(address) IncrementPackedBcdMsf(address)
#define INTERLEAVED_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
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
