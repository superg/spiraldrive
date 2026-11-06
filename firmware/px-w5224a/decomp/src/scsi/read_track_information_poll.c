#include "read_track_information_poll.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../platform/track_state.h"
#include "full_toc_track_span.h"
#include "read_track_information_free_blocks.h"
#include "read_track_information_size_blocks.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_track_information_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern s32 PLEXTOR_NEAR_FUNCTION ComputeReadTrackInformationTrackSize(u8 track_number);

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_track_information_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/scsi/read_track_information_poll_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u16 ReadReadTrackWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

/* Reconstruction helper; no direct Ghidra address. */
static void WriteReadTrackU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadReadTrackU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:faeef2 */
u8 PLEXTOR_NEAR_FUNCTION PollReadTrackInformationRawDescriptorFlags(void)
{
    fw_addr_t output_address = CAPTURE_READ_TRACK_OUTPUT_ADDRESS();
    u8 producer_status = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
    u16 descriptor_address;

    if(producer_status != 0u)
    {
        u8 sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        u16 sense_code = ReadReadTrackWord(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84468) + (fw_addr_t)((u16)sense_index * 2u));

        sense_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE);
        SET_READ_TRACK_SENSE(FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F84454) + sense_index), sense_code);
        return 0u;
    }

    if(PREPARE_READ_TRACK_DESCRIPTOR_BATCH() == 0u)
        return 0xfeu;

    descriptor_address = GET_READ_TRACK_SECONDARY_DESCRIPTOR();
    if(FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x10u)) != 0u && VALIDATE_READ_TRACK_DESCRIPTOR(descriptor_address) == 0u)
        return 0u;

    FirmwareWrite8(output_address, FirmwareRead8((fw_addr_t)(u16)(descriptor_address + 0x12u)));
    return 1u;
}

/* RAM:faee30 */
u8 PLEXTOR_NEAR_FUNCTION AcquireReadTrackInformationFlagsFromRawDescriptor(u8 toc_point_index) PLEXTOR_REENTRANT
{
    fw_addr_t tuple_address = FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)toc_point_index * 6u);
    s32 target_position = ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
    u8 output_flags;
    u8 status;

    if(target_position == 0)
        target_position = 150l;

    CLEAR_READ_TRACK_PINS();
    WriteReadTrackU32(FW_NEAR_UNKNOWN_1455, 1ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1468, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1467, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1466, 0u);
    WriteReadTrackU32(FW_NEAR_UNKNOWN_1459, 0ul);
    CLEAR_READ_TRACK_MODE_BITS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F06, 0u);
    WriteReadTrackU32(FW_NEAR_UNKNOWN_0F0C, 0ul);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2A) == 2u && FirmwareReadBit0008() == 1u)
    {
        POSITION_READ_TRACK_PIPELINE(target_position);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C2A, 2u);
        RESET_READ_TRACK_QUEUE();
        INITIALIZE_READ_TRACK_COROUTINE_F90000();
        WriteReadTrackU32(FW_NEAR_UNKNOWN_1598, (u32)target_position);
        YIELD_READ_TRACK_COROUTINE0();
    }

    do
    {
        status = POLL_READ_TRACK_FLAGS(&output_flags);
        if(status == 0xfeu)
            YIELD_READ_TRACK_COROUTINE0();
    } while(status == 0xfeu);

    COMPLETE_READ_TRACK_PINS();
    if(status != 0u)
        return output_flags;

    RESET_READ_TRACK_QUEUE();
    INITIALIZE_READ_TRACK_COROUTINE_F84A00();
    DISPATCH_READ_TRACK_REQUEST5();
    YIELD_READ_TRACK_COROUTINE0();
    return 0x0fu;
}

/* RAM:faecfc */
u8 PLEXTOR_NEAR_FUNCTION BuildReadTrackInformationResponseFromFullToc(void)
{
    u8 toc_point_index;
    u16 descriptor_index;
    u16 descriptor_count;
    u8 adr_control;
    fw_addr_t tuple_address;
    u32 requested_position;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) == 0u)
    {
        requested_position = ReadReadTrackU32(FW_NEAR_UNKNOWN_0B0C) + 150ul;
        if((s32)requested_position >= (s32)ReadReadTrackU32(FW_NEAR_UNKNOWN_12C1))
        {
            UNSUPPORTED_FULL_TOC_READ_TRACK();
            return 0u;
        }
        toc_point_index = FIND_FULL_TOC_TRACK((s32)requested_position);
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) != 0u)
        {
            UNSUPPORTED_FULL_TOC_READ_TRACK();
            return 0u;
        }
        toc_point_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
        if(toc_point_index < FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) || toc_point_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
        {
            UNSUPPORTED_FULL_TOC_READ_TRACK();
            return 0u;
        }
    }

    for(descriptor_index = 0u; descriptor_index < 28u; descriptor_index++)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + descriptor_index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x1au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, toc_point_index);

    descriptor_count = (u16)(ReadReadTrackWord(FW_NEAR_UNKNOWN_12CA) / 11u);
    descriptor_index = 1u;
    while(descriptor_index <= descriptor_count && FirmwareRead8(FW_HUGE_UNKNOWN_2B8003 + (fw_addr_t)descriptor_index * 11u) != toc_point_index)
        descriptor_index++;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareRead8(FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)descriptor_index * 11u));

    adr_control = FirmwareRead8(FW_HUGE_UNKNOWN_00F2B7 + (fw_addr_t)((u16)toc_point_index * 6u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, (u8)(adr_control >> 4));
    if((adr_control & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, ACQUIRE_FULL_TOC_READ_TRACK_FLAGS(toc_point_index));
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, 0x0fu);

    tuple_address = FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)toc_point_index * 6u);
    WriteReadTrackU32(FW_NEAR_UNKNOWN_0B32, (u32)ConvertBinaryMsfToAdjustedFramePosition(tuple_address) - 150ul);
    WriteReadTrackU32(FW_NEAR_UNKNOWN_0B42, (u32)COMPUTE_FULL_TOC_READ_TRACK_SPAN(toc_point_index));
    return 1u;
}

/* Reconstruction helper; no direct Ghidra address. */
static u8 RejectRecordableReadTrackInformation(void)
{
    UNSUPPORTED_FULL_TOC_READ_TRACK();
    return 0u;
}

/* Reconstruction helper; no direct Ghidra address. */
static u8 SelectRecordableReadTrackDataMode(u8 response_source)
{
    u8 data_mode;

    if(response_source == 5u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
            {
                data_mode = 0x40u;
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 2u)
            {
                data_mode = 0xc0u;
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u)
            {
                data_mode = 0xa0u;
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                    data_mode |= 0x10u;
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
            {
                data_mode = 0x20u;
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                    data_mode |= 0x10u;
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u)
            {
                data_mode = 0x80u;
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
                {
                    data_mode |= 0x20u;
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                        data_mode |= 0x10u;
                }
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u)
            {
                data_mode = 0u;
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
                {
                    data_mode |= 0x20u;
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                        data_mode |= 0x10u;
                }
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u)
            {
                data_mode = 0x80u;
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
                {
                    data_mode |= 0x20u;
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                        data_mode |= 0x10u;
                }
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 7u)
            {
                data_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD5F) == 0u ? 0u : 0x80u;
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
                    data_mode |= 0x20u;
            }
            else
            {
                data_mode = 0u;
            }
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u)
        {
            data_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD61) == 0u ? 0u : 0x80u;
        }
        else
        {
            data_mode = 0u;
        }
    }
    else
    {
        data_mode = 0x80u;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
        {
            data_mode |= 0x20u;
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
                data_mode |= 0x10u;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u
        || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) == 0u)
            data_mode |= 0x0fu;
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) == 1u)
            data_mode |= 1u;
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) == 2u)
            data_mode |= 2u;
        else
            data_mode |= 0x0fu;
    }
    else
    {
        data_mode |= 0x0fu;
    }
    return data_mode;
}

/* RAM:fae689 */
u8 PLEXTOR_NEAR_FUNCTION BuildReadTrackInformationResponse(void)
{
    u8 inherited_b = CAPTURE_READ_TRACK_INHERITED_B();
    u8 track_number;
    u8 response_source;
    u16 descriptor_count;
    u16 descriptor_index;
    s32 requested_position;
    u8 mode_byte;
    u32 packet_size;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) == 0u)
    {
        requested_position = (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_0B0C) + 150ul);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
        {
            if(requested_position > (s32)ReadReadTrackU32(FW_NEAR_UNKNOWN_12C1))
                return RejectRecordableReadTrackInformation();
            track_number = FindTrackNumberForFramePositionFromTocTable(requested_position);
            response_source = 3u;
        }
        else if(requested_position < (s32)ReadReadTrackU32(FW_NEAR_UNKNOWN_DD7A))
        {
            track_number = FindTrackNumberForFramePositionFromTocTable(requested_position);
            response_source = 3u;
        }
        else
        {
            track_number = FindTrackTableIndexForFramePosition(requested_position);
            response_source = 5u;
        }
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0D) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) != 0u)
            return RejectRecordableReadTrackInformation();

        track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
        {
            if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF))
                return RejectRecordableReadTrackInformation();
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) != 0u)
        {
            if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_1147))
                return RejectRecordableReadTrackInformation();
        }
        else if(track_number == 0u)
        {
            return RejectRecordableReadTrackInformation();
        }

        if(track_number != 0xffu && track_number > 99u)
            return RejectRecordableReadTrackInformation();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u && track_number == 0xffu)
            return RejectRecordableReadTrackInformation();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u && track_number > FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0))
            return RejectRecordableReadTrackInformation();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
        {
            response_source = 3u;
        }
        else if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
        {
            response_source = 3u;
        }
        else
        {
            response_source = 5u;
            if(track_number == 0xffu)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) == 100u)
                    return RejectRecordableReadTrackInformation();
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
                {
                    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC);
                }
                else
                {
                    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
                    mode_byte = FirmwareRead8(FW_HUGE_UNKNOWN_00DD87 + (fw_addr_t)((u16)track_number * 19u));
                    if(mode_byte != 0u && mode_byte != 1u)
                        return RejectRecordableReadTrackInformation();
                }
            }
        }
    }

    if(response_source == 5u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x80u) != 0u && track_number != FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35))
                return RejectRecordableReadTrackInformation();
            CLEAR_RECORDABLE_READ_TRACK();
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
            {
                if(LOAD_RECORDABLE_READ_TRACK(track_number, inherited_b) == 0u)
                    return 0u;
            }
            else if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
            {
                if(LOAD_RECORDABLE_READ_TRACK(track_number, inherited_b) == 0u)
                    return 0u;
            }
            else if(LOAD_RECORDABLE_READ_TRACK_SNAPSHOT(track_number, inherited_b) == 0u)
            {
                return 0u;
            }
        }
        else if(track_number != FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35))
        {
            return RejectRecordableReadTrackInformation();
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
    {
        CLEAR_RECORDABLE_READ_TRACK();
        if(LOAD_RECORDABLE_READ_TRACK(track_number, inherited_b) == 0u)
            return 0u;
    }
    else if(track_number != FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35))
    {
        return RejectRecordableReadTrackInformation();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x1au);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, track_number);

    if(response_source == 3u)
    {
        descriptor_count = (u16)(ReadReadTrackWord(FW_NEAR_UNKNOWN_12CA) / 11u);
        descriptor_index = 1u;
        while(descriptor_index <= descriptor_count && FirmwareRead8(FW_HUGE_UNKNOWN_2B8003 + (fw_addr_t)(descriptor_index * 11u)) != track_number)
            descriptor_index++;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareRead8(FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)(descriptor_index * 11u)));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, 0u);

    if(response_source == 3u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F2B7 + (fw_addr_t)((u16)track_number * 6u)) >> 4));
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
            mode_byte = 0u;
        else
            mode_byte = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) >> 4);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 7u)
            mode_byte |= 0x20u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, mode_byte);
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, SelectRecordableReadTrackDataMode(response_source));

    if(response_source != 5u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 7u
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 0u);
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, 1u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, IsTrackTableTailEntryUninitialized() != 0u ? 1u : 0u);

    if(response_source == 3u)
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B32, (u32)ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_F2B9 + (fw_addr_t)((u16)track_number * 6u)) - 150ul);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B32, (u32)ResolveReadTrackInformationTrackStartFrame() - 150ul);
    else
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B32, ReadReadTrackU32(FW_NEAR_UNKNOWN_DD3D) - 150ul);

    if(response_source == 5u)
    {
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B36, (u32)SelectReadTrackInformationNextWritableFramePosition());
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B3A, COMPUTE_READ_TRACK_FREE_BLOCKS());
    }
    else
    {
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B36, 0ul);
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B3A, 0ul);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 5u
        || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 6u)
        packet_size = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u ? 0ul : ReadReadTrackU32(FW_NEAR_UNKNOWN_DD57);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u)
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) != 0u)
            packet_size = ReadReadTrackU32(FW_NEAR_UNKNOWN_0C4B);
        else
            packet_size = 0ul;
    else
        packet_size = 0ul;
    WriteReadTrackU32(FW_NEAR_UNKNOWN_0B3E, packet_size);

    if(response_source == 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B42, (u32)ComputeReadTrackInformationTrackSize(track_number));
    else
        WriteReadTrackU32(FW_NEAR_UNKNOWN_0B42, COMPUTE_READ_TRACK_SIZE_BLOCKS());
    return 1u;
}

/* RAM:faeffa */
s32 PLEXTOR_NEAR_FUNCTION SelectReadTrackInformationNextWritableFramePosition(void)
{
    s32 inherited_fallback = CAPTURE_READ_TRACK_NEXT_WRITABLE_FALLBACK();
    u8 write_type;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u)
    {
        write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
        if(write_type == 0u || write_type == 1u)
            return COMPUTE_READ_TRACK_NEXT_WRITABLE();
        if(write_type == 2u)
            return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_DD39) - 150ul);
        if(write_type == 3u)
            return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_DD69) - 150ul);
        return inherited_fallback;
    }

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
            return (s32)((u32)COMPUTE_READ_TRACK_PACKET_PROGRESS(1u) - 150ul);
        return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_DD4D) - 150ul);
    }
    if(write_type == 1u)
        return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_DD4D) - 150ul);
    if(write_type == 2u)
        return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_140A) - 150ul);
    if(write_type == 3u)
        return (s32)(ReadReadTrackU32(FW_NEAR_UNKNOWN_DD65) - 150ul);
    return inherited_fallback;
}

/* RAM:faf7ba */
s32 PLEXTOR_NEAR_FUNCTION ResolveReadTrackInformationTrackStartFrame(void)
{
    u16 record_count;
    u16 record_index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 4u) == 0u)
        return (s32)ReadReadTrackU32(FW_NEAR_UNKNOWN_DD3D);

    record_count = ReadReadTrackWord(FW_NEAR_UNKNOWN_B4BB);
    for(record_index = 0u; record_index < record_count; ++record_index)
    {
        fw_addr_t record_address = FW_NEAR_UNKNOWN_B4BD + (fw_addr_t)(record_index << 3);

        if((FirmwareRead8(record_address) & 0x0fu) == 1u && FirmwareRead8(record_address + 1u) != 0u && FirmwareRead8(record_address + 2u) != 0u)
            return ConvertBinaryMsfToAdjustedFramePosition(record_address + 4u);
    }

    /* Valid raw-track state always contains the qualifying ADR-1 record.
     * The native body reads an unwritten local if that invariant is broken. */
    return (s32)ReadReadTrackU32(FW_NEAR_UNKNOWN_DD3D);
}
