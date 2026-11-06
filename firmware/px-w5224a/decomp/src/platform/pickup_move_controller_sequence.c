#include "pickup_move_controller_sequence.h"
#include "controller_parameters.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "status_mapping.h"

extern void YieldCoroutine1(void);

static u16 ReadPickupMoveU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WritePickupMoveU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WaitForPickupMoveControllerIdle(void)
{
    while(((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) >> 7) & 1u) != 0u)
        YieldCoroutine1();
}

static void WaitForPickupMoveCompletion(void)
{
    while(((FirmwareReadController8(FW_CONTROLLER_UNKNOWN_0180F8) >> 7) & 1u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1302) & 0x08u) == 0u)
            YieldCoroutine1();
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1213) < 0)
            SetControllerFlagFC07Bit2AndPort8098Bit6(0u);
    }
}

static void UpdatePickupMoveRadialPosition(u16 distance, u8 move_direction)
{
    u16 delta = (u16)(ReadPickupMoveU16(FW_NEAR_UNKNOWN_0CF9) / 12000u);
    u16 position = ReadPickupMoveU16(FW_NEAR_UNKNOWN_1031);

    delta = (u16)(delta * distance);
    if(move_direction == 0u)
        position = (u16)(position + delta);
    else
        position = (u16)(position - delta);
    WritePickupMoveU16(FW_NEAR_UNKNOWN_1031, position);
}

static u8 PickupMoveControllerCacheMatches(u16 distance)
{
    if(DoesControllerModeOneClassificationDifferFromCache(distance) != 0u)
        return 0u;
    if((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 4u) != FirmwareReadNear8(FW_NEAR_UNKNOWN_FC2E))
        return 0u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC2F) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1006));
}

/* RAM:fd07de */
u8 PLEXTOR_NEAR_FUNCTION ExecutePickupMoveControllerSequence(u16 distance, u8 move_direction)
{
    u8 result;

    WaitForPickupMoveControllerIdle();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 2u))
        ClearControllerParameterD1ShadowBit6();
    WriteControllerParametersD2AndD3FromState();
    SubmitControllerWordPair(0x3fffu, 0x033au);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D8, 0x18u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1225, 2u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A4, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E) | 0x30u));

    UpdatePickupMoveRadialPosition(distance, move_direction);
    ConfigurePickupMoveControllerForDistance(distance, move_direction);
    if(PickupMoveControllerCacheMatches(distance) == 0u)
    {
        if(distance == 1u)
            ApplyFixedControllerParameterTableForOperatingCodeClass();
        else
            ApplyFixedControllerParameterTables1And2();
    }
    if(distance >= 10u)
    {
        ProgramPickupMoveProfileForDistance(move_direction, distance);
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A5, 3u);
    }

    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A6, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1213, 100u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1002, 0x10u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1003, 0xf0u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018498, FirmwareReadNear8(FW_NEAR_UNKNOWN_1002));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018499, FirmwareReadNear8(FW_NEAR_UNKNOWN_1003));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01809A, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0184A2, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018496, 0xc5u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_018497, 0x80u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180E4, 1u);

    WaitForPickupMoveCompletion();
    WriteControllerParametersD1D2D3FromConfiguration();
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A5, 4u);
    result = FinalizePickupMoveControllerSequenceAndClassifyStatus();
    if(result != 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
            ApplyControllerParameterD1ModeBits();
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_01849D, 0u);
        return 0u;
    }

    SetIndexedControllerChannelValue(2u, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98E));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A6, FirmwareReadNear8(FW_NEAR_UNKNOWN_F98F));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D8, 0u);
    SubmitControllerWordPair(0u, 0x033au);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        ApplyControllerParameterD1ModeBits();
    WritePickupMoveU16(FW_NEAR_UNKNOWN_FC2B, distance);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2D, move_direction);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 4u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC2F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1006));
    return 1u;
}
