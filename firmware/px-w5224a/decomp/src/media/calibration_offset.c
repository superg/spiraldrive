#include "calibration_offset.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"
#include "../platform/write_speed_reduction.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/calibration_offset_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCalibrationOffsetWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteCalibrationResultWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteCalibrationTarget(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f89fb5 */
u8 PLEXTOR_NEAR_FUNCTION SetMediaCalibrationRetryFlag(void)
{
    WriteCalibrationResultWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAD7) | 0x0008u));
    return 0u;
}

/* RAM:f89db9 */
void PLEXTOR_NEAR_FUNCTION InitializeMediaState6CalibrationRetryState(void)
{
    WriteCalibrationTarget(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF2, 0x1bu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
}

static fw_addr_t SelectCalibrationOffsetAddress(u8 write_speed)
{
    switch(write_speed)
    {
    case 1u:
        return FW_NEAR_UNKNOWN_FADC;
    case 4u:
        return FW_NEAR_UNKNOWN_FADD;
    case 8u:
        return FW_NEAR_UNKNOWN_FADE;
    case 16u:
    case 20u:
    case 24u:
    case 32u:
    case 40u:
    case 52u:
        return FW_NEAR_UNKNOWN_FADF;
    default:
        return 0ul;
    }
}

/* RAM:f89a08 */
void PLEXTOR_NEAR_FUNCTION AdjustEncodedMediaCalibrationOffsetForCurrentWriteSpeed(void)
{
    u8 adjustment = (ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAAC) & 0x0002u) != 0u ? 1u : 2u;
    fw_addr_t offset_address = SelectCalibrationOffsetAddress(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    u8 adjusted_offset = 0u;
    u8 calibration_state;

    if(offset_address != 0ul)
        adjusted_offset = (u8)DecodeControllerEncodedSignedOffset(FirmwareRead8(offset_address));

    calibration_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2);
    if(calibration_state == 2u)
        adjusted_offset = (u8)(adjusted_offset - adjustment);
    else if(calibration_state == 3u)
        adjusted_offset = (u8)(adjusted_offset + adjustment);

    offset_address = SelectCalibrationOffsetAddress(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
    if(offset_address != 0ul)
        FirmwareWrite8(offset_address, EncodeSignedByteAsSignMagnitude((s8)adjusted_offset));
}

/* RAM:f89c73 */
u8 PLEXTOR_NEAR_FUNCTION EvaluateMediaState6CalibrationResultAndRetry(void)
{
    u8 observed_result = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF2);
    u8 requested_result;

    if(observed_result == 0x1eu)
    {
        WriteCalibrationTarget(FW_NEAR_UNKNOWN_0E3B, 0x00000096ul);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, observed_result);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAAB, observed_result);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 3u);
        return 0u;
    }

    requested_result = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF3);
    if(requested_result == observed_result)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) != 0u)
        {
            WriteCalibrationResultWord(FW_NEAR_UNKNOWN_FAD7, (u16)(ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAD7) | 0x0008u));
            INITIALIZE_STATE6_CALIBRATION_RETRY();
            return 0u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 12u)
        {
            u16 calibration_word = ReadCalibrationOffsetWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92EB));

            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF3) == 3u)
                calibration_word = ReadCalibrationOffsetWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF932D));
            WriteCalibrationResultWord(FW_NEAR_UNKNOWN_FACF, calibration_word);
            return 1u;
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF3) != 3u)
        {
            WriteCalibrationResultWord(FW_NEAR_UNKNOWN_FACF, ReadCalibrationOffsetWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9337)));
            return 1u;
        }

        {
            u8 reduction_enabled;

            if((((ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAD7) & 0x0001u) == 0u || (ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAD7) & 0x0002u) == 0u)
                   && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
                && (ReadCalibrationOffsetWord(FW_NEAR_UNKNOWN_FAD7) & 0x1000u) == 0u)
                reduction_enabled = 0u;
            else
                reduction_enabled = 1u;

            if(TRY_REDUCE_STATE6_CALIBRATION(reduction_enabled) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 6u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF1) + 1u));
                return 0u;
            }
        }

        if(SET_STATE6_CALIBRATION_RETRY_FLAG() == 0u)
        {
            INITIALIZE_STATE6_CALIBRATION_RETRY();
            return 0u;
        }
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF1) >= 5u)
    {
        INITIALIZE_STATE6_CALIBRATION_RETRY();
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF1) < 10u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 6u);
        ADJUST_STATE6_CALIBRATION_OFFSET();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF1, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF1) + 1u));
        return 0u;
    }

    INITIALIZE_STATE6_CALIBRATION_RETRY();
    return 0u;
}
