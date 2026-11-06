#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_frame_prefix_wait.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x300000];
static u16 tuple_calls, select_calls, find_calls, decode_calls;
static u16 sequence_error_calls, sequence_calls, window_calls, poll_calls;
static u16 failure_calls, queue_sense_calls;
static u8 decode_result, sequence_result, window_result, poll_result;
static s32 last_sequence_count;
static u8 last_reverse, window_mode[2], window_source[2], last_failure;
static u32 helper_target;

static u16 ReadU16(fw_addr_t a)
{
    return (u16)(((u16)memory_image[a] << 8) | memory_image[a + 1u]);
}
static u32 ReadU32(fw_addr_t a)
{
    return ((u32)memory_image[a] << 24) | ((u32)memory_image[a + 1u] << 16) | ((u32)memory_image[a + 2u] << 8) | memory_image[a + 3u];
}
static void WriteU16(fw_addr_t a, u16 v)
{
    memory_image[a] = (u8)(v >> 8);
    memory_image[a + 1u] = (u8)v;
}
static void WriteU32(fw_addr_t a, u32 v)
{
    memory_image[a] = (u8)(v >> 24);
    memory_image[a + 1u] = (u8)(v >> 16);
    memory_image[a + 2u] = (u8)(v >> 8);
    memory_image[a + 3u] = (u8)v;
}
static void SetRecord(u16 index, u8 adr, u8 point)
{
    memory_image[0xb4bdu + ((u32)index << 3)] = adr;
    memory_image[0xb4bfu + ((u32)index << 3)] = point;
}
static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    tuple_calls = select_calls = find_calls = decode_calls = 0u;
    sequence_error_calls = sequence_calls = window_calls = poll_calls = 0u;
    failure_calls = queue_sense_calls = 0u;
    decode_result = 2u;
    sequence_result = window_result = poll_result = 1u;
    last_sequence_count = 0l;
    last_reverse = last_failure = 0u;
    helper_target = 0xfffffffful;
    WriteU32(0x1598u, 1000ul);
    WriteU16(0x149bu, 3u);
    SetRecord(2u, 1u, 1u);
}

u8 FirmwareRead8(fw_addr_t a)
{
    assert(a < sizeof(memory_image));
    return memory_image[a];
}
void FirmwareWrite8(fw_addr_t a, u8 v)
{
    assert(a < sizeof(memory_image));
    memory_image[a] = v;
}
void ObserveRawReadPrefixTuple(s32 frame, u8 *tuple)
{
    ++tuple_calls;
    assert(frame == 1000l);
    tuple[0] = tuple[1] = tuple[2] = 0u;
}
u8 ObserveRawReadPrefixSelectSectorClass(void)
{
    ++select_calls;
    return 3u;
}
u8 ObserveRawReadPrefixFindClass(u16 index, u8 forward)
{
    ++find_calls;
    assert(index == 2u && forward == 0u);
    return 2u;
}
u8 ObserveRawReadPrefixDecodeType(u16 index)
{
    ++decode_calls;
    assert(index == 2u);
    return decode_result;
}
void ObserveRawReadPrefixSequenceError(void)
{
    ++sequence_error_calls;
}
u8 ObserveRawReadPrefixAcquireSequence(s32 count, u8 reverse)
{
    ++sequence_calls;
    last_sequence_count = count;
    last_reverse = reverse;
    if(sequence_result != 0u && helper_target != 0xfffffffful)
        WriteU32(0x1598u, helper_target);
    return sequence_result;
}
u8 ObserveRawReadPrefixAcquireWindow(u8 mode, u8 source)
{
    assert(window_calls < 2u);
    window_mode[window_calls] = mode;
    window_source[window_calls] = source;
    ++window_calls;
    if(window_result != 0u && helper_target != 0xfffffffful)
        WriteU32(0x1598u, helper_target);
    return window_result;
}
u8 ObserveRawReadPrefixPoll(u8 count)
{
    ++poll_calls;
    assert(count == 1u);
    return poll_result;
}
void ObserveRawReadPrefixFailure(u8 code)
{
    ++failure_calls;
    last_failure = code;
}
void ObserveRawReadPrefixQueueSense(void)
{
    ++queue_sense_calls;
}

static void TestBypassAndValidation(void)
{
    ResetHarness();
    memory_image[0x14b5u] = 1u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(tuple_calls == 0u && poll_calls == 0u);

    ResetHarness();
    memory_image[0xc44u] = 4u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 0u);
    assert(tuple_calls == 1u && sequence_error_calls == 1u);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    memory_image[0x1273u] = 0xaau;
    memory_image[0x1272u] = 5u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(find_calls == 1u && decode_calls == 0u && sequence_calls == 0u);
}

static void TestShortSequences(void)
{
    ResetHarness();
    helper_target = 1003ul;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(select_calls == 1u && sequence_calls == 1u);
    assert(last_sequence_count == 3l && last_reverse == 0u);
    assert(ReadU32(0x1598u) == 1002ul && ReadU32(0x8000u) == 1002ul);

    ResetHarness();
    helper_target = 1002ul;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(1u, 0u) == 1u);
    assert(last_sequence_count == 2l && ReadU32(0x1598u) == 1002ul);
    assert(ReadU32(0x8000u) == 1002ul);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    SetRecord(2u, 1u, 0u);
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(decode_calls == 1u && last_reverse == 1u);

    ResetHarness();
    sequence_result = 0u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u);
    assert(poll_calls == 0u);
}

static void TestFixedWindows(void)
{
    ResetHarness();
    memory_image[0xc48u] = 4u;
    helper_target = 1003ul;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(window_calls == 2u && window_mode[0] == 1u && window_mode[1] == 2u);
    assert(window_source[0] == 4u && window_source[1] == 4u);
    assert(ReadU32(0x1598u) == 1002ul && ReadU32(0x8000u) == 1002ul);

    ResetHarness();
    memory_image[0xc48u] = 4u;
    memory_image[0xc46u] = 2u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(window_calls == 1u && window_mode[0] == 2u);

    ResetHarness();
    memory_image[0xc44u] = 2u;
    decode_result = 4u;
    SetRecord(2u, 1u, 0u);
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 0u) == 1u);
    assert(window_calls == 2u && window_source[0] == 6u && window_source[1] == 6u);

    ResetHarness();
    memory_image[0xc48u] = 4u;
    helper_target = 1001ul;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(1u, 0u) == 1u);
    assert(window_calls == 1u && window_mode[0] == 1u && window_source[0] == 4u);
    assert(ReadU32(0x1598u) == 1001ul && ReadU32(0x8000u) == 1001ul);

    ResetHarness();
    memory_image[0xc48u] = 4u;
    window_result = 0u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u);
    assert(window_calls == 1u && poll_calls == 0u);
}

static void TestWaitRoutes(void)
{
    ResetHarness();
    memory_image[0x14b5u] = 1u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 1u);
    assert(poll_calls == 1u && memory_image[0x14b3u] == 0u);

    ResetHarness();
    memory_image[0x14b5u] = 1u;
    memory_image[0xc84u] = 0x40u;
    poll_result = 0u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u);
    assert(failure_calls == 1u && last_failure == 8u);

    ResetHarness();
    memory_image[0x14b5u] = 1u;
    poll_result = 0u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u);
    assert(queue_sense_calls == 1u);

    ResetHarness();
    memory_image[0x14b5u] = 1u;
    WriteU16(0x1292u, 1u);
    poll_result = 0u;
    assert(PrepareRawReadFramePrefixAndOptionallyWait(0u, 1u) == 0u);
    assert(failure_calls == 1u && last_failure == 2u);
}

int main(void)
{
    TestBypassAndValidation();
    TestShortSequences();
    TestFixedWindows();
    TestWaitRoutes();
    puts("raw-read frame prefix/wait tests passed");
    return 0;
}
