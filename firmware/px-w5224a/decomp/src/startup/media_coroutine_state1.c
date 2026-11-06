#include "media_coroutine_state1.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/startup/media_coroutine_state1_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadMediaState1U32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteMediaState1U32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 MediaState1UsesOrdinaryTarget(void)
{
    return (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x30u) == 0u ? 1u : 0u;
}

#define FINISH_MEDIA_STATE1_WITH_PINS() MEDIA_STATE1_WRITE_P3(1u, 0u)

/* RAM:f85fdf */
void PLEXTOR_NEAR_FUNCTION ProcessMediaCoroutineState1(void)
{
    s32 completion_frame_position = 0;
    u8 used_wide_direction_path = 0u;
    u8 acquisition_status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1007, 3u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfbu));
    (void)MEDIA_STATE1_CALL(MEDIA_STATE1_UPDATE_MEASUREMENT, 0);

    for(;;)
    {
        s32 target;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, (FirmwareReadNear8(FW_NEAR_UNKNOWN_F79B) & 1u) != 0u ? 8u : 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
        {
            u8 indexed_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_130A);

            if(indexed_state != 0x0cu)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 1u);
            if(indexed_state == 5u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
            if(indexed_state == 6u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 0u);
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
                    (void)MEDIA_STATE1_CALL(MEDIA_STATE1_SET_OPERATING_ENABLE, 0);
            }
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            MEDIA_STATE1_WRITE_P3(0u, 0u);
        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_RESET_PORTS, 0);

        target = (s32)ReadMediaState1U32(FW_NEAR_UNKNOWN_0E3B);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);
        if(MediaState1UsesOrdinaryTarget() != 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
            {
                completion_frame_position = target;
            }
            else
            {
                completion_frame_position = target - 1l;
                target -= 6l;
                WriteMediaState1U32(FW_NEAR_UNKNOWN_0E3B, (u32)target);
                if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x40u) != 0u)
                {
                    target = (s32)ReadMediaState1U32(FW_NEAR_UNKNOWN_0E3B) - 10l;
                    WriteMediaState1U32(FW_NEAR_UNKNOWN_0E3B, (u32)target);
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) != 0u)
                    {
                        u8 retry_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAFD) + 1u);
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFD, retry_count);
                        if(retry_count == 10u)
                        {
                            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
                            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x52u);
                            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_RESTART_COROUTINE, (s32)FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
                        }
                    }
                }
            }
        }
        else
        {
            target -= 1l;
            WriteMediaState1U32(FW_NEAR_UNKNOWN_0E3B, (u32)target);
        }

        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_SELECT_OPERATING_CODE, 0);
        acquisition_status = (u8)MEDIA_STATE1_CALL(MEDIA_STATE1_ACQUIRE_POSITION, 0);
        if(MEDIA_STATE1_CALL(MEDIA_STATE1_HANDLE_ACQUISITION_STATUS, acquisition_status) != 0)
            return;

        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_CONFIGURE_PORTS_8085_8086, 0);
        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_OPERATING_CHANNELS, 0);
        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_BUILD_PORT_SHADOWS, 0);
        if(MediaState1UsesOrdinaryTarget() != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_121B, 5u);
            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_PORT_SHADOWS, 0);
            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_SET_P1_BIT4, 0);
        }
        else
        {
            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_PORT_SHADOWS, 0);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1218, 100u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBD, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_121B, 0xffu);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_130B, FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 0x0cu ? 100u : 10u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
            MEDIA_STATE1_WRITE_P3(0u, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAFD, 0u);
        used_wide_direction_path = 0u;

        for(;;)
        {
            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_REARM_Q_CAPTURE, 0);
            while(MEDIA_STATE1_CALL(MEDIA_STATE1_PROCESS_Q_PACKET, 0) == 0xfe && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
                (void)MEDIA_STATE1_CALL(MEDIA_STATE1_YIELD, 0);

            if(MediaState1UsesOrdinaryTarget() != 0u && completion_frame_position <= (s32)ReadMediaState1U32(FW_NEAR_UNKNOWN_1235) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u
                && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) | 0x80u));
                (void)MEDIA_STATE1_CALL(MEDIA_STATE1_YIELD, 0);
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
                    MEDIA_STATE1_WRITE_P3(0u, 0u);
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
                acquisition_status = 1u;
                break;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A) != 0u)
            {
                if(MEDIA_STATE1_CALL(MEDIA_STATE1_SERVICE_CALIBRATION, 0) != 0)
                {
                    acquisition_status = 2u;
                }
                else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF1) == 0u)
                {
                    acquisition_status = 1u;
                }
                else
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1009, 1u);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE7, 0x51u);
                    (void)MEDIA_STATE1_CALL(MEDIA_STATE1_RESTART_COROUTINE, (s32)FW_ENTRY_RUN_STARTUP_STATUS_COROUTINE1);
                }
                break;
            }
            if(MEDIA_STATE1_CALL(MEDIA_STATE1_PROCESS_DRIVE_FLAGS, 0) == 2)
                return;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1007, 3u);

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u)
            {
                if(completion_frame_position <= (s32)ReadMediaState1U32(FW_NEAR_UNKNOWN_1235))
                {
                    if(MEDIA_STATE1_CALL(MEDIA_STATE1_TEST_INDEXED_STATE, 6) != 0)
                        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_CONFIGURE_PORTS_8079_809D_809F, 0);
                    else
                        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_PORT_SHADOWS, 0);
                }
                acquisition_status = (u8)MEDIA_STATE1_CALL(MEDIA_STATE1_SELECT_NARROW_TARGET, 0);
                if(acquisition_status == 1u)
                {
                    FINISH_MEDIA_STATE1_WITH_PINS();
                    break;
                }
                if(acquisition_status == 2u)
                    break;
                (void)MEDIA_STATE1_CALL(MEDIA_STATE1_SET_TARGET_FROM_Q, 0);
                if(MEDIA_STATE1_CALL(MEDIA_STATE1_DISPATCH_INDEXED_STATE, 0) == 1)
                {
                    FINISH_MEDIA_STATE1_WITH_PINS();
                    acquisition_status = 1u;
                    break;
                }
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) == 5u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x40u) == 0u)
                {
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x20u) == 0u)
                    {
                        u8 calibrated = (u8)MEDIA_STATE1_CALL(MEDIA_STATE1_CALIBRATE_PORT_8097, 0);
                        calibrated &= 1u;
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xdfu) | (calibrated << 5)));
                        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x20u) != 0u)
                        {
                            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_CALIBRATE_PARAMETER_F4, 0);
                            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
                            WriteMediaState1U32(FW_NEAR_UNKNOWN_0E3B, 0x00020f58ul);
                            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
                        }
                    }
                    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0x40u) == 0u)
                        (void)MEDIA_STATE1_CALL(MEDIA_STATE1_INITIALIZE_CALIBRATION, 0);
                }
                continue;
            }

            if(MediaState1UsesOrdinaryTarget() != 0u)
            {
                if(completion_frame_position <= (s32)ReadMediaState1U32(FW_NEAR_UNKNOWN_1235) || used_wide_direction_path == 0u)
                    (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_PORT_SHADOWS, 0);
                acquisition_status = (u8)MEDIA_STATE1_CALL(MEDIA_STATE1_SELECT_NARROW_TARGET, 0);
            }
            else
            {
                (void)MEDIA_STATE1_CALL(MEDIA_STATE1_APPLY_PARSED_Q_PORT_STATE, 0);
                acquisition_status = (u8)MEDIA_STATE1_CALL(MEDIA_STATE1_EXECUTE_DIRECTION_MOVE, 0);
                used_wide_direction_path = 1u;
            }
            if(acquisition_status == 1u)
            {
                FINISH_MEDIA_STATE1_WITH_PINS();
                break;
            }
            if(acquisition_status == 2u)
                break;
            if(MediaState1UsesOrdinaryTarget() != 0u && MEDIA_STATE1_CALL(MEDIA_STATE1_SERVICE_TARGET_WINDOW, 0) == 0)
            {
                acquisition_status = 2u;
                break;
            }
            (void)MEDIA_STATE1_CALL(MEDIA_STATE1_SET_TARGET_FROM_Q, 0);
            if(MEDIA_STATE1_CALL(MEDIA_STATE1_PROCESS_INDEXED_COMPLETION, 0) == 1)
            {
                acquisition_status = 1u;
                break;
            }
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBA, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FBA) & 0x7fu));
        if(acquisition_status == 1u)
        {
            FINISH_MEDIA_STATE1_WITH_PINS();
            return;
        }
    }
}
