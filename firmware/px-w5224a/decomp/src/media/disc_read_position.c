#include "disc_read_position.h"
#include "../platform/controller_operating_state_initialization.h"
#include "../platform/controller_parameters.h"
#include "../platform/drive_control_reset.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/media_interrupt_leaves.h"
#include "../platform/radial_model.h"
#include "decoder.h"
#include "operating_code_selection.h"
#include "radial_step_scale_calibration.h"
#include "read_acquisition_distance.h"

extern void YieldCoroutine1(void);

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/disc_read_position_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadDiscPositionU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteDiscPositionU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void ApplyReadPositionModeBits(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EF9) >= 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 0x20u));
        ApplyControllerParameterD1ModeBits();
    }
}

static void SelectReadPositionOperatingCode(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        SelectAndApplyOperatingCodeForMediaState(1u);
    else
        SelectAndConfigureOperatingCodeForThreshold(ReadDiscPositionU32(FW_NEAR_UNKNOWN_0EF5));
}

/* RAM:fc818b */
u8 AcquireDiscReadPosition(void)
{
    u8 preserve_recovery_speed = 1u;
    u8 packet_result;
    u8 return_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
    SetP1Bits0And1();
    SET_READ_POSITION_P1_TIMER_PINS();
    ResetControllerPorts8087Through808B();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
    {
        SelectAndApplyOperatingCodeForMediaState(1u);
        if(ReadDiscPositionU32(FW_NEAR_UNKNOWN_FC37) != 0ul && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u)
            WriteDiscPositionU32(FW_NEAR_UNKNOWN_0CF5, ReadDiscPositionU32(FW_NEAR_UNKNOWN_FC37));
    }
    else
    {
        SelectAndConfigureOperatingCodeForThreshold(ReadDiscPositionU32(FW_NEAR_UNKNOWN_0EF5));
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0u);
        if(ReadDiscPositionU32(FW_NEAR_UNKNOWN_FC33) != 0ul && (FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u)
            WriteDiscPositionU32(FW_NEAR_UNKNOWN_0CF5, ReadDiscPositionU32(FW_NEAR_UNKNOWN_FC33));
    }

    InitializeReadAcquisitionRadialState();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EDA, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED7, 0u);
    WriteDiscPositionU32(FW_NEAR_UNKNOWN_0EE9, ReadDiscPositionU32(FW_NEAR_UNKNOWN_1235));

    if((s32)ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B) < 0l || ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0ED7, 1u);
        if(ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
            WriteDiscPositionU32(FW_NEAR_UNKNOWN_0EED, 0x10000000ul);
    }
    else if(ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B) <= 30ul)
    {
        WriteDiscPositionU32(FW_NEAR_UNKNOWN_0E3B, ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B) + (u32)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE4) * 3u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EDA, 1u);
    }

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D3, 0u);

retry_packet:
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1030, IsRadialPositionEstimateAtMost3542());
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        packet_result = AcquireAndValidateReadPositionPacket();
    else
        packet_result = AcquireReadPositionPacketWithChannelSetup();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
            ApplyControllerD1ModeBitsAfterPulseTest();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 0x7fu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
        return_value = 0xffu;
        goto finish;
    }

    if(packet_result == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED8) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u
            && (s32)ReadDiscPositionU32(FW_NEAR_UNKNOWN_1235) < (s32)ReadDiscPositionU32(FW_NEAR_UNKNOWN_12D8) && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) < FirmwareReadNear8(FW_NEAR_UNKNOWN_1233)
            && (FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 0x0cu) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
            return_value = 0u;
            goto finish;
        }

        AccumulateRadialStepScaleCalibrationSample();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3E) >= 5u && ReduceReadSpeedForAcquisitionRecovery(0u) == 1u)
        {
            SetControllerChannel0ValueForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
            SelectAndApplyOperatingCodeForMediaState(0u);
            ResetReadAcquisitionRecoveryState();
            goto retry_packet;
        }

        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1207) <= 0)
        {
            ApplyReadPositionModeBits();
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE2) == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
                return_value = 1u;
                goto finish;
            }
            if(ReduceReadSpeedForAcquisitionRecovery(0u) == 1u)
            {
                SelectAndApplyOperatingCodeForMediaState(0u);
                ResetReadAcquisitionRecoveryState();
                goto retry_packet;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
                ApplyControllerD1ModeBitsAfterPulseTest();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
            return_value = 0u;
            goto finish;
        }

        WriteDiscPositionU32(FW_NEAR_UNKNOWN_0EE9, ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B));
        packet_result = PositionPickupForAcquisition();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) != 1u)
            WriteDiscPositionU32(FW_NEAR_UNKNOWN_0E3B, ReadDiscPositionU32(FW_NEAR_UNKNOWN_0EED));
        ApplyReadPositionModeBits();

        if(packet_result == 1u)
        {
            if(ApplyPendingReadAcquisitionTargetNudge() != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
                return_value = 1u;
                goto finish;
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EDA) == 1u)
            {
                MovePickupByByteDistance(4u, 1u);
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
                    StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018007, 8u);
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
                ApplyControllerD1ModeBitsAfterPulseTest();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 0u);
            return_value = 1u;
            goto finish;
        }

        YieldCoroutine1();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            SelectAndApplyOperatingCodeForMediaState(1u);
        else
            SelectAndConfigureOperatingCodeForThreshold(ReadDiscPositionU32(FW_NEAR_UNKNOWN_0E3B));
        goto retry_packet;
    }

    ApplyReadPositionModeBits();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) == FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3B))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3C) | 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3B, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 7u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 3u)
    {
        if(TestP1Cex0PulseSuccessThreshold(20u) == 1u)
        {
            SetControllerChannel0ValueForOperatingCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
            if(ReduceReadSpeedForAcquisitionRecovery(0u) == 1u)
            {
                SelectAndApplyOperatingCodeForMediaState(0u);
                ResetReadAcquisitionRecoveryState();
                goto retry_packet;
            }
        }
        else
        {
            preserve_recovery_speed = 0u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 8u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
    }

    ResetDriveControlStateToEight();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3D, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3D) - 1u));
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3D) <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
        return_value = 0u;
        goto finish;
    }
    if(InitializeControllerOperatingStateAndRunCalibration() == 0u)
    {
        ResetDriveControlStateToEight();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 7u);
        return_value = 0u;
        goto finish;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED8) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, 5u);
        return_value = 0u;
        goto finish;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 2u) != 0u)
    {
        return_value = 0u;
        goto finish;
    }

    ResetReadAcquisitionRecoveryState();
    if(preserve_recovery_speed == 1u)
        (void)ReduceReadSpeedForAcquisitionRecovery(1u);
    SelectReadPositionOperatingCode();
    goto retry_packet;

finish:
    ConfigureControllerPorts80D0D1D3ForCurrentState();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
        FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01802B, 0x80u);
    return return_value;
}
