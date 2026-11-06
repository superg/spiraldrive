#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_transfer_range.h"

static u8 memory[1u << 24];
static u8 aligned_result, prepare_result, crosses_result, prefix_result;
static u8 sequence_result;
static u32 align_result;
static unsigned aligned_calls, align_calls, prepare_calls, wait_calls;
static unsigned prefix_calls, sequence_calls, yield_calls, failure_calls;
static u32 align_argument, prepare_argument;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 IsWritePipelineAtAlignedCursorOrDifferentRecord(void)
{
    ++aligned_calls;
    return aligned_result;
}
u32 AlignWriteTransferCursorForBlockCount(u32 value)
{
    ++align_calls;
    align_argument = value;
    return align_result;
}
u8 PreparePartialWriteControllerBufferRange(u32 value)
{
    ++prepare_calls;
    prepare_argument = value;
    return prepare_result;
}
void WaitForWritePipelineStatusCompletion(void)
{
    ++wait_calls;
}
u8 DoesTransferCrossNextThirtyTwoBoundary(u32 value)
{
    (void)value;
    return crosses_result;
}
u8 PrepareRawReadFramePrefixAndOptionallyWait(u8 omit, u8 wait)
{
    assert(omit == 0u && wait == 1u);
    ++prefix_calls;
    return prefix_result;
}
void YieldCoroutine0(void)
{
    ++yield_calls;
    memory[0x00fb9aul] = 0u;
}
void UpdateWorkingTrackAfterWriteFailure(void)
{
    ++failure_calls;
}
u8 ExecuteWriteControllerBufferRangeSequence(void)
{
    ++sequence_calls;
    return sequence_result;
}

static void WriteU16(fw_addr_t a, u16 v)
{
    memory[a] = (u8)(v >> 8);
    memory[a + 1u] = (u8)v;
}
static void WriteU32(fw_addr_t a, u32 v)
{
    memory[a] = (u8)(v >> 24);
    memory[a + 1u] = (u8)(v >> 16);
    memory[a + 2u] = (u8)(v >> 8);
    memory[a + 3u] = (u8)v;
}
static u32 ReadU32(fw_addr_t a)
{
    return ((u32)memory[a] << 24) | ((u32)memory[a + 1u] << 16) | ((u32)memory[a + 2u] << 8) | memory[a + 3u];
}
static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    aligned_result = prepare_result = prefix_result = sequence_result = 1u;
    crosses_result = 0u;
    align_result = 7u;
    aligned_calls = align_calls = prepare_calls = wait_calls = 0u;
    prefix_calls = sequence_calls = yield_calls = failure_calls = 0u;
    align_argument = prepare_argument = 0ul;
    WriteU32(0x00fb6eul, 0x12345678ul);
}

static void TestInactiveSuccessAndReadbackEndpoint(void)
{
    ResetState();
    memory[0x000c8aul] = 0x20u;
    WriteU32(0x002bc703ul, 100u);
    WriteU32(0x000b06ul, 25u);
    assert(ExecuteWriteTransferBlockRange(9u) == 1u);
    assert(aligned_calls == 0u && prepare_calls == 0u);
    assert(memory[0x001554ul] == 0u && memory[0x0014b5ul] == 1u);
    assert(ReadU32(0x002bc70bul) == 125u);
    assert((memory[0x000c8aul] & 0x20u) == 0u);
    assert(ReadU32(0x00fb6eul) == 0x12345678ul);
}

static void TestInitialPreparedRange(void)
{
    ResetState();
    memory[0x002ba5c6ul] = 4u;
    aligned_result = 1u;
    memory[0x002ba5d2ul] = 9u;
    memory[0x002ba5c7ul] = 2u;
    WriteU32(0x002ba5b4ul, 100u);
    assert(ExecuteWriteTransferBlockRange(40u) == 1u);
    assert(align_calls == 1u && align_argument == 40u);
    assert(prepare_calls == 1u && prepare_argument == 7u);
    assert(wait_calls == 1u && sequence_calls == 1u);
    assert(ReadU32(0x002ba5b4ul) == 107u);
    assert(memory[0x002ba5c8ul] == 0u && memory[0x002ba5cbul] == 0u);
}

static void TestQueuedPrefixRoundsAndRestores(void)
{
    ResetState();
    WriteU16(0x001292ul, 1u);
    WriteU32(0x001598ul, 200u);
    memory[0x00fb9aul] = 2u;
    assert(ExecuteWriteTransferBlockRange(1u) == 1u);
    assert(prefix_calls == 1u && yield_calls == 1u);
    assert(ReadU32(0x001598ul) == 200u);
    assert(ReadU32(0x0014bcul) == 0x80000000ul);
    assert(memory[0x00127dul] == 0u);

    ResetState();
    WriteU16(0x001292ul, 1u);
    WriteU32(0x001598ul, 200u);
    prefix_result = 0u;
    assert(ExecuteWriteTransferBlockRange(1u) == 0u);
    assert(failure_calls == 1u);
    assert(ReadU32(0x001598ul) == 184u);
}

static void TestSecondPreparationAndFailures(void)
{
    ResetState();
    memory[0x002ba5c6ul] = 4u;
    aligned_result = 0u;
    memory[0x000c84ul] = 0x10u;
    memory[0x002ba5c9ul] = 0u;
    crosses_result = 1u;
    memory[0x00fb9aul] = 2u;
    assert(ExecuteWriteTransferBlockRange(12u) == 1u);
    assert(prepare_calls == 2u && align_calls == 1u);
    assert(wait_calls == 2u && prefix_calls == 1u && sequence_calls == 1u);
    assert(yield_calls == 1u && (memory[0x00fb69ul] & 1u) != 0u);

    ResetState();
    memory[0x002ba5c6ul] = 1u;
    prepare_result = 0u;
    assert(ExecuteWriteTransferBlockRange(3u) == 0u);
    assert(sequence_calls == 0u);

    ResetState();
    memory[0x00fba0ul] = 1u;
    assert(ExecuteWriteTransferBlockRange(3u) == 0u);
    assert(memory[0x0014b5ul] == 0u);
}

int main(void)
{
    TestInactiveSuccessAndReadbackEndpoint();
    TestInitialPreparedRange();
    TestQueuedPrefixRoundsAndRestores();
    TestSecondPreparationAndFailures();
    return 0;
}
