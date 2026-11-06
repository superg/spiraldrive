#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/indexed_drive_state_transitions.h"

static u8 memory_bytes[0x20000ul];
static u8 far_f7c01f;
static u8 operation_in_toc_set;
static unsigned operation_query_count;
static unsigned reset_count;
static unsigned signal_count;
static unsigned replace_speed_count;
static u8 replaced_speed;
static unsigned persist_count;
static unsigned update_count;
static u8 updated_stage;
static unsigned restart_count;
static u32 restart_entry;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x00f7c01ful)
        return far_f7c01f;
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

u8 ObserveIndexedDriveStateOperationInTocSet(void)
{
    ++operation_query_count;
    return operation_in_toc_set;
}

void ObserveIndexedDriveStateReset(void)
{
    ++reset_count;
}

void ObserveIndexedDriveStateSignal(void)
{
    ++signal_count;
}

void ObserveIndexedDriveStateReplaceWriteSpeed(u8 value)
{
    ++replace_speed_count;
    replaced_speed = value;
}

void ObserveIndexedDriveStatePersistCalibration(void)
{
    ++persist_count;
}

void ObserveIndexedDriveStateUpdateCalibration(u8 stage)
{
    ++update_count;
    updated_stage = stage;
}

void ObserveIndexedDriveStateRestart(u32 entry_point)
{
    ++restart_count;
    restart_entry = entry_point;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    far_f7c01f = operation_in_toc_set = 0u;
    operation_query_count = reset_count = signal_count = 0u;
    replace_speed_count = persist_count = update_count = restart_count = 0u;
    replaced_speed = updated_stage = 0u;
    restart_entry = 0u;
    memory_bytes[0x1308u] = 1u;
}

static void AssertFatal(u8 reason)
{
    assert(memory_bytes[0x1009u] == 0x0du);
    assert(memory_bytes[0xfaeu] == 6u);
    assert(memory_bytes[0xfae7u] == reason);
    assert(reset_count == 1u && signal_count == 1u);
}

static void TestInactiveAndInitialRejection(void)
{
    ResetFixture();
    memory_bytes[0x1308u] = 0u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(operation_query_count == 0u);

    ResetFixture();
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0x1008u] = 2u;
    WriteWord(0xfaacu, 0x1237u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(operation_query_count == 1u);
    assert(memory_bytes[0x1009u] == 0x0du);
    assert(memory_bytes[0xfaeu] == 6u);
    assert(memory_bytes[0xfae7u] == 0x39u);
    assert(ReadWord(0xfaacu) == 0x1235u);
    assert(reset_count == 0u && signal_count == 0u);
}

static void PrepareTocRoute(void)
{
    operation_in_toc_set = 1u;
    memory_bytes[0x1008u] = 1u;
}

static void TestTocStrategyValidation(void)
{
    ResetFixture();
    PrepareTocRoute();
    far_f7c01f = 1u;
    memory_bytes[0xfae0u] = 2u;
    memory_bytes[0xfb5du] = 0x55u;
    ServiceActiveIndexedDriveStateTransitions();
    AssertFatal(0x3bu);

    ResetFixture();
    PrepareTocRoute();
    far_f7c01f = 1u;
    memory_bytes[0xfae0u] = 2u;
    memory_bytes[0xfb5du] = 0x3eu;
    WriteWord(0xfaacu, 3u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(signal_count == 0u && ReadWord(0xfaacu) == 1u);
}

static void TestTocConfigurationValidation(void)
{
    ResetFixture();
    PrepareTocRoute();
    far_f7c01f = 1u;
    memory_bytes[0xfc21u] = 4u;
    WriteWord(0xfaacu, 0x2802u);
    ServiceActiveIndexedDriveStateTransitions();
    AssertFatal(0x3du);

    ResetFixture();
    PrepareTocRoute();
    far_f7c01f = 1u;
    memory_bytes[0xfc21u] = 4u;
    WriteWord(0xfaacu, 0x0002u);
    ServiceActiveIndexedDriveStateTransitions();
    AssertFatal(0x3au);

    ResetFixture();
    PrepareTocRoute();
    far_f7c01f = 1u;
    memory_bytes[0xfc21u] = 4u;
    WriteWord(0xfaacu, 0x0803u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(replace_speed_count == 1u && replaced_speed == 0xffu);
    assert(ReadWord(0xfaacu) == 0x0801u);
    assert(signal_count == 0u);
}

static void TestTocProfileRejectionSignalGate(void)
{
    ResetFixture();
    PrepareTocRoute();
    memory_bytes[0x1132u] = 0x20u;
    WriteWord(0xfaacu, 2u);
    ServiceActiveIndexedDriveStateTransitions();
    AssertFatal(0x39u);

    ResetFixture();
    PrepareTocRoute();
    memory_bytes[0xfbf1u] = 1u;
    memory_bytes[0xfbecu] = 1u;
    memory_bytes[0x1132u] = 0x20u;
    WriteWord(0xfaacu, 2u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(memory_bytes[0x1009u] == 0x0du);
    assert(reset_count == 1u && signal_count == 0u);
    assert(ReadWord(0xfaacu) == 0u);
}

static void TestIndexedState1fRoutes(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0x1008u] = 1u;
    ServiceActiveIndexedDriveStateTransitions();
    AssertFatal(0x39u);

    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0xfb5du] = 0x90u;
    WriteWord(0xfaacu, 3u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(memory_bytes[0x1009u] == 0u && memory_bytes[0xfaeu] == 0u);
    assert(memory_bytes[0xfae7u] == 0x27u);
    assert(reset_count == 1u && signal_count == 1u);
    assert(ReadWord(0xfaacu) == 1u);

    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0xfb5du] = 0x91u;
    WriteWord(0xfaacu, 3u);
    ServiceActiveIndexedDriveStateTransitions();
    assert(reset_count == 0u && signal_count == 0u);
    assert(ReadWord(0xfaacu) == 1u);
}

static void TestStateFiveCalibrationPersistence(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 5u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(persist_count == 1u && memory_bytes[0xfab0u] == 1u);
    assert(update_count == 1u && updated_stage == 5u);

    ResetFixture();
    memory_bytes[0x130au] = 5u;
    memory_bytes[0x1008u] = 1u;
    memory_bytes[0x1301u] = 8u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(persist_count == 0u);

    ResetFixture();
    memory_bytes[0x130au] = 5u;
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0xfab2u] = 1u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(persist_count == 1u && update_count == 1u);

    ResetFixture();
    memory_bytes[0x130au] = 5u;
    memory_bytes[0x1009u] = 1u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(persist_count == 0u);
}

static void TestState12Restart(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 0x12u;
    ServiceActiveIndexedDriveStateTransitions();
    assert(restart_count == 1u && restart_entry == 0x00f859f0ul);
}

int main(void)
{
    TestInactiveAndInitialRejection();
    TestTocStrategyValidation();
    TestTocConfigurationValidation();
    TestTocProfileRejectionSignalGate();
    TestIndexedState1fRoutes();
    TestStateFiveCalibrationPersistence();
    TestState12Restart();
    puts("indexed_drive_state_transitions_tests: ok");
    return 0;
}
