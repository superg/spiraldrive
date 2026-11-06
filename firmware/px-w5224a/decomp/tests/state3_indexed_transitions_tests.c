#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/state3_indexed_transitions.h"
#include "platform/firmware_memory.h"

#define EVENT_CAPACITY 32u

struct OperationEvent
{
    u8 operation;
    u32 argument;
};

static u8 memory_bytes[0x14000ul];
static u8 far_f7c01f;
static struct OperationEvent events[EVENT_CAPACITY];
static unsigned event_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if(address == 0x00f7c01ful)
    {
        far_f7c01f = value;
        return;
    }
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

void ObserveMediaState3IndexedTransitionOperation(u8 operation, u32 argument)
{
    assert(event_count < EVENT_CAPACITY);
    events[event_count].operation = operation;
    events[event_count].argument = argument;
    ++event_count;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(events, 0, sizeof(events));
    far_f7c01f = 0u;
    event_count = 0u;
}

static void AssertEvent(unsigned index, u8 operation, u32 argument)
{
    assert(index < event_count);
    assert(events[index].operation == operation);
    assert(events[index].argument == argument);
}

static void TestSimpleIndexedStates(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 0x0cu;
    assert(ServiceMediaState3IndexedDriveTransitions() == 0u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_DISPATCH, 0ul);

    ResetFixture();
    memory_bytes[0x130au] = 0x0eu;
    WriteDword(0x0fb4u, 10000ul);
    WriteDword(0x0fb0u, 2499ul);
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, 0x00f84b0bul);

    ResetFixture();
    memory_bytes[0x130au] = 0x0eu;
    WriteDword(0x0fb4u, 10000ul);
    WriteDword(0x0fb0u, 2500ul);
    ServiceMediaState3IndexedDriveTransitions();
    assert(event_count == 0u);

    ResetFixture();
    memory_bytes[0x130au] = 0x13u;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_ADVANCE, 0ul);
}

static void TestState1fRoutes(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0xf9e1u] = 1u;
    memory_bytes[0x1008u] = 1u;
    memory_bytes[0xfab0u] = 1u;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_SERVICE_COUNTDOWN, 0ul);
    AssertEvent(1u, MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, 0x00f84b0bul);

    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0xf9e1u] = 1u;
    memory_bytes[0x1008u] = 2u;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_SERVICE_CALIBRATION, 0ul);

    ResetFixture();
    memory_bytes[0x130au] = 0x1fu;
    memory_bytes[0x1008u] = 1u;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_INITIALIZE_RANGE, 0ul);
}

static void TestState6Routes(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 6u;
    memory_bytes[0xf9e1u] = 0xffu;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_INITIALIZE_RANGE, 0ul);

    ResetFixture();
    memory_bytes[0x130au] = 6u;
    memory_bytes[0xf9e1u] = 2u;
    ServiceMediaState3IndexedDriveTransitions();
    AssertEvent(0u, MEDIA_STATE3_INDEXED_SERVICE_COUNTDOWN, 0ul);

    ResetFixture();
    memory_bytes[0x130au] = 6u;
    memory_bytes[0x1302u] = 4u;
    ServiceMediaState3IndexedDriveTransitions();
    assert(event_count == 0u);
}

static void TestState4InactiveDrive(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 4u;
    WriteWord(0xfaacu, 0x0400u);
    WriteDword(0x1153u, 0x00056cabul);
    ServiceMediaState3IndexedDriveTransitions();
    assert(far_f7c01f == 0x11u);
    assert(event_count == 0u);

    WriteDword(0x1153u, 0x00056cacul);
    ServiceMediaState3IndexedDriveTransitions();
    assert(ReadWord(0xfaacu) == 0x0600u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, 0x00fbfbb9ul);
}

static void TestState4BeginsTransition(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 4u;
    memory_bytes[0x1304u] = 0x20u;
    memory_bytes[0x11dcu] = 0x80u;
    WriteWord(0x11dau, 1u);
    ServiceMediaState3IndexedDriveTransitions();

    assert((ReadWord(0xfaacu) & 0x0800u) != 0u);
    assert(memory_bytes[0x0e47u] == 0x28u);
    assert(memory_bytes[0x0e46u] == 0x28u);
    assert(memory_bytes[0x0e44u] == 1u);
    assert(memory_bytes[0x130au] == 9u);
    assert(memory_bytes[0x11dcu] == 0x81u);
    assert(ReadWord(0x11dau) == 0x03c0u);
    assert(memory_bytes[0x0fb8u] == 10u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_DISPATCH, 0ul);
}

static void TestState4CountdownAndCoroutineInstallation(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 4u;
    memory_bytes[0x1304u] = 0x20u;
    memory_bytes[0xfae9u] = 2u;
    WriteWord(0xfaacu, 0x0800u);
    ServiceMediaState3IndexedDriveTransitions();
    assert(memory_bytes[0xfae9u] == 1u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, 0x00f84c30ul);

    ResetFixture();
    memory_bytes[0x130au] = 4u;
    memory_bytes[0x1304u] = 0x20u;
    WriteWord(0xfaacu, 0x0800u);
    ServiceMediaState3IndexedDriveTransitions();
    assert(memory_bytes[0xfae9u] == 0xffu);
    assert((ReadWord(0xfaacu) & 0x0600u) == 0x0200u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, 0x00feb6bbul);

    ResetFixture();
    memory_bytes[0x130au] = 4u;
    memory_bytes[0x1304u] = 0x20u;
    memory_bytes[0xfae9u] = 0xffu;
    WriteWord(0xfaacu, 0x0c00u);
    ServiceMediaState3IndexedDriveTransitions();
    assert((ReadWord(0xfaacu) & 0x0600u) == 0x0600u);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_INITIALIZE_COROUTINE0, 0x00fbfbb9ul);
}

static void TestState9CompletionAndService(void)
{
    ResetFixture();
    memory_bytes[0x130au] = 9u;
    memory_bytes[0x11dcu] = 0xffu;
    WriteWord(0xfaacu, 0x0800u);
    WriteWord(0x11dau, 0u);
    WriteDword(0x0fb4u, 1234ul);
    ServiceMediaState3IndexedDriveTransitions();

    assert(memory_bytes[0x11dcu] == 0xfeu);
    assert(memory_bytes[0x130au] == 4u);
    assert(memory_bytes[0x0fafu] == 0u);
    assert(memory_bytes[0x0e47u] == 0x34u);
    assert(memory_bytes[0x0fb8u] == 10u);
    assert(ReadDword(0x0fb4u) == 0ul);
    AssertEvent(0u, MEDIA_STATE3_INDEXED_RESTART_COROUTINE1, 0x00f84c30ul);
    AssertEvent(1u, MEDIA_STATE3_INDEXED_DISPATCH, 0ul);
}

int main(void)
{
    TestSimpleIndexedStates();
    TestState1fRoutes();
    TestState6Routes();
    TestState4InactiveDrive();
    TestState4BeginsTransition();
    TestState4CountdownAndCoroutineInstallation();
    TestState9CompletionAndService();
    puts("media state-3 indexed transition tests passed");
    return 0;
}
