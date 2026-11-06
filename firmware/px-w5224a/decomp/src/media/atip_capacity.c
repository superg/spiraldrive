#include "atip_capacity.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/atip_capacity_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSupplementalAtipDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteSupplementalAtipDword(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fb8249 */
u8 PLEXTOR_NEAR_FUNCTION AcquireSupplementalAtipCapacityState(void)
{
    u8 initial_countdown;
    u8 current_countdown;
    u8 position_acquired = 0u;
    u16 capacity_units;
    u32 capacity_adjustment;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        WriteSupplementalAtipDword(FW_NEAR_UNKNOWN_0E3B, ReadSupplementalAtipDword(FW_NEAR_UNKNOWN_114F) - 0x0d2ful);
        if(ACQUIRE_SUPPLEMENTAL_ATIP_POSITION() == 1u)
        {
            position_acquired = 1u;
            break;
        }
    }
    if(position_acquired == 0u)
    {
        RESET_SUPPLEMENTAL_ATIP_DRIVE_CONTROL();
        return 0u;
    }

    APPLY_SUPPLEMENTAL_ATIP_OPERATING_CODE();
    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0u);
    ARM_SUPPLEMENTAL_ATIP_PACKET();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 2u));
    initial_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1203);

    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        current_countdown = FirmwareReadNear8(FW_NEAR_UNKNOWN_1203);
        if((s16)(u16)initial_countdown > (s16)(u16)((u16)current_countdown + 2u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) == 0xe0u)
            return 1u;

        if(PARSE_SUPPLEMENTAL_ATIP_PACKET() != 0xfeu)
            ARM_SUPPLEMENTAL_ATIP_PACKET();
        YIELD_SUPPLEMENTAL_ATIP();
        if(VALIDATE_SUPPLEMENTAL_ATIP_PACKET(2u) != 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 2u) != 0u)
            {
                capacity_units = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB84) * 2u + 2u);
                capacity_adjustment = MULTIPLY_SUPPLEMENTAL_ATIP_CAPACITY(0x1194ul, (u32)capacity_units);
                WriteSupplementalAtipDword(FW_NEAR_UNKNOWN_FB90, capacity_adjustment);
                WriteSupplementalAtipDword(FW_NEAR_CURRENT_FRAME_POSITION, ReadSupplementalAtipDword(FW_NEAR_UNKNOWN_FB8C) + capacity_adjustment - 0x1a5eul);
            }
            return 1u;
        }
    }

    RESET_SUPPLEMENTAL_ATIP_DRIVE_CONTROL();
    return 0u;
}
