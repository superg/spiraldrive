#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/write_controller_range_build.h"

static u8 memory[1u << 24];
static s32 compressed_inputs[8];
static unsigned compress_count;
static s32 expanded_inputs[32];
static unsigned expand_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

s32 CompressThirtyNineFramePositionToLogicalBlockAddress(s32 value)
{
    compressed_inputs[compress_count++] = value;
    return value;
}

s32 ExpandLogicalBlockAddressToThirtyNineFramePosition(s32 value)
{
    expanded_inputs[expand_count++] = value;
    return value + 150l;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteU24(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 16);
    memory[address + 1u] = (u8)(value >> 8);
    memory[address + 2u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(compressed_inputs, 0, sizeof(compressed_inputs));
    memset(expanded_inputs, 0, sizeof(expanded_inputs));
    compress_count = 0u;
    expand_count = 0u;
}

static void SetSegmentHeader(u8 index, u32 first, u32 second, u16 record_count)
{
    fw_addr_t segment = 0x194000ul + (fw_addr_t)index * 0x800ul;

    WriteU16(segment + 8u, record_count);
    WriteU24(segment + 0x11u, first);
    WriteU24(segment + 0x15u, second);
}

static void SetSegmentRecord(u8 segment_index, u16 record_index, u32 first, u32 second)
{
    fw_addr_t record = 0x194020ul + (fw_addr_t)segment_index * 0x800ul + (fw_addr_t)record_index * 6u;

    WriteU24(record, first);
    WriteU24(record + 3u, second);
}

static void TestEmptyScanClearsOutputAndScratch(void)
{
    u16 index;

    ResetFixture();
    memset(&memory[0x2b9600u], 0xa5, 0x100u);
    memory[0xfc40u] = 0x12u;
    memory[0xfc41u] = 0x34u;

    assert(BuildSortedWriteControllerBufferRanges(100, 20ul) == 0u);
    assert(compress_count == 1u && compressed_inputs[0] == 100);
    assert(ReadU32(0x2b9714u) == 100ul);
    assert(ReadU32(0x2b9718u) == 119ul);
    assert(memory[0xfc40u] == 0u && memory[0xfc41u] == 0u);
    for(index = 0u; index < 0x100u; ++index)
        assert(memory[0x2b9600u + index] == 0u);
    assert(expand_count == 0u);
}

static void TestCollectsAndSortsAcceptedRecords(void)
{
    ResetFixture();
    SetSegmentHeader(3u, 90ul, 199ul, 3u);
    SetSegmentRecord(3u, 0u, 150ul, 1000ul);
    SetSegmentRecord(3u, 1u, 110ul, 900ul);
    SetSegmentRecord(3u, 2u, 250ul, 800ul);

    assert(BuildSortedWriteControllerBufferRanges(100, 100ul) == 2u);
    assert(expand_count == 4u);
    assert(expanded_inputs[0] == 1000 && expanded_inputs[1] == 150 && expanded_inputs[2] == 900 && expanded_inputs[3] == 110);
    assert(ReadU32(0x2b9600u) == 1050ul);
    assert(ReadU32(0x2b9604u) == 260ul);
    assert(ReadU32(0x2b9608u) == 1150ul);
    assert(ReadU32(0x2b960cu) == 300ul);
    assert(ReadU32(0x2b9610u) == 0ul);
    assert(memory[0xfc40u] == 0u && memory[0xfc41u] == 0u);
}

static void TestRetainedSecondCodeAdmitsFollowingZeroFirstCode(void)
{
    ResetFixture();
    SetSegmentHeader(0u, 0ul, 0ul, 4u);
    SetSegmentRecord(0u, 0u, 0ul, 9ul);
    SetSegmentRecord(0u, 1u, 0x200001ul, 6ul);
    SetSegmentRecord(0u, 2u, 5ul, 7ul);
    SetSegmentRecord(0u, 3u, 0ul, 8ul);

    assert(BuildSortedWriteControllerBufferRanges(0, 1ul) == 1u);
    assert(expand_count == 2u && expanded_inputs[0] == 8 && expanded_inputs[1] == 0);
    assert(ReadU32(0x2b9600u) == 158ul);
    assert(ReadU32(0x2b9604u) == 150ul);
}

static void TestRejectsSentinelAndNonoverlappingSegments(void)
{
    ResetFixture();
    SetSegmentHeader(1u, 0xfffffful, 1000ul, 1u);
    SetSegmentRecord(1u, 0u, 110ul, 900ul);
    SetSegmentHeader(2u, 200ul, 300ul, 1u);
    SetSegmentRecord(2u, 0u, 110ul, 900ul);
    SetSegmentHeader(3u, 50ul, 99ul, 1u);
    SetSegmentRecord(3u, 0u, 110ul, 900ul);

    assert(BuildSortedWriteControllerBufferRanges(100, 100ul) == 0u);
    assert(expand_count == 0u);
}

int main(void)
{
    TestEmptyScanClearsOutputAndScratch();
    TestCollectsAndSortsAcceptedRecords();
    TestRetainedSecondCodeAdmitsFollowingZeroFirstCode();
    TestRejectsSentinelAndNonoverlappingSegments();
    return 0;
}
