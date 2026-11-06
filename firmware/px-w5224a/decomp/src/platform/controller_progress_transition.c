#include "controller_progress_transition.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_progress_transition_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fcef6e */
void PLEXTOR_NEAR_FUNCTION ApplyControllerProgressProfileTransition(u8 profile_index, u8 progress_bucket, u8 transition_mode)
{
    u8 transition_threshold;
    u8 transition_was_active;

    if(BYPASS_TRANSITION(profile_index, progress_bucket, transition_mode) != 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x20u)
    {
        WRITE_PROFILE(profile_index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x7fu));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x34u)
        transition_threshold = 1u;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x28u)
        transition_threshold = 3u;
    else
        transition_threshold = 6u;
    if(transition_mode != 1u)
        transition_threshold--;

    if(progress_bucket < transition_threshold)
    {
        if(transition_mode != 1u)
            transition_threshold++;
        if(profile_index != (u8)(transition_threshold - 1u) || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x80u) == 0u)
        {
            WRITE_PROFILE(profile_index);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x7fu));
        }
        return;
    }

    if(transition_mode != 1u && progress_bucket == transition_threshold)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x34u && profile_index == 5u)
            profile_index = FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF907F));
        else
            profile_index++;
    }

    transition_was_active = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) >> 7);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x80u));
    if(transition_was_active == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D) >= 0x20u && transition_mode == 1u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) >> 5) & 1u) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1220, 5u);
        while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1220) > 0)
        {
            YIELD_TRANSITION();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                break;
        }
    }
    WRITE_PROFILE(profile_index);
    if(transition_was_active == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4D) >= 0x20u && transition_mode == 1u)
        WAIT_TRANSITION(200u, 0u);
}
