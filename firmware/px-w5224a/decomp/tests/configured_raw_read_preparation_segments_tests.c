#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/configured_raw_read_preparation_segments.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_SELECT,
    EVENT_RESUME,
    EVENT_ACQUIRE,
    EVENT_POPULATE,
    EVENT_PREFIX,
    EVENT_Q_UPDATE
};

typedef struct
{
    u8 operation;
    s32 argument0;
    s32 argument1;
} Event;

static u8 memory_image[0x20000];
static Event events[32];
static u8 event_count;
static u8 results[16];
static u8 result_count;
static u8 result_index;
static u8 sector_class;
static fw_addr_t ring_address;

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void Record(u8 operation, s32 argument0, s32 argument1)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].operation = operation;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;
}

static u8 NextResult(void)
{
    assert(result_index < result_count);
    return results[result_index++];
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(events, 0, sizeof(events));
    memset(results, 0, sizeof(results));
    event_count = result_count = result_index = 0u;
    sector_class = 9u;
    ring_address = 0x1b1234ul;
    WriteDword(0x1598u, 1000ul);
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

u8 ObserveConfiguredSegmentsSelectSectorClass(u8 prefer, u8 fallback)
{
    Record(EVENT_SELECT, prefer, fallback);
    return sector_class;
}

fw_addr_t ObserveConfiguredSegmentsResumeRing(void)
{
    Record(EVENT_RESUME, (s32)ring_address, 0l);
    return ring_address;
}

u8 ObserveConfiguredSegmentsAcquireFrames(s32 frame_count)
{
    Record(EVENT_ACQUIRE, frame_count, 0l);
    return NextResult();
}

void ObserveConfiguredSegmentsPopulateRange(fw_addr_t address, u32 count)
{
    Record(EVENT_POPULATE, (s32)address, (s32)count);
}

u8 ObserveConfiguredSegmentsAcquirePrefix(u8 mode, u8 source)
{
    Record(EVENT_PREFIX, mode, source);
    return NextResult();
}

u8 ObserveConfiguredSegmentsAcquireWithQUpdates(s32 count, u8 reverse)
{
    Record(EVENT_Q_UPDATE, count, reverse);
    return NextResult();
}

static void ExpectEvent(u8 index, u8 operation, s32 argument0, s32 argument1)
{
    assert(index < event_count);
    assert(events[index].operation == operation);
    assert(events[index].argument0 == argument0);
    assert(events[index].argument1 == argument1);
}

static void TestUnsupportedWriteTypeStillSelectsClass(void)
{
    ResetHarness();
    memory_image[0xc44u] = 3u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0x5au) == 1u);
    assert(memory_image[0x142bu] == sector_class && event_count == 1u);
    ExpectEvent(0u, EVENT_SELECT, 0l, 0x5al);
}

static void TestOrdinaryDirectAndQPaths(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    WriteDword(0x1430u, 1ul);
    WriteDword(0x1434u, 8ul);
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(event_count == 4u);
    ExpectEvent(1u, EVENT_RESUME, (s32)ring_address, 0l);
    ExpectEvent(2u, EVENT_ACQUIRE, 8l, 0l);
    ExpectEvent(3u, EVENT_POPULATE, (s32)ring_address, 8l);

    ResetHarness();
    memory_image[0xc44u] = 1u;
    WriteDword(0x1434u, 8ul);
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(0u, 1u) == 1u);
    assert(event_count == 2u);
    ExpectEvent(1u, EVENT_Q_UPDATE, 8l, 1l);
}

static void TestOrdinaryPrefixAndFailure(void)
{
    ResetHarness();
    memory_image[0xc44u] = 1u;
    WriteDword(0x1434u, 9ul);
    results[0] = results[1] = 1u;
    result_count = 2u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(event_count == 5u);
    ExpectEvent(2u, EVENT_PREFIX, 0l, 6l);
    ExpectEvent(3u, EVENT_ACQUIRE, 4l, 0l);
    ExpectEvent(4u, EVENT_POPULATE, (s32)ring_address, 9l);

    ResetHarness();
    memory_image[0xc44u] = 1u;
    WriteDword(0x1434u, 9ul);
    results[0] = 0u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 0u);
    assert(event_count == 3u && result_index == 1u);
}

static void TestFlaggedZeroFirstSegmentSplits(void)
{
    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1434u, 8ul);
    results[0] = results[1] = 1u;
    result_count = 2u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    ExpectEvent(2u, EVENT_PREFIX, 0l, 6l);
    ExpectEvent(3u, EVENT_ACQUIRE, 3l, 0l);
    ExpectEvent(4u, EVENT_POPULATE, (s32)ring_address, 8l);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x14b4u] = 1u;
    memory_image[0xc84u] = 0x40u;
    WriteDword(0x1434u, 8ul);
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    ExpectEvent(2u, EVENT_ACQUIRE, 8l, 0l);
    ExpectEvent(3u, EVENT_POPULATE, (s32)ring_address, 8l);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1434u, 15ul);
    results[0] = results[1] = results[2] = results[3] = 1u;
    result_count = 4u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(ReadDword(0x1598u) == 999ul);
    ExpectEvent(2u, EVENT_PREFIX, 0l, 6l);
    ExpectEvent(3u, EVENT_ACQUIRE, 3l, 0l);
    ExpectEvent(4u, EVENT_PREFIX, 1l, 6l);
    ExpectEvent(5u, EVENT_PREFIX, 2l, 4l);
    ExpectEvent(6u, EVENT_POPULATE, (s32)ring_address, 10l);
}

static void TestFlaggedNonzeroFirstSegmentAndFailure(void)
{
    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1430u, 2ul);
    WriteDword(0x1434u, 10ul);
    results[0] = results[1] = results[2] = 1u;
    result_count = 3u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(ReadDword(0x1598u) == 999ul);
    ExpectEvent(2u, EVENT_ACQUIRE, 3l, 0l);
    ExpectEvent(3u, EVENT_PREFIX, 1l, 6l);
    ExpectEvent(4u, EVENT_PREFIX, 2l, 4l);
    ExpectEvent(5u, EVENT_POPULATE, (s32)ring_address, 5l);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1430u, 2ul);
    WriteDword(0x1434u, 10ul);
    results[0] = 1u;
    results[1] = 0u;
    result_count = 2u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 0u);
    assert(ReadDword(0x1598u) == 1000ul && event_count == 4u);
}

static void TestFlaggedQPathAdjustsTargetOnlyAfterSuccess(void)
{
    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1434u, 11ul);
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(0u, 0u) == 1u);
    ExpectEvent(1u, EVENT_Q_UPDATE, 7l, 1l);
    assert(ReadDword(0x1598u) == 999ul);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128bu] = 1u;
    WriteDword(0x1434u, 11ul);
    results[0] = 0u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(0u, 0u) == 0u);
    assert(ReadDword(0x1598u) == 1000ul);
}

static void TestGenericAndForcedFiveFramePolicies(void)
{
    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0xdd34u] = 0u;
    memory_image[0x14b2u] = 1u;
    WriteDword(0x1434u, 6ul);
    results[0] = results[1] = 1u;
    result_count = 2u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(event_count == 4u);
    ExpectEvent(2u, EVENT_PREFIX, 0l, 6l);
    ExpectEvent(3u, EVENT_ACQUIRE, 1l, 0l);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0xdd34u] = 4u;
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(1u, 0u) == 1u);
    assert(event_count == 4u);
    ExpectEvent(2u, EVENT_PREFIX, 0l, 6l);
    ExpectEvent(3u, EVENT_POPULATE, (s32)ring_address, 5l);

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0xdd34u] = 4u;
    results[0] = 1u;
    result_count = 1u;
    assert(AcquireConfiguredRawReadPreparationSegments(0u, 0u) == 1u);
    ExpectEvent(1u, EVENT_Q_UPDATE, 5l, 1l);
}

int main(void)
{
    TestUnsupportedWriteTypeStillSelectsClass();
    TestOrdinaryDirectAndQPaths();
    TestOrdinaryPrefixAndFailure();
    TestFlaggedZeroFirstSegmentSplits();
    TestFlaggedNonzeroFirstSegmentAndFailure();
    TestFlaggedQPathAdjustsTargetOnlyAfterSuccess();
    TestGenericAndForcedFiveFramePolicies();
    puts("configured raw-read preparation segment tests passed");
    return 0;
}
