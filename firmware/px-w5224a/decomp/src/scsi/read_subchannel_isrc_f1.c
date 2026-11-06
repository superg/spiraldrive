#include "read_subchannel_isrc_f1.h"
#include "../platform/firmware_memory.h"

static fw_addr_t WriteIsrcPayloadByte(fw_addr_t destination, u8 value)
{
    FirmwareWrite8(destination, value);
    return FirmwareAddressIncrementLow16(destination, 1u);
}

static u32 ReadIsrcPackedDword(fw_addr_t source)
{
    return FirmwareReadU32(source);
}

static u32 ReadIsrcPackedThreeBytes(fw_addr_t source)
{
    u32 value = (u32)FirmwareRead8(source) << 16;

    value |= (u32)FirmwareRead8(source + 1u) << 8;
    value |= FirmwareRead8(source + 2u);
    return value;
}

static u8 IsrcNibbleCharacter(u8 nibble)
{
    return (u8)((nibble & 0x0fu) + 0x30u);
}

/* RAM:f99794 */
void PLEXTOR_NEAR_FUNCTION BuildReadSubChannelIsrcOrF1PayloadC(u8 response_format, fw_addr_t destination_address)
{
    u32 packed;
    u8 value;
    u8 index;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1259) & 4u) == 0u)
    {
        for(index = 0u; index < 16u; ++index)
            destination_address = WriteIsrcPayloadByte(destination_address, 0u);
        return;
    }

    destination_address = WriteIsrcPayloadByte(destination_address, 0x80u);
    packed = ReadIsrcPackedDword(0x000f51ful);
    destination_address = WriteIsrcPayloadByte(destination_address, (u8)(((packed >> 26) & 0x3ful) + 0x30u));
    destination_address = WriteIsrcPayloadByte(destination_address, (u8)(((packed >> 20) & 0x3ful) + 0x30u));
    destination_address = WriteIsrcPayloadByte(destination_address, (u8)(((packed >> 14) & 0x3ful) + 0x30u));
    destination_address = WriteIsrcPayloadByte(destination_address, (u8)(((packed >> 8) & 0x3ful) + 0x30u));
    destination_address = WriteIsrcPayloadByte(destination_address, (u8)(((packed >> 2) & 0x3ful) + 0x30u));

    value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F523);
    destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter((u8)(value >> 4)));
    destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter(value));

    if(response_format == 0xf1u)
    {
        packed = ReadIsrcPackedThreeBytes(FW_NEAR_UNKNOWN_F524);
        destination_address = WriteIsrcPayloadByte(destination_address, (u8)(packed >> 20));
        destination_address = WriteIsrcPayloadByte(destination_address, (u8)(packed >> 12));
        destination_address = WriteIsrcPayloadByte(destination_address, (u8)(packed >> 4));
        destination_address = WriteIsrcPayloadByte(destination_address, 0u);
        destination_address = WriteIsrcPayloadByte(destination_address, 0u);
    }
    else
    {
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F524);
        destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter((u8)(value >> 4)));
        destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter(value));
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F525);
        destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter((u8)(value >> 4)));
        destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter(value));
        value = FirmwareReadNear8(FW_NEAR_UNKNOWN_F526);
        destination_address = WriteIsrcPayloadByte(destination_address, IsrcNibbleCharacter((u8)(value >> 4)));
    }

    destination_address = WriteIsrcPayloadByte(destination_address, 0u);
    destination_address = WriteIsrcPayloadByte(destination_address, 0u);
    (void)WriteIsrcPayloadByte(destination_address, 0u);
}
