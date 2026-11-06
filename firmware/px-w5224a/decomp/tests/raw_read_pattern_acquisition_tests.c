#include <assert.h>
#include <string.h>
#include "media/raw_read_pattern_acquisition.h"
#include "platform/firmware_memory.h"

enum EventKind
{
    EVENT_CLEAR_BIT,
    EVENT_RESET_QUEUE,
    EVENT_INITIALIZE_COROUTINE,
    EVENT_RESET_TRANSFER,
    EVENT_YIELD,
    EVENT_POLL,
    EVENT_VALIDATE,
    EVENT_DISPATCH
};

typedef struct Event
{
    enum EventKind kind;
    u32 value;
} Event;

static u8 memory[1u << 24];
static Event events[128];
static unsigned event_count;
static u8 poll_results[16];
static u8 poll_operating_codes[16];
static unsigned poll_count;
static unsigned poll_index;
static u8 validation_results[8];
static unsigned validation_count;
static unsigned validation_index;
static u32 reset_transfer_result;

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

static void Record(enum EventKind kind, u32 value)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].value = value;
    ++event_count;
}

void ObserveRawReadPatternClearBit(u8 bit_address)
{
    Record(EVENT_CLEAR_BIT, bit_address);
}

void ObserveRawReadPatternResetQueue(void)
{
    Record(EVENT_RESET_QUEUE, 0u);
}

void ObserveRawReadPatternInitializeCoroutine2(u32 entry_point)
{
    Record(EVENT_INITIALIZE_COROUTINE, entry_point);
}

u32 ObserveRawReadPatternResetTransfer(void)
{
    Record(EVENT_RESET_TRANSFER, reset_transfer_result);
    return reset_transfer_result;
}

void ObserveRawReadPatternYield(void)
{
    Record(EVENT_YIELD, 0u);
}

u8 ObserveRawReadPatternPollBatch(void)
{
    assert(poll_index < poll_count);
    memory[0x000e43u] = poll_operating_codes[poll_index];
    Record(EVENT_POLL, poll_results[poll_index]);
    return poll_results[poll_index++];
}

u8 ObserveRawReadPatternValidateBatch(u16 descriptor_count)
{
    assert(validation_index < validation_count);
    Record(EVENT_VALIDATE, descriptor_count);
    return validation_results[validation_index++];
}

void ObserveRawReadPatternDispatchRequest5(void)
{
    Record(EVENT_DISPATCH, 0u);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(poll_results, 0, sizeof(poll_results));
    memset(poll_operating_codes, 0, sizeof(poll_operating_codes));
    memset(validation_results, 0, sizeof(validation_results));
    event_count = 0u;
    poll_count = 0u;
    poll_index = 0u;
    validation_count = 0u;
    validation_index = 0u;
    reset_transfer_result = 0x12345678ul;
    WriteBe16(0x00129cu, 32u);
    WriteBe32(0x001153u, 0x00016cdau);
}

static void AssertInitialization(void)
{
    static const u8 clear_bits[] = { 0x0c, 0x0d, 0x1d, 0x1c, 0x13 };
    unsigned index;

    assert(ReadBe32(0x000f0cu) == 0u);
    assert(memory[0x000c37u] == 40u && memory[0x000c3du] == 15u);
    for(index = 0u; index < sizeof(clear_bits); ++index)
    {
        assert(events[index].kind == EVENT_CLEAR_BIT);
        assert(events[index].value == clear_bits[index]);
    }
}

static void AssertRestored(void)
{
    assert(memory[0x001037u] == 0u);
    assert(memory[0x000c2au] == 7u);
    assert(ReadBe32(0x001598u) == 150u);
    assert(events[event_count - 4u].kind == EVENT_RESET_QUEUE);
    assert(events[event_count - 3u].kind == EVENT_INITIALIZE_COROUTINE);
    assert(events[event_count - 3u].value == 0x00f84a00ul);
    assert(events[event_count - 2u].kind == EVENT_DISPATCH);
    assert(events[event_count - 1u].kind == EVENT_YIELD);
}

static void TestBoundaryRejection(void)
{
    s8 result;

    ResetState();
    result = AcquireAndValidateRawReadPatternRange(99990l, 10ul);
    assert(result == (s8)0xfdu);
    AssertInitialization();
    assert(ReadBe32(0x0015dcu) == 99990ul);
    assert(ReadBe32(0x000b02u) == 99990ul);
    assert(ReadBe32(0x000b06u) == 16u);
    assert(event_count == 5u);
}

static void TestSingleBatchWithPendingPoll(void)
{
    s8 result;

    ResetState();
    poll_results[0] = 0xfeu;
    poll_results[1] = 1u;
    poll_operating_codes[0] = 6u;
    poll_operating_codes[1] = 6u;
    poll_count = 2u;
    validation_results[0] = 1u;
    validation_count = 1u;
    result = AcquireAndValidateRawReadPatternRange(100l, 10ul);
    assert(result == 1);
    AssertInitialization();
    assert(ReadBe32(0x0015dcu) == 100u);
    assert(ReadBe32(0x000b06u) == 10u);
    assert(ReadBe32(0x000b02u) == 110u);
    assert(ReadBe32(0x001598u) == 150u);
    assert(validation_index == 1u && poll_index == 2u);
    assert(events[5].kind == EVENT_RESET_QUEUE);
    assert(events[6].kind == EVENT_INITIALIZE_COROUTINE && events[6].value == 0x00f90000ul);
    assert(events[7].kind == EVENT_RESET_TRANSFER);
    assert(events[8].kind == EVENT_YIELD && events[9].kind == EVENT_POLL);
    assert(events[10].kind == EVENT_YIELD && events[11].kind == EVENT_POLL);
    assert(events[12].kind == EVENT_VALIDATE && events[12].value == 10u);
    AssertRestored();
}

static void TestMultipleBatches(void)
{
    static const u16 expected[] = { 8u, 8u, 4u };
    unsigned index;

    ResetState();
    WriteBe16(0x00129cu, 24u);
    for(index = 0u; index < 3u; ++index)
    {
        poll_results[index] = 1u;
        poll_operating_codes[index] = 5u;
        validation_results[index] = 1u;
    }
    poll_count = 3u;
    validation_count = 3u;
    assert(AcquireAndValidateRawReadPatternRange(200l, 20ul) == 1);
    assert(ReadBe32(0x000b02u) == 220u);
    assert(validation_index == 3u);
    index = 0u;
    for(unsigned event = 0u; event < event_count; ++event)
    {
        if(events[event].kind == EVENT_VALIDATE)
        {
            assert(index < 3u && events[event].value == expected[index]);
            ++index;
        }
    }
    assert(index == 3u);
    AssertRestored();
}

static void TestFailureStatuses(void)
{
    ResetState();
    poll_results[0] = 0u;
    poll_operating_codes[0] = 6u;
    poll_count = 1u;
    assert(AcquireAndValidateRawReadPatternRange(100l, 1ul) == (s8)0xffu);
    assert(validation_index == 0u);
    AssertRestored();

    ResetState();
    poll_results[0] = 1u;
    poll_operating_codes[0] = 5u;
    poll_count = 1u;
    validation_results[0] = 0u;
    validation_count = 1u;
    assert(AcquireAndValidateRawReadPatternRange(100l, 1ul) == (s8)0xfeu);
    assert(validation_index == 1u);
    AssertRestored();
}

static void TestThreeLowCodeRetries(void)
{
    unsigned index;

    ResetState();
    for(index = 0u; index < 3u; ++index)
    {
        poll_results[index] = 1u;
        poll_operating_codes[index] = 4u;
    }
    poll_count = 3u;
    assert(AcquireAndValidateRawReadPatternRange(100l, 4ul) == (s8)0xffu);
    assert(poll_index == 3u && validation_index == 0u);
    assert(ReadBe32(0x000b02u) == 100u);
    AssertRestored();
}

int main(void)
{
    TestBoundaryRejection();
    TestSingleBatchWithPendingPoll();
    TestMultipleBatches();
    TestFailureStatuses();
    TestThreeLowCodeRetries();
    return 0;
}
