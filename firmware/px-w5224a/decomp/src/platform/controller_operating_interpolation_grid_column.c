#include "controller_operating_interpolation_grid_column.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_interpolation_grid_column_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PROFILE_SOURCE_BASE PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F8372D)
#define PROFILE_ROW_SIZE 24u
#define PROFILE_RANGE_STRIDE 240u
#define GENERATED_PROFILE_BASE FW_NEAR_UNKNOWN_E806
#define GENERATED_PROFILE_SIZE 34u

static void StoreInterpolationFloat(fw_addr_t address, float value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_operating_interpolation_grid_column_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

/* RAM:f9e142 */
/* The native range selector starts at zero and the signed <= 0 guard exits
 * after its first increment. Cases one and two are present but unreachable. */
void PLEXTOR_NEAR_FUNCTION BuildControllerOperatingInterpolationGridColumn(void)
{
    fw_addr_t low_source;
    fw_addr_t high_source;
    float low_value;
    float high_value;
    float slope;
    float intercept;
    u8 operating_code;

    low_source = PROFILE_SOURCE_BASE + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F) * PROFILE_ROW_SIZE);
    high_source = PROFILE_SOURCE_BASE + PROFILE_RANGE_STRIDE + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F) * PROFILE_ROW_SIZE);

    low_value = (float)FirmwareRead8(low_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    high_value = (float)FirmwareRead8(high_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    slope = (high_value - low_value) / (float)(0x18u - 0x14u);
    intercept = low_value - slope * (float)0x14u;
    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12DE, slope);
    StoreInterpolationFloat(FW_NEAR_UNKNOWN_12E2, intercept);

    for(operating_code = 0x14u; operating_code <= 0x18u; ++operating_code)
    {
        fw_addr_t destination = GENERATED_PROFILE_BASE + (fw_addr_t)((u16)(operating_code - 0x14u) * GENERATED_PROFILE_SIZE) + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057);
        float value = slope * (float)operating_code;
        s8 byte_value;

        value = value + intercept;
        byte_value = (s8)(u8)(s32)FLOOR_GRID_VALUE(value);
        if(byte_value < 0)
            FirmwareWrite8(destination, 0u);
        else if(byte_value <= 31)
            FirmwareWrite8(destination, (u8)byte_value);
        else
            FirmwareWrite8(destination, 31u);
    }
}
