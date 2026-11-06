#include <assert.h>
#include <string.h>
#include "media/raw_read_pattern_validation.h"
#include "platform/firmware_memory.h"

enum
{
    MEMORY_SIZE = 1u << 24
};

static u8 memory[MEMORY_SIZE];
static unsigned yield_calls;
static unsigned descriptor_calls;
static unsigned convert_calls;
static unsigned pack_calls;
static s32 converted_frames[8];

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static u16 ReadBe16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadBe32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

void YieldCoroutine0(void)
{
    assert(memory[0x000fafu] == 3u);
    assert(memory[0x000e3fu] == 1u);
    ++yield_calls;
}

u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    u16 cursor = ReadBe16(0x001298u);
    u16 base = (u16)ReadBe32(0x00129eu);

    ++descriptor_calls;
    return (u16)(base + (u16)(cursor * 0x16u));
}

void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output)
{
    u32 position;
    u32 total_seconds;

    assert(convert_calls < 8u);
    converted_frames[convert_calls++] = frame_position;
    if(frame_position < 0)
        frame_position += 450000l;
    position = (u32)frame_position;
    total_seconds = position / 75u;
    memory[output] = (u8)(total_seconds / 60u);
    memory[output + 1u] = (u8)(total_seconds % 60u);
    memory[output + 2u] = (u8)(position % 75u);
}

static u8 PackedBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

void ConvertThreeBinaryBytesToPackedBcd(u16 tuple_address)
{
    unsigned index;

    ++pack_calls;
    for(index = 0u; index < 3u; ++index)
        memory[tuple_address + index] = PackedBcd(memory[tuple_address + index]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    yield_calls = 0u;
    descriptor_calls = 0u;
    convert_calls = 0u;
    pack_calls = 0u;
    WriteBe16(0x001294u, 0u);
    WriteBe16(0x00129au, 3u);
    WriteBe32(0x00129eu, 0x00008000ul);
}

static fw_addr_t PayloadForIndex(u16 index)
{
    return 0x100010ul + (fw_addr_t)index * 0x1000ul;
}

static void BuildValidDescriptor(u16 index, s32 frame_position)
{
    u16 descriptor = (u16)(0x8000u + index * 0x16u);
    fw_addr_t payload_pointer = PayloadForIndex(index) - 0x10ul;
    fw_addr_t payload = payload_pointer + 0x10ul;
    s32 relative = (s32)((u32)frame_position - 150ul);
    u32 position;
    u32 total_seconds;
    u8 tuple[3];
    u8 repeated = (u8)relative;
    u16 offset;

    WriteBe32((u16)(descriptor + 2u), (u32)frame_position);
    WriteBe32((u16)(descriptor + 6u), (u32)payload_pointer);
    memory[payload] = 0u;
    memset(&memory[payload + 1u], 0xff, 10u);
    memory[payload + 11u] = 0u;

    position = frame_position < 0 ? (u32)(frame_position + 450000l) : (u32)frame_position;
    total_seconds = position / 75u;
    tuple[0] = PackedBcd((u8)(total_seconds / 60u));
    tuple[1] = PackedBcd((u8)(total_seconds % 60u));
    tuple[2] = PackedBcd((u8)(position % 75u));
    memcpy(&memory[payload + 12u], tuple, 3u);
    memory[payload + 15u] = 1u;
    WriteBe32(payload + 16u, (u32)relative);
    memcpy(&memory[payload + 20u], tuple, 3u);
    memory[payload + 23u] = 1u;
    for(offset = 24u; offset < 0x810u; ++offset)
        memory[payload + offset] = repeated;
}

static void TestZeroAndSingleDescriptor(void)
{
    ResetState();
    WriteBe16(0x001294u, 2u);
    assert(ValidateRawReadDescriptorPayloadPatternBatch(0u) == 1u);
    assert(ReadBe16(0x001298u) == 2u);
    assert(yield_calls == 0u && descriptor_calls == 0u && convert_calls == 0u && pack_calls == 0u);

    ResetState();
    WriteBe16(0x001294u, 1u);
    BuildValidDescriptor(1u, 12345l);
    assert(ValidateRawReadDescriptorPayloadPatternBatch(1u) == 1u);
    assert(ReadBe16(0x001298u) == 2u);
    assert(yield_calls == 1u && descriptor_calls == 1u && convert_calls == 1u && pack_calls == 1u);
    assert(converted_frames[0] == 12345l);
}

static void TestTwoDescriptorsAndCursorWrap(void)
{
    ResetState();
    WriteBe16(0x001294u, 3u);
    BuildValidDescriptor(3u, 150l);
    BuildValidDescriptor(0u, -1l);
    assert(ValidateRawReadDescriptorPayloadPatternBatch(2u) == 1u);
    assert(ReadBe16(0x001298u) == 1u);
    assert(yield_calls == 2u && descriptor_calls == 2u && convert_calls == 2u && pack_calls == 2u);
    assert(converted_frames[0] == 150l && converted_frames[1] == -1l);
}

static void TestMismatchOffsets(void)
{
    static const u16 offsets[] = { 0u, 1u, 10u, 11u, 12u, 14u, 15u, 16u, 19u, 20u, 22u, 23u, 24u, 0x80fu };
    unsigned index;

    for(index = 0u; index < sizeof(offsets) / sizeof(offsets[0]); ++index)
    {
        fw_addr_t payload;

        ResetState();
        BuildValidDescriptor(0u, 9876l);
        payload = PayloadForIndex(0u);
        memory[payload + offsets[index]] ^= 0x5au;
        assert(ValidateRawReadDescriptorPayloadPatternBatch(1u) == 0u);
        assert(ReadBe16(0x001298u) == 0u);
        assert(yield_calls == 1u && descriptor_calls == 1u && convert_calls == 1u && pack_calls == 1u);
    }
}

static void TestStopsAtFirstFailingDescriptor(void)
{
    ResetState();
    BuildValidDescriptor(0u, 300u);
    BuildValidDescriptor(1u, 301u);
    memory[PayloadForIndex(0u) + 0x400u] ^= 1u;
    assert(ValidateRawReadDescriptorPayloadPatternBatch(2u) == 0u);
    assert(ReadBe16(0x001298u) == 0u);
    assert(yield_calls == 1u && descriptor_calls == 1u);

    ResetState();
    BuildValidDescriptor(0u, 300u);
    BuildValidDescriptor(1u, 301u);
    memory[PayloadForIndex(1u) + 0x400u] ^= 1u;
    assert(ValidateRawReadDescriptorPayloadPatternBatch(2u) == 0u);
    assert(ReadBe16(0x001298u) == 1u);
    assert(yield_calls == 2u && descriptor_calls == 2u);
}

int main(void)
{
    TestZeroAndSingleDescriptor();
    TestTwoDescriptorsAndCursorWrap();
    TestMismatchOffsets();
    TestStopsAtFirstFailingDescriptor();
    return 0;
}
