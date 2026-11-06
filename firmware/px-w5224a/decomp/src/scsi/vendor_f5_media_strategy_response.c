#include "vendor_f5_media_strategy_response.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/vendor_f5_media_strategy_response_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadVendorF5StrategyWord(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadVendorF5StrategyDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteVendorF5StrategyWord(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteVendorF5StrategyDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u16 ReverseVendorF5CapabilityBits(u16 value)
{
    u16 reversed = 0u;
    u8 index;

    for(index = 0u; index < 16u; ++index)
    {
        if((value & 1u) != 0u)
            reversed |= 1u;
        value >>= 1;
        if(index != 15u)
            reversed <<= 1;
    }
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_FC21) & 4u) != 0u)
        reversed |= 1u;
    return reversed;
}

static void CopyVendorF5CommonHeader(void)
{
    u8 index;

    for(index = 0u; index < 0x52u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2A + index, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B2B, 0x50u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B30, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB53));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B31, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB54));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B32, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB55));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B33, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5D));
}

static void BuildVendorF5FixedMediaStrategyResponse(void)
{
    float scaled;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B34, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5E));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B36, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_FB63));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B38, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_FB65));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B3A, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_FB67));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3C, FirmwareReadNear8(FW_NEAR_UNKNOWN_F964));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3D, FirmwareReadNear8(FW_NEAR_UNKNOWN_F965));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B3E, FirmwareReadNear8(FW_NEAR_UNKNOWN_F966));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B40, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_E752));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B42, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_E754));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B44, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_E756));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B46, ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_E758));

    scaled = (float)ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_F9FA);
    scaled /= 100.0f;
    scaled *= (float)ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_0FFE);
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B48, (u16)(s32)scaled);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4A, FirmwareReadNear8(FW_NEAR_UNKNOWN_F968));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4B, FirmwareReadNear8(FW_NEAR_UNKNOWN_F969));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4C, FirmwareReadNear8(FW_NEAR_UNKNOWN_F96A));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B4D, FirmwareReadNear8(FW_NEAR_UNKNOWN_F96B));
    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B56, ReverseVendorF5CapabilityBits(ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_FAD7)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B58, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB57));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B59, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB58));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B5A, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB59));
    WriteVendorF5StrategyDword(FW_NEAR_UNKNOWN_0B5C, ReadVendorF5StrategyDword(FW_NEAR_UNKNOWN_F9EE));
    WriteVendorF5StrategyDword(FW_NEAR_UNKNOWN_0B60, ReadVendorF5StrategyDword(FW_NEAR_UNKNOWN_F9F2));
    WriteVendorF5StrategyDword(FW_NEAR_UNKNOWN_0B64, ReadVendorF5StrategyDword(FW_NEAR_UNKNOWN_F9F6));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B68, FirmwareReadNear8(FW_NEAR_UNKNOWN_FAD4));
}

static void WriteVendorF5SignedByteWord(fw_addr_t destination, fw_addr_t source)
{
    WriteVendorF5StrategyWord(destination, (u16)(s16)(s8)FirmwareRead8(source));
}

static void CopyVendorF5RecordableProfile(fw_addr_t profile, fw_addr_t auxiliary, fw_addr_t fixed, fw_addr_t destination)
{
    WriteVendorF5StrategyWord(destination, (u16)FirmwareRead8(profile));
    WriteVendorF5SignedByteWord(destination + 2u, profile + 3u);
    WriteVendorF5StrategyWord(destination + 4u, ReadVendorF5StrategyWord(profile + 4u));
    WriteVendorF5SignedByteWord(destination + 6u, profile + 6u);
    WriteVendorF5SignedByteWord(destination + 8u, profile + 7u);
    WriteVendorF5SignedByteWord(destination + 10u, profile + 8u);
    FirmwareWrite8(destination + 12u, FirmwareRead8(profile + 1u));
    FirmwareWrite8(destination + 13u, FirmwareRead8(profile + 9u));
    FirmwareWrite8(destination + 14u, FirmwareRead8(fixed));
    FirmwareWrite8(destination + 15u, FirmwareRead8(fixed + 1u));
    FirmwareWrite8(destination + 16u, FirmwareRead8(fixed + 2u));
    FirmwareWrite8(destination + 17u, FirmwareRead8(fixed + 3u));
    FirmwareWrite8(destination + 18u, FirmwareRead8(auxiliary));
    FirmwareWrite8(destination + 19u, FirmwareRead8(auxiliary + 1u));
    FirmwareWrite8(destination + 20u, FirmwareRead8(auxiliary + 2u));
}

static void BuildVendorF5RecordableMediaStrategyResponse(u32 inherited_dr12)
{
    u8 record_family = FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80);
    u8 strategy_index;
    fw_addr_t profile = inherited_dr12;
    fw_addr_t auxiliary = 0ul;

    if(record_family != 0u && record_family < 6u)
    {
        strategy_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F);
        if(record_family == 2u)
        {
            profile = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83084) + VENDOR_F5_MULTIPLY(11ul, (u32)strategy_index);
            auxiliary = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83469) + VENDOR_F5_MULTIPLY(3ul, (u32)strategy_index);
        }
        else if(record_family == 4u)
        {
            profile = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F831AD) + VENDOR_F5_MULTIPLY(11ul, (u32)strategy_index);
            auxiliary = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F834BA) + VENDOR_F5_MULTIPLY(3ul, (u32)strategy_index);
        }
        CopyVendorF5RecordableProfile(profile, auxiliary, PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83451), FW_NEAR_UNKNOWN_0B34);

        if(record_family == 4u)
        {
            strategy_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_F05F);
            profile = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F832B5) + VENDOR_F5_MULTIPLY(11ul, (u32)strategy_index);
            auxiliary = PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83505) + VENDOR_F5_MULTIPLY(3ul, (u32)strategy_index);
            CopyVendorF5RecordableProfile(profile, auxiliary, PLEXTOR_MAIN_ROM_ADDRESS(FW_ROM_UNKNOWN_F83459), FW_NEAR_UNKNOWN_0B4A);
        }
    }

    WriteVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B76, ReverseVendorF5CapabilityBits(ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_FAD7)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B78, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB57));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B79, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB58));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_0B7A, FirmwareReadNear8(FW_NEAR_UNKNOWN_FB59));
}

/* RAM:fbeb4c */
void BuildVendorF5MediaStrategyResponse(void)
{
    u32 inherited_dr12 = CAPTURE_VENDOR_F5_RESPONSE_DR12();
    u16 allocation_length;
    u8 media_state;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C85) & 2u) == 0u)
    {
        VENDOR_F5_SET_SENSE(2u, 0x0401u);
        return;
    }

    CopyVendorF5CommonHeader();
    media_state = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    if(media_state == 1u)
        BuildVendorF5FixedMediaStrategyResponse();
    else if(media_state == 2u)
        BuildVendorF5RecordableMediaStrategyResponse(inherited_dr12);

    allocation_length = MinU16(ReadVendorF5StrategyWord(FW_NEAR_UNKNOWN_0B12), 0x52u);
    if(VENDOR_F5_TRANSFER(FW_NEAR_UNKNOWN_0B2A, (u32)allocation_length) != 4u)
        VENDOR_F5_NO_SENSE();
}
