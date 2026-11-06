#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_preparation_range_end.h"
#include "platform/firmware_memory.h"

#define WRITE_CAPACITY 64u

struct WriteEvent
{
    fw_addr_t address;
    u8 value;
};

static u8 memory_bytes[0x20000ul];
static struct WriteEvent writes[WRITE_CAPACITY];
static unsigned write_count;
static s32 boundary_distance;
static u16 boundary_index;
static unsigned boundary_call_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    assert(write_count < WRITE_CAPACITY);
    memory_bytes[address] = value;
    writes[write_count].address = address;
    writes[write_count].value = value;
    ++write_count;
}

s32 ObserveFrameDistanceToNextType1TrackRecordBoundary(u16 record_index)
{
    boundary_index = record_index;
    ++boundary_call_count;
    return boundary_distance;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(writes, 0, sizeof(writes));
    write_count = 0u;
    boundary_distance = 0l;
    boundary_index = 0u;
    boundary_call_count = 0u;
    memory_bytes[0xdd34u] = 2u;
}

static void TestZeroFirstSegmentSelection(void)
{
    ResetFixture();
    WriteU32(0x1434u, 321ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 321l);

    ResetFixture();
    memory_bytes[0xdd34u] = 1u;
    WriteU32(0x1434u, 321ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 5l);

    ResetFixture();
    memory_bytes[0x0c44u] = 2u;
    WriteU16(0x149bu, 0x1234u);
    boundary_distance = -77l;
    assert(SelectRawReadPreparationRangeEnd(0u) == -77l);
    assert(boundary_call_count == 1u);
    assert(boundary_index == 0x1234u);

    ResetFixture();
    memory_bytes[0x0c44u] = 3u;
    assert(SelectRawReadPreparationRangeEnd(0u) == 1l);

    ResetFixture();
    memory_bytes[0x0c44u] = 1u;
    WriteU32(0x1434u, 444ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 444l);
}

static void TestNonzeroFirstSegmentSelection(void)
{
    ResetFixture();
    memory_bytes[0x0c44u] = 1u;
    WriteU32(0x1430u, 100ul);
    WriteU32(0x1434u, 200ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 300l);

    ResetFixture();
    memory_bytes[0x0c44u] = 0u;
    WriteU32(0x1430u, 100ul);
    WriteU32(0x1434u, 200ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 300l);

    ResetFixture();
    memory_bytes[0x0c44u] = 0u;
    memory_bytes[0x0c46u] = 2u;
    memory_bytes[0xdd34u] = 1u;
    WriteU32(0x1430u, 100ul);
    WriteU32(0x1434u, 200ul);
    assert(SelectRawReadPreparationRangeEnd(0u) == 5l);
}

static void TestShortPartition(void)
{
    ResetFixture();
    WriteU32(0xdd3du, 1000ul);
    WriteU32(0xdd51u, 900ul);
    WriteU32(0xdd39u, 800ul);
    assert(SelectRawReadPreparationRangeEnd(1u) == 100l);
    assert(ReadU32(0x1430u) == 0ul);
    assert(ReadU32(0x1434u) == 100ul);
    assert(memory_bytes[0x1438u] == 3u);
    assert(memory_bytes[0x1439u] == 2u);
}

static void TestLongPartitionAndOverwrittenFirstCount(void)
{
    ResetFixture();
    memory_bytes[0xdd62u] = 2u;
    WriteU32(0xdd3du, 1000ul);
    WriteU32(0xdd51u, 700ul);
    WriteU32(0xdd39u, 800ul);
    assert(SelectRawReadPreparationRangeEnd(1u) == 300l);
    assert(ReadU32(0x1430u) == 150ul);
    assert(ReadU32(0x1434u) == 150ul);
    assert(memory_bytes[0x1438u] == 1u);
    assert(memory_bytes[0x1439u] == 2u);
    assert(write_count == 14u);
}

static void TestShortTrackDistanceRetainsModeThree(void)
{
    ResetFixture();
    WriteU32(0xdd3du, 1000ul);
    WriteU32(0xdd51u, 700ul);
    WriteU32(0xdd39u, 900ul);
    assert(SelectRawReadPreparationRangeEnd(1u) == 300l);
    assert(ReadU32(0x1430u) == 150ul);
    assert(memory_bytes[0x1438u] == 3u);
}

static void TestPartitionRequiresExactOne(void)
{
    ResetFixture();
    memory_bytes[0x0c44u] = 1u;
    WriteU32(0x1430u, 10ul);
    WriteU32(0x1434u, 20ul);
    WriteU32(0xdd3du, 1000ul);
    WriteU32(0xdd51u, 900ul);
    assert(SelectRawReadPreparationRangeEnd(2u) == 30l);
    assert(write_count == 0u);
}

int main(void)
{
    TestZeroFirstSegmentSelection();
    TestNonzeroFirstSegmentSelection();
    TestShortPartition();
    TestLongPartitionAndOverwrittenFirstCount();
    TestShortTrackDistanceRetainsModeThree();
    TestPartitionRequiresExactOne();
    puts("raw-read preparation range-end tests passed");
    return 0;
}
