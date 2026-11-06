#include "controller_q.h"
#include "../platform/controller_parameters.h"
#include "../platform/controller_progress.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/controller_q_bindings.h"
#undef PLEXTOR_BINDING_SECTION

/* RAM:fcf967 */
void PLEXTOR_NEAR_FUNCTION ProcessControllerQAdr0PacketForState2(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, 0u);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1250) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) | 8u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, FirmwareReadNear8(FW_NEAR_UNKNOWN_1250));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, FirmwareReadNear8(FW_NEAR_UNKNOWN_1251));
    DispatchDecodeControllerQPacketBcdTuples();

    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) >> 3) & 1u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123D, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123E, FirmwareReadNear8(FW_NEAR_UNKNOWN_124D));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123F, FirmwareReadNear8(FW_NEAR_UNKNOWN_124E));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1248));
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_125C) >= 0x5au)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, 0xffu);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 8u)
        DispatchUpdateControllerWord035FFromProgressBucket();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        DispatchUpdateDecodedFramePositionAndRadialEstimate(FirmwareReadNear8(FW_NEAR_UNKNOWN_124C), FW_NEAR_UNKNOWN_123D);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, FirmwareReadNear8(FW_NEAR_UNKNOWN_124F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1305, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1305) | 0x80u));
}

/* RAM:fcfa0e */
void PLEXTOR_NEAR_FUNCTION ProcessControllerQAdr1Packet(void)
{
    u8 track_number;
    u8 index_or_point;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1250) == 0xaau && FirmwareReadNear8(FW_NEAR_UNKNOWN_1246) != 0xaau)
        CopyBytesWithinFarBanks(FW_NEAR_UNKNOWN_1245, FW_NEAR_UNKNOWN_F578, 10u);

    ParseControllerQPacket();

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_1250);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, track_number);
    if(track_number < 0xa0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1246, PackedBcdByteToBinary(track_number));

    index_or_point = FirmwareReadNear8(FW_NEAR_UNKNOWN_1251);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, index_or_point);
    if(index_or_point < 0xa0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1247, PackedBcdByteToBinary(index_or_point));

    DispatchDecodeControllerQPacketBcdTuplesForAdr1();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1247) == 0xa0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_124D, FirmwareReadNear8(FW_NEAR_UNKNOWN_1257));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1241, FirmwareReadNear8(FW_NEAR_UNKNOWN_1248));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1242, FirmwareReadNear8(FW_NEAR_UNKNOWN_1249));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1243, FirmwareReadNear8(FW_NEAR_UNKNOWN_124A));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_125A) & 8u) == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123D, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123E, FirmwareReadNear8(FW_NEAR_UNKNOWN_124D));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_123F, FirmwareReadNear8(FW_NEAR_UNKNOWN_124E));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_124C));
    }
    else
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, FirmwareReadNear8(FW_NEAR_UNKNOWN_1248));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_125C) >= 0x5au)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_125C, 0u);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_125D, 0xffu);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0E43) >= 8u)
        DispatchUpdateControllerWord035FFromAdr1Progress();

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1245, FirmwareReadNear8(FW_NEAR_UNKNOWN_124F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1233, FirmwareReadNear8(FW_NEAR_UNKNOWN_1246));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1006) == 0u)
        DispatchUpdateDecodedFramePositionFromAdr1(FirmwareReadNear8(FW_NEAR_UNKNOWN_124C), FW_NEAR_UNKNOWN_123D);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_124B, FirmwareReadNear8(FW_NEAR_UNKNOWN_1255));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1303, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1303) | 4u));
}
