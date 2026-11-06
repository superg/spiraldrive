#include "controller_measurement_series00_01.h"
#include "controller_measurement_interpolation.h"
#include "controller_measurement_mode2_adjustment.h"
#include "firmware_memory.h"
#include "status_mapping.h"

static u16 ReadSeriesEvaluationWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadSeriesEvaluationDword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static float ReadSeriesEvaluationFloat(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ReadSeriesEvaluationDword(address);
    return converted.value;
}

static void WriteSeriesEvaluationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteSeriesEvaluationDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 IsControllerMeasurementSeriesConstant(fw_addr_t series_address)
{
    u8 index;

    for(index = 1u; index < 15u; ++index)
    {
        fw_addr_t offset = (fw_addr_t)((u16)index * 2u);

        if(ReadSeriesEvaluationWord(series_address + offset) != ReadSeriesEvaluationWord(series_address))
            return 0u;
    }
    return 1u;
}

static void SmoothControllerMeasurementSeries06(void)
{
    s16 snapshot[15];
    u8 index;

    for(index = 0u; index < 15u; ++index)
        snapshot[index] = (s16)ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_F03F + (fw_addr_t)((u16)index * 2u));

    for(index = 1u; index < 14u; ++index)
    {
        s32 average = ((s32)snapshot[index - 1u] + (s32)snapshot[index] + (s32)snapshot[index + 1u]) / 3L;

        if(average < -0x8000L)
            average = -0x8000L;
        else if(average > 0x7fffL)
            average = 0x7fffL;
        WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_F03F + (fw_addr_t)((u16)index * 2u), (u16)(s16)average);
    }
}

static void AccumulateControllerMeasurementRegression(fw_addr_t series_address)
{
    u8 index;

    WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12F2, 0ul);
    WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EE, 0ul);
    WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EA, 0ul);
    WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12E6, 0ul);

    index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05E);
    while(index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_F05D))
    {
        fw_addr_t offset = (fw_addr_t)((u16)index * 2u);
        u16 first_value = ReadSeriesEvaluationWord(series_address + offset);
        u16 second_value = ReadSeriesEvaluationWord(series_address + offset);
        u32 product = (u32)first_value * (u32)second_value;
        u16 dependent_value;

        WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12E6, ReadSeriesEvaluationDword(FW_NEAR_UNKNOWN_12E6) + product);
        dependent_value = ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_F03F + offset);
        product = (u32)dependent_value * (u32)ReadSeriesEvaluationWord(series_address + offset);
        WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12F2, ReadSeriesEvaluationDword(FW_NEAR_UNKNOWN_12F2) + product);
        WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EA, ReadSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EA) + ReadSeriesEvaluationWord(series_address + offset));
        WriteSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EE, ReadSeriesEvaluationDword(FW_NEAR_UNKNOWN_12EE) + ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_F03F + offset));
        ++index;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F05D) - FirmwareReadNear8(FW_NEAR_UNKNOWN_F05E) + 1u));
    SolveControllerMeasurementInterpolationAtSelectedInterval();
}

static u16 EvaluateControllerMeasurementRegressionResult(void)
{
    fw_addr_t profile_address = FirmwareAddressIncrementLow16(ReadSeriesEvaluationDword(FW_NEAR_UNKNOWN_F1BB), 8u);
    float result = (float)(s8)FirmwareRead8(profile_address);

    result = result - ReadSeriesEvaluationFloat(FW_NEAR_UNKNOWN_12E2);
    result = result / ReadSeriesEvaluationFloat(FW_NEAR_UNKNOWN_12DE);
    return (u16)(s32)result;
}

/* RAM:f95ce3 */
void PLEXTOR_NEAR_FUNCTION EvaluateControllerMeasurementSeries00And01(void)
{
    u8 series00_constant = IsControllerMeasurementSeriesConstant(FW_NEAR_UNKNOWN_EF89);
    u16 result;

    SetControllerMeasurementTerminalStatus0E02();
    SmoothControllerMeasurementSeries06();

    if(series00_constant == 0u)
    {
        AccumulateControllerMeasurementRegression(FW_NEAR_UNKNOWN_EF89);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) == 2u)
            return;
        result = EvaluateControllerMeasurementRegressionResult();
        WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF85, result);
        if((s16)ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF85) > 126)
            WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF85, 126u);
    }
    else
    {
        WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF85, ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF89));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 1u);
    }

    if(IsControllerMeasurementSeriesConstant(FW_NEAR_UNKNOWN_EFA7) != 0u)
    {
        WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF87, ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_EFA7));
        return;
    }

    AccumulateControllerMeasurementRegression(FW_NEAR_UNKNOWN_EFA7);
    WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF87, EvaluateControllerMeasurementRegressionResult());
    ClassifyAndAdjustControllerMeasurementForMode2();
    SelectControllerMeasurementSeries00AverageOffset();
    result = (u16)(ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_F074) + 0x1100u);
    if(ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF87) > result)
        WriteSeriesEvaluationWord(FW_NEAR_UNKNOWN_EF87, (u16)(ReadSeriesEvaluationWord(FW_NEAR_UNKNOWN_F074) + 0x1100u));
}
