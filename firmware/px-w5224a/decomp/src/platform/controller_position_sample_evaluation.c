#include "controller_position_sample_evaluation.h"
#include "firmware_memory.h"
#include "high_write_speed_calibration_flags.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_position_sample_evaluation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPositionEvaluationWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WritePositionEvaluationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static s32 DividePositionEvaluationS32(s32 dividend, s32 divisor)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_position_sample_evaluation_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u16 InterpolateControllerPositionWord(u16 lower_word, u16 upper_word, s16 lower_sample, s16 upper_sample, s16 target_sample)
{
    u32 calibration_delta;
    s16 sample_delta;
    s32 slope_times_100;
    s32 numerator;

    calibration_delta = (u32)upper_word - (u32)lower_word;
    sample_delta = (s16)((u16)upper_sample - (u16)lower_sample);
    slope_times_100 = DividePositionEvaluationS32((s32)(100ul * calibration_delta), (s32)sample_delta);
    numerator = (s32)((u32)lower_word * 100ul - (u32)slope_times_100 * (u32)(s32)lower_sample);
    numerator = (s32)((u32)numerator + (u32)slope_times_100 * (u32)(s32)target_sample);
    return (u16)DividePositionEvaluationS32(numerator, 100l);
}

static void ClearPositionEvaluationWindowBit(void)
{
    WritePositionEvaluationWord(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_0FF5) & 0xffefu));
}

static u8 HasIndexedCalibrationFailure(void)
{
    u8 firmware_mode;
    u16 flags;

    if(INDEXED_STATE_ACTIVE(4u) == 0u)
        return 0u;
    firmware_mode = FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F));
    if(firmware_mode != 1u && firmware_mode != 0x11u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD4) != 0x34u)
        return 0u;
    flags = ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_FAD7);
    return (u8)((flags & 0x03e0u) != 0u);
}

/* RAM:f8a172 */
/* The original caller leaves the signed high word of target+5 in WR0.  The C
 * interface makes that otherwise hidden spacing reference explicit. */
u8 PLEXTOR_NEAR_FUNCTION EvaluateControllerPositionSamplesAndUpdateCalibration(s16 interior_reference)
{
    u8 nearest_index;
    u8 left_index;
    u8 table_index;
    u8 direction;
    u8 speed;
    u16 configuration;
    u16 lower_word;
    u16 upper_word;
    u16 accumulated;
    u16 published;
    s16 target_sample;
    s16 selected_sample;
    s16 lower_sample;
    s16 upper_sample;
    fw_addr_t table_base;

    SELECT_NEAREST_SAMPLE();
    nearest_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF0);
    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);
    configuration = ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_FAAC);
    target_sample = (s16)ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_FB5F);
    selected_sample = (s16)ReadPositionEvaluationWord((fw_addr_t)(FW_NEAR_UNKNOWN_0FDB + (u16)nearest_index * 2u));

    if((configuration & 2u) != 0u)
    {
        if((speed < 2u && nearest_index == 0u) || (speed >= 2u && nearest_index < 2u))
            return 2u;
    }
    else
    {
        if(nearest_index == 0u && (target_sample < 0 || selected_sample >= target_sample))
            return 2u;
        if(speed >= 12u && (nearest_index == 0u || (nearest_index == 1u && selected_sample >= target_sample)))
            return 2u;
    }

    if((configuration & 2u) != 0u)
    {
        if(nearest_index == 4u)
            return 3u;
    }
    else if(nearest_index == 4u && selected_sample < target_sample)
    {
        return 3u;
    }

    if(nearest_index == 0u)
        direction = 1u;
    else if(nearest_index == 4u)
        direction = 2u;
    else
        direction = selected_sample < target_sample ? 1u : 2u;

    table_index = (u8)(nearest_index + FirmwareReadNear8(FW_NEAR_UNKNOWN_F962));
    table_base = speed < 12u ? PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92EB) : PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9337);
    if(direction == 1u)
    {
        left_index = nearest_index;
        lower_word = ReadPositionEvaluationWord(table_base + (u16)table_index * 2u);
        upper_word = ReadPositionEvaluationWord(table_base + (u16)(u8)(table_index + 1u) * 2u);
        lower_sample = selected_sample;
        upper_sample = (s16)ReadPositionEvaluationWord((fw_addr_t)(FW_NEAR_UNKNOWN_0FDB + (u16)(nearest_index + 1u) * 2u));
    }
    else
    {
        left_index = (u8)(nearest_index - 1u);
        lower_word = ReadPositionEvaluationWord(table_base + (u16)(u8)(table_index - 1u) * 2u);
        upper_word = ReadPositionEvaluationWord(table_base + (u16)table_index * 2u);
        lower_sample = (s16)ReadPositionEvaluationWord((fw_addr_t)(FW_NEAR_UNKNOWN_0FDB + (u16)(nearest_index - 1u) * 2u));
        upper_sample = selected_sample;
    }

    if(upper_sample < 0 && lower_sample >= 0)
        return 3u;
    if(INTERIOR_SPACING_ACCEPTED(interior_reference) != 1u || lower_sample >= upper_sample)
    {
        ClearPositionEvaluationWindowBit();
        return 0u;
    }

    accumulated = (u16)(ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_0FD3) + InterpolateControllerPositionWord(lower_word, upper_word, lower_sample, upper_sample, target_sample));
    WritePositionEvaluationWord(FW_NEAR_UNKNOWN_0FD3, accumulated);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE1) != 1u)
    {
        WritePositionEvaluationWord(FW_NEAR_UNKNOWN_0FD7, ReadPositionEvaluationWord((fw_addr_t)(FW_NEAR_UNKNOWN_100A + (u16)left_index * 2u)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE1) + 1u));
        return 0u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 2u) != 0u && speed == 4u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 8u) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_158E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) | 8u));
            WritePositionEvaluationWord(FW_NEAR_UNKNOWN_FACF, (u16)(accumulated >> 1));
            WritePositionEvaluationWord(FW_NEAR_UNKNOWN_0FD3, 0u);
            ClearPositionEvaluationWindowBit();
            return 0u;
        }
        published = (u16)((u16)(accumulated >> 1) + ReadPositionEvaluationWord(FW_NEAR_UNKNOWN_FACF));
        published >>= 1;
    }
    else
    {
        published = (u16)(accumulated >> 1);
    }
    WritePositionEvaluationWord(FW_NEAR_UNKNOWN_FACF, published);
    UPDATE_HIGH_SPEED_FLAGS();
    return HasIndexedCalibrationFailure() != 0u ? 5u : 1u;
}
