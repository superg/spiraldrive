#include "controller_operating_interpolation.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "operating_code14_measurement_bounds.h"

extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0033Atomic(void);
extern u8 PLEXTOR_NEAR_FUNCTION ReadBit0039Atomic(void);
extern void PLEXTOR_NEAR_FUNCTION SetBit0039Atomic(void);

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_interpolation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_operating_interpolation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_operating_interpolation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/controller_operating_interpolation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadInterpolationWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static s32 ReadInterpolationDword(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WriteInterpolationWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fa6b2a */
void PLEXTOR_NEAR_FUNCTION WriteScaledOperatingProfileWordViaSfr8B8D(u8 scale)
{
    u8 profile_index;
    u16 profile_word;
    u16 reload_value;

    CLEAR_PROFILE_SCALE_TIMER_OVERFLOW();
    SET_PROFILE_SCALE_TIMER_RUN();

    profile_index = MapOperatingCodeToProfileIndex(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));
    profile_word = ReadInterpolationWord((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9212) + (u16)profile_index * 2u));
    reload_value = (u16)(0xffffu - (u16)((u16)scale * profile_word));
    FirmwareWriteDirectSfr8(8D, (u8)(reload_value >> 8));
    FirmwareWriteDirectSfr8(8B, (u8)reload_value);

    while((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_01807F) & 0x08u) != 0u && READ_PROFILE_SCALE_TIMER_OVERFLOW() == 0u)
    {
    }

    CLEAR_PROFILE_SCALE_TIMER_RUN();
    CLEAR_PROFILE_SCALE_TIMER_OVERFLOW();
}

/* RAM:fa6a02 */
void PLEXTOR_NEAR_FUNCTION ApplyControllerOperatingProfileAfterResponse(void)
{
    fw_addr_t delay_table;
    u8 operating_code;
    u8 profile_index;
    u8 delay;
    u16 reload_value;
    u16 flags;

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1055));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51);
        delay_table = FirmwareReadBit0031() == 0u ? PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91DE) : PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91EB);
    }
    else
    {
        operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
        delay_table = FirmwareReadBit0031() == 0u ? PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91AA) : PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91B7);
    }
    profile_index = MapOperatingCodeToProfileIndex(operating_code);
    delay = FirmwareRead8(FirmwareAddressIncrement24(delay_table, profile_index));

    RESPONSE_WRITE_SCALED_PROFILE(10u);
    reload_value = (u16)(0xffffu - (u16)delay);
    FirmwareWriteDirectSfr8(8D, (u8)(reload_value >> 8));
    FirmwareWriteDirectSfr8(8B, (u8)reload_value);
    RESPONSE_CLEAR_TIMER_OVERFLOW();
    RESPONSE_SET_TIMER_RUN();
    do
    {
    } while(RESPONSE_READ_TIMER_OVERFLOW() == 0u);
    RESPONSE_CLEAR_TIMER_RUN();
    RESPONSE_CLEAR_TIMER_OVERFLOW();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
        RESPONSE_WRITE_PARAMETER((u16)(0xfa00u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0D)));
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
    RESPONSE_WRITE_D4_E2(RESPONSE_READ_BIT0044() == 0u ? 0x14u : 0xffu);
    RESPONSE_WRITE_PARAMETER((u16)(0xe720u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));
    RESPONSE_SET_BIT0038();
    RESPONSE_CLEAR_BIT003A();

    flags = ReadInterpolationWord(FW_NEAR_UNKNOWN_FAD9);
    WriteInterpolationWord(FW_NEAR_UNKNOWN_FAD9, (u16)(flags & 0xfffdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_121C, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 0x01u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) | 0x06u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xefu));
}

/* RAM:fadcba */
u16 LookupControllerOperatingAdjustmentForCurrentPosition(void)
{
    u32 raw_position = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1235) << 24;
    s32 position;
    u16 middle_word;
    u16 seconds;
    u16 bucket;
    u16 selector_offset;
    fw_addr_t selector_address;
    u8 row;

    raw_position |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1236) << 16;
    raw_position |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1237) << 8;
    raw_position |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1238);
    position = (s32)raw_position;
    if(position <= 0)
        return 0u;

    raw_position = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1235) << 24;
    raw_position |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1236) << 16;
    raw_position |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1237) << 8;
    raw_position |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1238);
    middle_word = (u16)(raw_position >> 8);
    seconds = (u16)((u16)((middle_word / 75u) << 8) + (u16)((u16)((middle_word % 75u) << 8) + (u8)raw_position) / 75u);
    bucket = (u16)(seconds / 300u);

    if(bucket < 6u)
        return 0u;
    bucket = (u16)(bucket - 6u);
    if(bucket > 10u)
        bucket = 10u;

    selector_offset = (u16)(((u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F) + 1u) * 11u) - 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
        selector_address = FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F833BD), selector_offset);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 10u)
        selector_address = FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F832B5), selector_offset);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) == 2u)
        selector_address = FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83084), selector_offset);
    else
        selector_address = FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F831AD), selector_offset);

    row = FirmwareRead8(selector_address);
    if(row == 0u)
        return 0u;
    return FirmwareRead8(FirmwareAddressIncrement24(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82320), (u16)(bucket + (u16)((u16)(row - 1u) * 11u))));
}

/* RAM:fadb03 */
u16 LookupControllerOperatingAdjustmentByFrameBucket(s32 frame_position)
{
    u8 row;
    u16 bucket = 0u;
    u16 table_offset;
    fw_addr_t table_base;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) < 9u)
        return 0u;
    row = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D) - 9u);

    if(frame_position > 0)
    {
        u32 raw_position = (u32)frame_position;
        u16 middle_word = (u16)(raw_position >> 8);
        u16 seconds = (u16)((u16)((middle_word / 75u) << 8) + (u16)((u16)((middle_word % 75u) << 8) + (u8)raw_position) / 75u);

        bucket = (u16)(seconds / 600u);
    }
    if(bucket >= 9u)
        bucket = 8u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x28u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x20u)
        return 0u;
    if(bucket < 1u)
        return 0u;

    table_offset = (u16)((u16)row * 8u + bucket - 1u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x18u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82A50);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x14u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82808);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x10u)
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F825C0);
    else
        table_base = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F82378);

    return FirmwareRead8(FirmwareAddressIncrement24(table_base, table_offset));
}

/* RAM:fa6765 */
void PLEXTOR_NEAR_FUNCTION ComputeControllerOperatingProfileAdjustment(void)
{
    u16 adjustment;

    WriteInterpolationWord(FW_NEAR_UNKNOWN_10B4, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        u32 profile_value = 0ul;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1281) != 2u || (ReadBit0033Atomic() == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u))
            SetBit0039Atomic();
        if(ReadBit0039Atomic() == 0u)
            return;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) < 0x34u)
        {
            adjustment = (u16)(LookupControllerOperatingAdjustmentByFrameBucket(ReadInterpolationDword(FW_NEAR_UNKNOWN_FBA2)) * 0x12u);
            WriteInterpolationWord(FW_NEAR_UNKNOWN_10B4, adjustment);
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 2u) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0xffu)
                profile_value = ReadInterpolationWord(FirmwareAddressIncrement24(FW_NEAR_UNKNOWN_EEAA, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1137) * 2u)));
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x34u)
                profile_value = ReadInterpolationWord(FW_NEAR_UNKNOWN_E75A);
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x28u)
                profile_value = ReadInterpolationWord(FW_NEAR_UNKNOWN_E758);
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x20u)
                profile_value = ReadInterpolationWord(FW_NEAR_UNKNOWN_E756);
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x18u)
                profile_value = ReadInterpolationWord(FW_NEAR_UNKNOWN_E754);
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0x14u)
                profile_value = ReadInterpolationWord(FW_NEAR_UNKNOWN_E752);

            if(profile_value != 0ul)
            {
                profile_value = MultiplyU32Low(profile_value, (u32)ReadInterpolationWord(FW_NEAR_UNKNOWN_F9E2));
                profile_value = DIVIDE_OPERATING_PROFILE(profile_value, 10000ul);
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) >= 0x10u)
                WriteInterpolationWord(FW_NEAR_UNKNOWN_FACF, (u16)((u16)profile_value + ReadInterpolationWord(FW_NEAR_UNKNOWN_FAD1)));
        }

        adjustment = (u16)(ReadInterpolationWord(FW_NEAR_UNKNOWN_10B4) + ReadInterpolationWord(FW_NEAR_UNKNOWN_FACF));
        WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC3, adjustment);
        WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC5, adjustment);
        return;
    }

    adjustment = LookupControllerOperatingAdjustmentForCurrentPosition();
    WriteInterpolationWord(FW_NEAR_UNKNOWN_10B4, adjustment);
    adjustment = (u16)(adjustment + ReadInterpolationWord(FW_NEAR_UNKNOWN_EF81));
    WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC3, adjustment);
    WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC5, adjustment);
    if((s16)adjustment > 0x007e)
    {
        WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC3, 0x007eu);
        WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC5, 0x007eu);
    }
}

/* RAM:f9e08d */
void PLEXTOR_NEAR_FUNCTION ApplyControllerOperatingInterpolationAtIndex(u8 profile_index)
{
    u8 normalized_index = profile_index;
    u16 table_offset;
    u16 base_value;
    u16 scaled_value;

    if(normalized_index >= 0x14u)
        normalized_index = (u8)(normalized_index - 0x14u);

    if(profile_index == 0x10u)
    {
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF81, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF85));
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF83, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF87));
    }

    table_offset = (u16)((u16)normalized_index * 2u);
    base_value = ReadInterpolationWord((fw_addr_t)(FW_NEAR_UNKNOWN_F157 + table_offset));
    WriteInterpolationWord(FW_NEAR_UNKNOWN_EF81, base_value);
    WriteInterpolationWord(FW_NEAR_UNKNOWN_EF83, ReadInterpolationWord((fw_addr_t)(FW_NEAR_UNKNOWN_F187 + table_offset)));

    scaled_value = (u16)(LOOKUP_OPERATING_ADJUSTMENT() + base_value);
    WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC3, scaled_value);
    if(scaled_value > 0x007eu)
    {
        scaled_value = 0x007eu;
        WriteInterpolationWord(FW_NEAR_UNKNOWN_FBC3, scaled_value);
    }

    WRITE_EA_EB(ReadInterpolationWord(FW_NEAR_UNKNOWN_EF83));
    WRITE_EF((u8)ReadInterpolationWord(FW_NEAR_UNKNOWN_FBC3));
}

/* RAM:f9de74 */
u8 PLEXTOR_NEAR_FUNCTION SelectControllerProfileForOperatingCode(u8 radial_coordinate)
{
    u16 captured_inputs = CAPTURE_CONTROLLER_PROFILE_SELECTOR_INPUTS(radial_coordinate);
    u8 profile;
    u8 minimum_profile;

    radial_coordinate = (u8)captured_inputs;
    profile = (u8)(captured_inputs >> 8);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 24u)
        profile = COMPUTE_CONTROLLER_PROFILE_BYTE(radial_coordinate, ReadInterpolationWord(FW_NEAR_UNKNOWN_F9FC));

    minimum_profile = FirmwareReadNear8(FW_NEAR_UNKNOWN_1058);
    if(profile < minimum_profile)
    {
        profile = minimum_profile;
    }
    else
    {
        u8 maximum_profile = FirmwareReadNear8(FW_NEAR_UNKNOWN_1059);

        if(profile > maximum_profile)
            profile = maximum_profile;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 0u)
        profile = 0x10u;
    return profile;
}

/* RAM:f9de23 */
void UpdateSecondaryControllerOperatingProfile(void)
{
    u8 inherited_profile = CAPTURE_SECONDARY_PROFILE_INHERITED_R15();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x80u) != 0u)
    {
        u8 profile = SELECT_SECONDARY_PROFILE(FirmwareReadNear8(FW_NEAR_UNKNOWN_1137), inherited_profile);

        if(profile != FirmwareReadNear8(FW_NEAR_UNKNOWN_FA03))
        {
            APPLY_SECONDARY_PROFILE(profile);
            if(READ_SECONDARY_PROFILE_INTERPOLATION_BIT() != 0u)
                INTERPOLATE_SECONDARY_PROFILE(profile);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 10u && profile >= 8u)
                WRITE_SECONDARY_CONTROLLER_PARAMETER(0xf049u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA03, profile);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, profile);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, profile);
        }
    }
}

/* RAM:f9dd92 */
void UpdateMediaState5ControllerProfileForCurrentPosition(void)
{
    u8 inherited_profile = CAPTURE_MEDIA_STATE5_PROFILE_INHERITED_R15();

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x80u) != 0u)
    {
        u8 profile = SELECT_MEDIA_STATE5_PROFILE(DIVIDE_MEDIA_STATE5_POSITION_BY_4500(ReadInterpolationDword(FW_NEAR_UNKNOWN_103D)), inherited_profile);
        u8 table_index = profile;

        APPLY_MEDIA_STATE5_PROFILE(profile);
        if(table_index >= 0x14u)
            table_index = (u8)(table_index - 0x14u);
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF81, ReadInterpolationWord((fw_addr_t)(FW_NEAR_UNKNOWN_F157 + (u16)table_index * 2u)));
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF83, ReadInterpolationWord((fw_addr_t)(FW_NEAR_UNKNOWN_F187 + (u16)table_index * 2u)));
        if(profile == 0x10u)
        {
            WriteInterpolationWord(FW_NEAR_UNKNOWN_EF81, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF85));
            WriteInterpolationWord(FW_NEAR_UNKNOWN_EF83, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF87));
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA03, profile);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, profile);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4F, profile);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x14u)
    {
        COMPUTE_OPERATING_CODE14_BOUNDS();
    }
    else
    {
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF81, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF85));
        WriteInterpolationWord(FW_NEAR_UNKNOWN_EF83, ReadInterpolationWord(FW_NEAR_UNKNOWN_EF87));
    }
}

/* RAM:f9dd3d */
void UpdateControllerOperatingCodeEnableState(void)
{
    u8 state = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x7fu);
    u8 controller_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88);

    if(controller_type == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) != 0x0au && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x20u) == 0u)
            state = (u8)(state | 0x80u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 0x18u)
    {
        state = (u8)(state | 0x80u);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, state);
    if((state & 0x80u) != 0u)
        ROUTE_CONTROLLER_OPERATING_CODE_RANGE();
}
