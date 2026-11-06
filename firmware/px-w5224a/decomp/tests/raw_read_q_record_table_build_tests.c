#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_q_record_table_build.h"
#include "platform/firmware_memory.h"

#define MEMORY_SIZE 0x2c2000ul

static u8 memory_image[MEMORY_SIZE];
static u8 bit0041;
static int wait_failure_call;
static int capture_failure_call;
static u16 wait_calls;
static u16 capture_calls;
static u16 write_block_calls;
static u16 rid_calls;
static u16 increment_calls;
static u16 update_calls;
static u16 enqueue_calls;
static u16 report_calls;
static u16 latch_calls;
static u16 sense_calls;
static u16 information_calls;
static u16 convert_calls;
static u16 pack_calls;
static u16 unpack_calls;
static u16 tuple_to_frame_calls;
static u8 report_code;
static u8 sense_key;
static u16 sense_code;
static u32 sense_information;
static s32 converted_frames[8];
static fw_addr_t capture_cursors[64];
static u8 capture_kinds[64];
static u8 enqueued_q[64][10];

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

static void SetTrackRecord(u16 index, u8 control, u8 track, u8 track_index, u8 minute, u8 second, u8 frame)
{
    fw_addr_t record = TrackRecord(index);

    memory_image[record] = control;
    memory_image[record + 1u] = track;
    memory_image[record + 2u] = track_index;
    memory_image[record + 4u] = minute;
    memory_image[record + 6u] = second;
    memory_image[record + 7u] = frame;
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    bit0041 = 0u;
    wait_failure_call = -1;
    capture_failure_call = -1;
    wait_calls = 0u;
    capture_calls = 0u;
    write_block_calls = 0u;
    rid_calls = 0u;
    increment_calls = 0u;
    update_calls = 0u;
    enqueue_calls = 0u;
    report_calls = 0u;
    latch_calls = 0u;
    sense_calls = 0u;
    information_calls = 0u;
    convert_calls = 0u;
    pack_calls = 0u;
    unpack_calls = 0u;
    tuple_to_frame_calls = 0u;
    report_code = 0u;
    sense_key = 0u;
    sense_code = 0u;
    sense_information = 0u;
    memset(converted_frames, 0, sizeof(converted_frames));
    memset(capture_cursors, 0, sizeof(capture_cursors));
    memset(capture_kinds, 0, sizeof(capture_kinds));
    memset(enqueued_q, 0, sizeof(enqueued_q));
    memory_image[0x1149u] = 1u;
    memory_image[0x114eu] = 1u;
    memory_image[0xc4au] = 0x20u;
    memory_image[0x2bb5e3u] = 0x20u;
    WriteU32(0x1143u, 1200ul);
    WriteU32(0x114fu, 5000ul);
    WriteU32(0x156au, 4ul);
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

u8 ObserveRawQTableBit0041(void)
{
    return bit0041;
}

void ObserveRawQTableConvert(s32 frame_position, fw_addr_t tuple_address)
{
    assert(convert_calls < 8u);
    converted_frames[convert_calls++] = frame_position;
    memory_image[tuple_address] = 1u;
    memory_image[tuple_address + 1u] = 2u;
    memory_image[tuple_address + 2u] = 3u;
}

void ObserveRawQTablePack(u16 tuple_address)
{
    ++pack_calls;
    memory_image[tuple_address] = (u8)(memory_image[tuple_address] + 0x10u);
    memory_image[tuple_address + 1u] = (u8)(memory_image[tuple_address + 1u] + 0x10u);
    memory_image[tuple_address + 2u] = (u8)(memory_image[tuple_address + 2u] + 0x10u);
}

void ObserveRawQTableUnpack(u16 tuple_address)
{
    ++unpack_calls;
    memory_image[tuple_address] = (u8)(memory_image[tuple_address] - 0x10u);
    memory_image[tuple_address + 1u] = (u8)(memory_image[tuple_address + 1u] - 0x10u);
    memory_image[tuple_address + 2u] = (u8)(memory_image[tuple_address + 2u] - 0x10u);
}

void ObserveRawQTableCopyTuple(u16 destination, u16 source)
{
    memory_image[destination] = memory_image[source];
    memory_image[destination + 1u] = memory_image[source + 1u];
    memory_image[destination + 2u] = memory_image[source + 2u];
}

s32 ObserveRawQTableTupleToFrame(fw_addr_t tuple_address)
{
    ++tuple_to_frame_calls;
    return (s32)(memory_image[tuple_address] * 10000ul + memory_image[tuple_address + 1u] * 100ul + memory_image[tuple_address + 2u]);
}

u8 ObserveRawQTableBinaryToBcd(u8 value)
{
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

u32 ObserveRawQTableMultiply(u32 left, u32 right)
{
    return left * right;
}

u8 ObserveRawQTableWaitCapacity(void)
{
    int call = (int)wait_calls++;
    return (u8)(call != wait_failure_call);
}

void ObserveRawQTableWriteControllerBlock(fw_addr_t source)
{
    ++write_block_calls;
    assert(source == 0x1428ul);
}

u8 ObserveRawQTableCapture(fw_addr_t cursor, u8 source_kind)
{
    int call = (int)capture_calls;

    assert(capture_calls < 64u);
    capture_cursors[capture_calls] = cursor;
    capture_kinds[capture_calls] = source_kind;
    ++capture_calls;
    return (u8)(call != capture_failure_call);
}

void ObserveRawQTableBuildRid01(fw_addr_t cursor)
{
    ++rid_calls;
    assert((cursor & 0xfu) == 0u);
}

void ObserveRawQTableIncrement(u16 tuple_address)
{
    ++increment_calls;
    assert(tuple_address == 0x1428u);
    ++memory_image[tuple_address + 2u];
}

void ObserveRawQTableUpdateQ(void)
{
    ++update_calls;
}

void ObserveRawQTableEnqueue(void)
{
    u8 index;

    assert(enqueue_calls < 64u);
    for(index = 0u; index != 10u; ++index)
        enqueued_q[enqueue_calls][index] = memory_image[0x1272u + index];
    ++enqueue_calls;
}

void ObserveRawQTableReportFailure(u8 failure_code)
{
    ++report_calls;
    report_code = failure_code;
}

void ObserveRawQTableLatchFailure(void)
{
    ++latch_calls;
}

void ObserveRawQTableSetSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}

void ObserveRawQTableSetInformation(u32 information)
{
    ++information_calls;
    sense_information = information;
}

static void TestInitialAdr2RecordAndFullGroup(void)
{
    u16 index;

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128du] = 4u;
    memory_image[0x1149u] = 0u;
    memory_image[0x114eu] = 0u;
    memory_image[0x2bb5deu] = 1u;
    memory_image[0x2bb5dfu] = 12u;
    memory_image[0x2bb5e0u] = 34u;
    memset(&memory_image[0x8012u], 0xa5, 2200u);

    assert(BuildRawReadQRecordTable() == 1u);
    assert(ReadU16(0x800eu) == 1u && ReadU16(0x8010u) == 11u);
    assert(memory_image[0x8012u] == 2u);
    assert(memory_image[0x8015u] == 0x01u);
    assert(memory_image[0x8016u] == 0x12u);
    assert(memory_image[0x8017u] == 0x34u);
    assert(memory_image[0x801au] == 0x20u);
    for(index = 10u; index != 2200u; ++index)
        assert(memory_image[0x8012u + index] == 0u);
    assert(capture_calls == 11u && wait_calls == 10u);
    assert(capture_kinds[0] == 10u && capture_kinds[10] == 6u);
    assert(enqueued_q[0][6] == 0u && enqueued_q[9][6] == 9u && enqueued_q[10][6] == 10u);
    assert(memory_image[0x1289u] == 0u);
}

static void TestUnsupportedWriteTypeAndTypeTwoBitBypass(void)
{
    ResetHarness();
    memory_image[0xc44u] = 9u;
    memory_image[0x8012u] = 0xa5u;
    assert(BuildRawReadQRecordTable() == 0u);
    assert(memory_image[0x8012u] == 0u && capture_calls == 0u);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    bit0041 = 1u;
    assert(BuildRawReadQRecordTable() == 1u);
    assert(capture_calls == 0u && memory_image[0x1289u] == 1u);
}

static void TestSpecialTypeTwoSequence(void)
{
    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[0xc45u] = 0u;
    memory_image[0x12c9u] = 1u;

    assert(BuildRawReadQRecordTable() == 1u);
    assert(ReadU16(0x800eu) == 0u && ReadU16(0x8010u) == 1u);
    assert(ReadU32(0x1598u) == 5001ul);
    assert(capture_calls == 11u && wait_calls == 10u);
    assert(capture_kinds[9] == 10u && capture_kinds[10] == 6u);
    assert(enqueued_q[10][6] == 10u);
    assert(converted_frames[0] == 4990l);
}

static void TestTypeTwoRecordPairing(void)
{
    u16 index;

    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[0xc45u] = 0xc0u;
    WriteU16(0xb4bbu, 4u);
    SetTrackRecord(0u, 0x11u, 1u, 1u, 2u, 3u, 4u);
    SetTrackRecord(1u, 0x11u, 1u, 2u, 5u, 6u, 7u);
    SetTrackRecord(2u, 0x21u, 2u, 1u, 8u, 9u, 10u);
    SetTrackRecord(3u, 0x21u, 2u, 2u, 11u, 12u, 13u);

    assert(BuildRawReadQRecordTable() == 1u);
    assert(ReadU16(0x800eu) == 2u && ReadU16(0x8010u) == 11u);
    assert(memory_image[0x8012u] == 0x11u && memory_image[0x8014u] == 0x01u);
    assert(memory_image[0x8019u] == 0x12u && memory_image[0x801au] == 0x13u && memory_image[0x801bu] == 0x14u);
    assert(memory_image[0x8015u] == 0x15u && memory_image[0x8016u] == 0x16u && memory_image[0x8017u] == 0x17u);
    assert(capture_calls == 11u);
    for(index = 0u; index != 5u; ++index)
        assert(enqueued_q[index][2] == 0x01u && enqueued_q[index][6] == (u8)index);
    for(; index != 10u; ++index)
        assert(enqueued_q[index][2] == 0x02u && enqueued_q[index][6] == (u8)index);
    assert(enqueued_q[10][2] == 0x01u && enqueued_q[10][6] == 10u);
}

static void TestModeThreeLimitFailure(void)
{
    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128du] = 3u;
    memory_image[0x114au] = 21u;

    assert(BuildRawReadQRecordTable() == 0u);
    assert(latch_calls == 1u && sense_calls == 1u);
    assert(sense_key == 3u && sense_code == 0x7304u);
    assert(information_calls == 1u && sense_information == 1200ul);
    assert(capture_calls == 0u);
}

static void TestModeFiveRecord(void)
{
    ResetHarness();
    memory_image[0xc44u] = 1u;
    memory_image[0x128du] = 5u;
    memory_image[0x114cu] = 4u;
    memory_image[0xdd36u] = 0xa7u;
    WriteU32(0x8000u, 600ul);

    assert(BuildRawReadQRecordTable() == 1u);
    assert(ReadU16(0x800eu) == 1u && ReadU16(0x8010u) == 11u);
    assert(memory_image[0x8012u] == 0xa5u);
    assert(memory_image[0x8014u] == 0x05u);
    assert(memory_image[0x8015u] == 0x11u && memory_image[0x8017u] == 0x13u);
    assert(memory_image[0x8019u] == 0x11u && memory_image[0x801bu] == 0x13u);
    assert(convert_calls == 3u);
    assert(converted_frames[0] == 600l && converted_frames[1] == 599l && converted_frames[2] == 1200l);
}

static void TestStagedModeZeroRecord(void)
{
    u8 index;

    ResetHarness();
    memory_image[0xc44u] = 0u;
    memory_image[0x128du] = 0u;
    memory_image[0x128bu] = 1u;
    for(index = 0u; index != 10u; ++index)
        memory_image[0x2bc11bu + index] = (u8)(0x10u + index);

    assert(BuildRawReadQRecordTable() == 1u);
    for(index = 0u; index != 10u; ++index)
        assert(memory_image[0x8012u + index] == (u8)(0x10u + index));
    assert(memory_image[0x128bu] == 0u);
    assert(unpack_calls == 2u && tuple_to_frame_calls == 2u);
    assert(ReadU32(0x8000u) == 30405ul);
    assert(capture_calls == 11u);
}

static void TestCapacityAndCaptureFailures(void)
{
    ResetHarness();
    memory_image[0xc44u] = 1u;
    memory_image[0x128du] = 5u;
    wait_failure_call = 0;
    assert(BuildRawReadQRecordTable() == 0u);
    assert(report_calls == 1u && report_code == 3u);
    assert(capture_calls == 0u && sense_calls == 0u);

    ResetHarness();
    memory_image[0xc44u] = 1u;
    memory_image[0x128du] = 5u;
    capture_failure_call = 0;
    assert(BuildRawReadQRecordTable() == 0u);
    assert(capture_calls == 1u && sense_calls == 1u);
    assert(sense_key == 4u && sense_code == 0x4400u);
    assert(information_calls == 1u && sense_information == 1200ul);
}

int main(void)
{
    TestInitialAdr2RecordAndFullGroup();
    TestUnsupportedWriteTypeAndTypeTwoBitBypass();
    TestSpecialTypeTwoSequence();
    TestTypeTwoRecordPairing();
    TestModeThreeLimitFailure();
    TestModeFiveRecord();
    TestStagedModeZeroRecord();
    TestCapacityAndCaptureFailures();
    puts("raw_read_q_record_table_build_tests: ok");
    return 0;
}
