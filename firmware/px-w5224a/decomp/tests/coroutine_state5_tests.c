#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/coroutine_state5.h"
#include "platform/firmware_memory.h"

#define RESULT_CAPACITY 8u
#define EVENT_CAPACITY 160u

struct OperationEvent
{
    u8 operation;
    s32 argument;
};

static u8 memory_bytes[0x20000ul];
static u8 controller_range_state;
static s32 results[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES + 1u][RESULT_CAPACITY];
static s32 defaults[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES + 1u];
static unsigned result_lengths[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES + 1u];
static unsigned operation_counts[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES + 1u];
static struct OperationEvent events[EVENT_CAPACITY];
static unsigned event_count;
static u8 mutate_target_during_acquisition;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x002ba5d8ul)
        return controller_range_state;
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

s32 ObserveMediaCoroutineState5Operation(u8 operation, s32 argument)
{
    unsigned index;

    assert(operation <= MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES);
    assert(event_count < EVENT_CAPACITY);
    events[event_count].operation = operation;
    events[event_count].argument = argument;
    ++event_count;
    index = operation_counts[operation]++;
    if(operation == MEDIA_STATE5_RUN_ACQUISITION && mutate_target_during_acquisition != 0u)
        WriteDword(0x0e3bu, 0x76543210ul);
    if(index < result_lengths[operation])
        return results[operation][index];
    return defaults[operation];
}

static void SetResult(u8 operation, unsigned index, s32 result)
{
    assert(index < RESULT_CAPACITY);
    results[operation][index] = result;
    if(result_lengths[operation] <= index)
        result_lengths[operation] = index + 1u;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(results, 0, sizeof(results));
    memset(defaults, 0, sizeof(defaults));
    memset(result_lengths, 0, sizeof(result_lengths));
    memset(operation_counts, 0, sizeof(operation_counts));
    memset(events, 0, sizeof(events));
    controller_range_state = 0u;
    event_count = 0u;
    mutate_target_during_acquisition = 0u;
    defaults[MEDIA_STATE5_COMPUTE_POSITION_WINDOW] = 900l;
    defaults[MEDIA_STATE5_UPDATE_POSITION_TRACKING] = 12345l;
    defaults[MEDIA_STATE5_DISPATCH_INDEXED_STATE] = 1l;
    WriteDword(0x0e3bu, 0x00123456ul);
}

static void AssertEventArgument(u8 operation, unsigned occurrence, s32 argument)
{
    unsigned index;
    unsigned seen = 0u;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].operation == operation)
        {
            if(seen == occurrence)
            {
                assert(events[index].argument == argument);
                return;
            }
            ++seen;
        }
    }
    assert(0);
}

static void TestAcquisitionRetryRestoresTarget(void)
{
    ResetFixture();
    SetResult(MEDIA_STATE5_RUN_ACQUISITION, 0u, 2l);
    SetResult(MEDIA_STATE5_RUN_ACQUISITION, 1u, 3l);
    mutate_target_during_acquisition = 1u;
    memory_bytes[0x1301u] = 0xffu;
    memory_bytes[0xf971u] = 0xffu;
    ProcessMediaCoroutineState5();

    assert(operation_counts[MEDIA_STATE5_RUN_ACQUISITION] == 2u);
    AssertEventArgument(MEDIA_STATE5_RUN_ACQUISITION, 0u, 900l);
    AssertEventArgument(MEDIA_STATE5_RUN_ACQUISITION, 1u, 900l);
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    assert(memory_bytes[0x1806au] == 0u);
    assert(memory_bytes[0xf971u] == 0xfeu);
}

static void TestRetryLimitRoutes(void)
{
    ResetFixture();
    defaults[MEDIA_STATE5_TEST_INDEXED_STATE] = 1l;
    SetResult(MEDIA_STATE5_RUN_ACQUISITION, 0u, 2l);
    memory_bytes[0x0e40u] = 1u;
    memory_bytes[0x0fafu] = 9u;
    ProcessMediaCoroutineState5();
    assert(operation_counts[MEDIA_STATE5_RUN_ACQUISITION] == 1u);
    assert(memory_bytes[0x0e40u] == 0u);
    assert(memory_bytes[0x0fafu] == 0u);

    ResetFixture();
    defaults[MEDIA_STATE5_TEST_INDEXED_STATE] = 1l;
    controller_range_state = 1u;
    SetResult(MEDIA_STATE5_RUN_ACQUISITION, 0u, 2l);
    SetResult(MEDIA_STATE5_RUN_ACQUISITION, 1u, 5l);
    memory_bytes[0xf971u] = 1u;
    ProcessMediaCoroutineState5();
    assert(operation_counts[MEDIA_STATE5_RUN_ACQUISITION] == 2u);
    assert(memory_bytes[0x0fafu] == 3u);
    assert(memory_bytes[0xf971u] == 0u);
}

static void TestSuccessfulIndexedCompletion(void)
{
    ResetFixture();
    SetResult(MEDIA_STATE5_TEST_INDEXED_STATE, 0u, 0l);
    SetResult(MEDIA_STATE5_TEST_INDEXED_STATE, 1u, 1l);
    SetResult(MEDIA_STATE5_PARSE_PACKET, 0u, 0xfeu);
    SetResult(MEDIA_STATE5_PARSE_PACKET, 1u, 0u);
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x0e43u] = 0x14u;
    memory_bytes[0x1301u] = 0xffu;
    ProcessMediaCoroutineState5();

    assert(operation_counts[MEDIA_STATE5_YIELD] == 2u);
    assert(operation_counts[MEDIA_STATE5_UPDATE_CONTROLLER_PROFILE] == 1u);
    assert(operation_counts[MEDIA_STATE5_APPLY_STATE_F6_PARAMETERS] == 1u);
    assert(operation_counts[MEDIA_STATE5_SERVICE_POST_TARGET] == 1u);
    assert(operation_counts[MEDIA_STATE5_SERVICE_SECONDARY_CODE] == 1u);
    assert(operation_counts[MEDIA_STATE5_UPDATE_LEAD_OUT_STAGE] == 1u);
    assert(operation_counts[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES] == 0u);
    AssertEventArgument(MEDIA_STATE5_CONFIGURE_TARGET, 0u, 0x00123456l);
    AssertEventArgument(MEDIA_STATE5_SERVICE_POST_TARGET, 0u, 0x00123456l);
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    AssertEventArgument(MEDIA_STATE5_RESTORE_READ_PATH, 0u, 1l);
}

static void TestExternalCompletionCopiesBit0033(void)
{
    ResetFixture();
    memory_bytes[0x0e3fu] = 1u;
    memory_bytes[0x1302u] = 0xffu;
    memory_bytes[0x1301u] = 0xffu;
    defaults[MEDIA_STATE5_READ_BIT0033] = 0l;
    ProcessMediaCoroutineState5();

    assert(memory_bytes[0x0e3fu] == 0u);
    assert(memory_bytes[0x1302u] == 0xf7u);
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    assert(operation_counts[MEDIA_STATE5_DISPATCH_INDEXED_STATE] == 0u);
    assert(operation_counts[MEDIA_STATE5_RESTORE_READ_PATH] == 1u);
}

static void TestControllerErrorPublication(void)
{
    ResetFixture();
    memory_bytes[0xfb94u] = 9u;
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0xf977u] = 3u;
    memory_bytes[0x1301u] = 0xffu;
    ProcessMediaCoroutineState5();

    assert(memory_bytes[0xfab0u] == 1u);
    assert(memory_bytes[0xfab1u] == 0u);
    assert(memory_bytes[0xfab2u] == 8u);
    assert(memory_bytes[0xfb95u] == 0xffu);
    assert(memory_bytes[0x0e41u] == 9u);
    assert(memory_bytes[0x0fafu] == 0u);
    assert(operation_counts[MEDIA_STATE5_RESET_PORTS_80C4_80C5] == 1u);
    assert(operation_counts[MEDIA_STATE5_RESTORE_READ_PATH] == 1u);
}

static void TestPassiveDiscontinuityBecomesErrorSeven(void)
{
    ResetFixture();
    memory_bytes[0xf978u] = 2u;
    ProcessMediaCoroutineState5();
    assert(memory_bytes[0xfb94u] == 7u);
    assert(memory_bytes[0x0e41u] == 7u);
    assert((memory_bytes[0x11efu] & 4u) != 0u);
}

static void TestControllerUpdateLoopRetainsPreviousPosition(void)
{
    ResetFixture();
    SetResult(MEDIA_STATE5_DISPATCH_INDEXED_STATE, 0u, 0l);
    SetResult(MEDIA_STATE5_DISPATCH_INDEXED_STATE, 1u, 1l);
    SetResult(MEDIA_STATE5_UPDATE_POSITION_TRACKING, 0u, 111l);
    SetResult(MEDIA_STATE5_UPDATE_POSITION_TRACKING, 1u, 222l);
    ProcessMediaCoroutineState5();
    assert(operation_counts[MEDIA_STATE5_SERVICE_CONTROLLER_UPDATES] == 1u);
    AssertEventArgument(MEDIA_STATE5_UPDATE_POSITION_TRACKING, 0u, 0l);
    AssertEventArgument(MEDIA_STATE5_UPDATE_POSITION_TRACKING, 1u, 111l);
}

int main(void)
{
    TestAcquisitionRetryRestoresTarget();
    TestRetryLimitRoutes();
    TestSuccessfulIndexedCompletion();
    TestExternalCompletionCopiesBit0033();
    TestControllerErrorPublication();
    TestPassiveDiscontinuityBecomesErrorSeven();
    TestControllerUpdateLoopRetainsPreviousPosition();
    puts("media coroutine state-5 tests passed");
    return 0;
}
