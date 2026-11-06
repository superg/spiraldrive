#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern void ObserveFixedPacketPaddingYield(void);
extern fw_addr_t ObserveFixedPacketPaddingResumeRing(void);
extern void ObserveFixedPacketPaddingCopyTuple(u16 destination, u16 source);
extern void ObserveFixedPacketPaddingWriteControllerBlock(fw_addr_t source);
extern u8 ObserveFixedPacketPaddingCapture(fw_addr_t address, u8 source_kind);
extern void ObserveFixedPacketPaddingPopulateSubheader(fw_addr_t address, u8 source_kind);
extern void ObserveFixedPacketPaddingUpdateQ(u8 skip, u8 decrement);
extern void ObserveFixedPacketPaddingEnqueue(void);
extern void ObserveFixedPacketPaddingAdvanceMsf(u8 reverse_relative);
extern u32 ObserveFixedPacketPaddingAdvanceRing(u32 cursor);
extern void ObserveFixedPacketPaddingSetSense(u8 key, u16 code);
extern void ObserveFixedPacketPaddingSetSenseInformation(u32 information);
#define PADDING_YIELD() ObserveFixedPacketPaddingYield()
#define PADDING_RESUME_RING() ObserveFixedPacketPaddingResumeRing()
#define PADDING_COPY_TUPLE(destination, source) ObserveFixedPacketPaddingCopyTuple((destination), (source))
#define PADDING_WRITE_CONTROLLER_BLOCK(source) ObserveFixedPacketPaddingWriteControllerBlock(source)
#define PADDING_CAPTURE(address, source_kind) ObserveFixedPacketPaddingCapture((address), (source_kind))
#define PADDING_POPULATE_SUBHEADER(address, source_kind) ObserveFixedPacketPaddingPopulateSubheader((address), (source_kind))
#define PADDING_UPDATE_Q(skip, decrement) ObserveFixedPacketPaddingUpdateQ((skip), (decrement))
#define PADDING_ENQUEUE() ObserveFixedPacketPaddingEnqueue()
#define PADDING_ADVANCE_MSF(reverse) ObserveFixedPacketPaddingAdvanceMsf(reverse)
#define PADDING_ADVANCE_RING(cursor) ObserveFixedPacketPaddingAdvanceRing(cursor)
#define PADDING_SET_SENSE(key, code) ObserveFixedPacketPaddingSetSense((key), (code))
#define PADDING_SET_INFORMATION(value) ObserveFixedPacketPaddingSetSenseInformation(value)
#else
extern void YieldCoroutine0(void);
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern void CopyThreeBytes16(u16 destination, u16 source);
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern u8 CaptureRawSectorToControllerBufferAndWait(fw_addr_t controller_buffer_address, u8 raw_sector_source_kind);
extern void PopulateConfiguredMode2SubheaderWhenRequired(fw_addr_t raw_sector_address, u8 raw_sector_source_kind);
extern void PLEXTOR_NEAR_FUNCTION UpdateRawReadQPacketControlAndIdentification(u8 skip_identification_packets, u8 decrement_relative_msf_on_restore);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern void AdvanceRawReadQPacketMsfTuples(u8 reverse_relative);
extern u32 AdvanceRawReadRingPosition(u32 main_data_cursor);
extern void SetSenseData(u8 sense_key, u16 sense_code);
extern void SetSenseInformation(u32 information);
#define PADDING_YIELD() YieldCoroutine0()
#define PADDING_RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define PADDING_COPY_TUPLE(destination, source) CopyThreeBytes16((destination), (source))
#define PADDING_WRITE_CONTROLLER_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define PADDING_CAPTURE(address, source_kind) CaptureRawSectorToControllerBufferAndWait((address), (source_kind))
#define PADDING_POPULATE_SUBHEADER(address, source_kind) PopulateConfiguredMode2SubheaderWhenRequired((address), (source_kind))
#define PADDING_UPDATE_Q(skip, decrement) UpdateRawReadQPacketControlAndIdentification((skip), (decrement))
#define PADDING_ENQUEUE() EnqueueCurrentRawSectorDescriptorAtomic()
#define PADDING_ADVANCE_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define PADDING_ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define PADDING_SET_SENSE(key, code) SetSenseData((key), (code))
#define PADDING_SET_INFORMATION(value) SetSenseInformation(value)
#endif
#endif
