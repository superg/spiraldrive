#include "identify_packet_device.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

static u16 ReadIdentifyBigEndianU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadBigEndianU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteIdentifyBigEndianU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static fw_addr_t ResponseAddress(fw_addr_t response_base, u16 offset)
{
    return FirmwareAddressIncrementLow16(response_base, offset);
}

static u16 ReadIndexedTableWord(fw_addr_t table, u8 index)
{
    return ReadIdentifyBigEndianU16(FirmwareAddressIncrementLow16(table, (u16)((u16)index * 2u)));
}

/* RAM:fdb4d9 */
void PLEXTOR_NEAR_FUNCTION BuildAtaIdentifyPacketDeviceData(void)
{
    fw_addr_t reserved_region_base = ReadBigEndianU32(FW_NEAR_UNKNOWN_155A);
    /* RAM:fdb4df adds 0x10 to WR12, the HIGH half of DR12: +1 MiB. */
    fw_addr_t response = FirmwareAddressIncrement24(reserved_region_base, FW_ADDRESS_BANK_STRIDE);
    u16 offset;
    u8 pio_mode;
    u8 multiword_dma_mode;
    u8 ultra_dma_enabled;

    for(offset = 0u; offset < 0x200u; ++offset)
        FirmwareWrite8(ResponseAddress(response, offset), 0u);

    WriteIdentifyBigEndianU16(response, 0x85c0u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x14u), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF941D), 0x14u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x22u), 0x0000dd25ul, 0x06u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x2eu), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9431), 0x08u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x2eu), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9418), 0x04u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x36u), 0x0000dccdul, 0x08u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x3eu), 0x0000dcd5ul, 0x10u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x4eu), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9439), 0x04u);
    CopyBytesWith8BitCount(ResponseAddress(response, 0x52u), PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF943D), 0x0cu);

    pio_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2E);
    multiword_dma_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2D);
    ultra_dma_enabled = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C86) & 0x08u);

    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x62u), 0x0f00u);
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x66u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9449), pio_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x68u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9471), multiword_dma_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x6au), ultra_dma_enabled != 0u ? 6u : 2u);
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x7cu), 7u);
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x7eu), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9479), multiword_dma_mode));
    if(ultra_dma_enabled != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        WriteIdentifyBigEndianU16(ResponseAddress(response, 0x7eu), (u16)(ReadIdentifyBigEndianU16(ResponseAddress(response, 0x7eu)) & 0x00ffu));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x80u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9453), pio_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x82u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9481), multiword_dma_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x84u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9481), multiword_dma_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x86u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF945D), pio_mode));
    WriteIdentifyBigEndianU16(ResponseAddress(response, 0x88u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9467), pio_mode));

    if(ultra_dma_enabled != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2B) == 7u)
        WriteIdentifyBigEndianU16(ResponseAddress(response, 0xb0u), ReadIndexedTableWord(PLEXTOR_FIXED_ROM_ADDRESS(FW_ROM_UNKNOWN_FF9489), FirmwareReadNear8(FW_NEAR_UNKNOWN_0C2C)));
    else
        WriteIdentifyBigEndianU16(ResponseAddress(response, 0xb0u), ultra_dma_enabled != 0u ? 7u : 0u);

    SwapByteOrderInWords(response, 0xb2u);
}
