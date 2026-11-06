#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/indexed_state0f_coroutine.h"

static u8 memory[1u << 24];
static jmp_buf terminal_jump;
static u8 yield_count;
static u8 ready_on_yield;
static u8 execute_result;
static u8 validate_result;
static u8 timed_loop_count;
static u8 yield_loop_count;
static u8 reset_track_index;
static u8 load_track_number;
static u32 coroutine_entries[2];
static u8 coroutine_entry_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void ObserveIndexedState0FYield(void)
{
    yield_count++;
    if(ready_on_yield != 0u && yield_count == ready_on_yield)
    {
        memory[0x000faeu] = 0u;
        memory[0x000c85u] |= 2u;
    }
}

void ObserveIndexedState0FInitializeCoroutine1(u32 entry_point)
{
    assert(coroutine_entry_count < 2u);
    coroutine_entries[coroutine_entry_count++] = entry_point;
}

void ObserveIndexedState0FResetTrackRecords(u8 first_index)
{
    reset_track_index = first_index;
}

void ObserveIndexedState0FLoadTrack(u8 track_number)
{
    load_track_number = track_number;
}

u8 ObserveIndexedState0FExecute(void)
{
    return execute_result;
}

u8 ObserveIndexedState0FValidatePayloads(void)
{
    return validate_result;
}

void ObserveIndexedState0FTimedLoop(void)
{
    timed_loop_count++;
    longjmp(terminal_jump, 1);
}

void ObserveIndexedState0FYieldLoop(void)
{
    yield_loop_count++;
    longjmp(terminal_jump, 2);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    yield_count = 0u;
    ready_on_yield = 0u;
    execute_result = 0u;
    validate_result = 0u;
    timed_loop_count = 0u;
    yield_loop_count = 0u;
    reset_track_index = 0u;
    load_track_number = 0u;
    memset(coroutine_entries, 0, sizeof(coroutine_entries));
    coroutine_entry_count = 0u;
    memory[0x000c85u] = 2u;
    memory[0x000c84u] = 0xffu;
    memory[0x000c46u] = 0x80u;
    memory[0x00f1b7u] = 0xaau;
    WriteU32(0x00114fu, 0x12345678ul);
}

static int RunCoroutine(void)
{
    int terminal = setjmp(terminal_jump);
    if(terminal == 0)
    {
        RunIndexedState0FOperationCoroutine0();
        assert(0);
    }
    return terminal;
}

static void AssertTimedFailure(u8 status)
{
    assert(timed_loop_count == 1u);
    assert(yield_loop_count == 0u);
    assert(memory[0x00f1b8u] == status);
    assert(memory[0x00f1b7u] == 0xa7u);
}

static void TestUnsupportedStrategy(void)
{
    ResetState();
    memory[0x00fb5du] = 0x22u;

    assert(RunCoroutine() == 1);

    assert(yield_count == 3u);
    AssertTimedFailure(1u);
    assert(memory[0x000c32u] == 0u);
}

static void TestSpeedAndOperationFailure(u8 strategy, u8 expected_speed)
{
    ResetState();
    memory[0x00fb5du] = strategy;

    assert(RunCoroutine() == 1);

    assert(memory[0x000e47u] == expected_speed);
    assert(memory[0x000c32u] == 1u);
    assert(memory[0x000c44u] == 0u && memory[0x000c45u] == 0u);
    assert(memory[0x000c46u] == 0x82u);
    assert(memory[0x000c47u] == 5u && memory[0x000c48u] == 10u);
    assert(ReadU32(0x000c4bu) == 32u);
    assert(ReadU32(0x00e5fcu) == 180000u);
    assert(ReadU32(0x00e612u) == 0x12345678ul);
    assert(ReadU32(0x00e616u) == 0xfffffffful);
    assert(ReadU32(0x00e61au) == 219520u);
    assert(ReadU32(0x00e61eu) == 226270u);
    assert(memory[0x00dd62u] == 8u);
    assert(reset_track_index == 1u && load_track_number == 1u);
    assert(memory[0x00dd7eu] == 1u && memory[0x00dd7fu] == 1u);
    assert((memory[0x000c84u] & 8u) == 0u);
    AssertTimedFailure(4u);
}

static void TestOperationSuccessPayloadFailure(void)
{
    ResetState();
    memory[0x00fb5du] = 0x91u;
    execute_result = 1u;
    validate_result = 0u;
    memory[0x000faeu] = 1u;
    memory[0x000c85u] = 0u;
    ready_on_yield = 4u;

    assert(RunCoroutine() == 1);

    assert(yield_count == 7u);
    assert(coroutine_entry_count == 1u);
    assert(coroutine_entries[0] == 0xf84f6aul);
    AssertTimedFailure(6u);
}

static void TestFullSuccessEntersYieldLoop(void)
{
    ResetState();
    memory[0x00fb5du] = 0xd1u;
    execute_result = 1u;
    validate_result = 1u;

    assert(RunCoroutine() == 2);

    assert(yield_count == 6u);
    assert(coroutine_entry_count == 2u);
    assert(coroutine_entries[0] == 0xf84f6aul);
    assert(coroutine_entries[1] == 0xf84b0bul);
    assert(yield_loop_count == 1u);
    assert(timed_loop_count == 0u);
}

int main(void)
{
    TestUnsupportedStrategy();
    TestSpeedAndOperationFailure(0x91u, 4u);
    TestSpeedAndOperationFailure(0xb1u, 10u);
    TestSpeedAndOperationFailure(0xd1u, 24u);
    TestOperationSuccessPayloadFailure();
    TestFullSuccessEntersYieldLoop();
    return 0;
}
