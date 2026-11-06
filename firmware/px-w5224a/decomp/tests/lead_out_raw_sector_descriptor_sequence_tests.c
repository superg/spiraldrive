#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/lead_out_raw_sector_descriptor_sequence.h"
#include "platform/firmware_memory.h"

#define MEMORY_SIZE 0x2c2000ul

static u8 memory_image[MEMORY_SIZE];
static u8 bit0030;
static u8 bit0032;
static u8 adjacent_class;
static u8 decoded_type;
static int capture_failure_call;
static int wait_failure_call;
static u16 latest_descriptor;
static u16 capture_calls;
static u16 wait_calls;
static u16 populate_calls;
static u16 latch_calls;
static u16 update_calls;
static u16 enqueue_calls;
static u16 increment_calls;
static u16 advance_calls;
static u16 yield_calls;
static u16 restart_calls;
static u16 clear_bit35_calls;
static u16 acquire_current_calls;
static u16 acquire_updating_calls;
static u16 copy_bytes_calls;
static fw_addr_t capture_cursors[256];
static u8 capture_kinds[256];
static u8 captured_q[256][10];

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

static fw_addr_t TrackRecord(u16 index)
{
    return 0xb4bdul + (fw_addr_t)index * 8u;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    bit0030 = 0u;
    bit0032 = 0u;
    adjacent_class = 3u;
    decoded_type = 5u;
    capture_failure_call = -1;
    wait_failure_call = -1;
    latest_descriptor = 0u;
    capture_calls = 0u;
    wait_calls = 0u;
    populate_calls = 0u;
    latch_calls = 0u;
    update_calls = 0u;
    enqueue_calls = 0u;
    increment_calls = 0u;
    advance_calls = 0u;
    yield_calls = 0u;
    restart_calls = 0u;
    clear_bit35_calls = 0u;
    acquire_current_calls = 0u;
    acquire_updating_calls = 0u;
    copy_bytes_calls = 0u;
    memset(capture_cursors, 0, sizeof(capture_cursors));
    memset(capture_kinds, 0, sizeof(capture_kinds));
    memset(captured_q, 0, sizeof(captured_q));
    WriteU32(0x156au, 4ul);
    WriteU32(0x1562u, 0x126100ul);
    WriteU32(0xdd69u, 0ul);
    WriteU32(0xdd71u, 1000ul);
    WriteU32(0x1484u, 3245ul);
    WriteU16(0x2bc119u, 1u);
    memory_image[0x2bc11bu] = 0x10u;
    memory_image[0x12c9u] = 1u;
    memory_image[0x1149u] = 1u;
    memory_image[0xc44u] = 0u;
    memory_image[0xc48u] = 8u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < MEMORY_SIZE);
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory_image[address] = value;
}

void ObserveLeadOutWriteBit0030(u8 value)
{
    bit0030 = value;
}
u8 ObserveLeadOutReadBit0032(void)
{
    return bit0032;
}
void ObserveLeadOutClearBit0035(void)
{
    ++clear_bit35_calls;
}
u16 ObserveLeadOutLatestDescriptor(void)
{
    return latest_descriptor;
}
u32 ObserveLeadOutMultiply(u32 left, u32 right)
{
    return left * right;
}

void ObserveLeadOutCopyFirstTrackStart(u16 destination)
{
    memory_image[destination] = 1u;
    memory_image[destination + 1u] = 2u;
    memory_image[destination + 2u] = 3u;
}

u8 ObserveLeadOutBinaryToBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

s32 ObserveLeadOutTupleToFrame(fw_addr_t tuple)
{
    return (s32)(((u32)memory_image[tuple] << 16) | ((u32)memory_image[tuple + 1u] << 8) | memory_image[tuple + 2u]);
}

void ObserveLeadOutFrameToTuple(s32 frame, fw_addr_t tuple)
{
    memory_image[tuple] = (u8)((u32)frame >> 16);
    memory_image[tuple + 1u] = (u8)((u32)frame >> 8);
    memory_image[tuple + 2u] = (u8)frame;
}

void ObserveLeadOutPackTuple(u16 tuple)
{
    memory_image[tuple] ^= 0x10u;
    memory_image[tuple + 1u] ^= 0x10u;
    memory_image[tuple + 2u] ^= 0x10u;
}

u16 ObserveLeadOutFindTrack(u8 track_number)
{
    assert(track_number == 0xaau);
    return 7u;
}

u8 ObserveLeadOutDecodeType(u16 record_index)
{
    assert(record_index == 7u);
    return decoded_type;
}

u8 ObserveLeadOutFindAdjacent(u16 record_index, u8 forward)
{
    assert(record_index == (u16)(ReadU16(0x149fu) - 1u));
    assert(forward == 0u);
    return adjacent_class;
}

u8 ObserveLeadOutAcquireCurrent(s32 count, u8 source_kind)
{
    ++acquire_current_calls;
    assert(count == 1l && source_kind == 4u);
    WriteU32(0x1598u, ReadU32(0x1598u) + 1ul);
    return 1u;
}

u8 ObserveLeadOutAcquireUpdating(s32 count, u8 reverse)
{
    ++acquire_updating_calls;
    assert(count == 1l && reverse == 0u);
    WriteU32(0x1598u, ReadU32(0x1598u) + 1ul);
    return 1u;
}

u8 ObserveLeadOutCapture(fw_addr_t tuple, fw_addr_t cursor, u8 source_kind)
{
    int call = (int)capture_calls;
    u8 index;

    assert(tuple == 0x7000ul);
    assert(capture_calls < 256u);
    capture_cursors[capture_calls] = cursor;
    capture_kinds[capture_calls] = source_kind;
    for(index = 0u; index != 10u; ++index)
        captured_q[capture_calls][index] = memory_image[0x1272u + index];
    ++capture_calls;
    return (u8)(call != capture_failure_call);
}

u8 ObserveLeadOutWaitCapacity(void)
{
    int call = (int)wait_calls++;
    return (u8)(call != wait_failure_call);
}

void ObserveLeadOutLatchFailure(void)
{
    ++latch_calls;
}

void ObserveLeadOutPopulate(fw_addr_t cursor, u8 source_kind)
{
    ++populate_calls;
    assert(source_kind == 8u);
    assert(cursor < 0x10000ul);
}

void ObserveLeadOutUpdateQ(void)
{
    ++update_calls;
}
void ObserveLeadOutEnqueue(void)
{
    ++enqueue_calls;
}

void ObserveLeadOutIncrement(u16 tuple)
{
    ++increment_calls;
    ++memory_image[tuple + 2u];
}

fw_addr_t ObserveLeadOutAdvance(fw_addr_t cursor)
{
    u32 span = 0x930ul * ReadU32(0x156au);
    fw_addr_t next = cursor + 0x930ul;

    ++advance_calls;
    return next > span - 1ul ? 0ul : next;
}

void ObserveLeadOutYield(void)
{
    ++yield_calls;
}
void ObserveLeadOutRestart(void)
{
    ++restart_calls;
}

void ObserveLeadOutCopyBytes(fw_addr_t source, fw_addr_t destination, u16 count)
{
    u16 index;

    ++copy_bytes_calls;
    assert(source == 0x1275ul && destination == 0x7010ul && count == 3u);
    for(index = 0u; index != count; ++index)
        memory_image[destination + index] = memory_image[source + index];
}

static void TestOrdinaryRestartAndTail(void)
{
    ResetHarness();
    memory_image[0x1292u] = 0x55u;

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 1u);
    assert(ReadU16(0x1292u) == 0u);
    assert(clear_bit35_calls == 1u && bit0030 == 0u);
    assert(acquire_updating_calls == 1u && acquire_current_calls == 0u);
    assert(capture_calls == 5u);
    assert(capture_kinds[0] == 8u && capture_kinds[2] == 8u);
    assert(capture_kinds[3] == 7u && capture_kinds[4] == 7u);
    assert(copy_bytes_calls == 2u);
    assert(update_calls == 5u && enqueue_calls == 5u);
    assert(increment_calls == 12u && advance_calls == 6u);
    assert(ReadU32(0x15f2u) == 3250ul);
    assert(memory_image[0x1289u] == 0u);
    assert(captured_q[3][0] == 0x15u);
    assert(captured_q[3][1] == 0xaau);
    assert(captured_q[3][2] == 0xd1u);
}

static void TestMode2CapacityMetadata(void)
{
    ResetHarness();
    WriteU32(0x156au, 1ul);
    memory_image[0x2bc11bu] = 0x41u;
    memory_image[0xdd88u + 0x13u] = 0x40u;

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 1u);
    assert(acquire_current_calls == 1u);
    assert(capture_calls == 3u);
    assert(wait_calls == 2u && populate_calls == 2u);
    assert(memory_image[0x10000cu] != 0u || memory_image[0x10000du] != 0u || memory_image[0x10000eu] != 0u);
    assert(memory_image[0x10000fu] == 1u);
}

static void TestCapacityFailureLatchesContext(void)
{
    ResetHarness();
    WriteU32(0x156au, 0ul);
    wait_failure_call = 0;

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 0u);
    assert(wait_calls == 1u && latch_calls == 1u);
    assert(capture_calls == 0u);
    assert(bit0030 == 1u);
    assert(memory_image[0x1289u] == 1u);
}

static void TestTypeTwoLeadOutRoute(void)
{
    fw_addr_t record;

    ResetHarness();
    memory_image[0xc44u] = 2u;
    WriteU16(0x149fu, 2u);
    record = TrackRecord(2u);
    memory_image[record + 4u] = 0u;
    memory_image[record + 6u] = 0u;
    memory_image[record + 7u] = 100u;
    memory_image[TrackRecord(1u)] = 0x62u;
    WriteU32(0x1484u, 2348ul);

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 1u);
    assert(acquire_current_calls == 0u && acquire_updating_calls == 0u);
    assert(capture_calls == 3u);
    assert(capture_kinds[0] == 8u && capture_kinds[1] == 7u);
    assert(memory_image[0x1272u] == 0x62u);
    assert(memory_image[0x1285u] == decoded_type);
    assert(memory_image[0x7003u] == adjacent_class);
}

static void TestLongRegionRestartAndCaptureFailure(void)
{
    ResetHarness();
    WriteU32(0xdd69u, 0xfffffffful);
    WriteU32(0x1484u, 7746ul);
    bit0032 = 1u;
    capture_failure_call = 1;

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 0u);
    assert(capture_calls == 2u);
    assert(restart_calls == 0u);
    assert(bit0030 == 1u);
}

static void TestToggleYieldAndProducerRestart(void)
{
    ResetHarness();
    WriteU32(0x156au, 200ul);
    WriteU32(0x1484u, 3088ul);
    bit0032 = 1u;

    assert(AcquireLeadOutRawSectorDescriptorSequence(1u) == 1u);
    assert(capture_calls == 162u);
    assert(yield_calls == 5u);
    assert(memory_image[0x127cu] == 1u);
    assert(restart_calls == 1u);
    assert(bit0030 == 0u);
}

int main(void)
{
    TestOrdinaryRestartAndTail();
    TestMode2CapacityMetadata();
    TestCapacityFailureLatchesContext();
    TestTypeTwoLeadOutRoute();
    TestLongRegionRestartAndCaptureFailure();
    TestToggleYieldAndProducerRestart();
    puts("lead_out_raw_sector_descriptor_sequence_tests: ok");
    return 0;
}
