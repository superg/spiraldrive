#include "controller_measurement_mode6_series05.h"
#include "arithmetic.h"
#include "firmware_memory.h"

extern u16 PLEXTOR_NEAR_FUNCTION SelectMode6MeasurementThresholdFromProfile(void);
extern u8 PLEXTOR_NEAR_FUNCTION ValidateControllerMeasurementSeriesOrderingAtIndex(u8 series_index);
extern void PLEXTOR_NEAR_FUNCTION SolveControllerMeasurementInterpolationAtSelectedInterval(void);
extern void PLEXTOR_NEAR_FUNCTION SelectControllerMeasurementSeries00AverageOffset(void);

static u16 ReadMode6Word(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadMode6Dword(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static float ReadMode6Float(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ReadMode6Dword(address);
    return converted.value;
}

static void WriteMode6Word(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteMode6Dword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void AccumulateMode6Regression(fw_addr_t independent_series)
{
    u8 index;

    WriteMode6Dword(FW_NEAR_UNKNOWN_12F2, 0ul);
    WriteMode6Dword(FW_NEAR_UNKNOWN_12EE, 0ul);
    WriteMode6Dword(FW_NEAR_UNKNOWN_12EA, 0ul);
    WriteMode6Dword(FW_NEAR_UNKNOWN_12E6, 0ul);

    for(index = 2u; index <= 13u; ++index)
    {
        fw_addr_t offset = (fw_addr_t)((u16)index * 2u);
        u32 independent = ReadMode6Word(independent_series + offset);
        u32 dependent = ReadMode6Word(FW_NEAR_UNKNOWN_F01F + offset);

        WriteMode6Dword(FW_NEAR_UNKNOWN_12E6, ReadMode6Dword(FW_NEAR_UNKNOWN_12E6) + MultiplyU32Low(independent, independent));
        WriteMode6Dword(FW_NEAR_UNKNOWN_12F2, ReadMode6Dword(FW_NEAR_UNKNOWN_12F2) + MultiplyU32Low(independent, dependent));
        WriteMode6Dword(FW_NEAR_UNKNOWN_12EA, ReadMode6Dword(FW_NEAR_UNKNOWN_12EA) + independent);
        WriteMode6Dword(FW_NEAR_UNKNOWN_12EE, ReadMode6Dword(FW_NEAR_UNKNOWN_12EE) + dependent);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12F6, 12u);
}

/* RAM:f97a1b */
void PLEXTOR_NEAR_FUNCTION DeriveMode6ControllerMeasurementStateFromSeries05(void)
{
    u16 threshold = SelectMode6MeasurementThresholdFromProfile();
    u8 series_index = 13u;
    s32 predicted;
    u16 result;
    u16 limit;

    while((s16)ReadMode6Word(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u)) <= (s16)threshold && series_index != 0u)
        --series_index;

    if(ValidateControllerMeasurementSeriesOrderingAtIndex(series_index) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 2u);
        return;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05D, 13u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F05E, 2u);
    AccumulateMode6Regression(FW_NEAR_UNKNOWN_EF89);
    SolveControllerMeasurementInterpolationAtSelectedInterval();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) == 2u)
        return;

    predicted = (s32)(((float)(s16)threshold - ReadMode6Float(FW_NEAR_UNKNOWN_12E2)) / ReadMode6Float(FW_NEAR_UNKNOWN_12DE));
    result = (u16)predicted;
    WriteMode6Word(FW_NEAR_UNKNOWN_EF85, result);
    if(result > 126u)
        WriteMode6Word(FW_NEAR_UNKNOWN_EF85, 126u);

    AccumulateMode6Regression(FW_NEAR_UNKNOWN_EFA7);
    SolveControllerMeasurementInterpolationAtSelectedInterval();
    predicted = (s32)(((float)(s16)threshold - ReadMode6Float(FW_NEAR_UNKNOWN_12E2)) / ReadMode6Float(FW_NEAR_UNKNOWN_12DE));
    result = (u16)predicted;
    WriteMode6Word(FW_NEAR_UNKNOWN_EF87, result);
    limit = (u16)(ReadMode6Word(FW_NEAR_UNKNOWN_F074) + 0x1100u);
    if(result > limit)
        WriteMode6Word(FW_NEAR_UNKNOWN_EF87, limit);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 1u);
    SelectControllerMeasurementSeries00AverageOffset();
}
