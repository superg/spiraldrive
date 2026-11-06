#include "radial_position_table.h"
#include "controller_fixed_profile_generation.h"
#include "firmware_memory.h"
#include "frame_radial_float.h"
#include "radial_model.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/radial_position_table_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRadialTableU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRadialTableU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRadialTableU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fce645 */
void RebuildRadialPositionTableAndThresholds(void)
{
    volatile float radial_value;
    volatile float scaled_value;
    volatile float numerator_source;
    volatile float numerator;
    volatile float threshold_value;
    volatile float table_scale;
    s32 frame_endpoint;
    u8 index;

    radial_value = MapFramePositionToRadialModelFloat(306000l);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u && (s32)ReadRadialTableU32(FW_NEAR_UNKNOWN_12C1) >= 306000l)
        radial_value = MapFramePositionToRadialModelFloat((s32)ReadRadialTableU32(FW_NEAR_UNKNOWN_12C1));

    scaled_value = radial_value * 10.0f;
    WriteRadialTableU16(FW_NEAR_UNKNOWN_105A, (u16)(s32)scaled_value);

    numerator_source = 7056.0f;
    numerator = numerator_source * 500.0f;
    threshold_value = numerator / scaled_value;
    WriteRadialTableU16(FW_NEAR_UNKNOWN_0CE9, (u16)(s32)threshold_value);

    scaled_value = radial_value * 24.0f;
    numerator_source = 28224.0f;
    numerator = numerator_source * 500.0f;
    threshold_value = numerator / scaled_value;
    WriteRadialTableU16(FW_NEAR_UNKNOWN_0CEB, (u16)(s32)threshold_value);

    scaled_value = radial_value * 32.2f;
    numerator_source = 28224.0f;
    numerator = numerator_source * 500.0f;
    threshold_value = numerator / scaled_value;
    WriteRadialTableU16(FW_NEAR_UNKNOWN_0CED, (u16)(s32)threshold_value);

    frame_endpoint = (s32)ReadRadialTableU32(FW_NEAR_UNKNOWN_12C1);
    if(frame_endpoint >= 0l && (frame_endpoint < 382500l || (FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u))
        scaled_value = radial_value * 40.2f;
    else
        scaled_value = radial_value * 38.0f;
    numerator_source = 28224.0f;
    numerator = numerator_source * 500.0f;
    threshold_value = numerator / scaled_value;
    WriteRadialTableU16(FW_NEAR_UNKNOWN_0CEF, (u16)(s32)threshold_value);

    RecalculateRadialOperatingThresholdState();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA05, 0x34u);
        BuildFixedAndGeneratedControllerRegisterProfiles(ReadRadialTableU16(FW_NEAR_UNKNOWN_F9FE));
    }

    table_scale = (float)ReadRadialTableU32(FW_NEAR_UNKNOWN_0CF5);
    table_scale = table_scale / 65.536f;
    for(index = 0u; index < 160u; ++index)
    {
        volatile float table_value;
        volatile float span_value;

        table_value = (float)index;
        table_value = table_value * 6.0f;
        table_value = table_value * table_scale;
        span_value = (float)ReadRadialTableU16(FW_NEAR_UNKNOWN_0CF9);
        table_value = table_value * span_value;
        table_value = table_value / 1000.0f;
        table_value = table_value / 3.1415927f;
        table_value = table_value + 62500.0f;
        table_value = (float)RADIAL_TABLE_SQRT((double)table_value);
        table_value = table_value / 10.0f;
        table_value = table_value * 1000.0f;
        table_value = table_value / 11.72f;
        WriteRadialTableU16(FW_NEAR_UNKNOWN_0CFB + (fw_addr_t)((u16)index * 2u), (u16)(s32)table_value);
    }
}
