#include "state5_read_acquisition_sequence.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/state5_read_acquisition_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadState5AcquisitionU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadState5AcquisitionU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState5AcquisitionU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static s32 ScaleState5AcquisitionOffset(u8 operating_code, u8 multiplier)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/state5_read_acquisition_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 IsState5WriteSpeedReductionEligible(void)
{
    if((ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_FAD7) & 1u) != 0u && (ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_FAD7) & 2u) != 0u)
        return 1u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        return 1u;
    return (u8)((ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) != 0u);
}

static s32 ComputeState5RadialCoordinate(void)
{
    u8 profile_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1137);
    u16 discarded_endpoint = ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_F9FE);
    u16 endpoint;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x34u)
        discarded_endpoint = ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_F9FC);
    endpoint = ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_F9FC);
#define PLEXTOR_BINDING_SECTION 3
#include "../platform/bindings/media/state5_read_acquisition_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static u8 ReturnState5WaitFailure(u32 original_target)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) != 0u)
        return 2u;
    WriteState5AcquisitionU32(FW_NEAR_UNKNOWN_0E3B, original_target);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    return 1u;
}

/* RAM:f870cd */
u8 PLEXTOR_NEAR_FUNCTION RunMediaState5ReadAcquisitionSequence(s32 position_window_frames)
{
    u32 original_target = ReadState5AcquisitionU32(FW_NEAR_UNKNOWN_0E3B);
    u32 adjusted_target = original_target;
    s8 acquisition_result;
    u8 operating_code;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) == 0x34u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAEC) == 3u)
        adjusted_target -= 100ul;
    WriteState5AcquisitionU32(FW_NEAR_UNKNOWN_0E3B, original_target - (u32)position_window_frames);

    acquisition_result = (s8)STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACQUIRE_POSITION, 0l);
    if(acquisition_result == 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xdfu));
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3C) & 1u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
            return 1u;
        }
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_RESET_DRIVE_CONTROL, 0l);
        if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER, 0l) != 0l)
        {
            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_SET_OPERATING_ENABLE, 1l);
            return 2u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        return 1u;
    }

    if(acquisition_result == -1)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
        if(ReadState5AcquisitionU32(FW_NEAR_UNKNOWN_0E3B) == original_target - (u32)position_window_frames)
            WriteState5AcquisitionU32(FW_NEAR_UNKNOWN_0E3B, original_target);
        return 1u;
    }
    if(acquisition_result != 1)
    {
        for(;;)
        {
        }
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 0u);
        return 1u;
    }

    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACQ_UPDATE_SECONDARY_CODE, 0l);
    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_CHECK_TOLERANCE, (FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 2u) == 0u ? 1l : 0l) == 0l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x20u));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x14u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0x80u);
            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_STROBE_PORT_8080, 0x80l);
        }
        return 2u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0x80u);
    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_STROBE_PORT_8080, 0x80l);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xdfu));
    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_WRITE_PARAMETERS_FC_FD_F6_F7, 0l);

    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    if(operating_code == 0xffu)
        operating_code = 0x28u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) != 1u)
    {
        u8 attempt_count = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD5) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD5, attempt_count);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0u);
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_STROBE_PORT_8080, 0l);
        if(attempt_count >= 2u && IsState5WriteSpeedReductionEligible() != 0u)
        {
            u8 eligible;

            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACCUMULATE_SPEED_FLAGS, 0l);
            eligible = IsState5WriteSpeedReductionEligible();
            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_TRY_REDUCE_WRITE_SPEED, eligible);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD5, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
        return 2u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FA05) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x34u)
        {
            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_GENERATE_PROFILES, ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_F9FE));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA05, 0x34u);
        }
        else
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA05) == 0x34u)
                (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_GENERATE_PROFILES, ReadState5AcquisitionU16(FW_NEAR_UNKNOWN_F9FC));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA05, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
        }
    }

    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_APPLY_OPERATING_CHANNELS, 0l);
    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_WAIT_FOR_POSITION, (s32)(adjusted_target - (u32)ScaleState5AcquisitionOffset(operating_code, 65u))) != 1l)
        return ReturnState5WaitFailure(original_target);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_CONFIGURE_PORTS, 1l);
    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_APPLY_OPERATING_CHANNELS, 0l);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_CONFIGURE_ACQUISITION_CODE, 0l);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 0u)
    {
        WriteState5AcquisitionU32(FW_NEAR_UNKNOWN_F97C, ReadState5AcquisitionU32(FW_NEAR_UNKNOWN_FBA2));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F979, 0u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_F977) == 2u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F978, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F978) | 4u));
    }

    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_WAIT_FOR_POSITION, (s32)(adjusted_target - (u32)ScaleState5AcquisitionOffset(operating_code, 50u))) != 1l)
        return ReturnState5WaitFailure(original_target);

    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_READ_P0_BIT7, 0l) == 0l)
    {
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_REFRESH_PARAMETERS_FC_F6, 0l);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_RESET_DRIVE_CONTROL, 0l);
        if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACQ_INITIALIZE_CONTROLLER, 0l) != 0l)
        {
            (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_SET_OPERATING_ENABLE, 1l);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
            return 2u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 0u);
        return 1u;
    }

    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_CONFIGURE_ACQUISITION_STATE, 0l);
    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_CONFIGURE_REGISTER_PROFILE, 0l);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
    {
        if((s32)ReadState5AcquisitionU32(FW_NEAR_UNKNOWN_1235) < 0l)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x34u ? 0x13u : 0x15u);
        }
        else
        {
            u8 coordinate = (u8)ComputeState5RadialCoordinate();

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, coordinate);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) <= FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51))
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E51, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
        }
    }

    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_PROBE_READINESS, 0l) == 0l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x13u);
        return 2u;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0x10u) == 0u && STATE5_ACQUISITION_CALL(MEDIA_STATE5_PROBE_THRESHOLDS, 0l) == 0l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0u);
        (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_STROBE_PORT_8080, 0l);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x13u);
        return 2u;
    }

    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_WAIT_FOR_POSITION, (s32)(adjusted_target - 30ul)) != 1l)
        return ReturnState5WaitFailure(original_target);

    (void)STATE5_ACQUISITION_CALL(MEDIA_STATE5_ACQ_RESTORE_READ_PATH, 0l);
    if(STATE5_ACQUISITION_CALL(MEDIA_STATE5_PROBE_READINESS, 0l) == 0l)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x13u);
        return 2u;
    }
    return 0u;
}
