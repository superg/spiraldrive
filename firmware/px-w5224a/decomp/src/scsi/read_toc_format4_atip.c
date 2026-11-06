#include "read_toc_format4_atip.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_toc_format4_atip_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadReadTocFormat4Word(u16 address)
{
    return FirmwareReadNearU16(address);
}

static void WriteReadTocFormat4UnavailableHeader(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, 0u);
}

static void ClearReadTocFormat4OptionalFields(void)
{
    u8 zero;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A, 0u);
    zero = FirmwareReadNear8(FW_NEAR_UNKNOWN_0B2A);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B45, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B44, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B43, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B42, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B41, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B40, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3F, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3E, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3D, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3B, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3A, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B39, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B35, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2D, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2C, zero);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x1au);
}

static void BuildReadTocFormat4Response(void)
{
    u8 low_bits;
    u8 high_bits;

    ClearReadTocFormat4OptionalFields();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2E, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7A) | 0x80u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7B)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2F, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) & 0x40u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7D) | 0x80u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, FirmwareReadNear8(FW_NEAR_UNKNOWN_113F));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, FirmwareReadNear8(FW_NEAR_UNKNOWN_1140));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, FirmwareReadNear8(FW_NEAR_UNKNOWN_1141));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B36, FirmwareReadNear8(FW_NEAR_UNKNOWN_113B));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B37, FirmwareReadNear8(FW_NEAR_UNKNOWN_113C));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B38, FirmwareReadNear8(FW_NEAR_UNKNOWN_113D));

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E) & 4u) == 4u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) >> 4) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B30) | 4u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3A, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7F)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3B, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB82) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81)));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB83) | 0x80u | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB84)));
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E) & 2u) == 2u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) >> 3) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B30) | 2u));
        low_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB85) & 7u) << 4);
        high_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB85) >> 3) & 0x0fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3E, (u8)(low_bits | high_bits));

        low_bits = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB86) & 7u) << 4) | 0x80u);
        high_bits = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB86) >> 3) & 7u) << 1);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3F, (u8)(low_bits | high_bits));

        high_bits = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) >> 3) & 7u) << 1);
        low_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB87) & 7u) << 4);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B40, (u8)(low_bits | high_bits));
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E) & 1u) == 1u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) >> 2) & 1u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B30) | 1u));
        low_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) & 3u) << 5);
        high_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB89) >> 2) & 0x1fu);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B42, (u8)(low_bits | high_bits));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B43, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8A) & 0x7fu) | 0x80u));

        low_bits = (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8B) & 0x0fu) << 3) | 0x80u);
        high_bits = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB8B) >> 4) & 7u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B44, (u8)(low_bits | high_bits));
    }
}

/* RAM:f9b178 */
void PLEXTOR_NEAR_FUNCTION HandleReadTocFormat4Atip(void)
{
    u16 allocation_length;
    u16 transfer_length;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0B10) != 0u)
    {
        REJECT_READ_TOC_FORMAT4();
        return;
    }

    allocation_length = ReadReadTocFormat4Word(FW_NEAR_UNKNOWN_0B11);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 0u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) != 0u && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C88) >> 1) & 1u) != 0u))
    {
        WriteReadTocFormat4UnavailableHeader();
        transfer_length = READ_TOC_FORMAT4_MIN(allocation_length, 4u);
    }
    else
    {
        BuildReadTocFormat4Response();
        transfer_length = READ_TOC_FORMAT4_MIN(allocation_length, 0x1cu);
    }

    if(TRANSFER_READ_TOC_FORMAT4(FW_NEAR_UNKNOWN_0B2A, (u32)transfer_length) != 4u)
        SET_READ_TOC_FORMAT4_NO_SENSE();
}
