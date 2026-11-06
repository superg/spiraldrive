#include "close_track_session.h"
#include "media/active_write_pipeline.h"
#include "media/close_track_sync_operation.h"
#include "mmc_write.h"
#include "platform/firmware_memory.h"
#include "platform/leaf_workers.h"
#include "platform/status_mapping.h"
#include "platform/track_state.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/close_track_session_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static void ClearCloseCommandFlags(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
}

static void BeginCloseCommandOperation(void)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0B) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 1u));
        CLOSE_COMPLETE_NO_SENSE();
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xfeu));
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) | 8u));
}

static void PatchLastA0FullTocDescriptor(void)
{
    u8 descriptor_index = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_12CA) / 11u);

    while(descriptor_index != 0u)
    {
        fw_addr_t descriptor = FW_HUGE_UNKNOWN_2B8003 + (fw_addr_t)((u16)descriptor_index * 11u);

        if(FirmwareRead8(descriptor) == 0xb0u)
        {
            FirmwareWrite8(descriptor + 1u, 0xffu);
            FirmwareWrite8(descriptor + 2u, 0xffu);
            FirmwareWrite8(descriptor + 3u, 0xffu);
            return;
        }
        descriptor_index--;
    }
}

/* RAM:fb4a22 */
void HandleCloseTrackSessionCommand(void)
{
    u8 inherited_b = CLOSE_CAPTURE_B();
    u8 original_parameter = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45);
    u8 special_session_tail = 0u;
    u8 state;
    u8 selector;
    u8 track_number;
    u8 working_type;

    if(CLOSE_VALIDATE_CDB(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8CD4)) == 0u)
    {
        CLOSE_UNSUPPORTED();
        return;
    }
    CLOSE_NORMALIZE_SPEED();

    state = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);
    if(state == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0xbfu));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    }
    else if(state == 1u || state == 3u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) != 0u)
            (void)CLOSE_FINALIZE_ACTIVE();
        ClearCloseCommandFlags();
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) != 0u)
        {
            (void)CLOSE_FINALIZE_ACTIVE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        }
        BeginCloseCommandOperation();
        CLOSE_RESET_RAW_READ();
        CLOSE_INIT_COROUTINE2(FW_ENTRY_INITIALIZE_THEN_IDLE_COROUTINE2);
        CLOSE_YIELD();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1455, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1456, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1457, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1458, 0u);
        if(CLOSE_PREPARE_CONTROLLER() == 0u)
        {
            CLOSE_UPDATE_FAILURE();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x7fu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xbfu));
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 1u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) | 8u));
        }
        ClearCloseCommandFlags();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 8u) != 0u)
        CLOSE_FINALIZE_EXISTING();

    selector = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0C);
    if(selector == 3u)
    {
        CLOSE_UNSUPPORTED();
        return;
    }
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0F);
    if(CLOSE_VALIDATE_MEDIA() == 0u)
        return;

    if(selector == 2u)
    {
        if(CLOSE_VALIDATE_SESSION() == 0u)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0xc0u)
        {
            CLOSE_UPDATE_TAIL_BOUNDS();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) >= 99u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E4E0) == 4u)
                special_session_tail = 1u;
        }
    }
    else if(selector == 1u)
    {
        if(track_number == 0xffu)
        {
            track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
            if(track_number == 100u || FirmwareReadNear8(FW_NEAR_UNKNOWN_DD87 + (fw_addr_t)((u16)track_number * 19u)) == 0u)
            {
                CLOSE_UNSUPPORTED();
                return;
            }
        }
        else
        {
            if(track_number == 0u || track_number > FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F))
            {
                CLOSE_UNSUPPORTED();
                return;
            }
            if(track_number < FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
            {
                CLOSE_NO_SENSE();
                return;
            }
        }
        CLOSE_CLEAR_WORKING();
        if(CLOSE_LOAD_TRACK(track_number, inherited_b) == 0u)
            return;
    }
    else
    {
        CLOSE_UNSUPPORTED();
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) != 0u)
    {
        CLOSE_SEQUENCE_ERROR();
        return;
    }

    if((selector & 1u) != 0u)
    {
        working_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
        if(working_type == 0u)
        {
            CLOSE_UNSUPPORTED();
            return;
        }
        if(working_type == 2u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 1u)
            {
                CLOSE_SEQUENCE_ERROR();
                return;
            }
        }
        else if(working_type == 4u)
        {
            CLOSE_NO_SENSE();
            return;
        }
        else if(working_type == 3u || working_type == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
            {
                CLOSE_SEQUENCE_ERROR();
                return;
            }
            if(CLOSE_VALIDATE_WRITE_STATE() == 0u)
                return;
        }
        else if(working_type == 7u)
        {
            CLOSE_SEQUENCE_ERROR();
            return;
        }
        if((working_type == 1u || working_type == 6u) && CLOSE_VALIDATE_PMA() == 0u)
            return;
    }

    BeginCloseCommandOperation();
    if((selector & 1u) != 0u)
    {
        CLOSE_EXECUTE_OPERATION();
    }
    else
    {
        if(special_session_tail != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C45, 0x40u);
        CLOSE_COMPLETE_SESSION();
        if(special_session_tail != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C45, original_parameter);
            PatchLastA0FullTocDescriptor();
        }
    }
    ClearCloseCommandFlags();
}
