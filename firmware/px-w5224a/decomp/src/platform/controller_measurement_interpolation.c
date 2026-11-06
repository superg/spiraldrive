#include "controller_measurement_interpolation.h"
#include "firmware_memory.h"

static u32 ReadInterpolationDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteInterpolationFloat(fw_addr_t address, float value)
{
    union
    {
        float value;
        u32 bits;
    } converted;

    converted.value = value;
    FirmwareWrite8(address, (u8)(converted.bits >> 24));
    FirmwareWrite8(address + 1u, (u8)(converted.bits >> 16));
    FirmwareWrite8(address + 2u, (u8)(converted.bits >> 8));
    FirmwareWrite8(address + 3u, (u8)converted.bits);
}

/* RAM:f95fbf */
void PLEXTOR_NEAR_FUNCTION SolveControllerMeasurementInterpolationAtSelectedInterval(void)
{
    u32 sum_x_squared_bits = ReadInterpolationDword(FW_NEAR_UNKNOWN_12E6);
    u32 first_sum_x_bits = ReadInterpolationDword(FW_NEAR_UNKNOWN_12EA);
    u32 sum_xy_bits = ReadInterpolationDword(FW_NEAR_UNKNOWN_12F2);
    u32 second_sum_x_bits = ReadInterpolationDword(FW_NEAR_UNKNOWN_12EA);
    u32 count = FirmwareReadNear8(FW_NEAR_UNKNOWN_12F6);
    u32 sum_y_bits = ReadInterpolationDword(FW_NEAR_UNKNOWN_12EE);
    float denominator;
    float numerator;
    float coefficient;

    if(second_sum_x_bits * first_sum_x_bits == count * sum_x_squared_bits)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 2u);
        return;
    }

    denominator = (float)(s32)count * (float)(s32)sum_x_squared_bits;
    denominator = denominator / (float)(s32)second_sum_x_bits;
    denominator = denominator - (float)(s32)first_sum_x_bits;

    numerator = (float)(s32)sum_y_bits * (float)(s32)sum_x_squared_bits;
    numerator = numerator / (float)(s32)second_sum_x_bits;
    numerator = numerator - (float)(s32)sum_xy_bits;
    coefficient = numerator / denominator;
    WriteInterpolationFloat(FW_NEAR_UNKNOWN_12E2, coefficient);

    numerator = (float)(s32)first_sum_x_bits * coefficient;
    numerator = (float)(s32)sum_xy_bits - numerator;
    coefficient = numerator / (float)(s32)sum_x_squared_bits;
    WriteInterpolationFloat(FW_NEAR_UNKNOWN_12DE, coefficient);
}
