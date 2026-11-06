#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/startup_status_coroutine1.h"

static u8 memory_bytes[0x20000ul];
static jmp_buf terminal_jump;
static unsigned reset_count;
static unsigned write_d6_count;
static unsigned persist_count;
static unsigned apply_configuration_count;
static unsigned p1_write_count;
static u8 p1_bit0;
static u8 p1_bit1;
static unsigned p3_write_count;
static u8 p3_bit0;
static u8 p3_bit1;
static unsigned pattern_count;
static u8 pattern_values[8];
static unsigned yield_count;
static unsigned yield_limit;
static unsigned pattern_limit;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

void ObserveStartupStatusCoroutineResetDrive(void)
{
    ++reset_count;
}

void ObserveStartupStatusCoroutineWriteD6(void)
{
    ++write_d6_count;
}

void ObserveStartupStatusCoroutineWriteP1(u8 bit0, u8 bit1)
{
    ++p1_write_count;
    p1_bit0 = bit0;
    p1_bit1 = bit1;
}

void ObserveStartupStatusCoroutineWriteP3(u8 bit0, u8 bit1)
{
    ++p3_write_count;
    p3_bit0 = bit0;
    p3_bit1 = bit1;
}

void ObserveStartupStatusCoroutinePersist(void)
{
    ++persist_count;
}

void ObserveStartupStatusCoroutinePattern(u8 restart_pattern)
{
    assert(pattern_count < sizeof(pattern_values));
    pattern_values[pattern_count++] = restart_pattern;
    if(pattern_limit != 0u && pattern_count >= pattern_limit)
        longjmp(terminal_jump, 2);
}

void ObserveStartupStatusCoroutineApplyConfiguration(void)
{
    ++apply_configuration_count;
}

void ObserveStartupStatusCoroutineYield(void)
{
    ++yield_count;
    if(yield_limit != 0u && yield_count >= yield_limit)
        longjmp(terminal_jump, 1);
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(pattern_values, 0, sizeof(pattern_values));
    reset_count = write_d6_count = persist_count = 0u;
    apply_configuration_count = p1_write_count = p3_write_count = 0u;
    p1_bit0 = p1_bit1 = p3_bit0 = p3_bit1 = 1u;
    pattern_count = yield_count = yield_limit = pattern_limit = 0u;
    memory_bytes[0x1302u] = 0xffu;
    memory_bytes[0x1309u] = 0x55u;
}

static int RunUntilTerminalObserver(void)
{
    int terminal = setjmp(terminal_jump);

    if(terminal == 0)
    {
        RunStartupStatusCoroutine1();
        assert(0 && "non-returning coroutine returned");
    }
    return terminal;
}

static void AssertCommonInitialization(void)
{
    assert(memory_bytes[0x1309u] == 0u);
    assert(memory_bytes[0x1302u] == 0xfdu);
    assert(reset_count == 1u && write_d6_count == 1u);
    assert(p1_write_count == 1u && p1_bit0 == 0u && p1_bit1 == 0u);
}

static void TestPersistentStatusLoop(void)
{
    ResetFixture();
    memory_bytes[0x12f7u] = 1u;
    yield_limit = 1u;
    assert(RunUntilTerminalObserver() == 1);
    AssertCommonInitialization();
    assert(persist_count == 1u);
    assert(pattern_count == 2u && pattern_values[0] == 1u && pattern_values[1] == 0u);

    ResetFixture();
    memory_bytes[0x12f7u] = 2u;
    yield_limit = 1u;
    assert(RunUntilTerminalObserver() == 1);
    assert(persist_count == 0u && pattern_count == 2u);
}

static void TestRetainedMediaStatusMappings(void)
{
    ResetFixture();
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0xfae7u] = 0x39u;
    yield_limit = 1u;
    assert(RunUntilTerminalObserver() == 1);
    assert(memory_bytes[0x1009u] == 0x0du);
    assert(memory_bytes[0xfbf1u] == 1u);

    ResetFixture();
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0xfae7u] = 0x20u;
    memory_bytes[0x1308u] = 1u;
    yield_limit = 2u;
    assert(RunUntilTerminalObserver() == 1);
    assert(memory_bytes[0x1009u] == 2u);
    assert(memory_bytes[0xfbf1u] == 0u && memory_bytes[0x1308u] == 0u);

    ResetFixture();
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0xfae7u] = 0x20u;
    memory_bytes[0xc84u] = 0x88u;
    yield_limit = 2u;
    assert(RunUntilTerminalObserver() == 1);
    assert(memory_bytes[0x1009u] == 1u);
    assert(memory_bytes[0xc84u] == 0x80u);
}

static void TestPendingStatusMappings(void)
{
    static const u8 pending[] = { 1u, 2u, 3u };
    static const u8 expected[] = { 1u, 2u, 5u };
    unsigned index;

    for(index = 0u; index < sizeof(pending); ++index)
    {
        ResetFixture();
        memory_bytes[0xfaefu] = 1u;
        memory_bytes[0xfaf1u] = pending[index];
        memory_bytes[0x1009u] = 0x55u;
        yield_limit = expected[index] == 5u ? 1u : 2u;
        assert(RunUntilTerminalObserver() == 1);
        assert(memory_bytes[0x1009u] == expected[index]);
    }
}

static void TestStatusFourStateNormalization(void)
{
    ResetFixture();
    memory_bytes[0x1009u] = 4u;
    memory_bytes[0xfaedu] = 4u;
    memory_bytes[0xfaefu] = 5u;
    memory_bytes[0xfaf1u] = 0u;
    yield_limit = 1u;
    assert(RunUntilTerminalObserver() == 1);
    AssertCommonInitialization();
    assert(p3_write_count == 1u && p3_bit0 == 1u && p3_bit1 == 1u);
    assert(apply_configuration_count == 1u);
    assert(memory_bytes[0xfaedu] == 1u);
    assert(memory_bytes[0xfaefu] == 0u);
    assert(memory_bytes[0x1308u] == 1u);
    assert(pattern_count == 0u);
}

static void TestStatusLoopClasses(void)
{
    static const u8 false_then_yield[] = { 3u, 5u, 8u, 0x0bu, 0x0cu, 0x0du, 0x0eu };
    static const u8 tight[] = { 6u, 7u };
    static const u8 yield_then_false[] = { 0u, 1u, 2u, 9u, 10u, 15u };
    unsigned index;

    for(index = 0u; index < sizeof(false_then_yield); ++index)
    {
        ResetFixture();
        memory_bytes[0x1009u] = false_then_yield[index];
        yield_limit = 1u;
        assert(RunUntilTerminalObserver() == 1);
        assert(pattern_count == 2u && pattern_values[0] == 1u && pattern_values[1] == 0u && yield_count == 1u);
    }

    for(index = 0u; index < sizeof(tight); ++index)
    {
        ResetFixture();
        memory_bytes[0x1009u] = tight[index];
        pattern_limit = 2u;
        assert(RunUntilTerminalObserver() == 2);
        assert(pattern_values[0] == 1u && pattern_values[1] == 0u);
        assert(yield_count == 0u);
    }

    for(index = 0u; index < sizeof(yield_then_false); ++index)
    {
        ResetFixture();
        memory_bytes[0x1009u] = yield_then_false[index];
        yield_limit = 2u;
        assert(RunUntilTerminalObserver() == 1);
        assert(pattern_count == 2u && pattern_values[0] == 1u && pattern_values[1] == 0u && yield_count == 2u);
    }
}

int main(void)
{
    TestPersistentStatusLoop();
    TestRetainedMediaStatusMappings();
    TestPendingStatusMappings();
    TestStatusFourStateNormalization();
    TestStatusLoopClasses();
    puts("startup_status_coroutine1_tests: ok");
    return 0;
}
