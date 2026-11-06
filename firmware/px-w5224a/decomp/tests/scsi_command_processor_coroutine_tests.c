#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/command_processor_coroutine.h"

struct Event
{
    u8 operation;
    u32 argument;
};

static u8 memory[1u << 24];
static struct Event events[64];
static unsigned event_count;
static u8 results[32];
static u8 cdb_bytes[12];
static unsigned cdb_index;
static jmp_buf terminal_jump;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    if(address == 0x018000ul)
    {
        assert(cdb_index < 12u);
        return cdb_bytes[cdb_index++];
    }
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void ObserveCommandProcessorCall(u8 operation, u32 argument)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].operation = operation;
    events[event_count].argument = argument;
    ++event_count;
    if(operation == 8u)
        longjmp(terminal_jump, 1);
}

u8 ObserveCommandProcessorResult(u8 operation)
{
    ObserveCommandProcessorCall(operation, 0ul);
    return results[operation];
}

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(results, 0, sizeof(results));
    event_count = cdb_index = 0u;
    for(index = 0u; index < 12u; ++index)
        cdb_bytes[index] = (u8)(0x20u + index);
    memory[0x018043u] = 1u;
}

static void RunToYield(void)
{
    if(setjmp(terminal_jump) == 0)
    {
        ScsiCommandProcessorCoroutine0();
        assert(0 && "non-returning coroutine returned");
    }
}

static int FindEvent(u8 operation)
{
    unsigned index;

    for(index = 0u; index < event_count; ++index)
        if(events[index].operation == operation)
            return (int)index;
    return -1;
}

static void TestColdInitialization(void)
{
    ResetFixture();
    memory[0x0c83u] = memory[0x0c84u] = memory[0x0c85u] = 0xffu;
    RunToYield();
    assert(memory[0x018041u] == 0u);
    assert(memory[0x0c32u] == 1u);
    assert(memory[0x0c83u] == 0xdcu);
    assert(memory[0x0c84u] == 0x37u);
    assert(memory[0x0c85u] == 0xf2u);
    assert(memory[0x0c2au] == 7u && memory[0x0c33u] == 5u);
    assert(memory[0x018059u] == 0x97u);
    assert(FindEvent(1u) >= 0 && FindEvent(2u) >= 0);
    assert(FindEvent(3u) >= 0 && FindEvent(4u) >= 0);
}

static void TestPreparedControllerAndStartupHandoff(void)
{
    ResetFixture();
    memory[0x0c88u] = 0x20u;
    memory[0xfaefu] = 0x80u;
    RunToYield();
    assert(FindEvent(5u) == 0);
    assert(FindEvent(6u) >= 0 && FindEvent(7u) >= 0);
    assert(events[FindEvent(6u)].argument == 0xf84b0bul);
}

static void TestScsiPacketDispatchAndCompletion(void)
{
    unsigned index;

    ResetFixture();
    memory[0x0c88u] = 0x20u;
    memory[0x0c32u] = 0x11u;
    memory[0x0c78u] = 1u;
    results[18u] = 1u;
    RunToYield();
    assert(cdb_index == 12u);
    for(index = 0u; index < 12u; ++index)
        assert(memory[0x0b0au + index] == cdb_bytes[index]);
    assert(FindEvent(18u) >= 0);
    assert(FindEvent(19u) >= 0);
    assert(events[FindEvent(19u)].argument == cdb_bytes[0]);
    assert(FindEvent(21u) >= 0 && FindEvent(22u) >= 0);
}

static void TestAtaAndIdleMaintenance(void)
{
    ResetFixture();
    memory[0x0c88u] = 0x20u;
    memory[0x0c32u] = 0x10u;
    RunToYield();
    assert(FindEvent(16u) >= 0);
    assert(FindEvent(21u) >= 0 && FindEvent(22u) >= 0);

    ResetFixture();
    memory[0x0c88u] = 0x20u;
    memory[0x0c32u] = 1u;
    memory[0xfaefu] = 3u;
    memory[0x1008u] = 2u;
    memory[0x0c89u] = 0x88u;
    memory[0x122bu] = 0xffu;
    memory[0x122cu] = 0xffu;
    memory[0x2ba5afu] = 1u;
    RunToYield();
    assert(FindEvent(9u) >= 0);
    assert(FindEvent(13u) >= 0);
}

int main(void)
{
    TestColdInitialization();
    TestPreparedControllerAndStartupHandoff();
    TestScsiPacketDispatchAndCompletion();
    TestAtaAndIdleMaintenance();
    puts("scsi command processor coroutine tests passed");
    return 0;
}
