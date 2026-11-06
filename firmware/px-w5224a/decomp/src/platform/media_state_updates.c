#include "media_state_updates.h"
#include "controller_register_profile_application.h"
#include "controller_threshold_configuration.h"
#include "firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "bindings/platform/media_state_updates_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* Reconstruction helper; no direct Ghidra function address. */
static u16 ReadBigEndianU16ForMediaStateUpdate(u16 address)
{
    return FirmwareReadNearU16(address);
}

/* Reconstruction helpers; no direct Ghidra function addresses. */
static u32 ReadBigEndianU32ForMediaStateUpdate(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32ForMediaStateUpdate(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static void WriteBigEndianU16ForMediaStateUpdate(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

/* RAM:fa6373 */
void InitializeMediaState5PositionAndControllerState(void)
{
    u32 target_position = ReadBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_FBA2);

    if(DispatchReadBit0033ForMediaState5Position() != 0u)
        target_position += 4ul;
    WriteBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_103D, target_position);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) | 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1308) == 1u && (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_130A) + 0x0au) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfeu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0x3fu) | 0x80u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 2u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103B) & 0xf1u) | 0x0au));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103C, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_103C) & 0xfcu) | 1u));
    DispatchWriteControllerParameterEF00ForMediaState5(0u);
    DispatchUpdateControllerOperatingCodeEnableForMediaState5();
}

/* RAM:fa63ff */
void BuildMediaState5AdjacentPackedBcdMsfTuples(void)
{
    u8 operating_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E51);

    if(operating_state <= 0x0cu && DispatchReadBit0033ForMediaState5Position() == 0u)
    {
        DispatchConvertFramePositionToBinaryMsfForMediaState5(0x10bau, (s32)(ReadBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_103D) - 2ul));
        DispatchConvertFramePositionToBinaryMsfForMediaState5(0x10b6u, (s32)(ReadBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_103D) - 1ul));
    }
    else
    {
        DispatchConvertFramePositionToBinaryMsfForMediaState5(0x10bau, (s32)(ReadBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_103D) - 3ul));
        DispatchConvertFramePositionToBinaryMsfForMediaState5(0x10b6u, (s32)(ReadBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_103D) - 2ul));
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10B6, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10B6)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10B7, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10B7)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10B8, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10B8)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BA, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10BA)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BB, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10BB)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_10BC, DispatchBinaryByteToPackedBcdForMediaState5(FirmwareReadNear8(FW_NEAR_UNKNOWN_10BC)));
}

/* RAM:fa6490 */
void InitializeDiscControlAndCalibrationState(void)
{
    u8 index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF74, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF78, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF77, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1082, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1080, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xfeu));
    WriteBigEndianU32ForMediaStateUpdate(FW_NEAR_UNKNOWN_1083, 0ul);
    for(index = 0u; index != 8u; ++index)
        WriteBigEndianU16ForMediaStateUpdate((fw_addr_t)(FW_NEAR_UNKNOWN_1095 - (fw_addr_t)index * 2u), 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_105C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_105C) & 0xdfu));

    for(index = 0u; index < 99u; ++index)
        WriteBigEndianU16ForMediaStateUpdate((fw_addr_t)(FW_NEAR_UNKNOWN_F1F0 + (fw_addr_t)index * 2u), 0u);

    DispatchResetControllerCommand0EForDiscControl();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_EF72, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_EF72) & 0xf7u));
    if(DispatchIndexedDriveStateForDiscControl(0x1fu) != 1u)
        return;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1045, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_103A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_103A) & 0xefu));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_104D, 0xfeu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_104E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_104F, 2u);
    DispatchReadDriveConfigurationForDiscControl(FW_NEAR_UNKNOWN_1041, 0x002eu, 1u);
}

/* RAM:f87625 */
void PLEXTOR_NEAR_FUNCTION ServiceMediaState5ControllerAndOperatingUpdates(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) >> 1) & 3u) == 3u)
        DispatchAdvanceOperatingUpdateStageAtFrameBoundaries();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB69) >> 1) & 1u) == 0u)
        DispatchUpdateMeasurementPositionSnapshotState();
    DispatchUpdateWriteSpeedFlagsAfterPositionCount();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) >> 3) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FD0, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 0xf7u));
        SetMediaState5UpdateRequestBitsAtomic();
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0FD0) & 1u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB9B) != 0u)
        DispatchServiceControllerOperatingThresholdSamplingCycle();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) != 0xffu)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FA00) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1137))
            return;
        DispatchRecomputeMediaState5ControllerThreshold();
        DispatchApplyControllerRegisterProfile(FirmwareReadNear8(FW_NEAR_UNKNOWN_1137), 0xffu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA00, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
        return;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        DispatchUpdateSecondaryControllerOperatingProfile();
}

/* RAM:f87587 */
void PLEXTOR_NEAR_FUNCTION ConfigureControllerForReadAcquisitionTarget(s32 target_frame_position)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA01, 0xffu);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FA04, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) & 1u) == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4F) == 0xffu)
            {
                u8 coordinate = (u8)(target_frame_position / 4500l);

                DispatchApplyControllerRegisterProfile(coordinate, 0xffu);
                DispatchWriteControllerParametersEAEB(ReadBigEndianU16ForMediaStateUpdate(FW_NEAR_UNKNOWN_FBC3));
            }
            else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 0x14u)
            {
                u8 coordinate = (u8)(target_frame_position / 4500l);

                DispatchApplyControllerRegisterProfile(coordinate, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
                DispatchWriteControllerParametersEAEB(ReadBigEndianU16ForMediaStateUpdate(FW_NEAR_UNKNOWN_FBC3));
            }
            else
            {
                DispatchWriteControllerParametersEAEB(ReadBigEndianU16ForMediaStateUpdate(FW_NEAR_UNKNOWN_FACF));
            }
        }
        else
        {
            if(DispatchReadBit0044ForReadAcquisitionTarget() == 0u)
                DispatchWriteControllerParametersD4E2(0x14u);
            DispatchWriteControllerParametersEAEB(0u);
        }
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u)
        DispatchWriteControllerParametersE8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
    else
        DispatchWriteControllerParametersE8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
}

/* RAM:fc8e1a */
void SelectAndConfigureOperatingCodeForThresholdBody(u32 threshold_value)
{
    u8 selection_value;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4D, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1307, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1307) & 0xfeu));
        DispatchWriteControllerParametersE8E9(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAB5));
    }

    selection_value = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44);
    if(selection_value == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
    }
    else if(selection_value == 5u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) == 0u ? 1u : 8u);
    }
    else if(selection_value == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E45) == 1u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) == 1u)
            {
                DispatchSetCurrentOperatingCodeWithHighCodeTransitionFloor(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E47));
            }
            else
            {
                DispatchSetCurrentOperatingCodeWithHighCodeTransitionFloor(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B));
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E4B) == 0u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E43, 1u);
            }
        }
    }
    else
    {
        DispatchSetCurrentOperatingCodeWithHighCodeTransitionFloor(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E44));
    }

    DispatchConfigureControllerForOperatingCodeThreshold(threshold_value);
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) >> 3) & 1u) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 0xf7u));
}
