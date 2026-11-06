#include "read_acquisition_calibration_sequence.h"
#include "../media/decoder.h"
#include "arithmetic.h"
#include "controller_measurement_range.h"
#include "controller_parameters.h"
#include "drive_control_reset.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/read_acquisition_calibration_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadCalibrationU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCalibrationU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static u8 ReadCalibrationMode(void)
{
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F08C) & 3u);
}

static void SetReadPathMeasurementActive(u8 active)
{
    if(active != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) | 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0x40u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0x40u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1301, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 0xfeu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EFC, 0u);
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_018005, 0u);
    }
}

static u8 SelectReadCalibrationOperatingCode(void)
{
    u8 controller_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88);

    if(controller_state == 2u || controller_state == 3u)
        return ReadCalibrationMode() == 2u ? 8u : 0x14u;
    if(controller_state == 1u)
        return FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 8u ? 4u : 8u;
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47) < 4u ? 2u : 4u;
}

/* RAM:fa6eeb */
u8 PLEXTOR_NEAR_FUNCTION ExecuteReadAcquisitionCalibrationSequence(void)
{
    u8 initialize_countdown = 1u;
    u8 restart_timeout = 1u;
    u8 attempt_count = 0u;
    u8 initial_mode;
    float positioning_scale = 1.0f;
    u32 range_start;

    CALIBRATION_SET_ENABLE(1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 0xfcu) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 1u));

    initial_mode = ReadCalibrationMode();
    if(initial_mode == 3u)
    {
        range_start = ReadCalibrationU32(FW_NEAR_UNKNOWN_114F) - 0x0a65ul;
        WriteCalibrationU32(FW_NEAR_UNKNOWN_F091, range_start);
        WriteCalibrationU32(FW_NEAR_UNKNOWN_F095, ReadCalibrationU32(FW_NEAR_CURRENT_FRAME_POSITION) + 0x1a5ful);
    }
    else if(initial_mode == 2u)
    {
        range_start = ReadCalibrationU32(FW_NEAR_UNKNOWN_114F) - 0x0a65ul;
        WriteCalibrationU32(FW_NEAR_UNKNOWN_F091, range_start);
        WriteCalibrationU32(FW_NEAR_UNKNOWN_F095, ReadCalibrationU32(FW_NEAR_UNKNOWN_114F) - 0x0405ul);
    }
    WriteCalibrationU32(FW_NEAR_UNKNOWN_0E3B, ReadCalibrationU32(FW_NEAR_UNKNOWN_F091) - 75ul);

    for(;;)
    {
        u8 acquisition_result;
        u8 operating_code;
        u8 range_result;
        u32 minimum_start;
        u32 positioning_offset;
        s32 positioning_target;

        if(attempt_count >= 3u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
            return 0u;
        }

        operating_code = SelectReadCalibrationOperatingCode();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, operating_code);
        range_start = ReadCalibrationU32(FW_NEAR_UNKNOWN_F091);
        WriteCalibrationU32(FW_NEAR_UNKNOWN_0E3B, range_start - CALIBRATION_MULTIPLY((u32)operating_code, 75ul));

        minimum_start = ReadCalibrationU32(FW_NEAR_UNKNOWN_114F) - 0x0b2cul;
        if((s32)ReadCalibrationU32(FW_NEAR_UNKNOWN_0E3B) <= (s32)minimum_start)
        {
            u16 frame_span = (u16)((u16)operating_code * 75u);

            WriteCalibrationU32(FW_NEAR_UNKNOWN_0E3B, minimum_start);
            positioning_scale = (float)(s32)(range_start - minimum_start) / (float)frame_span;
        }

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 0u);
        acquisition_result = CALIBRATION_ACQUIRE_POSITION();

        if(acquisition_result == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));
            if(attempt_count >= 3u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                return 0u;
            }
            CALIBRATION_RESET_DRIVE_CONTROL();
            if(CALIBRATION_INITIALIZE_CONTROLLER() == 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 7u);
                return 0u;
            }
            CALIBRATION_SET_ENABLE(1u);
            ++attempt_count;
            if(ReadCalibrationMode() == 2u)
            {
                u32 increment = CALIBRATION_MULTIPLY((u32)operating_code, CALIBRATION_MULTIPLY(150ul, (u32)attempt_count));

                range_start = ReadCalibrationU32(FW_NEAR_UNKNOWN_F091) + increment;
                WriteCalibrationU32(FW_NEAR_UNKNOWN_F091, range_start);
                WriteCalibrationU32(FW_NEAR_UNKNOWN_0E3B, range_start - CALIBRATION_MULTIPLY((u32)operating_code, 75ul));
            }
            continue;
        }

        if(acquisition_result == 1u)
        {
            if(CALIBRATION_CHECK_TOLERANCE(restart_timeout) == 0u)
            {
                restart_timeout = 0u;
                continue;
            }
            if(initialize_countdown != 0u)
            {
                initialize_countdown = 0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1217, 0x32u);
            }
            if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1217) > 0)
                continue;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 0u);
        }
        else if(acquisition_result == 0xffu && ReadCalibrationMode() != 2u)
        {
            return 1u;
        }

        CALIBRATION_APPLY_OPERATING_CODE();
        positioning_offset = (u32)(s32)((float)(u16)((u16)operating_code * 20u) * positioning_scale);
        positioning_target = (s32)(ReadCalibrationU32(FW_NEAR_UNKNOWN_F091) - positioning_offset);
        if(CALIBRATION_WAIT_FOR_TARGET(positioning_target) != 1u)
        {
            ++attempt_count;
            continue;
        }

        CALIBRATION_CONFIGURE_PORTS(1u);
        CALIBRATION_APPLY_OPERATING_CODE();
        if(ReadCalibrationMode() == 1u)
            CALIBRATION_CONFIGURE_MODE1();
        CALIBRATION_RESET_DATA_PATH();
        CALIBRATION_CONFIGURE_PROFILE();
        CALIBRATION_STROBE(FW_CONTROLLER_UNKNOWN_018080, 0x80u);
        CALIBRATION_STROBE(FW_CONTROLLER_UNKNOWN_0180B9, 0x40u);
        CALIBRATION_STROBE(FW_CONTROLLER_UNKNOWN_0180BA, 0x50u);
        CALIBRATION_STROBE(FW_CONTROLLER_UNKNOWN_0180BD, 0x81u);
        CALIBRATION_STROBE(FW_CONTROLLER_UNKNOWN_0180BD, 0x85u);

        if(CALIBRATION_PROBE_READINESS() == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 8u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F08D, 8u);
            ++attempt_count;
            continue;
        }

        SetReadPathMeasurementActive(1u);
        range_result = CALIBRATION_WAIT_RANGE_START();
        SetReadPathMeasurementActive(0u);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u && ReadCalibrationMode() != 2u)
        {
            CALIBRATION_RESTORE(1u);
            return 1u;
        }
        if(range_result != 1u)
        {
            ++attempt_count;
            CALIBRATION_RESTORE(1u);
            continue;
        }

        if(((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) >> 3) & 1u) != 0u && FirmwareRead8(PLEXTOR_CONFIGURATION_FLASH_ADDRESS(FW_ROM_UNKNOWN_F7C01F)) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
            CALIBRATION_SET_P3_RX(1u);
            CALIBRATION_SET_P3_TX(0u);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) | 0x10u));
            CALIBRATION_SET_P3_RX(0u);
            CALIBRATION_SET_P3_TX(1u);
        }

        SetReadPathMeasurementActive(1u);
        range_result = CALIBRATION_ACQUIRE_RANGE();
        SetReadPathMeasurementActive(0u);
        CALIBRATION_RESTORE(1u);

        if(ReadCalibrationMode() != 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1302, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0xefu));
            CALIBRATION_SET_P3_RX(1u);
            CALIBRATION_SET_P3_TX(0u);
        }
        return range_result;
    }
}
