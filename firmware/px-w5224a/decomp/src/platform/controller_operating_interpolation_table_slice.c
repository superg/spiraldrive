#include "controller_operating_interpolation_table_slice.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_interpolation_table_slice_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define GENERATED_PROFILE_FIELD_BASE FW_NEAR_UNKNOWN_E823
#define GENERATED_PROFILE_SIZE 34u

static void StoreSliceFloat(fw_addr_t address, float value)
{
#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_operating_interpolation_table_slice_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static fw_addr_t SelectLowSource(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1058) == 4u)
        return PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83451);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1058) == 0x14u)
        return PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83461);
    return 0ul;
}

static fw_addr_t SelectHighSource(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1059) == 0x0au)
        return PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83459);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1059) == 0x18u)
        return PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83465);
    return 0ul;
}

static fw_addr_t SliceDestination(u8 operating_code)
{
    u8 record_index = operating_code;

    if(record_index >= 0x14u)
        record_index = (u8)(record_index - 0x14u);
    return GENERATED_PROFILE_FIELD_BASE + (fw_addr_t)((u16)record_index * GENERATED_PROFILE_SIZE) + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057);
}

static void StorePackedInterpolationPart(float low_value, float high_value, float low_code, float high_code, u8 high_part)
{
    float slope = (high_value - low_value) / (high_code - low_code);
    float intercept = low_value - slope * low_code;
    u8 operating_code = (u8)(s32)low_code;

    StoreSliceFloat(FW_NEAR_UNKNOWN_12DE, slope);
    StoreSliceFloat(FW_NEAR_UNKNOWN_12E2, intercept);
    while((float)operating_code <= high_code)
    {
        fw_addr_t destination = SliceDestination(operating_code);
        float value = slope * (float)operating_code;
        u8 byte_value;

        value = value + intercept;
        byte_value = (u8)(s32)FLOOR_SLICE_VALUE(value);
        if(high_part != 0u)
            FirmwareWrite8(destination, (u8)((byte_value & 7u) << 5));
        else
            FirmwareWrite8(destination, (u8)(FirmwareRead8(destination) | byte_value));
        ++operating_code;
    }
}

/* The only canonical callers supply either 4..10 or 0x14..0x18. The native
 * function leaves its source pointers uninitialized for every other pair. */
/* RAM:f9d49d */
void PLEXTOR_NEAR_FUNCTION BuildControllerOperatingInterpolationTableSlice(void)
{
    fw_addr_t low_source = SelectLowSource();
    fw_addr_t high_source = SelectHighSource();
    float low_code;
    float high_code;
    u8 low_byte;
    u8 high_byte;

    if(low_source == 0ul || high_source == 0ul)
        return;

    low_byte = FirmwareRead8(low_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    high_byte = FirmwareRead8(high_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    low_code = (float)FirmwareReadNear8(FW_NEAR_UNKNOWN_1058);
    high_code = (float)FirmwareReadNear8(FW_NEAR_UNKNOWN_1059);
    StorePackedInterpolationPart((float)(low_byte >> 5), (float)(high_byte >> 5), low_code, high_code, 1u);

    low_byte = FirmwareRead8(low_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    high_byte = FirmwareRead8(high_source + FirmwareReadNear8(FW_NEAR_UNKNOWN_1057));
    StorePackedInterpolationPart((float)(low_byte & 0x1fu), (float)(high_byte & 0x1fu), low_code, high_code, 0u);
}
