#include "controller_measurement_mode6_transform.h"
#include "controller_measurement_mode6_series05.h"
#include "controller_measurement_series04_value.h"
#include "controller_measurement_series05.h"
#include "firmware_memory.h"

static u16 ReadMode6TransformWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteMode6TransformWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static float ComputeMode6TransformCoordinate(u8 series_index)
{
    fw_addr_t offset = (fw_addr_t)((u16)series_index * 2u);
    u16 reference_delta = (u16)(ReadMode6TransformWord(FW_NEAR_UNKNOWN_EFA7 + offset) - ReadMode6TransformWord(FW_NEAR_UNKNOWN_F074));
    double scale = ((double)reference_delta * 0.0089 + 14.3) * 0.01;
    u16 measurement_delta = (u16)(ReadMode6TransformWord(FW_NEAR_UNKNOWN_EF89 + offset) - FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A));
    double coordinate = (double)measurement_delta * scale;

    reference_delta = (u16)(ReadMode6TransformWord(FW_NEAR_UNKNOWN_EFA7 + offset) - ReadMode6TransformWord(FW_NEAR_UNKNOWN_F074));
    coordinate = coordinate + (double)reference_delta * 0.001;
    coordinate = coordinate - 12.0;
    return (float)coordinate;
}

static s16 TransformMode6Series04Value(u8 series_index)
{
    fw_addr_t offset = (fw_addr_t)((u16)series_index * 2u);
    float gradient = ComputeControllerMeasurementSeries04ValueAtIndex(series_index);
    float coordinate = ComputeMode6TransformCoordinate(series_index);
    u16 denominator_bits = ReadMode6TransformWord(FW_NEAR_UNKNOWN_F001 + offset);
    s32 result;

    if(denominator_bits == 0u)
        return 0x7fff;

    coordinate = gradient * coordinate;
    coordinate = coordinate / (float)(s16)ReadMode6TransformWord(FW_NEAR_UNKNOWN_F001 + offset);
    coordinate = coordinate * 1000.0f;
    result = (s32)coordinate;
    if(result > 0x7fffL)
        return 0x7fff;
    if(result < -0x8000L)
        return (s16)-0x8000L;
    return (s16)result;
}

/* Shared C-only reconstruction helper; no canonical firmware address. */
void PLEXTOR_NEAR_FUNCTION TransformControllerMeasurementSeries04To05AndDerive(u8 mode6)
{
    s16 snapshot[15];
    u8 series_index;

    for(series_index = 0u; series_index < 15u; ++series_index)
        WriteMode6TransformWord(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u), (u16)TransformMode6Series04Value(series_index));

    for(series_index = 0u; series_index < 15u; ++series_index)
        snapshot[series_index] = (s16)ReadMode6TransformWord(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u));

    for(series_index = 1u; series_index < 14u; ++series_index)
    {
        s32 average = ((s32)snapshot[series_index - 1u] + (s32)snapshot[series_index] + (s32)snapshot[series_index + 1u]) / 3L;

        if(average < -0x8000L)
            average = -0x8000L;
        else if(average > 0x7fffL)
            average = 0x7fffL;
        WriteMode6TransformWord(FW_NEAR_UNKNOWN_F01F + (fw_addr_t)((u16)series_index * 2u), (u16)(s16)average);
    }

    WriteMode6TransformWord(FW_NEAR_UNKNOWN_F01F, 0x7fffu);
    WriteMode6TransformWord(FW_NEAR_UNKNOWN_F03B, 0x8000u);
    if(mode6 != 0u)
        DeriveMode6ControllerMeasurementStateFromSeries05();
    else
        DeriveControllerMeasurementStateFromSeries05();
}

/* RAM:f96a65 */
void PLEXTOR_NEAR_FUNCTION TransformControllerMeasurementSeries04To05ForMode6(void)
{
    TransformControllerMeasurementSeries04To05AndDerive(1u);
}
