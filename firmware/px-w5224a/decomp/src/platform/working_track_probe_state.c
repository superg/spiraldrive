#include "working_track_probe_state.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/working_track_probe_state_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadWorkingTrackProbeDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteWorkingTrackProbeDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:fc46c4 */
u8 PLEXTOR_NEAR_FUNCTION ApplyRawQProbeStatusToWorkingTrackState(void)
{
    u8 inherited_r15 = CAPTURE_WORKING_TRACK_PROBE_R15();
    s32 upper_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION);
    s32 initial_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD3D);
    s32 refined_frame = REFINE_WORKING_TRACK_PROBE_BOUNDARY(initial_frame, upper_frame);
    s32 reference_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 status = PROBE_WORKING_TRACK_OFFSETS(refined_frame, reference_frame);

    if(status == 1u)
    {
        WriteWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD45, ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION));
        return 1u;
    }
    if(status == 5u)
        return 0u;
    if(status == 6u)
    {
        WriteWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD45, ReadWorkingTrackProbeDword(FW_NEAR_UNKNOWN_0F41));
        return 1u;
    }
    if(status == 4u || status == 7u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 6u);
        WriteWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD51, ReadWorkingTrackProbeDword(FW_NEAR_UNKNOWN_0F41));
        return 1u;
    }
    return inherited_r15;
}

/* RAM:fc471d */
u8 PLEXTOR_NEAR_FUNCTION ApplyRawQProbeStatusToWorkingTrackTail(void)
{
    u8 inherited_r15 = CAPTURE_WORKING_TRACK_PROBE_R15();
    s32 upper_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION);
    s32 initial_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD3D);
    s32 refined_frame = REFINE_WORKING_TRACK_PROBE_BOUNDARY(initial_frame, upper_frame);
    s32 reference_frame = (s32)ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION);
    u8 status = PROBE_WORKING_TRACK_OFFSETS(refined_frame, reference_frame);

    if(status == 1u)
    {
        WriteWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD45, ReadWorkingTrackProbeDword(FW_NEAR_CURRENT_FRAME_POSITION));
        return 1u;
    }
    if(status == 5u)
        return 0u;
    if(status == 4u || status == 6u || status == 7u)
    {
        WriteWorkingTrackProbeDword(FW_NEAR_UNKNOWN_DD51, ReadWorkingTrackProbeDword(FW_NEAR_UNKNOWN_0F41));
        return 1u;
    }
    return inherited_r15;
}
