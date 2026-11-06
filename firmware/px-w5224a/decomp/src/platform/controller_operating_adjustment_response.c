#include "controller_operating_adjustment_response.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_operating_adjustment_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadAdjustmentWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteAdjustmentWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:fa6894 */
void ApplyControllerOperatingAdjustmentAndSelectResponse(void)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    u8 profile_index;
    fw_addr_t response_table;

    WRITE_E8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
    if(mode != 1u)
        WRITE_EAEB(ReadAdjustmentWord(FW_NEAR_UNKNOWN_EF83));

    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if(mode == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 4u && READ_BIT31() != 0u)
        {
            s16 quotient = DIVIDE_BY_SEVEN((s16)(u16)(ReadAdjustmentWord(FW_NEAR_UNKNOWN_10B4) << 1));
            WRITE_ECED((u16)(quotient + ReadAdjustmentWord(FW_NEAR_UNKNOWN_102A)));
            WRITE_EF();
            SET_EE_BIT7();
        }
    }
    else
    {
        if(READ_BIT33() == 0u)
        {
            u16 scale_input = (u16)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F086) + ReadAdjustmentWord(FW_NEAR_UNKNOWN_10B4));
            WriteAdjustmentWord(FW_NEAR_UNKNOWN_102A, (u16)SCALE_83_POINT_5(scale_input));
        }
        WRITE_ECED(ReadAdjustmentWord(FW_NEAR_UNKNOWN_102A));
        WRITE_EF();
        SET_EE_BIT7();
        WRITE_EAEB(ReadAdjustmentWord(FW_NEAR_UNKNOWN_EF83));
    }

    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if(mode == 1u)
        WRITE_EAEB(ReadAdjustmentWord(FW_NEAR_UNKNOWN_FBC3));
    else
        WRITE_EF00((u8)ReadAdjustmentWord(FW_NEAR_UNKNOWN_FBC3));

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01807F, 0u);
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802E, 8u);
    mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if(mode == 1u)
    {
        profile_index = MAP_OPERATING_CODE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51));
        response_table = READ_BIT31() != 0u ? PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9205) : PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91F8);
    }
    else
    {
        profile_index = MAP_OPERATING_CODE(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F));
        response_table = READ_BIT31() != 0u ? PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91D1) : PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF91C4);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1055, FirmwareRead8(response_table + profile_index));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 0x80u));
}
