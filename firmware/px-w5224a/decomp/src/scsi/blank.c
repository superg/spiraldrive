#include "blank.h"
#include "../media/blank_format_raw_read_plan.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/blank_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBlankU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadBlankU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBlankU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u32 ConvertBlankDescriptorMsf(fw_addr_t address)
{
    return (u32)BLANK_CONVERT_MSF(FirmwareRead8(address), FirmwareRead8(address + 1u), FirmwareRead8(address + 2u));
}

static u32 FindBlankSessionPoint(u8 session, u8 point, u8 msf_offset, u32 fallback)
{
    u16 descriptor_count = (u16)(ReadBlankU16(FW_NEAR_UNKNOWN_12CA) / 11u);
    u16 index;

    for(index = 1u; index <= descriptor_count; ++index)
    {
        fw_addr_t descriptor = FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)index * 11ul;
        if(FirmwareRead8(descriptor) == session && FirmwareRead8(descriptor + 3u) == point)
            return ConvertBlankDescriptorMsf(descriptor + msf_offset);
    }
    return fallback;
}

static void ConfigureBlankSessionBounds(u8 publish_saved_limit)
{
    u8 session_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9);
    u32 end_position = FindBlankSessionPoint(session_count, 0xa2u, 8u, ReadBlankU32(FW_NEAR_UNKNOWN_E61A));
    u32 reference;
    u32 limit;

    WriteBlankU32(FW_NEAR_UNKNOWN_E61A, end_position);
    if(session_count == 1u)
    {
        WriteBlankU32(FW_NEAR_UNKNOWN_E612, ReadBlankU32(FW_NEAR_UNKNOWN_114F));
        WriteBlankU32(FW_NEAR_UNKNOWN_E616, 0ul);
        limit = end_position + 0x1a5eul;
    }
    else
    {
        reference = FindBlankSessionPoint((u8)(session_count - 1u), 0xb0u, 4u, ReadBlankU32(FW_NEAR_UNKNOWN_DD7A));
        WriteBlankU32(FW_NEAR_UNKNOWN_DD7A, reference);
        WriteBlankU32(FW_NEAR_UNKNOWN_E616, reference);
        WriteBlankU32(FW_NEAR_UNKNOWN_E612, reference - 0x1194ul);
        limit = end_position + 0x08caul;
    }
    if(publish_saved_limit != 0u)
        WriteBlankU32(FW_NEAR_UNKNOWN_DD75, limit);
    WriteBlankU32(FW_NEAR_UNKNOWN_E61E, limit);
}

static void UpdateOpenTrackEnd(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        WriteBlankU32(FW_NEAR_UNKNOWN_DD41, ReadBlankU32(FW_NEAR_UNKNOWN_DD4D));
}

static u8 RequireBlankTrackTableReady(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
    {
        BLANK_SENSE(5u, 0x2700u);
        return 0u;
    }
    if(BLANK_TAIL_UNINITIALIZED() != 0u)
    {
        BLANK_SENSE(5u, 0x2c04u);
        return 0u;
    }
    return 1u;
}

static u8 ConfigureBlankType2(void)
{
    u8 track = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E601, track);
    if(track == 0u || BLANK_LAST_TRACK() < track)
    {
        BLANK_UNSUPPORTED();
        return 0u;
    }
    if(RequireBlankTrackTableReady() == 0u)
        return 0u;
    if(track < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
    {
        BLANK_UNSUPPORTED();
        return 0u;
    }
    BLANK_LOAD_TRACK(track);
    UpdateOpenTrackEnd();
    return 1u;
}

static u8 ConfigureBlankType3(void)
{
    u8 track;

    if(RequireBlankTrackTableReady() == 0u)
        return 0u;
    track = BLANK_LAST_TRACK();
    BLANK_LOAD_TRACK(track);
    UpdateOpenTrackEnd();
    return 1u;
}

static u8 ConfigureBlankType4(void)
{
    u32 requested = ReadBlankU32(FW_NEAR_UNKNOWN_0B0C);
    u8 track;
    u32 start;
    u32 packet_size;

    if(RequireBlankTrackTableReady() == 0u)
        return 0u;
    WriteBlankU32(FW_NEAR_UNKNOWN_E602, requested);
    track = BLANK_FIND_TRACK((s32)(requested + 150ul));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E601, track);
    BLANK_LOAD_TRACK(track);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) != 0x50u)
    {
        BLANK_SENSE(5u, 0x6400u);
        return 0u;
    }
    start = ReadBlankU32(FW_NEAR_UNKNOWN_DD3D) - 150ul;
    packet_size = ReadBlankU32(FW_NEAR_UNKNOWN_DD57);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) != 0u)
    {
        u32 packet_count;
        u32 remainder;
        if(packet_size == 0u)
        {
            BLANK_SENSE(5u, 0x6400u);
            return 0u;
        }
        packet_count = BLANK_DIVIDE(requested - start, packet_size);
        remainder = requested - (start + BLANK_MULTIPLY(packet_size, packet_count));
        requested = start + BLANK_MULTIPLY(packet_size + 7ul, packet_count) + remainder;
        WriteBlankU32(FW_NEAR_UNKNOWN_E602, requested);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        WriteBlankU32(FW_NEAR_UNKNOWN_DD41, ReadBlankU32(FW_NEAR_UNKNOWN_DD4D));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
    {
        if(requested + 150ul != ReadBlankU32(FW_NEAR_UNKNOWN_DD3D))
        {
            BLANK_UNSUPPORTED();
            return 0u;
        }
    }
    else if(BLANK_REMAINDER(requested + 150ul - ReadBlankU32(FW_NEAR_UNKNOWN_DD3D), packet_size + 7ul) != 0u)
    {
        BLANK_UNSUPPORTED();
        return 0u;
    }
    return 1u;
}

static u8 ConfigureBlankType5(void)
{
    u8 last_track = BLANK_LAST_TRACK();

    BLANK_LOAD_TRACK(last_track);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        if(BLANK_TAIL_UNINITIALIZED() == 0u)
        {
            BLANK_SENSE(5u, 0x2c03u);
            return 0u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        {
            BLANK_NO_SENSE();
            return 0u;
        }
        BLANK_LOAD_TRACK((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E) - 1u));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 2u);
    ConfigureBlankSessionBounds(1u);
    return 1u;
}

static u8 ConfigureBlankType6(void)
{
    u8 last_track;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
    {
        last_track = BLANK_LAST_TRACK();
        BLANK_LOAD_TRACK(last_track);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 2u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
        ConfigureBlankSessionBounds(0u);
        return 1u;
    }
    if(BLANK_TAIL_UNINITIALIZED() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        {
            BLANK_NO_SENSE();
            return 0u;
        }
        BLANK_LOAD_TRACK((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E) - 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 2u);
        ConfigureBlankSessionBounds(0u);
        return 1u;
    }

    last_track = BLANK_LAST_TRACK();
    BLANK_LOAD_TRACK(last_track);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 1u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 1u)
        WriteBlankU32(FW_NEAR_UNKNOWN_E61A, ReadBlankU32(FW_NEAR_UNKNOWN_DD4D));
    else
        WriteBlankU32(FW_NEAR_UNKNOWN_E61A, ReadBlankU32(FW_NEAR_UNKNOWN_DD41));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
    {
        WriteBlankU32(FW_NEAR_UNKNOWN_E612, ReadBlankU32(FW_NEAR_UNKNOWN_114F));
        WriteBlankU32(FW_NEAR_UNKNOWN_E616, 0ul);
        WriteBlankU32(FW_NEAR_UNKNOWN_DD75, ReadBlankU32(FW_NEAR_UNKNOWN_E61A) + 0x1a5eul);
    }
    else
    {
        u32 reference = FindBlankSessionPoint(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9), 0xb0u, 4u, ReadBlankU32(FW_NEAR_UNKNOWN_DD7A));
        WriteBlankU32(FW_NEAR_UNKNOWN_DD7A, reference);
        WriteBlankU32(FW_NEAR_UNKNOWN_E616, reference);
        WriteBlankU32(FW_NEAR_UNKNOWN_E612, reference - 0x1194ul);
        WriteBlankU32(FW_NEAR_UNKNOWN_DD75, ReadBlankU32(FW_NEAR_UNKNOWN_E61A) + 0x08caul);
    }
    WriteBlankU32(FW_NEAR_UNKNOWN_E61E, ReadBlankU32(FW_NEAR_UNKNOWN_DD75));
    return 1u;
}

static u8 ConfigureBlankPlan(u8 operation)
{
    if(operation == 0u)
    {
        WriteBlankU32(FW_NEAR_UNKNOWN_E606, ReadBlankU32(FW_NEAR_CURRENT_FRAME_POSITION) + 0x08caul);
        return 1u;
    }
    if(operation == 1u)
    {
        WriteBlankU32(FW_NEAR_UNKNOWN_E606, 150ul);
        return 1u;
    }
    if(operation == 2u)
        return ConfigureBlankType2();
    if(operation == 3u)
        return ConfigureBlankType3();
    if(operation == 4u)
        return ConfigureBlankType4();
    if(operation == 5u)
        return ConfigureBlankType5();
    if(operation == 6u)
        return ConfigureBlankType6();
    BLANK_UNSUPPORTED();
    return 0u;
}

/* RAM:fc0a70 */
void HandleBlankCommand(void)
{
    u8 operation;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        BLANK_SENSE(2u, 0x3006u);
        return;
    }
    if(BLANK_VALIDATE_CODE() == 0u || BLANK_VALIDATE_FLAGS() == 0u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
    {
        BLANK_SEQUENCE_ERROR();
        return;
    }
    if(BLANK_VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E76)) == 0u)
    {
        BLANK_UNSUPPORTED();
        return;
    }

    BLANK_NORMALIZE_SPEED();
    operation = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 7u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E600, operation);
    if(operation > 6u)
    {
        BLANK_UNSUPPORTED();
        return;
    }
    if((operation == 0u || operation == 1u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 4u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAE, 0u);
        WriteBlankU32(FW_NEAR_UNKNOWN_0E3B, 150ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 2u));
        BLANK_INITIALIZE_COROUTINE1(FW_ENTRY_RUN_MEDIA_STATE_COROUTINE1);
        BLANK_YIELD();
        operation = 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E600, 0u);
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            BLANK_STATUS_SENSE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        {
            BLANK_SENSE(2u, 0x0401u);
            return;
        }
    }

    if(ConfigureBlankPlan(operation) == 0u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x10u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        BLANK_COMPLETE_NO_SENSE();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
    BLANK_RESET_RAW_READ();
    BLANK_INITIALIZE_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
    BLANK_YIELD();
    if(BLANK_ENSURE_OPC() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E600) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_E600) == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E622, 1u);
            WriteBlankU32(FW_NEAR_UNKNOWN_E60A, ReadBlankU32(FW_NEAR_UNKNOWN_E606) - (ReadBlankU32(FW_NEAR_UNKNOWN_114F) - 1000ul));
        }
        WriteBlankU32(FW_NEAR_UNKNOWN_1455, 0ul);
        if(BLANK_EXECUTE_PLAN() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfdu));
            BLANK_UPDATE_TRACK_STATE();
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
                BLANK_RESET_TYPE2_RECORDS();
            BLANK_NO_SENSE();
        }
        else
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E622, 0u);
            BLANK_UPDATE_TRACK_STATE();
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_E622, 0u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}
