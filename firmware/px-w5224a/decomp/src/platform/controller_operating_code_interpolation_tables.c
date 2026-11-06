#include "controller_operating_code_interpolation_tables.h"
#include "../media/controller_measurement_dispatch.h"
#include "firmware_memory.h"
#include "operating_code_measurement_bounds.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_code_interpolation_tables_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadInterpolationU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteInterpolationU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void StoreInterpolationFloat(fw_addr_t address, float value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_operating_code_interpolation_tables_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static float ComputeMeasurementEndpoint(void)
{
    s16 position_delta = (s16)(ReadInterpolationU16(FW_NEAR_UNKNOWN_EF87) - ReadInterpolationU16(FW_NEAR_UNKNOWN_F074));
    double value = (double)position_delta * 0.0043;

    value = value - 4.6592;
    return (float)value;
}

static float ComputeAdjustmentEndpoint(void)
{
    s16 position_delta = (s16)(ReadInterpolationU16(FW_NEAR_UNKNOWN_EF87) - ReadInterpolationU16(FW_NEAR_UNKNOWN_F074));
    double value = (double)position_delta * 0.0089;
    s16 reference_offset;

    value = value + 14.3;
    value = value * 0.01;
    reference_offset = (s16)(ReadInterpolationU16(FW_NEAR_UNKNOWN_EF85) - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A));
    value = value * (double)reference_offset;
    position_delta = (s16)(ReadInterpolationU16(FW_NEAR_UNKNOWN_EF87) - ReadInterpolationU16(FW_NEAR_UNKNOWN_F074));
    value = value + (double)position_delta * 0.001;
    value = value - 12.0;
    return (float)value;
}

static float ScaleEndpoint(float value, fw_addr_t table_base)
{
    s16 scale = (s16)ReadInterpolationU16(table_base + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F) * 4u));

    value = value * (float)scale;
    return value / 100.0f;
}

static fw_addr_t InterpolationDestination(fw_addr_t base, u8 operating_code)
{
    u8 index = operating_code;

    if(index >= 0x14u)
        index = (u8)(index - 0x14u);
    return base + (fw_addr_t)((u16)index * 2u);
}

static float BuildMeasurementTable(float low_value, float high_value, u8 low_code, u8 high_code)
{
    float low_code_float = (float)low_code;
    float high_code_float = (float)high_code;
    float slope = (high_value - low_value) / (high_code_float - low_code_float);
    float intercept = low_value - slope * low_code_float;
    float interpolated = low_value;
    u8 operating_code;

    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12DE, slope);
    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12E2, intercept);
    for(operating_code = low_code; operating_code <= high_code; ++operating_code)
    {
        interpolated = slope * (float)operating_code;
        interpolated = interpolated + intercept;
        WriteInterpolationU16(InterpolationDestination(FW_NEAR_UNKNOWN_F187, operating_code), UPDATE_MEASUREMENT_BOUND(interpolated));
    }
    return interpolated;
}

static void BuildAdjustmentTable(float low_value, float high_value, u8 low_code, u8 high_code)
{
    float low_code_float = (float)low_code;
    float high_code_float = (float)high_code;
    float slope = (high_value - low_value) / (high_code_float - low_code_float);
    float intercept = low_value - slope * low_code_float;
    u8 operating_code;

    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12DE, slope);
    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12E2, intercept);
    for(operating_code = low_code; operating_code <= high_code; ++operating_code)
    {
        float interpolated = slope * (float)operating_code;
        fw_addr_t reference_address = InterpolationDestination(FW_NEAR_UNKNOWN_F187, operating_code);

        interpolated = interpolated + intercept;
        WriteInterpolationU16(InterpolationDestination(FW_NEAR_UNKNOWN_F157, operating_code), COMPUTE_ADJUSTMENT(interpolated, ReadInterpolationU16(reference_address)));
    }
}

/* RAM:f9d777 */
/* The canonical 4..10 route reads its first float32 low endpoint from an
 * automatic object which it never initializes. Keeping this function
 * reentrant and leaving that volatile object uninitialized preserves the
 * firmware's native stack-residue contract instead of inventing a value.
 * The second table then intentionally reuses the last interpolated float from
 * the first table as its low endpoint. */
void PLEXTOR_NEAR_FUNCTION BuildControllerOperatingCodeInterpolationTables(void) PLEXTOR_REENTRANT
{
    u8 low_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_1058);
    u8 high_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_1059);
    volatile float low_value;
    volatile float high_value;

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_operating_code_interpolation_tables_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(high_code == 10u)
        high_value = ComputeMeasurementEndpoint();
    else if(high_code == 0x18u)
        high_value = ScaleEndpoint(ComputeMeasurementEndpoint(), PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83937));
    if(low_code == 0x14u)
        low_value = ScaleEndpoint(ComputeMeasurementEndpoint(), PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8390F));
    low_value = BuildMeasurementTable(low_value, high_value, low_code, high_code);

    if(high_code == 10u)
        high_value = ComputeAdjustmentEndpoint();
    else if(high_code == 0x18u)
        high_value = ScaleEndpoint(ComputeAdjustmentEndpoint(), PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83935));
    if(low_code == 0x14u)
        low_value = ScaleEndpoint(ComputeAdjustmentEndpoint(), PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8390D));
    BuildAdjustmentTable(low_value, high_value, low_code, high_code);
}
