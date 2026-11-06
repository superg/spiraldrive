#include "controller_measurement_mode6_adjustment.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_mode6_adjustment_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadMode6AdjustmentWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadMode6AdjustmentDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static float ReadMode6AdjustmentFloat(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ReadMode6AdjustmentDword(address);
    return converted.value;
}

static void WriteMode6AdjustmentWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u16 ComputeMode6ReferenceWord(float adjusted_value)
{
    float narrowed = (float)((double)adjusted_value + 4.6592);
    s32 rounded = (s32)CONTROLLER_MEASUREMENT_CEIL((double)narrowed / 0.0043);

    return (u16)(ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_F074) + (u16)rounded);
}

static u16 ComputeMode6OutputCode(u16 reference_word, float measurement_value)
{
    u16 reference_delta = (u16)(reference_word - ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_F074));
    double value = (double)reference_delta * 0.001;
    float narrowed;
    s32 rounded;

    value = value - (double)measurement_value;
    value = value + 12.0;
    value = value / (((double)reference_delta * 0.0089 + 14.3) * 0.01);
    value = value + (double)(s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A);
    narrowed = (float)value;
    rounded = (s32)CONTROLLER_MEASUREMENT_CEIL((double)narrowed);
    return (u16)(u8)rounded;
}

static u8 IsMode6SpecialMedia(void)
{
    u8 media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88);

    return (u8)(media_state == 2u || media_state == 3u);
}

static void ApplyMode6OutOfRangeAdjustment(u8 state)
{
    u8 special_media = IsMode6SpecialMedia();
    float measurement_value = ReadMode6AdjustmentFloat(FW_NEAR_UNKNOWN_F06E);
    float adjusted_value;
    float output_measurement = measurement_value;
    u16 reference_word;

    if(special_media != 0u)
    {
        u8 adjustment_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08B) + 1u);
        u16 adjustment_word;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08B, adjustment_count);
        adjustment_word = ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_F089);
        if(adjustment_count < 2u)
        {
            if(state == 4u)
                adjustment_word = (u16)(adjustment_word + 20u);
            else
                adjustment_word = (u16)(adjustment_word - 20u);
            WriteMode6AdjustmentWord(FW_NEAR_UNKNOWN_F089, adjustment_word);
        }
        adjusted_value = measurement_value * (float)(s16)adjustment_word;
        adjusted_value = (float)((double)adjusted_value * 0.001);
        if(state == 4u)
            output_measurement = measurement_value - (float)((u16)adjustment_count * 3u);
        else
            output_measurement = measurement_value + (float)adjustment_count;
    }
    else
    {
        fw_addr_t profile_address = FirmwareAddressIncrementLow16(ReadMode6AdjustmentDword(FW_NEAR_UNKNOWN_F1BB), 4u);
        s16 profile_value = (s16)ReadMode6AdjustmentWord(profile_address);
        double scale = (double)profile_value * 0.001;

        if(state == 4u)
            scale = scale + 0.04;
        else
            scale = scale - 0.03;
        adjusted_value = (float)(scale * (double)measurement_value);
    }

    reference_word = ComputeMode6ReferenceWord(adjusted_value);
    WriteMode6AdjustmentWord(FW_NEAR_UNKNOWN_F117, reference_word);
    WriteMode6AdjustmentWord(FW_NEAR_UNKNOWN_F115, ComputeMode6OutputCode(reference_word, output_measurement));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) | 0x20u));
}

/* Shared C-only reconstruction helper; no canonical firmware address. */
void PLEXTOR_NEAR_FUNCTION ClassifyAndAdjustControllerMeasurementForMode(u8 required_mode)
{
    u16 measurement = ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_EF87);
    u8 state;

    if(measurement < ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_EFAB))
        state = 3u;
    else if(measurement > ReadMode6AdjustmentWord(FW_NEAR_UNKNOWN_EFBF))
        state = 4u;
    else
        state = 1u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, state);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == required_mode && (state == 3u || state == 4u))
        ApplyMode6OutOfRangeAdjustment(state);
}

/* RAM:f96ccb */
void PLEXTOR_NEAR_FUNCTION ClassifyAndAdjustControllerMeasurementForMode6(void)
{
    ClassifyAndAdjustControllerMeasurementForMode(6u);
}
