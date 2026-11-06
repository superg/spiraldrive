#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "media/drive_operation_result_coroutine.h"
#include "platform/firmware_memory.h"

enum TerminalEvent
{
    TERMINAL_YIELD = 1,
    TERMINAL_RESTART,
    TERMINAL_RECOVERY
};

struct ConfigurationCall
{
    fw_addr_t source;
    u16 offset;
    u16 count;
};

static u8 memory_bytes[0x20000ul];
static jmp_buf terminal_jump;
static u8 execute_mode;
static u8 execute_speed;
static unsigned execute_count;
static u8 indexed_result;
static u8 configuration_results[2];
static struct ConfigurationCall configuration_calls[2];
static unsigned configuration_count;
static unsigned state10_count;
static unsigned status_update_count;
static u8 pattern_values[2];
static unsigned pattern_count;
static unsigned yield_count;
static unsigned yield_before_terminal;
static u8 release_wait_on_first_yield;
static u32 coroutine_entry;

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

void ObserveDriveOperationResultExecute(u8 mode, u8 speed)
{
    ++execute_count;
    execute_mode = mode;
    execute_speed = speed;
}

u8 ObserveDriveOperationResultIndexed(u8 state)
{
    assert(state == 4u);
    return indexed_result;
}

u8 ObserveDriveOperationResultConfiguration(fw_addr_t source, u16 offset, u16 count)
{
    unsigned index = configuration_count++;

    assert(index < 2u);
    configuration_calls[index].source = source;
    configuration_calls[index].offset = offset;
    configuration_calls[index].count = count;
    return configuration_results[index];
}

void ObserveDriveOperationResultState10(void)
{
    ++state10_count;
}

void ObserveDriveOperationResultRestart(u32 entry_point)
{
    coroutine_entry = entry_point;
    longjmp(terminal_jump, TERMINAL_RESTART);
}

void ObserveDriveOperationResultRecovery(void)
{
    longjmp(terminal_jump, TERMINAL_RECOVERY);
}

void ObserveDriveOperationResultYield(void)
{
    ++yield_count;
    if(release_wait_on_first_yield != 0u && yield_count == 1u)
        memory_bytes[0x15c3u] = 1u;
    if(yield_count >= yield_before_terminal)
        longjmp(terminal_jump, TERMINAL_YIELD);
}

void ObserveDriveOperationResultStatusUpdate(void)
{
    ++status_update_count;
}

void ObserveDriveOperationResultPattern(u8 restart_pattern)
{
    assert(pattern_count < 2u);
    pattern_values[pattern_count++] = restart_pattern;
}

void ObserveDriveOperationResultInitialize(u32 entry_point)
{
    coroutine_entry = entry_point;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(configuration_calls, 0, sizeof(configuration_calls));
    memset(pattern_values, 0, sizeof(pattern_values));
    execute_mode = execute_speed = 0u;
    execute_count = configuration_count = state10_count = 0u;
    status_update_count = pattern_count = yield_count = 0u;
    indexed_result = release_wait_on_first_yield = 0u;
    configuration_results[0] = configuration_results[1] = 1u;
    yield_before_terminal = 1u;
    coroutine_entry = 0ul;
    memory_bytes[0x0c84u] = 0xffu;
    memory_bytes[0xfaacu] = 0xffu;
    memory_bytes[0xfaadu] = 0xffu;
    memory_bytes[0xfae9u] = 0xa5u;
}

static int RunUntilTerminalObserver(void)
{
    int terminal = setjmp(terminal_jump);

    if(terminal == 0)
    {
        RunDriveOperationResultCoroutine0();
        assert(0 && "non-returning coroutine returned");
    }
    return terminal;
}

static void TestStateDispatch(void)
{
    static const struct
    {
        u8 state;
        u8 mode;
        u8 speed;
    } cases[] = {
        { 1u,    0u, 0x18u },
        { 2u,    0u, 0x20u },
        { 3u,    0u, 0x28u },
        { 4u,    0u, 0x34u },
        { 6u,    3u, 0x30u },
        { 7u,    0u, 0x14u },
        { 0x11u, 4u, 2u    },
        { 0x14u, 0u, 0x10u },
        { 0x15u, 0u, 8u    },
        { 0x16u, 0u, 4u    },
        { 0x17u, 0u, 1u    },
        { 0x1fu, 3u, 0x10u },
        { 0xf2u, 0u, 0x0cu },
        { 0xfdu, 0u, 2u    },
        { 0xffu, 0u, 0x24u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetFixture();
        memory_bytes[0x130au] = cases[index].state;
        assert(RunUntilTerminalObserver() == TERMINAL_YIELD);
        assert(execute_count == 1u);
        assert(execute_mode == cases[index].mode);
        assert(execute_speed == cases[index].speed);
        assert(memory_bytes[0x0c84u] == 0xf7u);
        assert(memory_bytes[0xfaacu] == 0xfdu);
        assert(memory_bytes[0xfaadu] == 0xffu);
        assert(memory_bytes[0xfae9u] == 0u);
    }

    ResetFixture();
    memory_bytes[0x130au] = 0x80u;
    assert(RunUntilTerminalObserver() == TERMINAL_YIELD);
    assert(execute_count == 0u);
}

static void TestConfigurationFailureRoutes(void)
{
    ResetFixture();
    indexed_result = 2u;
    configuration_results[0] = 0u;
    assert(RunUntilTerminalObserver() == TERMINAL_RESTART);
    assert(configuration_count == 1u);
    assert(configuration_calls[0].source == 0xfb96ul);
    assert(configuration_calls[0].offset == 0x3cu);
    assert(configuration_calls[0].count == 2u);
    assert(state10_count == 1u && memory_bytes[0x15c4u] == 1u);
    assert(coroutine_entry == 0xf853cful);

    ResetFixture();
    indexed_result = 1u;
    memory_bytes[0xfad6u] = 3u;
    configuration_results[1] = 0u;
    assert(RunUntilTerminalObserver() == TERMINAL_RESTART);
    assert(configuration_count == 2u);
    assert(configuration_calls[1].source == 0xfad6ul);
    assert(configuration_calls[1].offset == 0x3eu);
    assert(configuration_calls[1].count == 1u);
    assert(memory_bytes[0xfad6u] == 0xf3u);
}

static void TestZeroResultRecovery(void)
{
    ResetFixture();
    memory_bytes[0xfbf1u] = 1u;
    assert(RunUntilTerminalObserver() == TERMINAL_RECOVERY);
    assert(memory_bytes[0xfbf1u] == 2u);
    assert(memory_bytes[0xfae9u] == 0u);
    assert(memory_bytes[0x0c84u] == 0xffu);
}

static void TestRetainedResultRoute(void)
{
    ResetFixture();
    memory_bytes[0xfba0u] = 5u;
    memory_bytes[0xfbf1u] = 1u;
    release_wait_on_first_yield = 1u;
    yield_before_terminal = 2u;
    assert(RunUntilTerminalObserver() == TERMINAL_YIELD);
    assert(memory_bytes[0x1009u] == 5u);
    assert(memory_bytes[0x0e3fu] == 1u);
    assert(memory_bytes[0x0fafu] == 0u);
    assert(memory_bytes[0xfba0u] == 0u);
    assert(memory_bytes[0xfae7u] == 0x35u);
    assert(coroutine_entry == 0xf853cful);
    assert(status_update_count == 0u && pattern_count == 0u);
}

static void TestOrdinaryResultPatternRoute(void)
{
    ResetFixture();
    memory_bytes[0xfba0u] = 7u;
    release_wait_on_first_yield = 1u;
    yield_before_terminal = 2u;
    assert(RunUntilTerminalObserver() == TERMINAL_YIELD);
    assert(memory_bytes[0x1009u] == 7u);
    assert(memory_bytes[0xfba0u] == 0u);
    assert(status_update_count == 1u);
    assert(pattern_count == 2u);
    assert(pattern_values[0] == 1u && pattern_values[1] == 0u);
}

int main(void)
{
    TestStateDispatch();
    TestConfigurationFailureRoutes();
    TestZeroResultRecovery();
    TestRetainedResultRoute();
    TestOrdinaryResultPatternRoute();
    puts("drive operation result coroutine tests passed");
    return 0;
}
