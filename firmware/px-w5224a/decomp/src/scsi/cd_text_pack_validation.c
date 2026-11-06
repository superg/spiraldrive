#include "cd_text_pack_validation.h"
#include "../platform/arithmetic.h"
#include "../platform/firmware_memory.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/cd_text_pack_validation_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadCdTextPackU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadCdTextPackU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteCdTextPackU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static fw_addr_t GetCdTextPackAddress(u32 pack_index)
{
    u32 address;
    u16 high_word;

    address = ReadCdTextPackU32(FW_NEAR_UNKNOWN_1556) - 0x8ffcul + MultiplyU32Low(pack_index, 0x12ul);
    high_word = (u16)((u16)(address >> 16) + 0x10u);
    return ((u32)high_word << 16) | (u16)address;
}

/* RAM:fd781e */
u8 PLEXTOR_NEAR_FUNCTION ValidateCdTextPackAndAccumulatePayloadLengths(u32 pack_index)
{
    u8 transformed[18];
    fw_addr_t pack_address;
    fw_addr_t table_address;
    u16 accumulated_length;
    u8 expected_sequence;
    u8 received_crc_high;
    u8 received_crc_low;
    u8 table_index;
    u8 payload_length;
    u8 index;

    pack_address = GetCdTextPackAddress(pack_index);
    for(index = 0u; index < 16u; ++index)
        transformed[index] = FirmwareRead8(pack_address + index);
    transformed[16] = 0u;
    transformed[17] = 0u;
    received_crc_high = FirmwareRead8(pack_address + 16u);
    received_crc_low = FirmwareRead8(pack_address + 17u);

    if((transformed[0] & 0xf0u) != 0x80u)
        return 0u;

    expected_sequence = FirmwareReadNear8(FW_NEAR_UNKNOWN_15F1);
    if(transformed[2] == expected_sequence)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F1, (u8)(expected_sequence + 1u));
    }
    else if(transformed[2] == 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_15F1, 1u);
    }
    else
    {
        CLEAR_CD_TEXT_PARTIAL_PACK_BIT();
        return 0u;
    }

    for(index = 0u; index < 16u; ++index)
    {
        table_index = transformed[index];
        table_address = FW_NEAR_UNKNOWN_F596 + (u16)table_index * 2u;
        transformed[index + 1u] ^= FirmwareRead8(table_address);
        transformed[index + 2u] ^= FirmwareRead8(table_address + 1u);
    }

    if(received_crc_high != (u8)(transformed[16] ^ 0xffu) || received_crc_low != (u8)(transformed[17] ^ 0xffu))
        return 0u;

    if(FirmwareRead8(pack_address) == 0x8fu)
    {
        if(FirmwareRead8(pack_address + 1u) == 1u)
        {
            WriteCdTextPackU16(FW_NEAR_UNKNOWN_F796, 0u);
            for(index = 12u; index <= 15u; ++index)
            {
                payload_length = FirmwareRead8(pack_address + index);
                if(payload_length != 0u)
                {
                    accumulated_length = ReadCdTextPackU16(FW_NEAR_UNKNOWN_F796);
                    accumulated_length = (u16)(accumulated_length + payload_length + 1u);
                    WriteCdTextPackU16(FW_NEAR_UNKNOWN_F796, accumulated_length);
                }
            }
        }
        if(FirmwareRead8(pack_address + 1u) == 2u)
        {
            WriteCdTextPackU16(FW_NEAR_UNKNOWN_F798, 0u);
            for(index = 4u; index <= 7u; ++index)
            {
                payload_length = FirmwareRead8(pack_address + index);
                if(payload_length != 0u)
                {
                    accumulated_length = ReadCdTextPackU16(FW_NEAR_UNKNOWN_F798);
                    accumulated_length = (u16)(accumulated_length + payload_length + 1u);
                    WriteCdTextPackU16(FW_NEAR_UNKNOWN_F798, accumulated_length);
                }
            }
        }
    }

    return 1u;
}
