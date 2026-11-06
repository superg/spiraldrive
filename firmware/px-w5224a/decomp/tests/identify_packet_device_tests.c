#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ata/identify_packet_device.h"
#include "platform/firmware_memory.h"

typedef struct
{
    fw_addr_t destination;
    fw_addr_t source;
    u8 count;
} CopyCall;

static u8 memory[0x1000000ul];
static CopyCall copy_calls[8];
static unsigned copy_count;
static unsigned swap_count;
static fw_addr_t swap_address;
static u16 swap_words;
static fw_addr_t response_address;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void CopyBytesWith8BitCount(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    u8 index;

    assert(copy_count < 8u);
    copy_calls[copy_count].destination = destination;
    copy_calls[copy_count].source = source;
    copy_calls[copy_count].count = byte_count;
    copy_count++;
    for(index = 0u; index < byte_count; ++index)
    {
        FirmwareWrite8(destination, FirmwareRead8(source));
        destination = FirmwareAddressIncrementLow16(destination, 1u);
        source = FirmwareAddressIncrementLow16(source, 1u);
    }
}

void SwapByteOrderInWords(fw_addr_t address, u16 word_count)
{
    u16 index;

    swap_count++;
    swap_address = address;
    swap_words = word_count;
    for(index = 0u; index < word_count; ++index)
    {
        u8 first = FirmwareRead8(address);
        fw_addr_t second_address = FirmwareAddressIncrementLow16(address, 1u);
        u8 second = FirmwareRead8(second_address);

        FirmwareWrite8(address, second);
        FirmwareWrite8(second_address, first);
        address = FirmwareAddressIncrementLow16(address, 2u);
    }
}

static void WriteBigEndianU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[FirmwareAddressIncrementLow16(address, 1u)] = (u8)value;
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u16 ReadSwappedWord(u16 offset)
{
    fw_addr_t address = FirmwareAddressIncrementLow16(response_address, offset);

    return (u16)(((u16)memory[address + 1u] << 8) | memory[address]);
}

static void FillSource(fw_addr_t address, u8 count, u8 seed)
{
    u8 index;

    for(index = 0u; index < count; ++index)
        memory[address + index] = (u8)(seed + index);
}

static void InitializeTables(void)
{
    unsigned index;

    for(index = 0u; index < 5u; ++index)
    {
        WriteBigEndianU16(0xff9449ul + index * 2u, (u16)(0x1100u + index));
        WriteBigEndianU16(0xff9453ul + index * 2u, (u16)(0x2200u + index));
        WriteBigEndianU16(0xff945dul + index * 2u, (u16)(0x3300u + index));
        WriteBigEndianU16(0xff9467ul + index * 2u, (u16)(0x4400u + index));
    }
    for(index = 0u; index < 4u; ++index)
    {
        WriteBigEndianU16(0xff9471ul + index * 2u, (u16)(0x5500u + index));
        WriteBigEndianU16(0xff9479ul + index * 2u, (u16)(0x6600u + index));
        WriteBigEndianU16(0xff9481ul + index * 2u, (u16)(0x7700u + index));
    }
    for(index = 0u; index < 8u; ++index)
        WriteBigEndianU16(0xff9489ul + index * 2u, (u16)(0x8800u + index));
}

static void ResetCase(void)
{
    memset(memory, 0, sizeof(memory));
    memset(copy_calls, 0, sizeof(copy_calls));
    copy_count = 0u;
    swap_count = 0u;
    swap_address = 0u;
    swap_words = 0u;
    WriteBigEndianU32(0x155aul, 0x00160000ul);
    response_address = 0x00260000ul;
    memset(&memory[response_address], 0xa5, 0x200u);
    FillSource(0xff9418ul, 9u, 0x10u);
    FillSource(0xff941dul, 20u, 0x20u);
    FillSource(0x00dd25ul, 6u, 0x30u);
    FillSource(0xff9431ul, 8u, 0x40u);
    FillSource(0x00dccdul, 8u, 0x50u);
    FillSource(0x00dcd5ul, 16u, 0x60u);
    FillSource(0xff9439ul, 4u, 0x70u);
    FillSource(0xff943dul, 12u, 0x80u);
    InitializeTables();
}

static void AssertCopyTuple(unsigned index, u16 destination_offset, fw_addr_t source, u8 count)
{
    assert(copy_calls[index].destination == FirmwareAddressIncrementLow16(response_address, destination_offset));
    assert(copy_calls[index].source == source);
    assert(copy_calls[index].count == count);
}

static void TestFixedPayloadAndCopyTuples(void)
{
    unsigned index;

    ResetCase();
    memory[0x0c2eu] = 2u;
    memory[0x0c2du] = 1u;
    BuildAtaIdentifyPacketDeviceData();

    assert(copy_count == 8u);
    AssertCopyTuple(0u, 0x14u, 0xff941dul, 0x14u);
    AssertCopyTuple(1u, 0x22u, 0x00dd25ul, 0x06u);
    AssertCopyTuple(2u, 0x2eu, 0xff9431ul, 0x08u);
    AssertCopyTuple(3u, 0x2eu, 0xff9418ul, 0x04u);
    AssertCopyTuple(4u, 0x36u, 0x00dccdul, 0x08u);
    AssertCopyTuple(5u, 0x3eu, 0x00dcd5ul, 0x10u);
    AssertCopyTuple(6u, 0x4eu, 0xff9439ul, 0x04u);
    AssertCopyTuple(7u, 0x52u, 0xff943dul, 0x0cu);
    assert(swap_count == 1u && swap_address == response_address);
    assert(swap_words == 0xb2u);
    assert(ReadSwappedWord(0u) == 0x85c0u);
    assert(ReadSwappedWord(0x62u) == 0x0f00u);
    assert(memory[response_address + 0x1feu] == 0u);
    assert(memory[response_address + 0x1ffu] == 0u);

    for(index = 0u; index < 4u; ++index)
        assert(memory[response_address + 0x2eu + index] == memory[0xff9418u + (index ^ 1u)]);
    for(index = 4u; index < 8u; ++index)
        assert(memory[response_address + 0x2eu + index] == memory[0xff9431u + (index ^ 1u)]);
}

static void TestTransferModeTableSelection(void)
{
    unsigned pio_mode;
    unsigned dma_mode;

    for(pio_mode = 0u; pio_mode <= 4u; ++pio_mode)
    {
        for(dma_mode = 0u; dma_mode <= 2u; ++dma_mode)
        {
            ResetCase();
            memory[0x0c2eu] = (u8)pio_mode;
            memory[0x0c2du] = (u8)dma_mode;
            BuildAtaIdentifyPacketDeviceData();
            assert(ReadSwappedWord(0x66u) == (u16)(0x1100u + pio_mode));
            assert(ReadSwappedWord(0x68u) == (u16)(0x5500u + dma_mode));
            assert(ReadSwappedWord(0x7eu) == (u16)(0x6600u + dma_mode));
            assert(ReadSwappedWord(0x80u) == (u16)(0x2200u + pio_mode));
            assert(ReadSwappedWord(0x82u) == (u16)(0x7700u + dma_mode));
            assert(ReadSwappedWord(0x84u) == (u16)(0x7700u + dma_mode));
            assert(ReadSwappedWord(0x86u) == (u16)(0x3300u + pio_mode));
            assert(ReadSwappedWord(0x88u) == (u16)(0x4400u + pio_mode));
            assert(ReadSwappedWord(0x6au) == 2u);
            assert(ReadSwappedWord(0xb0u) == 0u);
        }
    }
}

static void TestUltraDmaRoutes(void)
{
    ResetCase();
    memory[0x0c2du] = 1u;
    memory[0x0c2cu] = 3u;
    memory[0x0c2bu] = 7u;
    memory[0x0c86u] = 8u;
    BuildAtaIdentifyPacketDeviceData();
    assert(ReadSwappedWord(0x6au) == 6u);
    assert(ReadSwappedWord(0x7eu) == 0x0001u);
    assert(ReadSwappedWord(0xb0u) == 0x8803u);

    ResetCase();
    memory[0x0c2du] = 1u;
    memory[0x0c2bu] = 6u;
    memory[0x0c86u] = 8u;
    BuildAtaIdentifyPacketDeviceData();
    assert(ReadSwappedWord(0x6au) == 6u);
    assert(ReadSwappedWord(0x7eu) == 0x6601u);
    assert(ReadSwappedWord(0xb0u) == 7u);
}

static void TestControllerWindowAddressAndLowWordWrap(void)
{
    unsigned index;

    ResetCase();
    WriteBigEndianU32(0x155aul, 0x0016fff8ul);
    response_address = 0x0026fff8ul;
    memset(&memory[0x160000ul], 0x5a, 0x10000u);
    memset(&memory[0x260000ul], 0xa5, 0x10000u);
    BuildAtaIdentifyPacketDeviceData();
    assert(swap_address == response_address);
    assert(ReadSwappedWord(0u) == 0x85c0u);
    AssertCopyTuple(0u, 0x14u, 0xff941dul, 0x14u);
    assert(memory[0x2601f7ul] == 0u);
    assert(memory[0x2601f8ul] == 0xa5u);
    assert(memory[0x26fff7ul] == 0xa5u);
    for(index = 0u; index < 0x10000u; ++index)
        assert(memory[0x160000ul + index] == 0x5au);

    /* The simulator-safe seed must undergo the same +1 MiB on both images. */
    ResetCase();
    WriteBigEndianU32(0x155aul, 0x00f18000ul);
    response_address = 0x00018000ul;
    BuildAtaIdentifyPacketDeviceData();
    assert(swap_address == response_address);
    assert(ReadSwappedWord(0u) == 0x85c0u);
}

int main(void)
{
    TestFixedPayloadAndCopyTuples();
    TestTransferModeTableSelection();
    TestUltraDmaRoutes();
    TestControllerWindowAddressAndLowWordWrap();
    puts("ATA IDENTIFY PACKET DEVICE tests passed");
    return 0;
}
