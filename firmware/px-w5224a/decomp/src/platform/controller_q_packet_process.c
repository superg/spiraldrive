#include "controller_q_packet_process.h"
#include "../media/controller_q.h"
#include "controller_parameters.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "memory.h"

static void CopyCapturedQHeader(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, FirmwareReadNear8(FW_NEAR_UNKNOWN_1251));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, FirmwareReadNear8(FW_NEAR_UNKNOWN_124F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, FirmwareReadNear8(FW_NEAR_UNKNOWN_1250));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124B, FirmwareReadNear8(FW_NEAR_UNKNOWN_1255));
}

static void CopyCapturedQTuple1(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1248, FirmwareReadNear8(FW_NEAR_UNKNOWN_1252));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1249, FirmwareReadNear8(FW_NEAR_UNKNOWN_1253));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124A, FirmwareReadNear8(FW_NEAR_UNKNOWN_1254));
}

static void CopyCapturedQTuple2(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1256));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1257));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124E, FirmwareReadNear8(FW_NEAR_UNKNOWN_1258));
}

static u8 AcceptOrdinaryQPacket(u8 require_capture_gate)
{
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 1u) == 1u && AdvanceDecodedFramePositionOutsideLeadInLeadOut() == 1u
        && (require_capture_gate == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_0FF9) & 4u) == 0u))
        return 1u;
    RearmControllerQPacketCaptureWithTimeout();
    return 0xfeu;
}

static void PrepareOrdinaryAdr5Record(void)
{
    DecodeControllerQPacketBcdTuples();
    CopyCapturedQHeader();
}

static u8 ProcessOrdinaryAdr5Packet(void)
{
    u8 point = FirmwareReadNear8(FW_NEAR_UNKNOWN_1251);

    if(point == 0xb0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x10u));
        PrepareOrdinaryAdr5Record();
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F528, 10u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1252) == 0xffu && FirmwareReadNear8(FW_NEAR_UNKNOWN_1253) == 0xffu && FirmwareReadNear8(FW_NEAR_UNKNOWN_1254) == 0xffu)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F52D, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F52C, 0xffu);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_F52B, 0xffu);
        }
    }
    else if(point == 0xb1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x20u));
        PrepareOrdinaryAdr5Record();
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F564, 10u);
    }
    else if(point == 0xc0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x80u));
        PrepareOrdinaryAdr5Record();
        CopyCapturedQTuple1();
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F55A, 10u);
    }
    else if(point == 0xc1u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x40u));
        CopyCapturedQHeader();
        CopyCapturedQTuple1();
        CopyCapturedQTuple2();
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F56E, 10u);
    }
    else
    {
        if(point >= 0xb2u && point <= 0xb4u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x20u));
            PrepareOrdinaryAdr5Record();
            CopyControllerQAdr5PointB2ThroughB4Record();
        }
        if(point >= 1u && point <= 0x40u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1259, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) | 0x20u));
            PrepareOrdinaryAdr5Record();
            CopyControllerQAdr5TrackPointRecord();
        }
    }
    return AcceptOrdinaryQPacket(1u);
}

static u8 ProcessOrdinaryQPacket(u8 adr)
{
    if(adr == 1u)
    {
        ProcessControllerQAdr1Packet();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 1u));
        return 1u;
    }
    if(adr == 2u)
    {
        CaptureControllerQAdr2PacketIfEnabled();
        return AcceptOrdinaryQPacket(1u);
    }
    if(adr == 3u)
    {
        CaptureControllerQAdr3PacketIfEnabled();
        return AcceptOrdinaryQPacket(1u);
    }
    if(adr == 5u)
        return ProcessOrdinaryAdr5Packet();
    if(adr == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0xf7u));
            ProcessControllerQAdr0PacketForState2();
            return 1u;
        }
        return AcceptOrdinaryQPacket(0u);
    }
    return AcceptOrdinaryQPacket(1u);
}

static void PrepareAlternateQPacket(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1251)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, FirmwareReadNear8(FW_NEAR_UNKNOWN_124F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, FirmwareReadNear8(FW_NEAR_UNKNOWN_1250));
    DecodeControllerQPacketBcdTuples();
}

static u8 ProcessBoundedAlternateQPacket(u8 maximum, u8 mask)
{
    u8 point;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, FirmwareReadNear8(FW_NEAR_UNKNOWN_1250));
    point = PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1251));
    if(point == 0u || point > maximum)
    {
        RearmControllerQPacketCaptureWithTimeout();
        return 0u;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, PackedBcdByteToBinary(FirmwareReadNear8(FW_NEAR_UNKNOWN_1251)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, FirmwareReadNear8(FW_NEAR_UNKNOWN_124F));
    DecodeControllerQPacketBcdTuples();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125B) | mask));
    return 1u;
}

static u8 ProcessAlternateQPacket(u8 adr)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125B, 0u);
    if(adr == 1u)
    {
        PrepareAlternateQPacket();
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1252) == 0xffu)
            CopyCapturedQTuple1();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125B) | 1u));
        return 1u;
    }
    if(adr == 2u)
    {
        PrepareAlternateQPacket();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_124D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1257));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125B) | 2u));
        return 1u;
    }
    if(adr == 3u)
        return ProcessBoundedAlternateQPacket(0x15u, 4u);
    if(adr == 4u)
        return ProcessBoundedAlternateQPacket(0x15u, 8u);
    if(adr == 5u)
        return ProcessBoundedAlternateQPacket(0x28u, 0x10u);
    if(adr == 6u)
        return ProcessBoundedAlternateQPacket(0x28u, 0x20u);
    if(adr == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        ProcessControllerQAdr0PacketForState2();
        return 1u;
    }
    RearmControllerQPacketCaptureWithTimeout();
    return 0xfeu;
}

/* RAM:fcf36f */
u8 ProcessCapturedControllerQPacket(void)
{
    u8 flags;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) & 0x7fu));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) & 2u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79A, 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_11EF, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_11EF) | 4u));
        return 0u;
    }
    if((s8)FirmwareReadNear8(FW_NEAR_UNKNOWN_1205) <= 0)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79A, 6u);
        return 0u;
    }

    flags = FirmwareReadNear8(FW_NEAR_UNKNOWN_125A);
    if((flags & 0x80u) == 0u)
        return 0xfeu;
    if((flags & 0x40u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(flags & 0xbfu));
        if((flags & 1u) == 1u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 2u) == 0u)
            (void)AdvanceDecodedFramePositionOutsideLeadInLeadOut();
        DispatchEightChannelMeasurementCaptureForCurrentQ();
        DispatchControllerCommand0EMeasurementCaptureForCurrentQ();
        AcknowledgeControllerQPacketCapture();
        return 0xfeu;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_F79A, 0u);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1304) & 2u) == 0u)
        return ProcessOrdinaryQPacket((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_124F) & 0x0fu));
    return ProcessAlternateQPacket((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_124F) & 0x0fu));
}
