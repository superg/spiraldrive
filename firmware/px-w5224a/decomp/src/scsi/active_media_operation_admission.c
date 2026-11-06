#include "active_media_operation_admission.h"
#include "../media/active_write_pipeline.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

static u8 CurrentScsiOpcode(void)
{
    return FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u8 IsAllowedWhileReadOperationBusy(u8 opcode)
{
    return (u8)(opcode == 0x03u || opcode == 0x12u || opcode == 0x46u || opcode == 0x4au || opcode == 0xebu || opcode == 0xf5u);
}

static u8 IsImmediatelyAllowedDuringActiveOperation(u8 opcode)
{
    switch(opcode)
    {
    case 0x00u:
    case 0x03u:
    case 0x0au:
    case 0x12u:
    case 0x2au:
    case 0x35u:
    case 0x46u:
    case 0x4au:
    case 0x52u:
    case 0x5bu:
    case 0x5cu:
    case 0xaau:
    case 0xebu:
    case 0xf5u:
    case 0xf7u:
    case 0xf8u:
        return 1u;
    default:
        return 0u;
    }
}

static u8 IsWriteFinalizationOpcode(u8 opcode)
{
    return (u8)(opcode == 0x08u || opcode == 0x28u || opcode == 0xa8u || opcode == 0xb9u || opcode == 0xbeu || opcode == 0xd8u || opcode == 0xd9u);
}

static u8 IsAllowedOnlyWithMediaStatus(u8 opcode)
{
    return (u8)(opcode == 0x1au || opcode == 0x23u || opcode == 0x25u || opcode == 0x2eu || opcode == 0x51u || opcode == 0x55u || opcode == 0x5au);
}

static void PublishOperationCompletionCountdown(void)
{
    u8 media_status = FirmwareReadNear8(FW_NEAR_FORMAT_STATE);

    if(media_status == 2u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5B1) != 0u)
            return;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BA5AF) == 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) | 0x08u));
            FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, (FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 0x01u) != 0u ? 60u : 20u);
        }
        else if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x80u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x01u));
            FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 3u && (FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x80u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) | 0x01u));
        FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    }
}

static u8 FinalizeActiveWriteOperationAfterWriteTypeCheck(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C84, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0xf7u));
    if(FinalizeActiveWritePipelineAndTrackState() == 0u)
        return 0u;
    PublishOperationCompletionCountdown();
    return 1u;
}

static u8 FinalizeActiveWriteOperation(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u)
    {
        SetCommandSequenceErrorSense();
        return 0u;
    }
    return FinalizeActiveWriteOperationAfterWriteTypeCheck();
}

/* RAM:fdab34 */
u8 PLEXTOR_NEAR_FUNCTION ValidateCommandDuringActiveMediaOperation(void)
{
    u8 opcode;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) == 0u)
        return 1u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x01u) != 0u)
    {
        u32 remaining_position;
        u32 information_limit;

        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) & 0x40u) != 0u)
            return 1u;
        opcode = CurrentScsiOpcode();
        if(IsAllowedWhileReadOperationBusy(opcode) != 0u)
            return 1u;
        if(CurrentScsiOpcode() == 0x52u && FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            return 1u;

        SetSenseData(2u, (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C87) & 0x20u) != 0u ? 0x0404u : 0x0408u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_E622) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0E40) == 0u)
        {
            remaining_position = ReadBigEndianU32(FW_NEAR_UNKNOWN_E606) - ReadBigEndianU32(FW_NEAR_UNKNOWN_1235);
            WriteBigEndianU32(FW_NEAR_UNKNOWN_E60E, remaining_position);
            information_limit = ReadBigEndianU32(FW_NEAR_UNKNOWN_E60A);
            if((s32)remaining_position < (s32)information_limit)
            {
                SetSenseInformation(remaining_position);
                WriteBigEndianU32(FW_NEAR_UNKNOWN_E60A, remaining_position);
            }
            else
            {
                SetSenseInformation(information_limit);
            }
        }
        return 0u;
    }

    opcode = CurrentScsiOpcode();
    if(IsImmediatelyAllowedDuringActiveOperation(opcode) != 0u)
        return 1u;
    if(IsWriteFinalizationOpcode(opcode) != 0u)
        return FinalizeActiveWriteOperation();
    if(IsAllowedOnlyWithMediaStatus(opcode) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            return 1u;
        SetCommandSequenceErrorSense();
        return 0u;
    }

    if(opcode == 0x1eu)
    {
        if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x20u) != 0u)
        {
            SetTrayOpenOrNoSenseForCurrentCommand();
            return 0u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0B0E) & 0x01u) != 0u)
        {
            SetCommandSequenceErrorSense();
            return 0u;
        }
        return FinalizeActiveWriteOperationAfterWriteTypeCheck();
    }

    if((FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x20u) != 0u)
        SetTrayOpenOrNoSenseForCurrentCommand();
    else
        SetCommandSequenceErrorSense();
    return 0u;
}
