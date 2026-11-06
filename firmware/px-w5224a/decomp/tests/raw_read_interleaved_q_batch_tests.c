#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_interleaved_q_batch.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static fw_addr_t resume_cursor;
static u8 capture_result;
static u16 resume_calls;
static u16 convert_calls;
static u16 pack_calls;
static u16 capture_calls;
static u16 copy_calls;
static u16 update_calls;
static u16 enqueue_calls;
static u16 increment_calls;
static u16 advance_calls;
static s32 converted_frame;
static fw_addr_t converted_destination;
static fw_addr_t captured_addresses[16];
static u8 captured_kinds[16];
static u8 enqueued_track_numbers[16];

static u16 ReadU16(fw_addr_t address)
{
    return ((u16)memory_image[address] << 8) | memory_image[address + 1u];
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

static void SetQRecord(u16 record_index, u8 control, u8 track)
{
    fw_addr_t address = 0x8012u + (fw_addr_t)record_index * 10u;
    u8 index;

    for(index = 0u; index != 10u; ++index)
        memory_image[address + index] = (u8)(0x80u + index);
    memory_image[address] = control;
    memory_image[address + 1u] = track;
}

static void ResetHarness(void)
{
    memset(memory_image, 0xa5, sizeof(memory_image));
    resume_cursor = 0x120u;
    capture_result = 1u;
    resume_calls = 0u;
    convert_calls = 0u;
    pack_calls = 0u;
    capture_calls = 0u;
    copy_calls = 0u;
    update_calls = 0u;
    enqueue_calls = 0u;
    increment_calls = 0u;
    advance_calls = 0u;
    converted_frame = 0;
    converted_destination = 0u;
    memset(captured_addresses, 0, sizeof(captured_addresses));
    memset(captured_kinds, 0, sizeof(captured_kinds));
    memset(enqueued_track_numbers, 0, sizeof(enqueued_track_numbers));
    WriteU16(0x14c1u, 0u);
    WriteU16(0x14c3u, 0u);
    WriteU16(0x14c5u, 0u);
    WriteU16(0x14c7u, 0u);
    memory_image[0x14c9u] = 0u;
    WriteU16(0x8008u, 1u);
    WriteU16(0x800au, 1u);
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

fw_addr_t ObserveInterleavedQResumeRing(void)
{
    ++resume_calls;
    return resume_cursor;
}

void ObserveInterleavedQConvert(s32 frame_position, fw_addr_t output_tuple_address)
{
    ++convert_calls;
    converted_frame = frame_position;
    converted_destination = output_tuple_address;
    memory_image[output_tuple_address] = 11u;
    memory_image[output_tuple_address + 1u] = 22u;
    memory_image[output_tuple_address + 2u] = 33u;
}

void ObserveInterleavedQPack(u16 tuple_address)
{
    ++pack_calls;
    assert(tuple_address == 0x7000u);
    memory_image[tuple_address] = 0x11u;
    memory_image[tuple_address + 1u] = 0x22u;
    memory_image[tuple_address + 2u] = 0x33u;
}

u8 ObserveInterleavedQCapture(fw_addr_t controller_buffer_address, u8 source_kind)
{
    assert(capture_calls < 16u);
    captured_addresses[capture_calls] = controller_buffer_address;
    captured_kinds[capture_calls] = source_kind;
    ++capture_calls;
    return capture_result;
}

void ObserveInterleavedQCopyTuple(u16 destination, u16 source)
{
    ++copy_calls;
    assert(destination == 0x1275u && source == 0x7000u);
    memory_image[destination] = memory_image[source];
    memory_image[destination + 1u] = memory_image[source + 1u];
    memory_image[destination + 2u] = memory_image[source + 2u];
}

void ObserveInterleavedQUpdate(void)
{
    ++update_calls;
}

void ObserveInterleavedQEnqueue(void)
{
    assert(enqueue_calls < 16u);
    enqueued_track_numbers[enqueue_calls] = memory_image[0x1273u];
    ++enqueue_calls;
}

void ObserveInterleavedQIncrement(u16 tuple_address)
{
    ++increment_calls;
    assert(tuple_address == 0x7000u);
    ++memory_image[tuple_address + 2u];
}

fw_addr_t ObserveInterleavedQAdvanceRing(fw_addr_t cursor)
{
    ++advance_calls;
    return cursor + 0x20u;
}

static void AssertAuxiliarySlotCleared(fw_addr_t main_data_cursor)
{
    fw_addr_t address = main_data_cursor + 0x100000ul;
    u8 index;

    for(index = 0u; index != 16u; ++index)
        assert(memory_image[address + index] == 0u);
}

static void TestZeroCountStillInitializesTuple(void)
{
    ResetHarness();
    WriteU32(0x1598u, 0x12345678ul);

    assert(AcquireRawReadFrameBatchWithInterleavedQRecords(0u) == 1u);
    assert(resume_calls == 1u && convert_calls == 1u && pack_calls == 1u);
    assert(converted_frame == (s32)0x12345678ul);
    assert(converted_destination == 0x7000u);
    assert(capture_calls == 0u && enqueue_calls == 0u);
    assert(ReadU32(0x1598u) == 0x12345678ul);
}

static void TestPartialGroupAndTupleOverride(void)
{
    ResetHarness();
    capture_result = 0u;
    WriteU32(0x1598u, 100u);
    WriteU16(0x14c5u, 2u);
    SetQRecord(2u, 0x41u, 0x22u);

    assert(AcquireRawReadFrameBatchWithInterleavedQRecords(2u) == 1u);
    assert(capture_calls == 2u && update_calls == 2u && enqueue_calls == 2u && advance_calls == 2u);
    assert(captured_addresses[0] == 0x120u && captured_addresses[1] == 0x140u);
    assert(captured_kinds[0] == 7u && captured_kinds[1] == 7u);
    AssertAuxiliarySlotCleared(0x120u);
    AssertAuxiliarySlotCleared(0x140u);
    assert(copy_calls == 2u);
    assert(memory_image[0x1275u] == 0x11u && memory_image[0x1276u] == 0x22u && memory_image[0x1277u] == 0x34u);
    assert(ReadU32(0x1598u) == 102u);
    assert(ReadU16(0x14c7u) == 2u);
    assert(increment_calls == 2u);
}

static void TestAlternatingThreeFrameGroups(void)
{
    u8 index;

    ResetHarness();
    WriteU32(0x1598u, 200u);
    WriteU16(0x8008u, 8u);
    WriteU16(0x800au, 12u);
    WriteU16(0x14c1u, 4u);
    WriteU16(0x14c3u, 10u);
    WriteU16(0x14c5u, 2u);
    SetQRecord(2u, 0x42u, 0x22u);
    SetQRecord(5u, 0x42u, 0x55u);

    assert(AcquireRawReadFrameBatchWithInterleavedQRecords(6u) == 1u);
    assert(capture_calls == 6u && copy_calls == 0u);
    for(index = 0u; index != 3u; ++index)
        assert(enqueued_track_numbers[index] == 0x22u);
    for(; index != 6u; ++index)
        assert(enqueued_track_numbers[index] == 0x55u);
    assert(ReadU16(0x14c7u) == 0u);
    assert(memory_image[0x14c9u] == 0u);
    assert(ReadU16(0x14c1u) == 5u);
    assert(ReadU16(0x14c5u) == 10u);
    assert(ReadU16(0x14c3u) == 11u);
    assert(ReadU32(0x1598u) == 206u);
}

static void TestCursorWrapPolicies(void)
{
    ResetHarness();
    WriteU16(0x8008u, 6u);
    WriteU16(0x800au, 6u);
    WriteU16(0x14c1u, 5u);
    WriteU16(0x14c3u, 9u);
    WriteU16(0x14c5u, 1u);
    memory_image[0x14c9u] = 1u;
    SetQRecord(1u, 2u, 1u);
    assert(AcquireRawReadFrameBatchWithInterleavedQRecords(3u) == 1u);
    assert(memory_image[0x14c9u] == 1u);
    assert(ReadU16(0x14c1u) == 0u && ReadU16(0x14c5u) == 0u);

    ResetHarness();
    WriteU16(0x8008u, 4u);
    WriteU16(0x800au, 7u);
    WriteU16(0x14c3u, 6u);
    WriteU16(0x14c5u, 1u);
    memory_image[0x14c9u] = 1u;
    SetQRecord(1u, 2u, 1u);
    assert(AcquireRawReadFrameBatchWithInterleavedQRecords(3u) == 1u);
    assert(memory_image[0x14c9u] == 0u);
    assert(ReadU16(0x14c5u) == 6u && ReadU16(0x14c3u) == 4u);
}

int main(void)
{
    TestZeroCountStillInitializesTuple();
    TestPartialGroupAndTupleOverride();
    TestAlternatingThreeFrameGroups();
    TestCursorWrapPolicies();
    puts("raw-read interleaved-Q batch tests passed");
    return 0;
}
