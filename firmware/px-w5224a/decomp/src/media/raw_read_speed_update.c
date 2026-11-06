#include "raw_read_speed_update.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_speed_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_speed_update_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRawReadSpeedWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRawReadSpeedDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fbf444 */
void UpdateRawReadSpeedForCurrentPipelineState(void)
{
    u8 speed;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) < 5u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1591) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E49) < FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 1u));

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) != 1u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 24)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 24u);

        if(READ_SPEED_BIT_000C() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 32)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 32u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15D7) != 0u)
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 40)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 40u);
            if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 32)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 32u);
            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u && FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 24u);

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0CA1) == 1u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 24u)
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 10u)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
                    else
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
                else
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 32u);
            }

            if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE) == 0xb9u)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 4u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
                else
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
            }
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEF) & 1u) != 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 8)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) > 0 && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 8)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) > 0 && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) > 32)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 32u);
        }

        if(READ_SPEED_BIT_001C() != 0u || READ_SPEED_BIT_001D() != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) >> 1) & 1u) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1591) != 0u)
        {
            s32 target = (s32)ReadRawReadSpeedDword(FW_NEAR_UNKNOWN_1598);
            s32 high_water = (s32)(u32)ReadRawReadSpeedWord(FW_NEAR_UNKNOWN_129C);

            if(target < 150 || target >= high_water)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1591, 0u);
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_15E2) >= 10u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) < 8u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
            else
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        }

        speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F3A, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, speed);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15E2, 0u);
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45) > (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37))
        {
            speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45) > 0)
        {
            speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F45);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
        }
    }

    if(READ_SPEED_BIT_000F() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C37) >= 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);

    if(READ_SPEED_BIT_001B() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 8u);
    }
    if(READ_SPEED_BIT_0021() != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 8u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, 8u);
    }
}
