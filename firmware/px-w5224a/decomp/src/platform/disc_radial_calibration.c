#include "disc_radial_calibration.h"
#include "../media/disc_read_position.h"
#include "controller_parameters.h"
#include "controller_position_average.h"
#include "controller_q_packet_process.h"
#include "drive_control_reset.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "radial_model.h"
#include "radial_position_table.h"
#include "radial_refinement.h"
#include "status_mapping.h"

extern void YieldCoroutine1(void);

static u16 ReadDiscRadialU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadDiscRadialU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteDiscRadialU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteDiscRadialU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static s32 GetLeadInFirstTrackFramePosition(void)
{
    u16 tuple_address = (u16)(0xf2b9u + (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) * 6u);

    return ConvertBinaryMsfToAdjustedFramePosition((fw_addr_t)tuple_address);
}

static u8 AcquireInitialRadialMeasurement(void)
{
    u8 use_lead_in_target = 0u;

    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1203) > 0)
    {
        s32 target = GetLeadInFirstTrackFramePosition();

        WriteDiscRadialU32(FW_NEAR_UNKNOWN_0E3B, (u32)target);
        if(use_lead_in_target == 0u)
        {
            target = 0x93l;
            WriteDiscRadialU32(FW_NEAR_UNKNOWN_0E3B, (u32)target);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED6, 1u);
        if(AcquireDiscReadPosition() == 1u)
        {
            if(IsDecodedFramePositionWithinTwoFramesOfTarget(target) == 1u)
            {
                u16 measurement;

                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC32, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC32) | 1u));
                measurement = MeasureAverageControllerPositionWord();
                SetRadialModelMeasurementAndScale(measurement);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC32, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC32) & 0xfeu));
                return 1u;
            }
        }
        else
        {
            if(ResetDriveControlAndInitializeControllerCalibration() == 0u)
                return 0xffu;
            if(use_lead_in_target == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
            {
                use_lead_in_target = 1u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 7u);
            }
        }
    }
    return 0u;
}

static void PollRadialCalibrationControllerQ(void)
{
    u8 status;

    RearmControllerQPacketCaptureWithTimeout();
    do
    {
        status = ProcessCapturedControllerQPacket();
        if(status == 0xfeu)
            YieldCoroutine1();
    } while(status == 0xfeu);
}

static void RecalculateRadialStepScale(void)
{
    u32 product = (u32)ReadDiscRadialU16(PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F848FE)) * (u32)ReadDiscRadialU16(FW_NEAR_UNKNOWN_0CF9);

    WriteDiscRadialU16(FW_NEAR_UNKNOWN_1000, (u16)(product / 1600ul));
}

/* RAM:fce0d4 */
u8 CalibrateDiscRadialPositionModel(void)
{
    u8 initial_result;
    u8 span_result;

    WriteDiscRadialU16(FW_NEAR_UNKNOWN_F99C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E44, 4u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 2u);
        if(AcquireDiscPositionAtTargetWithRetryBudget(0x96l) == 0u)
            return 0x11u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 7u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 0x19u);
    initial_result = AcquireInitialRadialMeasurement();
    if(initial_result != 1u)
    {
        if(initial_result == 0xffu)
            return 0x11u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u)
            return 0u;
        ResetDriveControlStateToEight();
        return 0x11u;
    }

    UpdateP1Cex0PulseCalibrationStatus();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x20u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1203, 3u);
        if(AcquireDiscPositionAtTargetWithRetryBudget(0x84l) == 0u)
            return 0x11u;
        PollRadialCalibrationControllerQ();
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x80u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xdfu));
    }

    UpdateReadAcquisitionMeasurementFrom8Samples();
    span_result = CalibrateRadialModelSpanAtFrame90000();
    if(span_result != 0u)
        return span_result;
    RebuildRadialPositionTableAndThresholds();
    RecalculateRadialStepScale();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x40u) == 0u
        && (((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC18) & 0x08u) == 0u) || FirmwareReadNear8(FW_NEAR_UNKNOWN_F553) == 0xffu) && (s32)ReadDiscRadialU32(FW_NEAR_UNKNOWN_12C1) >= 99000l)
        RefineRadialCalibrationAtFrame150();

    WriteDiscRadialU32(FW_NEAR_UNKNOWN_FBE6, ReadDiscRadialU32(FW_NEAR_UNKNOWN_0CF5));
    WriteDiscRadialU16(FW_NEAR_UNKNOWN_FBEA, ReadDiscRadialU16(FW_NEAR_UNKNOWN_0CF9));
    return 0u;
}
