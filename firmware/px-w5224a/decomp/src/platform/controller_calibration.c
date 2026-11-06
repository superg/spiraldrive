#include "controller_calibration.h"
#include "arithmetic.h"
#include "controller_mapping.h"
#include "controller_parameters.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

extern s16 SelectControllerWord0255TargetForCurrentState(void);
extern void PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7ClearWithoutTimeout(void);
extern u8 PLEXTOR_NEAR_FUNCTION WaitControllerPort80F8Bit7Clear10AndYield(void);

#define PLEXTOR_BINDING_SECTION 2
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 3
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 4
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 5
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define PLEXTOR_BINDING_SECTION 6
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static s32 ReadCurrentDecodedFramePosition(void)
{
    u8 byte0 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1235);
    u8 byte1 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1236);
    u8 byte2 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1237);
    u8 byte3 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1238);

    return (s32)(((u32)byte0 << 24) | ((u32)byte1 << 16) | ((u32)byte2 << 8) | byte3);
}

static u8 CurrentFrameProbePort80CFValue(void)
{
    return FirmwareReadNear8(FW_NEAR_UNKNOWN_1157) == 2u ? 0xc4u : 0xc0u;
}

/* RAM:f89367 */
u8 PLEXTOR_NEAR_FUNCTION ProbeControllerReadPathAtCurrentFrame(void)
{
    u8 index;

    CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180C0, 0u);
    CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180CF, CurrentFrameProbePort80CFValue());

    for(index = 0u; index < 3u; ++index)
    {
        s32 target_frame_position = ReadCurrentDecodedFramePosition();

        if(index == 0u)
            target_frame_position = ReadCurrentDecodedFramePosition() + 1l;
        CURRENT_FRAME_PROBE_WAIT(target_frame_position);

        if(index == 1u)
        {
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180C0, 0x20u);
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180CF, CurrentFrameProbePort80CFValue());
        }
        else if(index == 2u)
        {
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180C0, 0x70u);
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180CF, CurrentFrameProbePort80CFValue());
        }
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1217, 0x12u);
    while((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1217) >= 0)
    {
        if(CURRENT_FRAME_PROBE_SAMPLE() == 1u)
        {
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180C0, 0x20u);
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_0180CF, CurrentFrameProbePort80CFValue());
            CURRENT_FRAME_PROBE_STROBE(FW_CONTROLLER_UNKNOWN_018006, 0xc8u);
            return 1u;
        }
        CURRENT_FRAME_PROBE_YIELD();
    }
    return 0u;
}

/* RAM:f88692 */
u8 PLEXTOR_NEAR_FUNCTION RunControllerReadPathCalibrationAtTarget(s32 target_frame_position)
{
    u8 operating_code;

    WRITE_READ_PATH_PARAMETERS();
    APPLY_READ_PATH_OPERATING_CODE();
    operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F);
    if(operating_code == 0xffu)
        operating_code = 0x28u;

    if(WAIT_FOR_READ_PATH_TARGET(target_frame_position - (s32)(u16)((u16)operating_code * 0x41u)) != 1u)
        return 2u;

    CONFIGURE_READ_PATH(1u);
    APPLY_READ_PATH_OPERATING_CODE();
    RESET_READ_PATH_DATA_REGISTERS();
    CONFIGURE_READ_PATH_PROFILE();
    if(WAIT_FOR_READ_PATH_TARGET(target_frame_position - (s32)(u16)((u16)operating_code * 0x2du)) != 1u)
        return 2u;
    if(PROBE_READ_PATH_READINESS() == 0u)
        return 2u;
    if(WAIT_FOR_READ_PATH_TARGET(target_frame_position - (s32)(u16)((u16)operating_code * 0x14u)) != 1u)
        return 2u;
    if(TEST_READ_PATH_OPERATING_WINDOW(1u) != 0u)
        return 0u;

    RESTORE_READ_PATH_CONFIGURATION(1u);
    return 2u;
}

#define PLEXTOR_BINDING_SECTION 7
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadControllerCalibrationTableWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static void WriteControllerCalibrationTableWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:f8d355 */
u8 PLEXTOR_NEAR_FUNCTION LoadOrCalibrateControllerParameterFC(void)
{
    u8 result;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u || IS_INDEXED_STATE_ACTIVE(5u) == 1u || IS_INDEXED_STATE_ACTIVE(8u) == 1u)
    {
        WRITE_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180AC, 2u);
        do
        {
            result = READ_CONTROLLER_PORT(FW_CONTROLLER_UNKNOWN_0180AC);
        } while((result & 0x7fu) != 0u);
        RESET_INDEXED_CHANNEL(2u);
        ENTER_MEASUREMENT_CONFIGURATION();
        result = CALIBRATE_PARAMETER_FC();
        LEAVE_MEASUREMENT_CONFIGURATION();
        RESET_INDEXED_CHANNEL(2u);
        WAIT_TIMER1(4000u, 0u);
        return result;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC0));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC0) == 0xffu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAC1) == 0xffu)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, 0u);
    }
    SELECT_AND_WRITE_PARAMETER_FC(4u);
    return 1u;
}

/* RAM:fb9b2c */
void PLEXTOR_NEAR_FUNCTION AdvanceControllerParameterEaEbCalibrationSequence(void)
{
    fw_addr_t table_base;
    fw_addr_t table_address;
    u16 table_value;
    u8 table_index;
    u8 phase;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) == 0u)
    {
        START_EA_EB_CALIBRATION_STATE();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x0cu)
            table_base = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92EB);
        else
            table_base = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9337);
        table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF4);
        table_address = table_base + (fw_addr_t)((u16)((u16)table_index + 2u) << 1);
        table_value = ReadControllerCalibrationTableWord(table_address);
        WRITE_EA_EB_CALIBRATION_VALUE(table_value);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) == 6u)
    {
        STOP_EA_EB_CALIBRATION_STATE();
        table_value = ReadControllerCalibrationTableWord(FW_NEAR_UNKNOWN_0FF5);
        WriteControllerCalibrationTableWord(FW_NEAR_UNKNOWN_0FF5, (u16)(table_value & 0xfffdu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, 0u);
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) < 0x0cu)
        table_base = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92EB);
    else
        table_base = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9337);
    table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF4);
    table_address = table_base + ((fw_addr_t)table_index << 1);
    table_value = ReadControllerCalibrationTableWord(table_address);

    phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED);
    WriteControllerCalibrationTableWord(FW_NEAR_UNKNOWN_EFA5 + ((fw_addr_t)phase << 1), table_value);
    WRITE_EA_EB_CALIBRATION_VALUE(table_value);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) == 5u)
    {
        WriteControllerCalibrationTableWord(FW_NEAR_UNKNOWN_0FD9, table_value);
        FINALIZE_EA_EB_CALIBRATION_PHASE_FIVE();
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF4) + 1u));
}

/* RAM:fb9c4d */
void PLEXTOR_NEAR_FUNCTION ServiceControllerParameterEaEbCalibrationState(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) == 0u)
    {
        u8 table_index;
        fw_addr_t table_address;

        START_EA_EB_CALIBRATION_STATE();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 8u)
            table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F965);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 4u)
            table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F964);
        else
            table_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F963);

        table_address = PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF92EB) + (fw_addr_t)((u16)((u16)table_index + 2u) << 1);
        WRITE_EA_EB_CALIBRATION_VALUE(ReadControllerCalibrationTableWord(table_address));
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED) == 2u)
    {
        u16 state_flags;

        STOP_EA_EB_CALIBRATION_STATE();
        state_flags = ReadControllerCalibrationTableWord(FW_NEAR_UNKNOWN_0FF5);
        state_flags &= 0xfffbu;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF5, (u8)(state_flags >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FF6, (u8)state_flags);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, 0u);
    }
}

#define PLEXTOR_BINDING_SECTION 8
#include "bindings/platform/controller_calibration_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u8 ClampControllerParameterToF8(s16 value)
{
    if(value < 0)
        return 0u;
    if(value >= 0xf8)
        return 0xf8u;
    return (u8)value;
}

/* RAM:f8c928 */
void WriteControllerParametersFCFDF6F7FromCurrentState(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        s16 fc_value = (s16)(u16)FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8566));
        s16 fd_value = fc_value;
        s16 f6_value = (s16)(u16)FirmwareRead8(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF856E));
        s16 f7_value = f6_value;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x0cu)
        {
            u8 operating_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43);
            u8 table_offset = (u8)(MapOperatingCodeToProfileIndex(operating_code) - 5u);

            fc_value = (s16)(u16)FirmwareRead8((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF8566) + table_offset));
            fd_value = fc_value;
            table_offset = (u8)(MapOperatingCodeToProfileIndex(operating_code) - 5u);
            f6_value = (s16)(u16)FirmwareRead8((fw_addr_t)(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF856E) + table_offset));
            f7_value = f6_value;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x10u)
        {
            fc_value = 0;
            f6_value = 0;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) == 0u)
        {
            fc_value = (s16)(fc_value + 0x68);
            fd_value = (s16)(fd_value + 0x68);
        }
        else
        {
            u16 offset = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F);

            fc_value = (s16)(fc_value + (s16)offset);
            fd_value = (s16)(fd_value + (s16)offset);
        }
        f6_value = (s16)(f6_value + 0xc0);
        f7_value = (s16)(f7_value + 0xc0);

        WriteControllerParameterFCValue(ClampControllerParameterToF8(fc_value));
        WriteControllerParameterF6ValueWithBits0And1Set(ClampControllerParameterToF8(f6_value));
        WriteControllerParameterFDValue(ClampControllerParameterToF8(fd_value));
        WriteControllerParameterF7Value(ClampControllerParameterToF8(f7_value));
    }
    else
    {
        s16 base_value;
        s16 fd_value;
        s16 f7_value = 0xb0;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F) == 0u)
            base_value = 0x68;
        else
            base_value = (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) == 0x10u)
                fd_value = (s16)(base_value + 8);
            else
                fd_value = (s16)(base_value - 0x10);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
            fd_value = (s16)(base_value + 0x18);
        else
            fd_value = (s16)(base_value + 0x18);

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 2u || FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 3u)
            f7_value = (s16)(f7_value - 8);
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) == 1u)
            f7_value = (s16)(f7_value + 8);
        else
            f7_value = (s16)(f7_value + 0x10);

        WriteControllerParameterFDValue((u8)fd_value);
        WriteControllerParameterF7Value((u8)f7_value);
    }
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteControllerCalibrationWord(u16 address, s16 value)
{
    FirmwareWrite8(address, (u8)((u16)value >> 8));
    FirmwareWriteNear8((u16)(address + 1u), (u8)value);
}

/* RAM:f8c820 */
void ComputeAndRampControllerWord0255Target(void)
{
    s16 target;
    s16 current_value;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        return;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        target = SELECT_CONTROLLER_WORD0255_TARGET();
    else
        target = ComputeControllerWord0255TargetFromCalibrationByte();

    WriteControllerCalibrationWord(FW_NEAR_UNKNOWN_F994, (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8));
    if(target != (s16)0x8000u)
    {
        WriteControllerCalibrationWord(FW_NEAR_UNKNOWN_F994, target);
        current_value = (s16)(u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_F9A8);
        while(target != current_value)
        {
            if(target < current_value)
                --current_value;
            else
                ++current_value;
            SubmitControllerByteAndWord((u8)current_value, 0x0255u);
            WaitTimer1ScaledDuration(200u, 1u);
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x80u));
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadControllerCalibrationDword(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

/* RAM:f8c519 */
void ConfigureControllerPortsAndParametersForCurrentOperatingCode(u8 ramp_word0255)
{
    u16 operating_pair;
    u8 port_806a_value;

    StrobeControllerPortWriteByteFromC(FW_CONTROLLER_UNKNOWN_018007, 8u);
    WriteControllerParameterWord((u16)(0xe700u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FC15)));

    switch(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43))
    {
    case 1u:
        port_806a_value = 0u;
        operating_pair = 0x1c00u;
        break;
    case 2u:
        port_806a_value = 0u;
        operating_pair = 0x1c10u;
        break;
    case 4u:
        port_806a_value = 0u;
        operating_pair = 0x1c24u;
        break;
    case 8u:
        port_806a_value = 0u;
        operating_pair = 0x2848u;
        break;
    case 10u:
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0x40u) != 0u)
        {
            port_806a_value = 3u;
            operating_pair = 0x2028u;
        }
        else
        {
            port_806a_value = 0u;
            operating_pair = 0x2058u;
        }
        break;
    case 12u:
        port_806a_value = 0u;
        operating_pair = 0x2058u;
        break;
    case 16u:
        port_806a_value = 0u;
        operating_pair = 0x346au;
        break;
    case 20u:
        port_806a_value = 4u;
        operating_pair = 0x3c8au;
        break;
    case 24u:
        port_806a_value = 4u;
        operating_pair = 0x488au;
        break;
    case 32u:
        port_806a_value = 5u;
        operating_pair = 0x548au;
        break;
    case 40u:
        port_806a_value = 5u;
        operating_pair = 0x648au;
        break;
    case 52u:
        port_806a_value = 7u;
        if(ReadControllerCalibrationDword(FW_NEAR_UNKNOWN_1235) < 0x00036ee8l)
            operating_pair = 0x588au;
        else
            operating_pair = 0x589au;
        break;
    default:
        for(;;)
        {
        }
    }

    FirmwareWriteController8(FW_CONTROLLER_UNKNOWN_01806A, port_806a_value);
    WriteControllerParameterWord((u16)(0xf001u | (u8)operating_pair));
    WriteControllerParameterWord((u16)(0xd500u | (operating_pair >> 8)));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018086, 0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018082, 0xb0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x80u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, 0x43u);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE0) & 0x20u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBF6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6) & 0xefu));
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809D, FirmwareReadNear8(FW_NEAR_UNKNOWN_FBF6));
    }
    APPLY_CONTROLLER_D1_FOR_CALIBRATION();
    CONFIGURE_CONTROLLER_D0_D1_D3_FOR_CALIBRATION();
    if(ramp_word0255 == 1u)
        COMPUTE_AND_RAMP_CONTROLLER_WORD0255();
}

/* RAM:f8e6d4 */
u8 RunControllerCommand06AndDiscardPort80C7(u8 command_selector)
{
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C5, command_selector);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 6u);
    if(WaitControllerPort80F8Bit7Clear10AndYield() == 0u)
    {
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
        WaitControllerPort80F8Bit7ClearWithoutTimeout();
    }
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180C4, 0u);
    return WaitControllerPortReadyAndReadByte(FW_CONTROLLER_UNKNOWN_0180C7);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u8 RunControllerReferenceCaptureTransaction(u8 command_value)
{
    WRITE_CONTROLLER_REFERENCE_PORT(FW_CONTROLLER_UNKNOWN_0180B2, 0x2cu);
    WRITE_CONTROLLER_REFERENCE_PORT(FW_CONTROLLER_UNKNOWN_0180C5, 3u);
    WRITE_CONTROLLER_REFERENCE_PORT(FW_CONTROLLER_UNKNOWN_0180B0, command_value);
    return WAIT_CONTROLLER_REFERENCE_READY_10();
}

/* RAM:f8cf2a */
u8 ControllerReferenceCaptureFunction(void)
{
    u8 result;
    u16 controller_word;

    if(RUN_CONTROLLER_PARAMETER_CALIBRATION_SET() != 1u)
        return 0u;

    result = RunControllerReferenceCaptureTransaction(0x50u);
    if(result != 1u)
        return result;
    SUBMIT_CONTROLLER_REFERENCE_WORD(0u, 0x0309u);
    WRITE_CONTROLLER_REFERENCE_PORT(FW_CONTROLLER_UNKNOWN_0180A0, 1u);

    result = RunControllerReferenceCaptureTransaction(0x60u);
    if(result != 1u)
        return result;
    SUBMIT_CONTROLLER_REFERENCE_WORD(0u, 0x0319u);
    WRITE_CONTROLLER_REFERENCE_PORT(FW_CONTROLLER_UNKNOWN_0180A2, 4u);

    result = RunControllerReferenceCaptureTransaction(0x80u);
    if(result != 1u)
        return result;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0x20u) != 0u)
    {
        controller_word = READ_CONTROLLER_REFERENCE_WORD(0x0309u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBE2, (u8)(controller_word >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBE3, (u8)controller_word);
        controller_word = READ_CONTROLLER_REFERENCE_WORD(0x0319u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBE4, (u8)(controller_word >> 8));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBE5, (u8)controller_word);
    }

    controller_word = READ_CONTROLLER_REFERENCE_WORD(0x0319u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1056, (u8)(controller_word >> 8));
    return 1u;
}

/* RAM:fa7f94 */
u8 PLEXTOR_NEAR_FUNCTION SelectControllerCalibrationCandidateFromFourSamples(void)
{
    u8 index;
    u8 any_valid = 0u;
    u8 selected_index = 3u;
    u8 selected_sample;

    for(index = 0u; index < 4u; ++index)
        if(FirmwareReadNear8((u16)(0x1046u + index)) != 0xffu)
            any_valid = 1u;
    if(any_valid == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 0x10u));
        return 0u;
    }

    selected_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_1049);
    for(index = 0u; index < 3u; ++index)
    {
        u8 sample = FirmwareReadNear8((u16)(0x1046u + index));

        if(sample < selected_sample)
        {
            selected_sample = FirmwareReadNear8((u16)(0x1046u + index));
            selected_index = index;
        }
    }
    if(selected_sample == FirmwareReadNear8(FW_NEAR_UNKNOWN_1047))
        selected_index = 1u;

    if(selected_index == 3u)
    {
        u8 selection_code = FirmwareReadNear8(FW_NEAR_UNKNOWN_1050);

        if(selection_code == 3u)
        {
            u16 difference = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1047) - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1049));

            if(difference <= 5u)
                selected_index = 2u;
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1050) == 0xfdu)
        {
            u16 difference = (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1047) - (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1049));

            if(difference <= 5u)
                selected_index = 0u;
        }
    }

    if(FirmwareReadNear8((u16)(0x1046u + selected_index)) >= 0x32u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xefu));
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1042, FirmwareReadNear8((u16)(0x104du + selected_index)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1054, selected_index);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1051, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1042) - 2u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1052, FirmwareReadNear8(FW_NEAR_UNKNOWN_1042));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1053, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1042) + 2u));
    }
    return 1u;
}

/* RAM:fa7faa */
/* The canonical address entered the shared portion of the algorithm with
 * scratch-register state supplied by RAM:fa7f94.  With the whole algorithm
 * maintained in C, keep this separately inventoried entry as a normal near-C
 * call so it has no hidden register precondition. */
u8 PLEXTOR_NEAR_FUNCTION SelectControllerCalibrationCandidateFromFourSamplesBody(void)
{
    return SelectControllerCalibrationCandidateFromFourSamples();
}

/* RAM:fa807e */
u8 PLEXTOR_NEAR_FUNCTION SelectControllerCalibrationValueFromSampleWindow(void)
{
    u8 index;
    u8 any_valid = 0u;
    u8 selected_index;
    u8 selected_sample;
    u8 left;
    u8 right;

    for(index = 4u; index < 7u; ++index)
        if(FirmwareReadNear8((u16)(0x1046u + index)) != 0xffu)
            any_valid = 1u;
    if(any_valid == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 0x10u));
        return 0u;
    }

    any_valid = 0u;
    for(index = 4u; index < 7u; ++index)
        if(FirmwareReadNear8((u16)(0x1046u + index)) < 0x32u)
            any_valid = 1u;
    if(any_valid == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xefu));
        return 0u;
    }

    left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104B);
    right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
    if(left == right)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
        return 1u;
    }
    left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
    right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104B);
    if(left == right)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
        return 1u;
    }
    left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
    right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
    if(left == right)
    {
        left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
        right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104B);
        if(left < right)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
            return 1u;
        }
    }

    selected_index = 4u;
    selected_sample = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
    for(index = 5u; index < 7u; ++index)
    {
        u8 sample = FirmwareReadNear8((u16)(0x1046u + index));

        if(sample < selected_sample)
        {
            selected_sample = FirmwareReadNear8((u16)(0x1046u + index));
            selected_index = index;
        }
    }

    if(selected_index != 5u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8((u16)(0x104du + selected_index)));
    }
    else
    {
        left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
        right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
        if(left < right)
        {
            u8 difference;
            u8 dividend_bits;

            left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
            right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104B);
            difference = (u8)(left - right);
            if(difference < 3u)
            {
                left = FirmwareReadNear8(FW_NEAR_UNKNOWN_1053);
                right = FirmwareReadNear8(FW_NEAR_UNKNOWN_1052);
                dividend_bits = (u8)(left + right);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, (u8)DIVIDE_CALIBRATION_BYTE_BY_TWO((s8)dividend_bits));
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
            }
        }
        else
        {
            left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104C);
            right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
            if(left > right)
            {
                u8 difference;
                u8 dividend_bits;

                left = FirmwareReadNear8(FW_NEAR_UNKNOWN_104A);
                right = FirmwareReadNear8(FW_NEAR_UNKNOWN_104B);
                difference = (u8)(left - right);
                if(difference < 3u)
                {
                    left = FirmwareReadNear8(FW_NEAR_UNKNOWN_1051);
                    right = FirmwareReadNear8(FW_NEAR_UNKNOWN_1052);
                    dividend_bits = (u8)(left + right);
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, (u8)DIVIDE_CALIBRATION_BYTE_BY_TWO((s8)dividend_bits));
                }
                else
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
                }
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, FirmwareReadNear8(FW_NEAR_UNKNOWN_1052));
            }
        }
    }
    return 1u;
}

/* RAM:fa8095 */
/* The canonical address entered the shared portion with scratch-register
 * state supplied by RAM:fa807e.  Keep its separately inventoried C entry
 * free of that hidden precondition by calling the complete C algorithm. */
u8 PLEXTOR_NEAR_FUNCTION SelectControllerCalibrationValueFromSampleWindowBody(void)
{
    return SelectControllerCalibrationValueFromSampleWindow();
}

/* RAM:fa7d9e */
void AccumulateMediaState5ControllerCalibrationSample(void)
{
    u8 sample_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_1045);

    FirmwareWriteNear8((u16)(0x1046u + sample_count), FirmwareReadNear8(FW_NEAR_UNKNOWN_FB95));
    sample_count = (u8)(sample_count + 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1045, sample_count);

    if(sample_count == 3u)
        CLASSIFY_MEDIA_STATE5_ORDERING();

    if(sample_count == 4u)
    {
        u8 selected = SELECT_MEDIA_STATE5_FOUR_SAMPLES();
        u16 configuration_word = FirmwareReadNearU16(FW_NEAR_UNKNOWN_FAAC);

        if((configuration_word & 4u) != 0u)
            selected = 1u;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
        if(selected == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
        }
        else
        {
            u8 calibration_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_1042);

            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC4, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC5, calibration_value);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1044, calibration_value);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC7, ENCODE_MEDIA_STATE5_OFFSET((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06D)));
        }
    }

    if(sample_count == 7u)
    {
        if(SELECT_MEDIA_STATE5_WINDOW() == 1u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC4, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC5, FirmwareReadNear8(FW_NEAR_UNKNOWN_1042));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAC7, ENCODE_MEDIA_STATE5_OFFSET((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_F06D)));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 2u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    }
}
