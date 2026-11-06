#include "coroutine_state5.h"
#include "../platform/firmware_memory.h"
#include "state5_read_acquisition_sequence.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/coroutine_state5_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadMediaCoroutineState5U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMediaCoroutineState5U32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f86d78 */
void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState5(void)
{
    s32 previous_frame_position = 0l;
    s32 position_window_frames;
    s32 saved_target;
    u8 retry_count = 0u;
    u8 acquisition_status;

    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_INITIALIZE_CONTROL, 0l);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_INITIALIZE_CONTROLLER, 0l);
    if(MEDIA_STATE5_CALL(MEDIA_STATE5_TEST_INDEXED_STATE, 0x1fl) == 1l || FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D8) == 2u)
        retry_count = 4u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x20u) != 0u)
        position_window_frames = MEDIA_STATE5_CALL(MEDIA_STATE5_COMPUTE_POSITION_WINDOW, 0l);
    else
        position_window_frames = MEDIA_STATE5_CALL(MEDIA_STATE5_COMPUTE_POSITION_WINDOW, 0l);

    for(;;)
    {
        if(retry_count >= 5u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5D8) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
                return;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) > 0x10u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) == 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0x10u);
        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_RESET_PORTS_8087_808B, 0l);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        saved_target = (s32)ReadMediaCoroutineState5U32(FW_NEAR_UNKNOWN_0E3B);
        acquisition_status = (u8)MEDIA_STATE5_CALL(MEDIA_STATE5_RUN_ACQUISITION, position_window_frames);
        if(acquisition_status == 0u)
            break;
        if(acquisition_status != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 0xfeu));
            return;
        }

        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806A, 0u);
        WriteMediaCoroutineState5U32(FW_NEAR_UNKNOWN_0E3B, (u32)saved_target);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x02u) == 0u)
            ++retry_count;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F971, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F971) & 0xfeu));
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_COMPUTE_SFR_DB_WINDOW, 0l);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_UPDATE_SECONDARY_CODE, 0l);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_CONFIGURE_TARGET, saved_target);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_BUILD_ADJACENT_MSF, 0l);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_SET_INDEXED_FLAGS, 0l);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_UPDATE_CONTROLLER_PROFILE, 0l);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAEC, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    (void)MEDIA_STATE5_CALL(MEDIA_STATE5_YIELD, 0l);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u)
        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_APPLY_STATE_F6_PARAMETERS, 0l);

    for(;;)
    {
        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_ARM_PACKET_CAPTURE, 0l);
        do
        {
            acquisition_status = (u8)MEDIA_STATE5_CALL(MEDIA_STATE5_PARSE_PACKET, 0l);
            if(acquisition_status != 0xfeu || FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
                break;
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_YIELD, 0l);
        } while(1);

        previous_frame_position = MEDIA_STATE5_CALL(MEDIA_STATE5_UPDATE_POSITION_TRACKING, previous_frame_position);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) & 0x02u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 4u));
            }
        }
        else
        {
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_SERVICE_POST_TARGET, saved_target);
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
        {
            u8 bit0033;

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
            bit0033 = (u8)MEDIA_STATE5_CALL(MEDIA_STATE5_READ_BIT0033, 0l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xf7u) | ((bit0033 & 1u) << 3)));
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_RESTORE_READ_PATH, 1l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
            return;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x1fu)
            {
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) == 3u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 8u);
                }
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB95, 0xffu);
            }
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_RESET_PORTS_80C4_80C5, 0l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_RESTORE_READ_PATH, 1l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return;
        }

        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_COPY_CURRENT_POSITION, 0l);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_SERVICE_SECONDARY_CODE, 0l);
        if(MEDIA_STATE5_CALL(MEDIA_STATE5_TEST_INDEXED_STATE, 4l) != 0l)
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_UPDATE_LEAD_OUT_STAGE, 0l);
        if(MEDIA_STATE5_CALL(MEDIA_STATE5_DISPATCH_INDEXED_STATE, 0l) == 1l)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
            (void)MEDIA_STATE5_CALL(MEDIA_STATE5_RESTORE_READ_PATH, 1l);
            return;
        }
        (void)MEDIA_STATE5_CALL(MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES, 0l);
    }
}
