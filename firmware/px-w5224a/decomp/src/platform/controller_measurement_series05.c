#include "controller_measurement_series05.h"
#include "../media/controller_measurement_dispatch.h"
#include "controller_measurement_capture.h"
#include "firmware_memory.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_measurement_series05_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadSeries05Word(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadSeries05Dword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSeries05Word(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteSeries05Dword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void WriteSeries05Float(fw_addr_t address, float value)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.value = value;
    WriteSeries05Dword(address, converted.bits);
}

static u16 Series05FloatHighWord(float value)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.value = value;
    return (u16)(converted.bits >> 16);
}

static float ComputeSeries05Coordinate(u8 series_index)
{
    fw_addr_t offset = (fw_addr_t)((u16)series_index * 2u);
    u16 reference_delta = (u16)(ReadSeries05Word(FW_NEAR_UNKNOWN_EFA7 + offset) - ReadSeries05Word(FW_NEAR_UNKNOWN_F074));
    u16 measurement_delta = (u16)(ReadSeries05Word(FW_NEAR_UNKNOWN_EF89 + offset) - FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A));
    double scale = ((double)reference_delta * 0.0089 + 14.3) * 0.01;

    return (float)((double)measurement_delta * scale + (double)reference_delta * 0.001 - 12.0);
}

static float ComputeSeries05AdjustedValue(float scaled_position)
{
    fw_addr_t profile_address = ReadSeries05Dword(FW_NEAR_UNKNOWN_F1BB);
    s16 profile_scale = (s16)ReadSeries05Word(profile_address + 4u);
    float product = scaled_position * (float)profile_scale;

    return (float)((double)product * 0.001);
}

static void PublishSeries05ScaledResult(float position, u16 scale_percent)
{
    float scaled_position = (position * (float)scale_percent) / 100.0f;
    float adjusted_value = ComputeSeries05AdjustedValue(scaled_position);
    s32 rounded_offset = (s32)CONTROLLER_MEASUREMENT_CEIL(((double)adjusted_value + 4.6592) / 0.0043);
    u16 reference_base = ReadSeries05Word(FW_NEAR_UNKNOWN_F074);

    WriteSeries05Word(FW_NEAR_UNKNOWN_F060, (u16)(s32)(position * 100.0f));
    WriteSeries05Word(FW_NEAR_UNKNOWN_F062, (u16)(s32)(scaled_position * 100.0f));
    WriteSeries05Word(FW_NEAR_UNKNOWN_F064, (u16)(s32)(adjusted_value * 100.0f));
    WriteSeries05Float(FW_NEAR_UNKNOWN_F06E, scaled_position);
    WriteSeries05Word(FW_NEAR_UNKNOWN_EF87, (u16)(reference_base + (u16)rounded_offset));
}

/* RAM:f9745b */
void PLEXTOR_NEAR_FUNCTION DeriveControllerMeasurementStateFromSeries05(void)
{
    u16 threshold = SelectMeasurementDurationFromOperatingCode();
    u8 series_index = 13u;
    float coordinate;
    float next_coordinate;
    float slope;
    float intercept;
    float position;
    u16 scale_percent;
    u16 output_limit;

    while((s16)ReadSeries05Word(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u)) <= (s16)threshold && series_index != 0u)
        --series_index;

    if(ValidateControllerMeasurementSeriesOrderingAtIndex(series_index) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 2u);
        return;
    }

    coordinate = ComputeSeries05Coordinate(series_index);
    next_coordinate = ComputeSeries05Coordinate((u8)(series_index + 1u));
    slope = (float)(s16)(u16)(ReadSeries05Word(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u)) - ReadSeries05Word(FW_NEAR_UNKNOWN_F021 + (fw_addr_t)((u16)series_index * 2u)))
          / (coordinate - next_coordinate);
    intercept = (float)(s16)ReadSeries05Word(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u)) - slope * coordinate;
    WriteSeries05Float(FW_NEAR_UNKNOWN_12DE, slope);
    WriteSeries05Float(FW_NEAR_UNKNOWN_12E2, intercept);
    position = ((float)(s16)threshold - intercept) / slope;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0xd0u
        && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) != 0xe0u)
    {
        u16 table_value = ReadSeries05Word(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83070) + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F) * 2u));

        position = (position + (float)(table_value / 10u)) / 2.0f;
    }

    scale_percent = SelectControllerMeasurementScalePercent(Series05FloatHighWord(position));
    PublishSeries05ScaledResult(position, scale_percent);
    output_limit = (u16)(ReadSeries05Word(FW_NEAR_UNKNOWN_F074) + 0x1100u);
    if(ReadSeries05Word(FW_NEAR_UNKNOWN_EF87) > output_limit)
        WriteSeries05Word(FW_NEAR_UNKNOWN_EF87, output_limit);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 0u && ReadSeries05Word(FW_NEAR_UNKNOWN_EF87) > (u16)(ReadSeries05Word(FW_NEAR_UNKNOWN_F072) - 0x23u))
    {
        scale_percent = 105u;
        PublishSeries05ScaledResult(position, scale_percent);
    }

    WriteSeries05Word(FW_NEAR_UNKNOWN_EF85, ComputeBoundedControllerAdjustmentCode((position * (float)scale_percent) / 100.0f, ReadSeries05Word(FW_NEAR_UNKNOWN_EF87)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 1u);
}
