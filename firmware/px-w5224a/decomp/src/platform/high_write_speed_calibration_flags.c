#include "high_write_speed_calibration_flags.h"
#include "firmware_memory.h"

static u16 ReadHighSpeedCalibrationWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void SetHighSpeedCalibrationFlag(u16 flag)
{
    u16 value = ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_FAD7);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD7, (u8)((value | flag) >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAD8, (u8)(value | flag));
}

static u16 ConvertHighSpeedCalibrationValue(float value)
{
    return (u16)(s32)value;
}

/* RAM:f8a5ae */
void PLEXTOR_NEAR_FUNCTION UpdateHighWriteSpeedFlagsFromCalibrationThresholds(void)
{
    float scaled_position;
    float positive_distance;
    float calibration_value;
    float calculated_threshold;
    u16 threshold0;
    u16 threshold1;
    u16 threshold2;
    u16 threshold3;
    u16 threshold4;
    u16 flags;
    u16 value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 16u)
        return;

    scaled_position = ((float)(s16)(u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F99F) + 0x78u) - (float)(s16)ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_0FD5)) * 8.35f;
    scaled_position = scaled_position / 1000.0f;
    scaled_position = scaled_position * 41.6667f;
    positive_distance = 70.0f - scaled_position;
    if(positive_distance <= 0.0f)
        positive_distance = 0.0f;
    else
        positive_distance = positive_distance * (float)(s16)ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_0FFE);

    calibration_value = positive_distance + (float)(s16)(u16)(ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_0FD9) - ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_FACF));

    threshold0 = (u16)(ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_E752) + 0x0100u);
    threshold1 = (u16)(ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_E754) + 0x0100u);
    threshold2 = (u16)(ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_E756) + 0x0100u);
    threshold3 = (u16)(ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_E758) + 0x0100u);
    calculated_threshold = ((float)(s16)ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_F9FA) / 100.0f) * (float)(s16)ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_0FFE) + 256.0f;
    threshold4 = ConvertHighSpeedCalibrationValue(calculated_threshold);

    flags = ReadHighSpeedCalibrationWord(FW_NEAR_UNKNOWN_FAD7);
    if(((flags & 3u) != 3u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u && (flags & 0x1000u) == 0u)
        return;

    value = ConvertHighSpeedCalibrationValue(calibration_value);
    if(value < threshold0)
        SetHighSpeedCalibrationFlag(0x0200u);
    else if(ConvertHighSpeedCalibrationValue(calibration_value) < threshold1)
        SetHighSpeedCalibrationFlag(0x0100u);
    else if(ConvertHighSpeedCalibrationValue(calibration_value) < threshold2)
        SetHighSpeedCalibrationFlag(0x0080u);
    else if(ConvertHighSpeedCalibrationValue(calibration_value) < threshold3)
        SetHighSpeedCalibrationFlag(0x0040u);
    else if(ConvertHighSpeedCalibrationValue(calibration_value) < threshold4)
        SetHighSpeedCalibrationFlag(0x0020u);
}
