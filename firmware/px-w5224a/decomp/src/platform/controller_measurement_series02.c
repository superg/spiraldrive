#include "controller_measurement_series02.h"
#include "firmware_memory.h"

extern s32 DivideS32Quotient(s32 dividend, s32 divisor);
extern u8 ValidateControllerMeasurementSeries02SelectionIndex(u8 series_index);

static u16 ReadSeries02Word(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteSeries02Word(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:f98099 */
void PLEXTOR_NEAR_FUNCTION SmoothAndEvaluateControllerMeasurementSeries02(void)
{
    s16 samples[15];
    s32 average;
    u16 threshold;
    u16 difference = 0u;
    u16 sample_reference = ReadSeries02Word(FW_NEAR_UNKNOWN_F9A0);
    u8 index;
    u16 selected_value;
    u16 baseline;

    for(index = 0u; index < 15u; ++index)
        samples[index] = (s16)ReadSeries02Word(FW_NEAR_UNKNOWN_EFC5 + (fw_addr_t)((u16)index * 2u));
    for(index = 1u; index < 14u; ++index)
    {
        average = DivideS32Quotient((s32)samples[index - 1u] + samples[index] + samples[index + 1u], 3l);
        if(average < -32768l)
            average = -32768l;
        else if(average > 32767l)
            average = 32767l;
        WriteSeries02Word(FW_NEAR_UNKNOWN_EFC5 + (fw_addr_t)((u16)index * 2u), (u16)(s16)average);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
        threshold = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83438));
    else
        threshold = FirmwareRead8(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83446));

    for(index = 1u; index < 15u; ++index)
    {
        u16 value = ReadSeries02Word(FW_NEAR_UNKNOWN_EFC5 + (fw_addr_t)((u16)index * 2u));

        if(value < sample_reference)
            difference = 0u;
        else
            difference = (u16)(value - sample_reference);
        if(difference > threshold)
            break;
    }

    if(index == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) - (difference > (u16)(threshold + 20u) ? 8u : 4u)));
    }
    else if(index == 15u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 3u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF77) + (difference < (u16)(threshold - 10u) ? 8u : 4u)));
    }
    else
    {
        index = (u8)(index - 1u);
        if(ValidateControllerMeasurementSeries02SelectionIndex(index) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 2u);
            return;
        }
        selected_value = ReadSeries02Word(FW_NEAR_UNKNOWN_EF89 + (fw_addr_t)((u16)index * 2u));
        WriteSeries02Word(FW_NEAR_UNKNOWN_F06B, selected_value);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
            baseline = ReadSeries02Word(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83436));
        else
            baseline = ReadSeries02Word(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83444));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F06D, (u8)(selected_value - baseline));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F06A, FirmwareReadNear8(FW_NEAR_UNKNOWN_F06D));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF78, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, 0u);
    }
}
