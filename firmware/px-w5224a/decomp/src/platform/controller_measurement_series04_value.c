#include "controller_measurement_series04_value.h"
#include "firmware_memory.h"

static u16 ReadSeries04Word(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadSeries04Dword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static float ComputeSeries04Scale(void)
{
    u16 reference_delta = (u16)(ReadSeries04Word(FW_NEAR_UNKNOWN_EFA7) - ReadSeries04Word(FW_NEAR_UNKNOWN_F074));
    fw_addr_t profile_address = ReadSeries04Dword(FW_NEAR_UNKNOWN_F1BB);
    float base_scale = (float)(((double)reference_delta * 0.0089 + 14.3) * 0.01);

    return base_scale * (float)(s8)FirmwareRead8(profile_address + 3u);
}

/* RAM:f972a6 */
float PLEXTOR_NEAR_FUNCTION ComputeControllerMeasurementSeries04ValueAtIndex(u8 series_index)
{
    float scale = ComputeSeries04Scale();
    float result;

    if(series_index == 0u)
    {
        result = ((float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F005) * -1.0f) / 2.0f;
        result = result + (float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F003) * 2.0f;
        result = result + ((float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F001) * -3.0f) / 2.0f;
    }
    else if(series_index == 14u)
    {
        result = ((float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F01D) * 3.0f) / 2.0f;
        result = result + (float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F01B) * -2.0f;
        result = result + (float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F019) / 2.0f;
    }
    else
    {
        fw_addr_t offset = (fw_addr_t)((u16)series_index * 2u);

        result = ((float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_EFFF + offset) * -1.0f) / 2.0f;
        result = result + (float)(s16)ReadSeries04Word(FW_NEAR_UNKNOWN_F003 + offset) / 2.0f;
    }
    return result / scale;
}
