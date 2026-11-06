#include "raw_read_position_tracker.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "raw_read_descriptor_completion.h"
#include "raw_read_position_probe.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_position_tracker_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadPositionTrackerU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePositionTrackerU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 RetryOrServicePositionTracker(void)
{
    u8 old_retry_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F1E);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, (u8)(old_retry_count - 1u));
    if((s8)old_retry_count > 0)
        return 1u;
    TRACKER_SERVICE_COUNTDOWN();
    return 0u;
}

/* RAM:f93b88 */
void PLEXTOR_NEAR_FUNCTION TrackRawReadPositionAndArmTargetCapture(void)
{
    s32 inferred_position;
    s32 target;
    s32 format_offset;

    if(TRACKER_READ_SPECIAL_MODE() != 0u)
    {
        TRACKER_ARM_CAPTURE();
        return;
    }

    if((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018022) & 1u) == 0u)
    {
        if(TRACKER_READ_SEQUENCE_VALID() != 0u)
        {
            WritePositionTrackerU32(FW_NEAR_UNKNOWN_15AE, ReadPositionTrackerU32(FW_NEAR_UNKNOWN_15AE) + 1ul);
        }
        else if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0F23) > 15)
        {
            (void)RetryOrServicePositionTracker();
            return;
        }
        else
        {
            TRACKER_SET_SEQUENCE_VALID();
            WritePositionTrackerU32(FW_NEAR_UNKNOWN_15AE, ReadPositionTrackerU32(FW_NEAR_UNKNOWN_1235) + 1ul);
        }
    }
    else
    {
        u8 index;

        for(index = 0u; index < 10u; ++index)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FA3 + index, FirmwareReadController8(FW_CONTROLLER_UNKNOWN_018026));

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FA3) & 0x0fu) == 1u)
        {
            TRACKER_SET_SEQUENCE_VALID();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A0, TRACKER_CONVERT_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAA)));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A1, TRACKER_CONVERT_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAB)));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_15A2, TRACKER_CONVERT_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAC)));
            WritePositionTrackerU32(FW_NEAR_UNKNOWN_15AE, (u32)TRACKER_CONVERT_MSF(FW_NEAR_UNKNOWN_15A0));
        }
        else if(TRACKER_READ_SEQUENCE_VALID() != 0u)
        {
            WritePositionTrackerU32(FW_NEAR_UNKNOWN_15AE, ReadPositionTrackerU32(FW_NEAR_UNKNOWN_15AE) + 1ul);
        }
        else
        {
            (void)RetryOrServicePositionTracker();
            return;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F1E, 0x1eu);
    }

    inferred_position = (s32)ReadPositionTrackerU32(FW_NEAR_UNKNOWN_15AE);
    target = (s32)ReadPositionTrackerU32(FW_NEAR_UNKNOWN_1598);
    format_offset = FirmwareReadNear8(FW_NEAR_UNKNOWN_0F06) == 0u ? 0L : -1L;

    if(inferred_position > (s32)((u32)target + (u32)format_offset))
    {
        TRACKER_SERVICE_COUNTDOWN();
        return;
    }
    if(inferred_position <= (s32)((u32)target + (u32)format_offset - 150ul))
    {
        TRACKER_SERVICE_COUNTDOWN();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 2u)
    {
        if(inferred_position == (s32)((u32)target + (u32)format_offset))
            TRACKER_ARM_CAPTURE();
    }
    else
    {
        TRACKER_ENQUEUE_TIMING(0x0200u, 3u);
    }
}
