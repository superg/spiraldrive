#include "platform/firmware_map.h"

/* Host/target substitutions for the corresponding firmware unit.
 * Intentionally included more than once with PLEXTOR_BINDING_SECTION. */

#if PLEXTOR_BINDING_SECTION == 1
#if defined(PLEXTOR_HOST_TEST)
extern u32 ObserveSyntheticWriteDescriptorResumeRing(void);
extern u32 ObserveSyntheticWriteDescriptorMapFixedPacket(s32 start_address);
extern s32 ObserveSyntheticWriteDescriptorExpandRecordOverhead(s32 start_address);
extern void ObserveSyntheticWriteDescriptorCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count);
extern void ObserveSyntheticWriteDescriptorWriteControllerBlock(fw_addr_t source);
extern void ObserveSyntheticWriteDescriptorSubmit(fw_addr_t cursor, u8 descriptor_mode);
extern void ObserveSyntheticWriteDescriptorPopulateSubheader(fw_addr_t cursor, u8 source_kind);
extern void ObserveSyntheticWriteDescriptorUpdateQ(u8 skip_identification_packets, u8 decrement_relative_msf_on_restore);
extern void ObserveSyntheticWriteDescriptorEnqueue(void);
extern void ObserveSyntheticWriteDescriptorAdvanceMsf(u8 reverse_relative);
extern u32 ObserveSyntheticWriteDescriptorAdvanceRing(u32 cursor);
extern void ObserveSyntheticWriteDescriptorYield(void);
#define RESUME_RING() ObserveSyntheticWriteDescriptorResumeRing()
#define MAP_FIXED_PACKET(address) ObserveSyntheticWriteDescriptorMapFixedPacket(address)
#define EXPAND_RECORD_OVERHEAD(address) ObserveSyntheticWriteDescriptorExpandRecordOverhead(address)
#define COPY_TUPLE(source, destination, count) ObserveSyntheticWriteDescriptorCopyTuple((source), (destination), (count))
#define WRITE_CONTROLLER_BLOCK(source) ObserveSyntheticWriteDescriptorWriteControllerBlock(source)
#define SUBMIT_DESCRIPTOR(cursor, mode) ObserveSyntheticWriteDescriptorSubmit((cursor), (mode))
#define POPULATE_SUBHEADER(cursor, source_kind) ObserveSyntheticWriteDescriptorPopulateSubheader((cursor), (source_kind))
#define UPDATE_Q(skip, decrement) ObserveSyntheticWriteDescriptorUpdateQ((skip), (decrement))
#define ENQUEUE_DESCRIPTOR() ObserveSyntheticWriteDescriptorEnqueue()
#define ADVANCE_MSF(reverse) ObserveSyntheticWriteDescriptorAdvanceMsf(reverse)
#define ADVANCE_RING(cursor) ObserveSyntheticWriteDescriptorAdvanceRing(cursor)
#define YIELD_COROUTINE0() ObserveSyntheticWriteDescriptorYield()
#else
extern void YieldCoroutine0(void);
extern fw_addr_t ResumeMainDataRingCursorFromLatestDescriptor(void);
extern u32 MapFixedPacketLogicalToPhysicalAddress(u32 logical_block_address);
extern s32 ExpandWriteLogicalAddressForRecordOverhead(s32 logical_address);
extern void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 byte_count) PLEXTOR_REENTRANT;
extern void WriteControllerRegisterBlock25FromMemory(fw_addr_t source);
extern void SubmitControllerCommand80AtBufferAddressAndWait(fw_addr_t controller_buffer_address, u8 command_parameter);
extern void PopulateConfiguredMode2SubheaderWhenRequired(fw_addr_t raw_sector_address, u8 raw_sector_source_kind);
extern void PLEXTOR_NEAR_FUNCTION UpdateRawReadQPacketControlAndIdentification(u8 skip_identification_packets, u8 decrement_relative_msf_on_restore);
extern void EnqueueCurrentRawSectorDescriptorAtomic(void);
extern u32 AdvanceRawReadRingPosition(u32 main_data_cursor);
#define RESUME_RING() ResumeMainDataRingCursorFromLatestDescriptor()
#define MAP_FIXED_PACKET(address) MapFixedPacketLogicalToPhysicalAddress((u32)(address))
#define EXPAND_RECORD_OVERHEAD(address) ExpandWriteLogicalAddressForRecordOverhead(address)
#define COPY_TUPLE(source, destination, count) CopyBytesWithinFarBanks((source), (destination), (count))
#define WRITE_CONTROLLER_BLOCK(source) WriteControllerRegisterBlock25FromMemory(source)
#define SUBMIT_DESCRIPTOR(cursor, mode) SubmitControllerCommand80AtBufferAddressAndWait((cursor), (mode))
#define POPULATE_SUBHEADER(cursor, source_kind) PopulateConfiguredMode2SubheaderWhenRequired((cursor), (source_kind))
#define UPDATE_Q(skip, decrement) UpdateRawReadQPacketControlAndIdentification((skip), (decrement))
#define ENQUEUE_DESCRIPTOR() EnqueueCurrentRawSectorDescriptorAtomic()
#define ADVANCE_MSF(reverse) AdvanceRawReadQPacketMsfTuples(reverse)
#define ADVANCE_RING(cursor) AdvanceRawReadRingPosition(cursor)
#define YIELD_COROUTINE0() YieldCoroutine0()
#endif
#endif

#if PLEXTOR_BINDING_SECTION == 2
#if defined(PLEXTOR_HOST_TEST)
static s32 DivideSyntheticWritePosition(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}
#else
extern s32 DivideS32QuotientNative(s32 dividend, s32 divisor);
#define DivideSyntheticWritePosition(dividend, divisor) DivideS32QuotientNative((dividend), (divisor))
#endif
#endif
