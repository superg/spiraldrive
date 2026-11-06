#include "read_acquisition_distance.h"
#include "../platform/controller_q_packet_process.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/read_acquisition_distance_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadBigEndianU16ForMoveDistance(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteBigEndianU16ForMoveDistance(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static u32 ReadBigEndianU32ForMoveDistance(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32ForMoveDistance(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

/* RAM:fc858f */
u8 PLEXTOR_NEAR_FUNCTION PositionPickupForAcquisition(void)
{
    u8 position_ready = 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
    {
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 3) & 1u) == 1u)
            position_ready = 1u;
    }
    else if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) >> 3) & 1u) == 1u)
    {
        position_ready = 1u;
    }

    if(position_ready != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 0u)
        {
            DispatchPositionPickupAdjustTarget();
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE2, 0u);
            DispatchPositionPickupUpdateDistance();
            WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDE, ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3F) + 1u));
            return DispatchPositionPickupIssueMove();
        }

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 1u) == 0u)
                return 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0xfeu));
            if(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
            {
                DispatchPositionPickupScaledMove(1u, 200u);
                return 0u;
            }
            return DispatchPositionPickupSpecialStep();
        }

        if(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
            return 1u;
        return DispatchPositionPickupSpecialStep();
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED9) != 0u)
            return 1u;
        WriteBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EE5, DispatchPositionPickupAbsoluteDistance());
        if(DispatchPositionPickupDistanceAccepted() != 0u)
            return 1u;
        if(DispatchPositionPickupNearTarget() != 0u)
        {
            WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 0u);
            return 0u;
        }
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, DispatchPositionPickupRadialDistance());
        DispatchPositionPickupYield();
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDE, ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC3F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3F) + 1u));
        return DispatchPositionPickupIssueMove();
    }

    if(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0E3B) == 0x10000000ul)
    {
        DispatchPositionPickupScaledMove(1u, 200u);
        return 0u;
    }
    return DispatchPositionPickupSpecialStep();
}

/* RAM:fc8705 */
u8 PLEXTOR_NEAR_FUNCTION IssueReadAcquisitionPickupMove(void)
{
    u16 magnitude = ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDE);
    u16 distance;
    u8 direction = FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE3);

    if(magnitude >= 150u)
    {
        distance = ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC);
        if(distance < 15000u)
        {
            if(distance >= 4000u
                && ((u16)(distance + 200u) >= ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0FF7) || FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 1u
                    || (s32)ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EE9) <= 2000l))
            {
                magnitude = (u16)(magnitude - 50u);
            }
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u && distance >= 800u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 1u)
                magnitude = (u16)(magnitude - 200u);
        }
        else if((u16)(distance + 400u) >= ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0FF7) || FirmwareReadNear8(FW_NEAR_UNKNOWN_0ED7) == 1u
                || (s32)ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EE9) <= 4000l)
        {
            magnitude = (u16)(magnitude - 50u);
        }
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDE, magnitude);
        DispatchMovePickupByScaledRadialDelta(direction, magnitude);
        return 0u;
    }

    if(magnitude >= 50u)
    {
        DispatchMovePickupByDistanceInChunks(direction, magnitude);
        return 0u;
    }

    distance = DispatchCalculateFineReadAcquisitionMoveMagnitude();
    WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, distance);
    if(distance >= 10u)
    {
        DispatchMovePickupByDistanceInChunks(direction, distance);
    }
    else
    {
        DispatchMovePickupByByteDistance((u8)distance, direction);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    }
    return 0u;
}

/* RAM:fc8ee4 */
u8 AcquireAndValidateReadPositionPacket(void)
{
    u8 packet_status;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    DispatchReadPositionRearmQCapture();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE2) == 1u)
        DispatchReadPositionApplyChannels0Through3();
    else
        DispatchReadPositionApplyChannels1Through3();

    for(;;)
    {
        packet_status = DispatchReadPositionProcessQPacket();
        if(packet_status != 0xfeu)
            break;
        if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1207) <= 0)
        {
            packet_status = 0u;
            break;
        }
        DispatchReadPositionYield();

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u || DispatchReadPositionPacketAccepted() == 1u)
            continue;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_F9E9) & 4u) != 0u && DispatchReadPositionBit001c() != 1u && DispatchReadPositionBit001d() == 0u)
            continue;
        if(DispatchReadPositionTimingHandshake() == 1u && (ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_FAAC) & 0x20u) == 0u)
            continue;

        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_FAAC, (u16)(ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_FAAC) & 0xffdfu));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE8, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE8) + 1u));
        DispatchReadPositionSetControllerOperating(1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) & 0xfbu));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E3F) == 1u)
            DispatchReadPositionSelectOperatingCode(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EE9));
        else
            DispatchReadPositionSelectOperatingCode(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0E3B));
        DispatchReadPositionArmThreeByteCapture();
        do
        {
            packet_status = DispatchReadPositionParseThreeBytePacket();
            if(packet_status == 0xfeu)
                DispatchReadPositionYield();
        } while(packet_status == 0xfeu);
        break;
    }

    if(packet_status == 1u)
        return 1u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, FirmwareReadNear8(FW_NEAR_UNKNOWN_F79A));
    return 0u;
}

/* RAM:fc87fe */
u8 ReduceReadSpeedForAcquisitionRecovery(u8 force_minimum_speed)
{
    u8 speed;

    if((FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 1u) != 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46) <= 4u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) > 5u)
        return 0u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 1u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E41) != 7u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 6u)
            return 0u;
    }

    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
    switch(speed)
    {
    case 8u:
        speed = 4u;
        break;
    case 24u:
        speed = 8u;
        break;
    case 32u:
        speed = 24u;
        break;
    case 40u:
        speed = 32u;
        break;
    case 52u:
        speed = 40u;
        break;
    default:
        for(;;)
        {
        }
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, speed);

    if(force_minimum_speed == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E4B, 4u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 2u));
    }

    speed = FirmwareReadNear8(FW_NEAR_UNKNOWN_0E46);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E49, speed);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0F45, speed);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 5u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1037) + 1u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC3E) >= 5u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1037, 5u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 4u));
    return 1u;
}

/* RAM:fc8ff3 */
void PLEXTOR_NEAR_FUNCTION UpdateReadAcquisitionMoveDistance(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0EE3, 0u);

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) != 0u)
    {
        DispatchUpdateReadAcquisitionFrameDistanceFromMsfTuple();
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, DispatchCalculateRadialModelDistanceToReadTarget());
        return;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE) & 4u) != 0u)
    {
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 50u);
        return;
    }

    if(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EF1) == 0ul)
    {
        WriteBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EF1, (u32)DispatchConvertReadAcquisitionMsfTuple(FW_NEAR_UNKNOWN_1241));
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 300u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
            WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 50u);
        return;
    }

    DispatchUpdateReadAcquisitionFrameDistanceFromMsfTuple();
    WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, (u16)(ReadBigEndianU32ForMoveDistance(FW_NEAR_UNKNOWN_0EE5) >> 3));
    WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, (u16)(ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC) + 5u));
    if((s16)ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC) <= 10)
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 10u);
    if(ReadBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC) >= 200u)
        WriteBigEndianU16ForMoveDistance(FW_NEAR_UNKNOWN_0EDC, 200u);
}
