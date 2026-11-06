#include "read_format_capacities.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

extern u32 DivideU32Quotient(u32 dividend, u32 divisor);
extern void LoadTrackInformationByNumber(u8 track_number);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_format_capacities_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helpers; no direct Ghidra addresses. */
static u16 ReadFormatWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFormatDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFormatDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void WriteFormatDescriptor(u8 response_offset, u32 block_count, u8 descriptor_type, u32 block_length)
{
    fw_addr_t descriptor = (fw_addr_t)(FW_NEAR_UNKNOWN_0B2A + response_offset);

    WriteFormatDword(descriptor, block_count);
    FirmwareWrite8(FirmwareAddressIncrement24(descriptor, 4u), (u8)(block_length >> 24));
    FirmwareWrite8(FirmwareAddressIncrement24(descriptor, 5u), (u8)(block_length >> 16));
    FirmwareWrite8(FirmwareAddressIncrement24(descriptor, 6u), (u8)(block_length >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(descriptor, 7u), (u8)block_length);
    FirmwareWrite8(FirmwareAddressIncrement24(descriptor, 4u), descriptor_type);
}

static s32 ConvertReadFormatMsf(u8 minute, u8 second, u8 frame)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_format_capacities_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u32 ReadTocRecordCapacity(u8 track_number)
{
    fw_addr_t record = (fw_addr_t)(FW_HUGE_UNKNOWN_2BB03B + (u16)track_number * 10u);
    u8 minute = FirmwareRead8(FirmwareAddressIncrement24(record, 3u));
    u8 second = FirmwareRead8(FirmwareAddressIncrement24(record, 4u));
    u8 frame = FirmwareRead8(FirmwareAddressIncrement24(record, 5u));

    return (u32)ConvertReadFormatMsf(minute, second, frame) - 152ul;
}

static u32 ReadFullTocLeadOutCapacity(void)
{
    u8 descriptor_count = (u8)(ReadFormatWord(FW_NEAR_UNKNOWN_12CA) / 11u);
    u8 selected_index = 0u;
    u8 index;
    u8 minute;
    u8 second;
    u8 frame;
    fw_addr_t descriptor;

    for(index = 1u; index <= descriptor_count; index++)
    {
        descriptor = (fw_addr_t)(FW_HUGE_UNKNOWN_2B8003 + (u16)index * 11u);
        if(FirmwareRead8(descriptor) == 0xa2u)
            selected_index = index;
    }

    descriptor = (fw_addr_t)(FW_HUGE_UNKNOWN_2B8001 + (u16)selected_index * 11u);
    minute = FirmwareRead8(FirmwareAddressIncrement24(descriptor, 7u));
    second = FirmwareRead8(FirmwareAddressIncrement24(descriptor, 8u));
    frame = FirmwareRead8(FirmwareAddressIncrement24(descriptor, 9u));
    return (u32)ConvertReadFormatMsf(minute, second, frame) - 150ul;
}

static u32 SelectReadFormatCapacity(u8 *descriptor_type)
{
    u8 last_toc_track;
    u32 capacity;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
    {
        *descriptor_type = 3u;
        return 0ul;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
    {
        *descriptor_type = 2u;
        return ReadFullTocLeadOutCapacity();
    }

    *descriptor_type = 3u;
    last_toc_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
    if(last_toc_track == 0u)
        return 0ul;

    READ_FORMAT_LOAD_TRACK((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1148) + 1u));
    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34))
    {
    case 0u:
        capacity = (u32)READ_FORMAT_NEXT_WRITABLE();
        READ_FORMAT_LOAD_TRACK(FirmwareReadNear8(FW_NEAR_UNKNOWN_1148));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 0u)
            capacity -= 152ul;
        return capacity;
    case 1u:
    case 2u:
    case 3u:
        return (u32)READ_FORMAT_NEXT_WRITABLE();
    default:
        return ReadTocRecordCapacity(last_toc_track);
    }
}

static u32 SelectPacketSize(u8 last_toc_track)
{
    fw_addr_t record = (fw_addr_t)(FW_HUGE_UNKNOWN_2BB03B + (u16)last_toc_track * 10u);

    if((FirmwareRead8(record) & 0x50u) == 0x50u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x10u) != 0u)
        return ReadFormatDword(FW_NEAR_UNKNOWN_0C4B);
    return 0x20ul;
}

static u32 ComputePacketAlignedFormatCapacity(u32 packet_size)
{
    u32 capacity;

    if(packet_size == 0ul)
        return 0ul;
    capacity = ReadFormatDword(FW_NEAR_CURRENT_FRAME_POSITION) + 7ul - 150ul;
    capacity = READ_FORMAT_DIVIDE(READ_FORMAT_MULTIPLY(capacity, packet_size), packet_size + 7ul);
    return READ_FORMAT_MULTIPLY(READ_FORMAT_DIVIDE(capacity, packet_size), packet_size);
}

static u8 AppendWritableFormatDescriptors(u8 response_offset, u32 current_capacity)
{
    u32 packet_size = SelectPacketSize(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149));
    u32 format_capacity = ComputePacketAlignedFormatCapacity(packet_size);
    u32 remaining_capacity = format_capacity - current_capacity;

    WriteFormatDescriptor(response_offset, format_capacity, 0u, 0x800ul);
    response_offset = (u8)(response_offset + 8u);
    WriteFormatDescriptor(response_offset, format_capacity, 0x40u, packet_size);
    response_offset = (u8)(response_offset + 8u);
    WriteFormatDescriptor(response_offset, remaining_capacity, 0x44u, packet_size);
    response_offset = (u8)(response_offset + 8u);
    WriteFormatDescriptor(response_offset, remaining_capacity, 0x48u, packet_size);
    response_offset = (u8)(response_offset + 8u);
    WriteFormatDescriptor(response_offset, READ_FORMAT_GEOMETRY(), 0x90u, 0ul);
    return (u8)(response_offset + 8u);
}

/* RAM:fd2192 */
void HandleReadFormatCapacitiesCommand(void)
{
    u16 allocation_length;
    u8 saved_track_number;
    u8 response_length;
    u8 descriptor_type;
    u32 capacity;
    u32 block_length;

    if(READ_FORMAT_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8D0B)) == 0u)
    {
        READ_FORMAT_UNSUPPORTED();
        return;
    }

    allocation_length = ReadFormatWord(FW_NEAR_UNKNOWN_0B11);
    saved_track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        capacity = READ_FORMAT_GEOMETRY();
        block_length = ReadFormatDword(FW_NEAR_UNKNOWN_0C96);
        WriteFormatDescriptor(4u, capacity, 0x92u, block_length);
        capacity = READ_FORMAT_GEOMETRY();
        WriteFormatDescriptor(12u, capacity, 0x90u, 0ul);
        response_length = 20u;
    }
    else
    {
        capacity = SelectReadFormatCapacity(&descriptor_type);
        WriteFormatDescriptor(4u, capacity, descriptor_type, 0x800ul);
        response_length = 12u;
        if(descriptor_type == 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u)
            response_length = AppendWritableFormatDescriptors(response_length, capacity);
        READ_FORMAT_LOAD_TRACK(saved_track_number);
    }

    WriteFormatDword(FW_NEAR_UNKNOWN_0B2A, (u32)(response_length - 4u));
    if(allocation_length > response_length)
        allocation_length = response_length;
    if(READ_FORMAT_TRANSFER(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length) != 4u)
        READ_FORMAT_NO_SENSE();
}
