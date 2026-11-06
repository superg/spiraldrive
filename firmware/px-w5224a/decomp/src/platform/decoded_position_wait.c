#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/decoded_position_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPositionWord(u16 address)
{
    return FirmwareReadNearU32(address);
}

/* RAM:fb8d4d */
u8 WaitForDecodedFramePositionPastTarget(s32 target_frame_position)
{
    u8 parse_status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1209, 40u);
    for(;;)
    {
        if((s32)ReadPositionWord(FW_NEAR_UNKNOWN_1235) > target_frame_position || FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1209) != 0u)
                return 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
            WaitPositionRestore(1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
            return 0u;
        }

        WaitPositionArm();
        do
        {
            parse_status = WaitPositionParse();
            if(parse_status == 0xfeu)
                WaitPositionYield();
        } while(parse_status == 0xfeu);
        if(parse_status == 1u)
            continue;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        WaitPositionRestore(1u);
        WaitPositionReset();
        if(WaitPositionInitialize() != 0u)
        {
            WaitPositionEnable(1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        }
        return 0u;
    }
}
