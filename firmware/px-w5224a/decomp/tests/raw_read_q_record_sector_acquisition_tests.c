#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_q_record_sector_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x20000];
static int wait_failure_call;
static int capture_failure_call;
static u16 convert_calls;
static u16 pack_calls;
static u16 copy_calls;
static u16 divide_calls;
static u16 multiply_calls;
static u16 wait_calls;
static u16 write_block_calls;
static u16 capture_calls;
static u16 rid_calls;
static u16 increment_calls;
static u16 update_calls;
static u16 enqueue_calls;
static u16 report_calls;
static u16 sense_calls;
static u16 information_calls;
static u8 reported_failure;
static u8 sense_key;
static u16 sense_code;
static u32 sense_information;
static fw_addr_t capture_addresses[32];
static u8 capture_kinds[32];
static fw_addr_t rid_addresses[32];
static fw_addr_t enqueued_cursors[32];
static u8 enqueued_tracks[32];
static u8 enqueued_separators[32];

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static void SetQRecord(u16 record_index, u8 track_number)
{
    fw_addr_t address = 0x8012u + (fw_addr_t)record_index * 10u;
    u8 index;

    for(index = 0u; index != 10u; ++index)
        memory_image[address + index] = (u8)(0x40u + index);
    memory_image[address + 1u] = track_number;
}

static void ResetHarness(void)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    wait_failure_call = -1;
    capture_failure_call = -1;
    convert_calls = 0u;
    pack_calls = 0u;
    copy_calls = 0u;
    divide_calls = 0u;
    multiply_calls = 0u;
    wait_calls = 0u;
    write_block_calls = 0u;
    capture_calls = 0u;
    rid_calls = 0u;
    increment_calls = 0u;
    update_calls = 0u;
    enqueue_calls = 0u;
    report_calls = 0u;
    sense_calls = 0u;
    information_calls = 0u;
    reported_failure = 0u;
    sense_key = 0u;
    sense_code = 0u;
    sense_information = 0u;
    memset(capture_addresses, 0, sizeof(capture_addresses));
    memset(capture_kinds, 0, sizeof(capture_kinds));
    memset(rid_addresses, 0, sizeof(rid_addresses));
    memset(enqueued_cursors, 0, sizeof(enqueued_cursors));
    memset(enqueued_tracks, 0, sizeof(enqueued_tracks));
    memset(enqueued_separators, 0, sizeof(enqueued_separators));
    WriteU32(0x114fu, 5000ul);
    WriteU32(0x156au, 2ul);
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

void ObserveQRecordSectorConvert(s32 frame_position, fw_addr_t output_tuple_address)
{
    ++convert_calls;
    assert(frame_position == 4000l && output_tuple_address == 0x7000u);
    memory_image[output_tuple_address] = 1u;
    memory_image[output_tuple_address + 1u] = 2u;
    memory_image[output_tuple_address + 2u] = 3u;
}

void ObserveQRecordSectorPack(u16 tuple_address)
{
    ++pack_calls;
    assert(tuple_address == 0x7000u);
}

void ObserveQRecordSectorCopyTuple(u16 destination, u16 source)
{
    ++copy_calls;
    assert(destination == 0x1428u && source == 0x7000u);
    memory_image[destination] = memory_image[source];
    memory_image[destination + 1u] = memory_image[source + 1u];
    memory_image[destination + 2u] = memory_image[source + 2u];
}

u32 ObserveQRecordSectorDivide(u32 dividend, u32 divisor)
{
    ++divide_calls;
    assert(divisor == 10ul);
    return dividend / divisor;
}

u32 ObserveQRecordSectorMultiply(u32 left, u32 right)
{
    ++multiply_calls;
    return left * right;
}

u8 ObserveQRecordSectorWaitCapacity(void)
{
    int call = (int)wait_calls++;
    return (u8)(call != wait_failure_call);
}

void ObserveQRecordSectorWriteControllerBlock(fw_addr_t source)
{
    ++write_block_calls;
    assert(source == 0x1428u);
}

u8 ObserveQRecordSectorCapture(fw_addr_t cursor, u8 source_kind)
{
    int call = (int)capture_calls;

    assert(capture_calls < 32u);
    capture_addresses[capture_calls] = cursor;
    capture_kinds[capture_calls] = source_kind;
    ++capture_calls;
    return (u8)(call != capture_failure_call);
}

void ObserveQRecordSectorBuildRid01(fw_addr_t cursor)
{
    assert(rid_calls < 32u);
    rid_addresses[rid_calls++] = cursor;
}

void ObserveQRecordSectorIncrement(u16 tuple_address)
{
    ++increment_calls;
    assert(tuple_address == 0x1428u);
    ++memory_image[tuple_address + 2u];
}

void ObserveQRecordSectorUpdateQ(void)
{
    ++update_calls;
}

void ObserveQRecordSectorEnqueue(void)
{
    assert(enqueue_calls < 32u);
    enqueued_cursors[enqueue_calls] = ReadU32(0x126eu);
    enqueued_tracks[enqueue_calls] = memory_image[0x1273u];
    enqueued_separators[enqueue_calls] = memory_image[0x1278u];
    ++enqueue_calls;
}

void ObserveQRecordSectorReportFailure(u8 failure_code)
{
    ++report_calls;
    reported_failure = failure_code;
}

void ObserveQRecordSectorSetSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}

void ObserveQRecordSectorSetInformation(u32 information)
{
    ++information_calls;
    sense_information = information;
}

static void TestZeroCountBypassesInitialization(void)
{
    ResetHarness();
    WriteU32(0x1598u, 0x12345678ul);

    assert(AcquireRawSectorsFromQRecordTable(0ul, 9u) == 1u);
    assert(ReadU32(0x1598u) == 0x12345678ul);
    assert(convert_calls == 0u && divide_calls == 0u && wait_calls == 0u && capture_calls == 0u);
}

static void TestProducerCapacityFailure(void)
{
    ResetHarness();
    wait_failure_call = 0;
    SetQRecord(0u, 0x11u);

    assert(AcquireRawSectorsFromQRecordTable(10ul, 1u) == 0u);
    assert(ReadU32(0x1598u) == 4000ul);
    assert(memory_image[0x1273u] == 0x11u);
    assert(report_calls == 1u && reported_failure == 3u);
    assert(capture_calls == 0u && sense_calls == 0u);
}

static void TestDataCaptureFailure(void)
{
    ResetHarness();
    capture_failure_call = 0;
    SetQRecord(0u, 0x11u);

    assert(AcquireRawSectorsFromQRecordTable(10ul, 1u) == 0u);
    assert(wait_calls == 1u && write_block_calls == 1u);
    assert(capture_calls == 1u && capture_kinds[0] == 10u);
    assert(rid_calls == 0u && enqueue_calls == 0u);
    assert(sense_calls == 1u && sense_key == 4u && sense_code == 0x4400u);
    assert(information_calls == 1u && sense_information == 4000ul);
}

static void TestTwoRecordGroupsAndRingWrap(void)
{
    u8 index;

    ResetHarness();
    SetQRecord(0u, 0x11u);
    SetQRecord(1u, 0x22u);

    assert(AcquireRawSectorsFromQRecordTable(20ul, 2u) == 1u);
    assert(divide_calls == 1u && wait_calls == 20u);
    assert(write_block_calls == 20u && capture_calls == 21u);
    assert(rid_calls == 20u && update_calls == 20u && enqueue_calls == 20u && increment_calls == 21u);
    for(index = 0u; index != 20u; ++index)
    {
        fw_addr_t expected_cursor = (index & 1u) == 0u ? 0u : 0x930u;

        assert(capture_addresses[index] == expected_cursor);
        assert(capture_kinds[index] == 10u);
        assert(rid_addresses[index] == expected_cursor + 0x10u);
        assert(enqueued_cursors[index] == expected_cursor);
        assert(enqueued_separators[index] == (u8)(index % 10u));
        assert(enqueued_tracks[index] == (index < 10u ? 0x11u : 0x22u));
    }
    assert(capture_addresses[20] == 0u && capture_kinds[20] == 6u);
    assert(ReadU32(0x1598u) == 4020ul);
    assert(memory_image[0x142au] == 24u);
    assert(memory_image[0x1273u] == 0x22u);
}

static void TestTerminalCaptureFailureUsesAdvancedTarget(void)
{
    ResetHarness();
    capture_failure_call = 10;
    SetQRecord(0u, 0x33u);

    assert(AcquireRawSectorsFromQRecordTable(10ul, 1u) == 0u);
    assert(capture_calls == 11u && capture_kinds[10] == 6u);
    assert(enqueue_calls == 10u && increment_calls == 10u);
    assert(sense_calls == 1u && information_calls == 1u);
    assert(sense_information == 4010ul);
}

int main(void)
{
    TestZeroCountBypassesInitialization();
    TestProducerCapacityFailure();
    TestDataCaptureFailure();
    TestTwoRecordGroupsAndRingWrap();
    TestTerminalCaptureFailureUsesAdvancedTarget();
    puts("raw-read Q-record sector acquisition tests passed");
    return 0;
}
