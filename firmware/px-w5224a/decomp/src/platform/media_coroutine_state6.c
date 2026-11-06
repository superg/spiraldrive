#include "media_coroutine_state6.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/media_coroutine_state6_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMediaCoroutineState6U16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadMediaCoroutineState6U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMediaCoroutineState6U16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteMediaCoroutineState6U32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void SelectMediaCoroutineState6Direction(u8 select_reverse)
{
    if(select_reverse != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
        STATE6_WRITE_P3_BIT0(1u);
        STATE6_WRITE_P3_BIT1(0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
        STATE6_WRITE_P3_BIT0(0u);
        STATE6_WRITE_P3_BIT1(1u);
    }
}

static void ResetMediaCoroutineState6FailurePath(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
    STATE6_CLEAR_FLAGS();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
    SelectMediaCoroutineState6Direction(1u);
}

/* RAM:f87af9 */
void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState6(void)
{
    u8 calibration_status;
    u8 saved_requested_code;
    u8 media_type;
    u16 calibration_flags;

    STATE6_WRITE_COMPLETION_BIT(0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 0xfdu));

    calibration_flags = ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAD7);
    media_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if((calibration_flags & 0x1000u) != 0u || media_type == 2u)
    {
        u8 reduction_enabled = (u8)((
            (ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAD7) & 3u) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u || (ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) != 0u));

        (void)STATE6_REDUCE_SPEED(reduction_enabled);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        STATE6_MEASUREMENT_RETRY();
        return;
    }
    if(STATE6_READ_COMPLETION_BIT() != 0u)
    {
        STATE6_CLEAR_FLAGS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x12u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        WriteMediaCoroutineState6U32(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        STATE6_YIELD();
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x11u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_15B2, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
    saved_requested_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    SelectMediaCoroutineState6Direction((u8)(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0x08u) != 0u) && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, saved_requested_code);

    if((ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_0FF5) & 0x1000u) == 0u)
        WriteMediaCoroutineState6U32(FW_NEAR_UNKNOWN_0FE9, ReadMediaCoroutineState6U32(FW_NEAR_UNKNOWN_114F));
    else if(FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
        WriteMediaCoroutineState6U32(FW_NEAR_UNKNOWN_0FE9, ReadMediaCoroutineState6U32(FW_NEAR_UNKNOWN_FB8C) + ReadMediaCoroutineState6U32(FW_NEAR_UNKNOWN_FB90) + 0x0a82ul);
    else
        WriteMediaCoroutineState6U32(FW_NEAR_UNKNOWN_0FE9, 0x00034bc0ul);

    calibration_status = STATE6_CALIBRATE();
    if(calibration_status == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0xf7u));
        STATE6_CLEAR_FLAGS();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        WriteMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAD1, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u)
            WriteMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAD1, ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FACF));

        calibration_status = STATE6_ACQUIRE_POSITION();
        SelectMediaCoroutineState6Direction(1u);
        if(calibration_status == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x12u);
            WriteMediaCoroutineState6U16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_0FF5) | 0x0040u));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) != 0u && (ReadMediaCoroutineState6U16(FW_NEAR_UNKNOWN_FAAC) & 2u) != 0u)
                STATE6_APPLY_OFFSET();
            STATE6_YIELD();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            else
            {
                WriteMediaCoroutineState6U32(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            }
        }
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) == 3u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) == 4u)
        return;

    ResetMediaCoroutineState6FailurePath();
    STATE6_YIELD();
}
