#include "command_admission.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "active_media_operation_admission.h"
#include "command_dispatch.h"

static u8 CurrentScsiOpcode(void)
{
    return FirmwareReadNear8(FW_NEAR_HOST_COMMAND_OPCODE);
}

static u8 IsUnsupportedScsiDispatchEntry(u8 opcode)
{
    return (u8)(IsSupportedScsiOpcode(opcode) == 0u);
}

static u8 CurrentCommandRequiresMedia(void)
{
    return ScsiCommandRequiresMedia(CurrentScsiOpcode());
}

static u8 IsReadFamilyRejectedByCurrentMode(u8 opcode)
{
    switch(opcode)
    {
    case 0x08u:
    case 0x28u:
    case 0x44u:
    case 0xa8u:
    case 0xb9u:
    case 0xbeu:
    case 0xd8u:
    case 0xd9u:
    case 0xdfu:
        return 1u;
    default:
        return 0u;
    }
}

static u8 IsAllowedWithoutMedia(u8 opcode)
{
    switch(opcode)
    {
    case 0x00u:
    case 0x03u:
    case 0x12u:
    case 0x1au:
    case 0x1bu:
    case 0x1eu:
    case 0x35u:
    case 0x4au:
    case 0x52u:
    case 0x58u:
    case 0x5au:
    case 0x5bu:
    case 0x5cu:
    case 0xf4u:
    case 0xf5u:
        return 1u;
    default:
        return 0u;
    }
}

/* RAM:fda843 */
u8 ValidateScsiCommandAdmission(void)
{
    u8 opcode;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12F7) != 0u)
    {
        SetSenseData(4u, 0x4000u);
        return 0u;
    }

    opcode = CurrentScsiOpcode();
    if(IsUnsupportedScsiDispatchEntry(opcode) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }
    if(CurrentScsiOpcode() == 0xeeu)
        return 1u;

    if(CurrentScsiOpcode() != 0x03u && CurrentScsiOpcode() != 0x12u && CurrentScsiOpcode() != 0x4au)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x08u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xf7u));
            return 0u;
        }

        if(CurrentScsiOpcode() != 0x12u && CurrentScsiOpcode() != 0x4au && CurrentScsiOpcode() != 0xe9u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x40u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0xbfu));
                if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x80u) != 0u)
                    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) | 0x08u));
                SetSenseData(6u, 0x2900u);
                return 0u;
            }
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x80u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C85, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x7fu));
                SetSenseData(6u, 0x2800u);
                return 0u;
            }
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x01u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_0C86, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0xfeu));
                SetSenseData(6u, 0x2a01u);
                return 0u;
            }
        }
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x02u) != 0u && IsReadFamilyRejectedByCurrentMode(CurrentScsiOpcode()) != 0u)
    {
        SetSenseData(5u, 0x6400u);
        return 0u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) != 0u)
    {
        if(IsDriveOperationStatusScsiEligible() != 0u)
        {
            if(CurrentCommandRequiresMedia() != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C3E) != 0u
                && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 5u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 4u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0x0du
                    || FirmwareReadNear8(FW_NEAR_UNKNOWN_1009) == 0x0eu))
            {
                SetSenseData(2u, 0x5700u);
                return 0u;
            }
        }
        else if(CurrentScsiOpcode() != 0x1cu && CurrentScsiOpcode() != 0x1du)
        {
            SetSenseData(4u, 0x4000u);
            return 0u;
        }
    }

    if(CurrentCommandRequiresMedia() != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) != 0u)
        {
            SetSenseDataFromStatusCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE));
            return 0u;
        }
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 0x02u) == 0u)
        {
            SetSenseData(2u, 0x0401u);
            return 0u;
        }
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u && (FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0x02u) != 0u && RejectIncompatibleMediumWriteCommand() == 0u)
        return 0u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x08u) != 0u && ValidateCommandDuringActiveMediaOperation() == 0u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAE) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) & 0x80u) != 0u && IsAllowedWithoutMedia(CurrentScsiOpcode()) == 0u)
    {
        SetSenseData(3u, 0x0c07u);
        return 0u;
    }
    return 1u;
}
