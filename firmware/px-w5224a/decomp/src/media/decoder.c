#include "decoder.h"
#include "../platform/controller_parameters.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/media_interrupt_leaves.h"
#include "../platform/status_mapping.h"

extern void YieldCoroutine1(void);
extern void RestartCoroutine1AtF84B0BAndYield(void);

/* Reconstruction helpers; no direct Ghidra function addresses. */
static u32 ReadBigEndianU32Decoder(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteBigEndianU32Decoder(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

/* RAM:f8ecad */
void InitializeReadAcquisitionRangeFromLastTocTrack(void)
{
    u8 track_number;
    fw_addr_t tuple_address;
    s32 frame_position;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBE, 4u);
    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1149);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E42, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FBF, track_number);

    tuple_address = FW_HUGE_UNKNOWN_2BB042 + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42) * 10u);
    frame_position = ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0E3B, (u32)frame_position);
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC2, (u32)frame_position);
    frame_position = (s32)ReadBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC2) + 300l;
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC2, (u32)frame_position);
    frame_position = (s32)ReadBigEndianU32Decoder(FW_NEAR_UNKNOWN_0E3B) + 300l;
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0E3B, (u32)frame_position);

    tuple_address = FW_HUGE_UNKNOWN_2BB03E + (fw_addr_t)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_0E42) * 10u);
    frame_position = ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC6, (u32)frame_position);
    frame_position = (s32)ReadBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC6) - 300l;
    WriteBigEndianU32Decoder(FW_NEAR_UNKNOWN_0FC6, (u32)frame_position);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E46, IsIndexedDriveStateActive(0x1fu) != 0u ? 8u : 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E40, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
}

/* RAM:f8ed42 */
void AdvanceMediaState3IndexedTransitionState(void)
{
    u8 transition_step;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE6) >= 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FAAB) != 0x12u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB2, 7u);
        RestartCoroutine1AtF84B0BAndYield();
    }

    transition_step = FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE6);
    if(transition_step == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 4u);
    }
    else if(transition_step == 1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 8u);
    }
    else if(transition_step == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E47, 16u);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB0, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAB1, 1u);
        RestartCoroutine1AtF84B0BAndYield();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1304, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) | 0x20u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FAF, 6u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E3F, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FAE6, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FAE6) + 1u));
}

/* RAM:f8f2a5 */
void ConfigureReadAcquisitionControllerForOperatingCode(void)
{
    u8 parameter_d1;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) > 10u || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u)
        return;

    parameter_d1 = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) & 0xf3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC13, parameter_d1);
    WriteControllerParameterWord((u16)(0xd100u | (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13)));
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180A1, 0xc2u);
    WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D0, 3u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) > 4u)
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D1, 0x10u);
    else
        WaitControllerPortReadyAndWriteByte(FW_CONTROLLER_UNKNOWN_0180D1, 0u);
}

/* Reconstruction helper; no direct Ghidra function address.
 * Uses shift/subtract division so target code does not introduce a C251
 * runtime-library divide helper.  The caller supplies the nonzero 30. */
static u32 DivideU32QuotientWithoutRuntimeLibrary(u32 dividend, u32 divisor)
{
    u32 quotient = 0ul;
    u32 remainder = 0ul;
    u8 bit_count = 32u;

    do
    {
        u8 high_bit = (u8)(dividend >> 31);
        u8 remainder_overflow = (u8)(remainder >> 31);

        dividend <<= 1;
        remainder = (remainder << 1) | (u32)high_bit;
        quotient <<= 1;
        if(remainder_overflow != 0u || remainder >= divisor)
        {
            remainder -= divisor;
            quotient |= 1ul;
        }
        bit_count--;
    } while(bit_count != 0u);
    return quotient;
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 SignedRemainderBy30(s32 dividend)
{
    u32 magnitude = (u32)dividend;
    u32 quotient;
    u32 remainder;

    if(dividend < 0)
        magnitude = 0ul - magnitude;
    quotient = DivideU32QuotientWithoutRuntimeLibrary(magnitude, 30ul);
    remainder = magnitude - ((quotient << 4) + (quotient << 3) + (quotient << 2) + (quotient << 1));
    return dividend < 0 ? (s32)(0ul - remainder) : (s32)remainder;
}

/* Reconstruction helper; no direct Ghidra function address. */
static s32 ReadBigEndianS32(u16 address)
{
    u32 value =
        ((u32)FirmwareReadNear8(address) << 24) | ((u32)FirmwareReadNear8((u16)(address + 1u)) << 16) | ((u32)FirmwareReadNear8((u16)(address + 2u)) << 8) | FirmwareReadNear8((u16)(address + 3u));
    return (s32)value;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void WriteBigEndianS32(u16 address, s32 value)
{
    u32 bits = (u32)value;

    FirmwareWriteNear8(address, (u8)(bits >> 24));
    FirmwareWriteNear8((u16)(address + 1u), (u8)(bits >> 16));
    FirmwareWriteNear8((u16)(address + 2u), (u8)(bits >> 8));
    FirmwareWriteNear8((u16)(address + 3u), (u8)bits);
}

/* RAM:fb93f8 */
void UpdateControllerPacketThirtyFramePhaseMatchFlag(u8 packet_phase_class)
{
    s32 remainder;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 0x40u) == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 1u)
        return;

    if(packet_phase_class == 2u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_113F, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB53)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1140, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB54)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1141, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB55)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 1u);
        WriteBigEndianS32(FW_NEAR_UNKNOWN_114F, ConvertBinaryMsfToAdjustedFramePosition(FW_NEAR_UNKNOWN_113F));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_102F, 0u);
    }

    remainder = SignedRemainderBy30((s32)((u32)ReadBigEndianS32(FW_NEAR_UNKNOWN_1235) - (u32)ReadBigEndianS32(FW_NEAR_UNKNOWN_114F)));
    if((packet_phase_class == 1u && remainder == 9) || (packet_phase_class == 2u && remainder == 19) || (packet_phase_class == 3u && remainder == 29))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 1u));
}

/* RAM:fb8e30 */
u8 ParseControllerThreeBytePacket(void)
{
    u8 raw0;
    u8 raw1;
    u8 raw2;
    u16 packet_class;
    u8 result = 0xfeu;
    u8 saved_ie;
    u8 saved_a9;
    u8 packet_bit;
    u8 packet_phase;
    u8 packet_first;
    u16 packet_destination;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 2u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 4u));
        return 0u;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1206) <= 0 && !(IsIndexedDriveStateActive(4u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0FAF) == 5u))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 6u);
        return 0u;
    }

    saved_ie = FirmwareReadDirectSfr8(A8);
    saved_a9 = FirmwareReadDirectSfr8(A9);
    FirmwareWriteDirectSfr8(A8, (u8)(saved_ie & 0x7fu));

    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) < 0)
    {
        raw0 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1133);
        raw1 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1134);
        raw2 = FirmwareReadNear8(FW_NEAR_UNKNOWN_1135);
        packet_class = (u16)(((raw0 & 0x80u) != 0u ? 0x100u : 0u) | ((raw1 & 0x80u) != 0u ? 0x010u : 0u) | ((raw2 & 0x80u) != 0u ? 0x001u : 0u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB94, 0u);

        if(packet_class == 1u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 0x10u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 2u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7F, (u8)(raw0 & 0x70u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB80, (u8)(raw0 & 0x0fu));
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) != 6u)
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) | 2u));
                else
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1306, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 0xfdu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB81, (u8)(raw1 & 0x70u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB82, (u8)(raw1 & 0x0fu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB83, (u8)(raw2 & 0x70u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB84, (u8)(raw2 & 0x0fu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) | 0x10u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                result = 1u;
            }
            else
            {
                ArmControllerThreeBytePacketCapture();
            }
        }
        else if(packet_class == 0x10u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 8u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 2u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB85, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB85) & 0x80u) | ((raw0 & 0x70u) >> 4) | ((raw0 & 0x0fu) << 3)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB86, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB86) & 0xc0u) | ((raw1 & 0x70u) >> 4) | ((raw1 & 0x0eu) << 2)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB87, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 0xc0u) | ((raw2 & 0x70u) >> 4) | ((raw2 & 0x0eu) << 2)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) | 8u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                result = 1u;
            }
            else
            {
                ArmControllerThreeBytePacketCapture();
            }
        }
        else if(packet_class == 0x11u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 4u) == 0u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 2u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB89, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) & 0x80u) | ((raw0 & 0x60u) >> 5) | ((raw0 & 0x1fu) << 2)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB8A, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8A) & 0x80u) | (raw1 & 0x7fu)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB8B, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8B) & 0x80u) | ((raw2 & 0x78u) >> 3) | ((raw2 & 7u) << 4)));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) | 4u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                result = 1u;
            }
            else
            {
                ArmControllerThreeBytePacketCapture();
            }
        }
        else if(packet_class == 0x100u)
        {
            if(PackedBcdByteToBinary(raw0) < 0x78u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1137, PackedBcdByteToBinary(raw0));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1138, PackedBcdByteToBinary(raw1));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1139, PackedBcdByteToBinary(raw2));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1241, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1242, FirmwareReadNear8(FW_NEAR_UNKNOWN_1138));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1243, FirmwareReadNear8(FW_NEAR_UNKNOWN_1139));
                UpdateDecodedFramePositionAndRadialEstimate(FirmwareReadNear8(FW_NEAR_UNKNOWN_1137), FW_NEAR_UNKNOWN_1241);
                if(ReadBigEndianS32(FW_NEAR_UNKNOWN_1235) >= 0)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1138));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_123F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1139));
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) & 0xf7u));
                    UpdateDecodedFramePositionAndRadialEstimate(FirmwareReadNear8(FW_NEAR_UNKNOWN_1137), FW_NEAR_UNKNOWN_123D);
                }
                saved_a9 = 0u;
                result = 1u;
            }
            else
            {
                ArmControllerThreeBytePacketCapture();
            }
        }
        else if(packet_class == 0x101u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 0x80u) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 2u) == 0u)
            {
                ArmControllerThreeBytePacketCapture();
            }
            else
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7A, (u8)(raw0 & 0x70u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7B, (u8)(raw0 & 7u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7C, raw1);
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7D, (u8)(raw2 & 0x40u));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB7E, (u8)(raw2 & 0x3fu));
                if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
                {
                    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB88, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E) >> 3));
                    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB88) != 0u)
                        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB87, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) | 0x40u));
                }
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) | 0x80u));
                UpdateControllerPacketThirtyFramePhaseMatchFlag(1u);
                saved_a9 = 0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                result = 1u;
            }
        }
        else if(packet_class == 0x110u || packet_class == 0x111u)
        {
            packet_bit = packet_class == 0x110u ? 0x40u : 0x20u;
            packet_phase = packet_class == 0x110u ? 2u : 3u;
            packet_destination = packet_class == 0x110u ? 0xfb53u : 0xfb57u;

            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & packet_bit) != 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1301) & 2u) == 0u)
            {
                ArmControllerThreeBytePacketCapture();
            }
            else
            {
                packet_first = (u8)(raw0 & 0x7fu);

                if(packet_class == 0x111u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBEC) == 1u && packet_first < 0x20u)
                    packet_first = raw0;
                FirmwareWriteNear8(packet_destination, packet_first);
                FirmwareWriteNear8((u16)(packet_destination + 1u), (u8)(raw1 & 0x7fu));
                FirmwareWriteNear8((u16)(packet_destination + 2u), (u8)(raw2 & 0x7fu));
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) | packet_bit));
                UpdateControllerPacketThirtyFramePhaseMatchFlag(packet_phase);
                saved_a9 = 0u;
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) | 8u));
                result = 1u;
            }
        }
        else if(packet_class == 0u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1030) == 0u && raw0 < 9u)
                raw0 = (u8)(raw0 + 0xa0u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1137, PackedBcdByteToBinary(raw0));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1138, PackedBcdByteToBinary(raw1));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1139, PackedBcdByteToBinary(raw2));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_123D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1137));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_123E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1138));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_FB5C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5C) & 0xf7u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_123F, FirmwareReadNear8(FW_NEAR_UNKNOWN_1139));
            UpdateDecodedFramePositionAndRadialEstimate(FirmwareReadNear8(FW_NEAR_UNKNOWN_1137), FW_NEAR_UNKNOWN_123D);
            saved_a9 = 0u;
            result = 1u;
        }
        else
        {
            ArmControllerThreeBytePacketCapture();
        }
    }

    FirmwareWriteDirectSfr8(A8, saved_ie);
    FirmwareWriteDirectSfr8(A9, saved_a9);
    return result;
}

/* RAM:fb8cff */
/* Native code returns through ERET after its ACC Boolean result.  The C form
 * exposes that result as an ordinary value; callers that require the native
 * return frame remain separate adapters. */
u8 AcquireReadPositionPacketWithChannelSetup(void)
{
    u8 packet_result;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1300, 1u);
    ArmControllerThreeBytePacketCapture();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0EE2) != 0u)
        ApplyCurrentOperatingCodeToControllerChannels0Through3();
    else
        ApplyCurrentOperatingCodeToControllerChannels1Through3();

    do
    {
        packet_result = ParseControllerThreeBytePacket();
        if(packet_result != 0xfeu)
            break;
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC13) & 0x20u) != 0u)
            ApplyControllerD1ModeBitsAfterPulseTest();
        else
            YieldCoroutine1();
    } while(1);

    if(packet_result != 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0E41, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB94));
    return (u8)(packet_result == 1u);
}

/* RAM:fb97ec */
/* The RAM:fb9838 worker leaves the captured word in WR6; its declaration in
 * media_interrupt_leaves.h records that specific C251 u16 result contract. */
void CaptureControllerD8D9WindowIntoMeasurementSeries(void)
{
    u32 current_position;
    u32 previous_position;
    u8 sequence_phase;
    u16 captured_word;
    fw_addr_t measurement_series_address;

    current_position = FirmwareReadNearU32(FW_NEAR_UNKNOWN_1235);
    previous_position = FirmwareReadNearU32(FW_NEAR_UNKNOWN_0FE5);
    if(current_position == previous_position)
        return;

    sequence_phase = FirmwareReadNear8(FW_NEAR_UNKNOWN_0FED);
    if(sequence_phase >= 5u)
        return;

    captured_word = CaptureControllerD8D9WindowAndUpdateAdjustment();
    measurement_series_address = (fw_addr_t)(FW_NEAR_UNKNOWN_F03F + ((fw_addr_t)sequence_phase << 1));
    FirmwareWrite8(measurement_series_address, (u8)(captured_word >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(measurement_series_address, 1u), (u8)captured_word);
    measurement_series_address = (fw_addr_t)(FW_NEAR_UNKNOWN_0FDB + ((fw_addr_t)sequence_phase << 1));
    FirmwareWrite8(measurement_series_address, (u8)(captured_word >> 8));
    FirmwareWrite8(FirmwareAddressIncrement24(measurement_series_address, 1u), (u8)captured_word);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FED, (u8)(sequence_phase + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FE5, (u8)(current_position >> 24));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FE6, (u8)(current_position >> 16));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FE7, (u8)(current_position >> 8));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0FE8, (u8)current_position);
}
