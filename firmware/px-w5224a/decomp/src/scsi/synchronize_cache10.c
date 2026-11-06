#include "synchronize_cache10.h"
#include "media/active_write_pipeline.h"
#include "platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/synchronize_cache10_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadSynchronizeCacheU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void UpdateSynchronizeCacheCountdown(void)
{
    u8 state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);

    if(state == 2u || state == 1u)
    {
        if(FirmwareRead8(FW_HUGE_ACTIVE_OPERATION_FAILURE_LATCH) == 0u)
        {
            if(FirmwareRead8(FW_HUGE_ACTIVE_RECORD_ACQUIRED_LATCH) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 8u));
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0u)
                    FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 60u);
                else
                    FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, 20u);
            }
            else if((s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
            {
                FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
                FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
            }
        }
    }
    else if(state == 3u && (s8)FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) < 0)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 1u));
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    }
}

/* RAM:fb42dc */
void HandleSynchronizeCache10Command(void)
{
    u8 write_type;

    if(SYNC_VALIDATE(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8DA7)) == 0u)
    {
        SYNC_UNSUPPORTED();
        return;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) == 0u && (s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >= 0)
    {
        SYNC_NO_SENSE();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 2u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        SYNC_COMPLETE();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) < 0)
    {
        SYNC_CLEAR_BIT30();
        SYNC_CLEAR_BIT35();
        if(write_type == 1u)
            SYNC_CLOSE_OPERATION();
        else if(write_type == 2u || write_type == 3u)
            SYNC_SEQUENCE_ERROR();
        else if(write_type == 0u)
            SYNC_FINALIZE_WORKING();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x7fu));
    }
    else
    {
        SYNC_FINALIZE_ACTIVE();
        if(write_type == 2u || write_type == 3u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1592) >= 0x51u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C39));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C39));
            }
        }
        else if((s32)ReadSynchronizeCacheU32(FW_NEAR_CURRENT_FRAME_POSITION) >= 0x58fd4l && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C38, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C39));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C39));
        }
    }

    UpdateSynchronizeCacheCountdown();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}
