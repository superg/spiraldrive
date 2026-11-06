#include "state5_position_window.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/state5_position_window_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadState5PositionU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u32 ScaleState5Speed(u8 speed, u16 frames_per_speed)
{
    return MultiplyU32Low((u32)speed, (u32)frames_per_speed);
}

/* RAM:f876bb */
s32 PLEXTOR_NEAR_FUNCTION ComputeMediaState5PositionWindowFrames(void)
{
    s16 current_bucket;
    s16 target_bucket;
    s16 bucket_distance;
    s32 target;
    s32 window;
    u32 extra = 0ul;
    u8 effective_speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEA, effective_speed);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 0u)
        effective_speed = 16u;

    current_bucket = (s16)DIVIDE_STATE5_POSITION((s32)ReadState5PositionU32(FW_NEAR_UNKNOWN_1235), 75l);
    target_bucket = (s16)DIVIDE_STATE5_POSITION((s32)ReadState5PositionU32(FW_NEAR_UNKNOWN_0E3B), 75l);
    bucket_distance = AbsoluteValueI16((s16)(u16)((u16)current_bucket - (u16)target_bucket));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFC, effective_speed < 4u ? 1u : 6u);
    window = (s32)ScaleState5Speed(effective_speed, 75u);
    target = (s32)ReadState5PositionU32(FW_NEAR_UNKNOWN_0E3B);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && target < 45000l)
        window = (s32)ScaleState5Speed(effective_speed, 90u);

    if(effective_speed >= 10u)
    {
        target = (s32)ReadState5PositionU32(FW_NEAR_UNKNOWN_0E3B);
        if(target <= -1200l)
        {
            extra = ScaleState5Speed(effective_speed, 52u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFC, 1u);
        }
        else if(effective_speed >= 20u)
        {
            u16 multiplier = 15u;

            if(bucket_distance >= 4200)
            {
                if(effective_speed >= 52u)
                    multiplier = 225u;
                else if(effective_speed >= 40u)
                    multiplier = 150u;
                else
                    multiplier = 75u;
            }
            else if(bucket_distance >= 2400)
            {
                multiplier = effective_speed >= 52u ? 150u : 75u;
            }
            else if(bucket_distance >= 1200 && effective_speed >= 52u)
            {
                multiplier = 75u;
            }
            extra = ScaleState5Speed(effective_speed, multiplier);
        }
    }

    window += (s32)extra;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 1u)
        window += 75l;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 52u)
        window += (s32)ScaleState5Speed(effective_speed, 30u);

    window = DIVIDE_STATE5_POSITION(window, 1l);
    if(window >= 0x816l && (s32)ReadState5PositionU32(FW_NEAR_UNKNOWN_0E3B) <= 0x57e4l)
        window = 0x816l;
    return window;
}
