#include "media_state6_controller_calibration_sequence.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/media_state6_controller_calibration_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadState6SequenceU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadState6SequenceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteState6SequenceU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteState6SequenceU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static s32 State6SequenceTarget(u32 adjustment)
{
    return (s32)(ReadState6SequenceU32(FW_NEAR_UNKNOWN_0FE9) - adjustment - (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE) * 15u));
}

static void ClearState6PositionSamples(void)
{
    u8 index;

    for(index = 0u; index < 16u; ++index)
        WriteState6SequenceU16((fw_addr_t)(FW_NEAR_UNKNOWN_100A + (fw_addr_t)((u16)index * 2u)), 0u);
}

static void ResetState6SequenceStrobe(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_131C, 0u);
    SEQUENCE_STROBE(FW_CONTROLLER_UNKNOWN_018080, 0u);
}

static void PublishState6SequenceFailure(u8 result, u8 clear_busy)
{
    WriteState6SequenceU32(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
    if(clear_busy != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, result);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
}

/* RAM:f87dca */
u8 PLEXTOR_NEAR_FUNCTION ExecuteMediaState6ControllerCalibrationSequence(void)
{
    u8 result;
    u8 attempt_count;
    u8 previous_result;
    u8 no_bit4_count;
    u8 nonzero_count;
    s32 target;

    SEQUENCE_ENABLE(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 10u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        if(SEQUENCE_POSITION(0u, 1u) != 1u)
            return 0u;
        SEQUENCE_CONFIGURE(0u);
        SEQUENCE_APPLY();
        result = SEQUENCE_ATIP_WINDOW();
        if(result == 0u)
            break;
        if(result != 2u)
            return 0u;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
    {
        PublishState6SequenceFailure(5u, 1u);
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 6u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        target = State6SequenceTarget(0x0488ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FEF, 3u);
        if(SEQUENCE_POSITION(1u, 0u) != 1u)
            return 0u;
        SEQUENCE_CONFIGURE(0u);
        SEQUENCE_APPLY();
        result = SEQUENCE_NEAR_TARGET(target);
        if(result == 0u)
            break;
        if(result != 2u)
            return 0u;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) <= 0)
    {
        PublishState6SequenceFailure(5u, 0u);
        return 0u;
    }

    attempt_count = 0u;
    for(;;)
    {
        ClearState6PositionSamples();
        if(attempt_count >= 3u)
        {
            PublishState6SequenceFailure(0x1eu, 0u);
            return 0u;
        }
        target = State6SequenceTarget(0x0488ul);
        if(SEQUENCE_POSITION(2u, 1u) != 1u)
            return 0u;
        if(SEQUENCE_READ_PATH(target) != 0u)
        {
            ResetState6SequenceStrobe();
            ++attempt_count;
            continue;
        }

        WriteState6SequenceU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadState6SequenceU16(FW_NEAR_UNKNOWN_0FF5) | 1u));
        SEQUENCE_DERIVE_THRESHOLD();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF4, FirmwareReadNear8(FW_NEAR_UNKNOWN_F962));
        SEQUENCE_WRITE_E2(0x50u);
        result = SEQUENCE_POSITION_WINDOW();
        SEQUENCE_WRITE_E2(0x58u);
        WriteState6SequenceU16(FW_NEAR_UNKNOWN_0FF5, (u16)(ReadState6SequenceU16(FW_NEAR_UNKNOWN_0FF5) & 0xfffeu));
        ResetState6SequenceStrobe();
        SEQUENCE_RESTORE(1u);
        if(result != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) != 0u)
            break;
        ++attempt_count;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE1, 0u);
    attempt_count = 0u;
    WriteState6SequenceU16(FW_NEAR_UNKNOWN_0FD3, 0u);
    previous_result = 0u;
    result = 0u;
    no_bit4_count = 0u;
    nonzero_count = 0u;

    for(;;)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) != 0u)
            break;
        if(attempt_count >= 4u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 0x1eu);
            break;
        }
        if(SEQUENCE_POSITION(3u, 1u) != 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 0x1eu);
            break;
        }
        SEQUENCE_CONFIGURE(0u);
        SEQUENCE_APPLY();
        SEQUENCE_WRITE_D4_E2(0x74u);
        SEQUENCE_RAISE_F4();
        result = SEQUENCE_ACQUIRE_SAMPLES();
        SEQUENCE_RESTORE_D4_E2();
        SEQUENCE_LOWER_F4();

        if(result == 1u)
        {
            u16 calibrated_word = ReadState6SequenceU16(FW_NEAR_UNKNOWN_FACF);

            WriteState6SequenceU16(FW_NEAR_UNKNOWN_1589, calibrated_word);
            WriteState6SequenceU16(FW_NEAR_UNKNOWN_EF85, calibrated_word);
            break;
        }
        if(result == 0x0bu)
        {
            WriteState6SequenceU32(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0x1bu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, 0x1bu);
            return result;
        }
        if(result == 5u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF3, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 3u);
            break;
        }

        ++attempt_count;
        if((ReadState6SequenceU16(FW_NEAR_UNKNOWN_0FF5) & 0x0010u) == 0u)
        {
            ++no_bit4_count;
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_158E) & 0x08u) != 0u)
                no_bit4_count = 2u;
            if(no_bit4_count > 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 4u);
                break;
            }
        }
        if(result != 0u)
        {
            if(result == 3u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) >= 16u && SEQUENCE_FIXED_SUCCESS() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF3, 3u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, result);
                break;
            }
            ++nonzero_count;
            if(nonzero_count > 2u || result == previous_result)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, result);
                break;
            }
            previous_result = result;
        }
        if(attempt_count == 4u && previous_result != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, previous_result);
            break;
        }
    }

    attempt_count = 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEF) == 1u)
    {
        while(attempt_count < 10u)
        {
            target = (s32)(ReadState6SequenceU32(FW_NEAR_UNKNOWN_0FE9) - 0x0406ul - FirmwareReadNear8(FW_NEAR_UNKNOWN_0FEE));
            if(SEQUENCE_POSITION(4u, 1u) != 1u)
            {
                attempt_count = (u8)(attempt_count + 10u);
            }
            else if(SEQUENCE_READ_PATH(target) != 0u)
            {
                ResetState6SequenceStrobe();
                ++attempt_count;
            }
            else
            {
                result = SEQUENCE_POSITION_WINDOW();
                ResetState6SequenceStrobe();
                SEQUENCE_RESTORE(1u);
                if(result != 0u)
                    break;
                ++attempt_count;
            }
        }
        if(attempt_count >= 10u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
    }

    SEQUENCE_SET_COMPLETION_BIT();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2) == 0u)
        return 1u;
    return SEQUENCE_EVALUATE_RESULT();
}
