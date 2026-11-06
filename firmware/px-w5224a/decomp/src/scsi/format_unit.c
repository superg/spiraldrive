#include "format_unit.h"
#include "media/blank_format_raw_read_plan.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/format_unit_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadFormatU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadFormatU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteFormatU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void SetFormatSense(u16 sense_code)
{
    FORMAT_SENSE(5u, sense_code);
}

static void SetFormatBounds(u32 lead_in, u32 reference, u32 block_count, u32 packet_overhead, u32 lead_out_extra)
{
    WriteFormatU32(FW_NEAR_UNKNOWN_E612, lead_in);
    WriteFormatU32(FW_NEAR_UNKNOWN_E616, reference);
    WriteFormatU32(FW_NEAR_UNKNOWN_E61A, reference + 1ul + block_count + packet_overhead);
    WriteFormatU32(FW_NEAR_UNKNOWN_E61E, ReadFormatU32(FW_NEAR_UNKNOWN_E61A) + lead_out_extra);
}

static u32 FindPreviousSessionReference(void)
{
    u16 descriptor_count = (u16)(ReadFormatU16(FW_NEAR_UNKNOWN_12CA) / 11u);
    u16 index;
    u8 wanted_session = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) - 1u);
    u32 reference = ReadFormatU32(FW_NEAR_UNKNOWN_DD7A);

    for(index = 1u; index <= descriptor_count; ++index)
    {
        fw_addr_t descriptor = FW_HUGE_UNKNOWN_2B8000 + (fw_addr_t)index * 11ul;
        if(FirmwareRead8(descriptor) == wanted_session && FirmwareRead8(descriptor + 3u) == 0xb0u)
        {
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/format_unit_bindings.h"
#undef PLEXTOR_BINDING_SECTION
            WriteFormatU32(FW_NEAR_UNKNOWN_DD7A, reference);
            break;
        }
    }
    return reference;
}

static void SetFirstSessionBounds(u32 block_count, u32 packet_overhead)
{
    SetFormatBounds(ReadFormatU32(FW_NEAR_UNKNOWN_114F), 0xfffffffful, block_count, packet_overhead, 0x1a5eul);
}

static void SetLaterSessionBounds(u32 block_count, u32 packet_overhead)
{
    u32 reference = FindPreviousSessionReference();
    SetFormatBounds(reference - 0x1194ul, reference - 1ul, block_count, packet_overhead, 0x08caul);
}

static u8 ConfigureAppendBounds(u32 block_count, u32 packet_overhead, u8 selected_mode)
{
    u8 last_track = FORMAT_GET_LAST_TRACK();
    u8 data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);

    FORMAT_LOAD_TRACK(last_track);
    if(ReadFormatU32(FW_NEAR_UNKNOWN_DD57) != ReadFormatU32(FW_NEAR_UNKNOWN_0C4B) || (u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) != (u16)((u16)selected_mode << 4))
    {
        SetFormatSense(0x6400u);
        return 0u;
    }
    if(data_block_type >= 9u && data_block_type <= 13u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) != 2u)
        {
            SetFormatSense(0x6400u);
            return 0u;
        }
    }
    else if(data_block_type == 8u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) != 1u)
    {
        SetFormatSense(0x6400u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) == 0u)
    {
        SetFormatSense(0x6400u);
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 0u)
    {
        if(FORMAT_TAIL_UNINITIALIZED() == 0u)
        {
            FORMAT_SEQUENCE_ERROR();
            return 0u;
        }
        last_track = FORMAT_GET_LAST_TRACK();
        FORMAT_LOAD_TRACK(last_track);
        FORMAT_UPDATE_TRACK_TABLE();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        {
            FORMAT_SEQUENCE_ERROR();
            return 0u;
        }
    }
    else
    {
        last_track = FORMAT_GET_LAST_TRACK();
        FORMAT_LOAD_TRACK(last_track);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 1u)
        SetFirstSessionBounds(block_count, packet_overhead);
    else
        SetLaterSessionBounds(block_count, packet_overhead);
    return 1u;
}

static u8 ConfigureNewFormatBounds(u32 block_count, u32 packet_overhead)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_E600, 1u);
            WriteFormatU32(FW_NEAR_UNKNOWN_E606, 0x96ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
            FORMAT_RESET_RAW_READ();
            FORMAT_INITIALIZE_COROUTINE(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
            FORMAT_YIELD();
            if(FORMAT_ENSURE_OPC() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
                return 0u;
            }
            WriteFormatU32(FW_NEAR_UNKNOWN_1455, 0ul);
            FORMAT_EXECUTE_RAW_READ_PLAN();
            FORMAT_UPDATE_TRACK_STATE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        }
        SetFirstSessionBounds(block_count, packet_overhead);
        FORMAT_RESET_TRACK_RECORDS(1u);
        FORMAT_LOAD_TRACK(1u);
        return 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u)
    {
        SetFormatSense(0x2700u);
        return 0u;
    }
    if(FORMAT_TAIL_UNINITIALIZED() == 0u)
    {
        FORMAT_SEQUENCE_ERROR();
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
        SetFirstSessionBounds(block_count, packet_overhead);
    else
        SetFormatBounds(ReadFormatU32(FW_NEAR_UNKNOWN_DD69), ReadFormatU32(FW_NEAR_UNKNOWN_DD69) + 0x1193ul, block_count, packet_overhead, 0x08caul);
    return 1u;
}

static u8 ValidateFormatGeometry(u32 block_count, u8 selected_mode)
{
    u32 packet_size = ReadFormatU32(FW_NEAR_UNKNOWN_0C4B);
    u32 quotient = FORMAT_DIVIDE(block_count, packet_size);
    u32 packet_overhead = FORMAT_MULTIPLY(7ul, quotient) + 0x91ul;
    u8 append = FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB);

    FORMAT_INITIALIZE_TRACK_FLAGS();
    if(append != 0u)
    {
        if(ConfigureAppendBounds(block_count, packet_overhead, selected_mode) == 0u)
            return 0u;
    }
    else if(ConfigureNewFormatBounds(block_count, packet_overhead) == 0u)
    {
        return 0u;
    }

    if((s32)ReadFormatU32(FW_NEAR_UNKNOWN_E61A) > (s32)ReadFormatU32(FW_NEAR_CURRENT_FRAME_POSITION))
    {
        SetFormatSense(0x2600u);
        return 0u;
    }
    if(append != 0u && (s32)ReadFormatU32(FW_NEAR_UNKNOWN_E61A) < (s32)ReadFormatU32(FW_NEAR_UNKNOWN_DD41))
    {
        SetFormatSense(0x2600u);
        return 0u;
    }
    if(FORMAT_EXACT_MULTIPLE(block_count) == 0u)
    {
        SetFormatSense(0x2600u);
        return 0u;
    }
    return 1u;
}

static u8 ReceiveAndValidateFormatParameters(u8 format_code, u8 selected_mode, u8 *code11_variant)
{
    u8 index;
    u32 block_count;

    if(FORMAT_RECEIVE(FW_NEAR_UNKNOWN_0B2A, format_code == 0x11u ? 12ul : 16ul) == 4u)
        return 0u;
    if(format_code == 0x11u)
    {
        *code11_variant = FORMAT_PARSE_CODE11();
        if(*code11_variant == 0xffu)
        {
            SetFormatSense(0x2600u);
            return 0u;
        }
        return 1u;
    }

    for(index = 0u; index < 12u; ++index)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A + index) & FirmwareRead8(FW_HUGE_UNKNOWN_F78E83 + index)) != 0u)
        {
            SetFormatSense(0x2600u);
            return 0u;
        }
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2D) != 8u)
    {
        SetFormatSense(0x2600u);
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E5FA, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32) & 0x80u) != 0u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_E5FB, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B32) & 0x40u) != 0u));
    block_count = ReadFormatU32(FW_NEAR_UNKNOWN_0B36);
    WriteFormatU32(FW_NEAR_UNKNOWN_E5FC, block_count);
    return ValidateFormatGeometry(block_count, selected_mode);
}

/* RAM:fb2af5 */
void HandleFormatUnitCommand(void)
{
    u8 format_code;
    u8 selected_mode;
    u8 code11_variant = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 4u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            FORMAT_STATUS_SENSE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            return;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
        {
            FORMAT_SENSE(2u, 0x0401u);
            return;
        }
    }
    if(FORMAT_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8E6F)) == 0u)
    {
        FORMAT_UNSUPPORTED();
        return;
    }

    format_code = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 0x1fu);
    if(format_code == 0x11u)
        FORMAT_INITIALIZE_DEFAULTS();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 2u) == 0u)
    {
        SetFormatSense(0x6400u);
        return;
    }
    selected_mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 0xfdu);
    if(selected_mode != 5u || ReadFormatU32(FW_NEAR_UNKNOWN_0C4B) == 0ul || FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) < 8u)
    {
        SetFormatSense(0x6400u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    if(format_code != 0x11u && format_code != 0x17u)
    {
        FORMAT_UNSUPPORTED();
        return;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
    {
        FORMAT_SENSE(2u, 0x3006u);
        return;
    }
    if(FORMAT_VALIDATE_CODE() == 0u || FORMAT_VALIDATE_FLAGS() == 0u)
        return;
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
    {
        FORMAT_SEQUENCE_ERROR();
        return;
    }

    FORMAT_NORMALIZE_SPEED();
    if(ReceiveAndValidateFormatParameters(format_code, selected_mode, &code11_variant) == 0u)
        return;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2B) & 2u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        FORMAT_COMPLETE_NO_SENSE();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));

    if(format_code == 0x11u)
    {
        FORMAT_START_CODE11((u32)code11_variant);
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u && (s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
            FORMAT_RUN_CODE11_SCAN();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) | 0x20u));
        FORMAT_EXECUTE_MEDIA();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0xdfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        WriteFormatU32(FW_NEAR_SCAN_POSITION_SECOND, 0ul);
        WriteFormatU32(FW_NEAR_SCAN_POSITION_FIRST, 0ul);
    }
}
