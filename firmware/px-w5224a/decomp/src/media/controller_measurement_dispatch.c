#include "controller_measurement_dispatch.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/controller_measurement_dispatch_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra address. */
static u32 ReadMeasurementDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static u16 ReadMeasurementWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteMeasurementWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

/* RAM:f97c26 */
u16 ComputeBoundedControllerAdjustmentCode(float measurement_value, u16 reference_word)
{
    u16 reference_delta = (u16)(reference_word - ReadMeasurementWord(FW_NEAR_UNKNOWN_F074));
    double numerator = ((double)measurement_value - (double)reference_delta * 0.001) + 12.0;
    double denominator = ((double)reference_delta * 0.0089 + 14.3) * 0.01;
    float rounded_input = (float)(numerator / denominator + (double)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A));
    s32 rounded_value = (s32)rounded_input;
    u8 result;

    if((float)rounded_value < rounded_input)
        rounded_value++;
    result = (u8)rounded_value;
    if(result > 126u)
        result = 126u;
    return result;
}

/* RAM:f95c1c */
u8 PLEXTOR_NEAR_FUNCTION EvaluateControllerMeasurementSeriesForCurrentMode(void)
{
    u8 fallback_result = CAPTURE_MEASUREMENT_FALLBACK();
    u8 mode;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 0u);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0x20u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 6u)
            TRANSFORM_MEASUREMENT_SERIES04_TO05_MODE6();
        else
            EVALUATE_MEASUREMENT_SERIES00_AND01();
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 4u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 6u)
            TRANSFORM_MEASUREMENT_SERIES04_TO05_MODE6();
        else
            EVALUATE_MEASUREMENT_SERIES00_AND01();
        return 1u;
    }

    mode = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u);
    if(mode == 2u || mode == 3u || mode == 6u)
    {
        TRANSFORM_MEASUREMENT_SERIES04_TO05();
        return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) != 2u);
    }
    if(mode == 4u)
    {
        SMOOTH_MEASUREMENT_SERIES02();
        return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) != 2u);
    }
    if(mode == 1u)
    {
        EVALUATE_MEASUREMENT_SERIES00_AND01();
        return 1u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF80, 0u);
    return fallback_result;
}

/* RAM:f95789 */
void PLEXTOR_NEAR_FUNCTION CaptureControllerMeasurementPacketInTargetWindow(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF76, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0x3fu) | 0x80u));

    for(;;)
    {
        u8 packet_status;
        s32 target_position;
        s32 current_position;

        ARM_TARGET_WINDOW_PACKET_CAPTURE();
        do
        {
            packet_status = PARSE_TARGET_WINDOW_PACKET();
        } while(packet_status == 0xfeu);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfdu));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
                COMPLETE_TARGET_WINDOW_MEASUREMENT();
            }
            return;
        }

        target_position = (s32)(ReadMeasurementDword(FW_NEAR_UNKNOWN_103D) - 2ul);
        current_position = (s32)ReadMeasurementDword(FW_NEAR_UNKNOWN_1235);
        if(current_position == target_position)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 2u));
        }

        target_position = (s32)(ReadMeasurementDword(FW_NEAR_UNKNOWN_103D) + 15ul);
        current_position = (s32)ReadMeasurementDword(FW_NEAR_UNKNOWN_1235);
        if(current_position < target_position)
            continue;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfdu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
            COMPLETE_TARGET_WINDOW_MEASUREMENT();
        }
        return;
    }
}

/* RAM:f95837 */
u8 PLEXTOR_NEAR_FUNCTION CaptureControllerMeasurementPacketAtComputedTarget(void)
{
    u8 complete = 0u;
    u16 adjusted_bound;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF76, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF75, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0x3fu) | 0x80u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
    {
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F066, 0x0050u);
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F068, 0x0cc0u);
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
    {
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F066, 0x0046u);
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F068, 0x0bb0u);
    }
    else
    {
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F066, 0x003bu);
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F068, 0x0b00u);
    }

    adjusted_bound = FirmwareReadNear8(FW_NEAR_UNKNOWN_F06A);
    adjusted_bound = (u16)(adjusted_bound + ReadMeasurementWord(FW_NEAR_UNKNOWN_F066));
    WriteMeasurementWord(FW_NEAR_UNKNOWN_F066, adjusted_bound);
    adjusted_bound = ReadMeasurementWord(FW_NEAR_UNKNOWN_F074);
    adjusted_bound = (u16)(adjusted_bound + ReadMeasurementWord(FW_NEAR_UNKNOWN_F068));
    WriteMeasurementWord(FW_NEAR_UNKNOWN_F068, adjusted_bound);

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF73) & 7u) == 4u && FirmwareReadNear8(FW_NEAR_UNKNOWN_EF80) != 1u)
        WriteMeasurementWord(FW_NEAR_UNKNOWN_F066, 0u);

    do
    {
        u8 packet_status;
        s32 target_position;
        s32 current_position;

        ARM_TARGET_WINDOW_PACKET_CAPTURE();
        do
        {
            packet_status = PARSE_TARGET_WINDOW_PACKET();
        } while(packet_status == 0xfeu);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7E, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfbu));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
                COMPLETE_TARGET_WINDOW_MEASUREMENT();
            }
            return 0u;
        }

        target_position = (s32)(ReadMeasurementDword(FW_NEAR_UNKNOWN_103D) - 2ul);
        current_position = (s32)ReadMeasurementDword(FW_NEAR_UNKNOWN_1235);
        if(current_position == target_position)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) | 4u));
        }

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 4u) == 0u)
            target_position = (s32)(ReadMeasurementDword(FW_NEAR_UNKNOWN_103D) + 1ul);
        else
            target_position = (s32)(ReadMeasurementDword(FW_NEAR_UNKNOWN_103D) + 2ul);
        current_position = (s32)ReadMeasurementDword(FW_NEAR_UNKNOWN_1235);
        if(current_position >= target_position)
            complete = 1u;
    } while(complete != 1u);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1039, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1039) & 0xfbu));

    complete = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF7F) != 0u);
    if(complete == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF7F, 3u);
        COMPLETE_TARGET_WINDOW_MEASUREMENT();
    }
    return complete;
}
