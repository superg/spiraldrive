#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_transfer_layout.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static u8 decoded_q_type;
static u16 decoded_index;
static u16 decode_calls;
static u16 update_calls;
static u16 tuple_calls;
static s32 tuple_frame;

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void ResetHarness(void)
{
    memset(memory_image, 0x5a, sizeof(memory_image));
    decoded_q_type = 0u;
    decoded_index = 0u;
    decode_calls = 0u;
    update_calls = 0u;
    tuple_calls = 0u;
    tuple_frame = 0l;
    memory_image[0xc44u] = 0u;
    WriteU32(0x1598u, 0x12345678ul);
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

u8 ObserveRawReadTransferLayoutDecode(u16 record_index)
{
    ++decode_calls;
    decoded_index = record_index;
    return decoded_q_type;
}

void ObserveRawReadTransferLayoutUpdateQType(void)
{
    ++update_calls;
}

void ObserveRawReadTransferLayoutTuple(s32 frame_position, u8 *tuple)
{
    ++tuple_calls;
    tuple_frame = frame_position;
    tuple[0] = 0x12u;
    tuple[1] = 0x34u;
    tuple[2] = 0x56u;
}

static void AssertDescriptor(u8 a, u8 b, u8 c, u8 tag)
{
    assert(memory_image[0x18025u] == a);
    assert(memory_image[0x18026u] == b);
    assert(memory_image[0x18027u] == c);
    assert(memory_image[0x18028u] == tag);
    assert(memory_image[0x18021u] == 0u);
}

static void TestUnknownAndFixedLayouts(void)
{
    ResetHarness();
    memory_image[0x1285u] = 0x55u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x5a5au);
    assert(memory_image[0x18021u] == 0x5au);

    ResetHarness();
    memory_image[0x1285u] = 1u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x0940u);
    assert(ReadU32(0x1470u) == 0x930ul);
    assert(ReadU32(0x1474u) == 0x10ul);
    assert(memory_image[0x1469u] == 2u && memory_image[0x146bu] == 0xc2u);
    AssertDescriptor(0u, 0u, 0u, 0u);

    ResetHarness();
    memory_image[0x1285u] = 0x41u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x60u && ReadU32(0x1470u) == 0x60ul);
    AssertDescriptor(0u, 0u, 0u, 0u);
}

static void TestOrdinaryTupleLayouts(void)
{
    static const u8 q_types[] = { 10u, 11u, 12u };
    static const u16 lengths[] = { 0x800u, 0x808u, 0x914u };
    static const u8 codes[] = { 0x47u, 0x46u, 0x47u };
    u8 i;

    for(i = 0u; i < 3u; ++i)
    {
        ResetHarness();
        memory_image[0x1285u] = q_types[i];
        ConfigureRawReadTransferLayoutFromQTypeCode();
        assert(ReadU16(0x146du) == lengths[i]);
        assert(ReadU32(0x1470u) == lengths[i]);
        assert(memory_image[0x146cu] == codes[i]);
        assert(update_calls == 1u && tuple_calls == 1u);
        assert(tuple_frame == (s32)0x12345678l);
        AssertDescriptor(0x12u, 0x34u, 0x56u, 2u);
    }

    ResetHarness();
    memory_image[0x1285u] = 8u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x800u && memory_image[0x146cu] == 0x46u);
    assert(update_calls == 0u);
    AssertDescriptor(0x12u, 0x34u, 0x56u, 1u);
}

static void TestType2RecordLayouts(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    WriteU16(0x149bu, 7u);
    decoded_q_type = 8u;
    memory_image[0xb4c0u + 7u * 8u] = 0xd2u;
    memory_image[0x146fu] = 1u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(decode_calls == 1u && decoded_index == 7u);
    assert(memory_image[0x1285u] == 8u);
    assert(ReadU16(0x146du) == 0x860u);
    assert(ReadU32(0x1470u) == 0x800ul && ReadU32(0x1474u) == 0x60ul);
    assert(memory_image[0x1469u] == 2u && memory_image[0x146au] == 0x0du && memory_image[0x146bu] == 0xc2u);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    WriteU16(0x149bu, 0u);
    decoded_q_type = 9u;
    memory_image[0xb4c0u] = 0x33u;
    memory_image[0x146fu] = 3u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x930u && memory_image[0x146au] == 8u);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    WriteU16(0x149bu, 0u);
    decoded_q_type = 13u;
    memory_image[0xb4c0u] = 0xe2u;
    memory_image[0x146fu] = 3u;
    memory_image[0x1801fu] = 9u;
    memory_image[0x18020u] = 8u;
    ConfigureRawReadTransferLayoutFromQTypeCode();
    assert(ReadU16(0x146du) == 0x980u && ReadU32(0x1470u) == 0x920ul);
    assert(memory_image[0x146au] == 0x0eu && update_calls == 1u);
    assert(memory_image[0x1801fu] == 0u && memory_image[0x18020u] == 0u);
    AssertDescriptor(0x12u, 0x34u, 0x56u, 2u);
}

int main(void)
{
    TestUnknownAndFixedLayouts();
    TestOrdinaryTupleLayouts();
    TestType2RecordLayouts();
    puts("raw-read transfer layout tests passed");
    return 0;
}
