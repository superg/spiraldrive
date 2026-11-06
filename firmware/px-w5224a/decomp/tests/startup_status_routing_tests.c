#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/status_routing.h"

static u8 memory[1u << 16];
static u8 events[32];
static u32 coroutine_entries[8];
static unsigned event_count;
static unsigned coroutine_count;
static u8 indexed_result;
static u8 indexed_argument;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[(u16)address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

void ObserveStartupStatusYieldCoroutine1(void)
{
    events[event_count++] = 1u;
}

void ObserveStartupStatusConfigurationMaintenance(void)
{
    events[event_count++] = 2u;
}

u8 ObserveStartupStatusIndexedStateActive(u8 state_code)
{
    indexed_argument = state_code;
    events[event_count++] = 3u;
    return indexed_result;
}

void ObserveStartupStatusSignalPattern(void)
{
    events[event_count++] = 4u;
}

void ObserveStartupStatusInitializeCoroutine0(u32 entry_point)
{
    coroutine_entries[coroutine_count++] = entry_point;
    events[event_count++] = 5u;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(coroutine_entries, 0, sizeof(coroutine_entries));
    event_count = 0u;
    coroutine_count = 0u;
    indexed_result = 0u;
    indexed_argument = 0u;
}

static void SetConfigurationWord(u16 value)
{
    memory[0xfaacu] = (u8)(value >> 8);
    memory[0xfaadu] = (u8)value;
}

static void TestPendingStatusMappings(void)
{
    static const struct
    {
        u8 pending;
        u8 mode;
        u8 sample;
        u16 configuration;
        u8 flags;
        u8 expected;
    } cases[] = {
        { 4u, 0u, 0u,    0x0100u, 0u,    0x25u },
        { 8u, 0u, 0u,    0x0100u, 0u,    0x26u },
        { 7u, 0u, 0u,    0x0100u, 0u,    0x49u },
        { 9u, 1u, 0xc0u, 0x0100u, 0u,    0x23u },
        { 9u, 1u, 0xbfu, 0x0104u, 0u,    0x24u },
        { 9u, 1u, 0xbfu, 0x0100u, 0u,    0x22u },
        { 9u, 0u, 0u,    0x0104u, 0u,    0x24u },
        { 9u, 0u, 0u,    0x0100u, 0x10u, 0x23u },
        { 9u, 0u, 0u,    0x0100u, 0u,    0x22u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetFixture();
        memory[0x1308u] = 1u;
        memory[0x130au] = 0x13u;
        memory[0xfab2u] = cases[index].pending;
        memory[0x1008u] = cases[index].mode;
        memory[0xf9b5u] = cases[index].sample;
        memory[0x103au] = cases[index].flags;
        memory[0xf976u] = 1u;
        SetConfigurationWord(cases[index].configuration);

        RouteStartupStatusAndScheduleCoroutine0();

        assert(memory[0x0faeu] == 6u);
        assert(memory[0xfae7u] == cases[index].expected);
        assert(memory[0xfab2u] == 0u);
        assert(event_count == 3u && events[0] == 1u && events[1] == 4u && events[2] == 5u);
        assert(coroutine_count == 1u && coroutine_entries[0] == 0x00feb6bbul);
    }
}

static void TestIndexedStateRouteNormalizesTransientState(void)
{
    ResetFixture();
    memory[0x1308u] = 1u;
    memory[0x130au] = 5u;
    memory[0xf972u] = 0u;
    memory[0xfab4u] = 0u;
    memory[0x11efu] = 0x08u;
    memory[0xfab0u] = 0xa5u;
    memory[0xfab2u] = 0x5au;
    memory[0xf973u] = 1u;
    memory[0xf974u] = 2u;
    memory[0xf975u] = 3u;
    memory[0x0fafu] = 4u;
    memory[0xe74eu] = 5u;
    indexed_result = 1u;
    SetConfigurationWord(0x0100u);

    RouteStartupStatusAndScheduleCoroutine0();

    assert(indexed_argument == 5u);
    assert(memory[0x130au] == 0x1fu);
    assert(memory[0xfab0u] == 0u && memory[0xfab2u] == 0u);
    assert(memory[0xf972u] == 0u && memory[0xf973u] == 0u && memory[0xf974u] == 0u && memory[0xf975u] == 0u);
    assert(memory[0x0fafu] == 0u && memory[0xe74eu] == 0u);
    assert(event_count == 2u && events[0] == 3u && events[1] == 5u);
    assert(coroutine_count == 1u && coroutine_entries[0] == 0x00feb6bbul);
}

static void TestStateSpecificCoroutineEntries(void)
{
    ResetFixture();
    memory[0x1308u] = 1u;
    memory[0x130au] = 0x0fu;
    memory[0xf976u] = 1u;
    SetConfigurationWord(0x0100u);
    RouteStartupStatusAndScheduleCoroutine0();
    assert(coroutine_count == 1u && coroutine_entries[0] == 0x00fb3ed6ul);

    ResetFixture();
    memory[0x1308u] = 1u;
    memory[0x130au] = 6u;
    memory[0xf976u] = 1u;
    memory[0x0fafu] = 0xa5u;
    memory[0xe74eu] = 0x5au;
    SetConfigurationWord(0x0100u);
    RouteStartupStatusAndScheduleCoroutine0();
    assert(memory[0x0fafu] == 0u && memory[0xe74eu] == 0u);
    assert(coroutine_count == 1u && coroutine_entries[0] == 0x00feb6bbul);
}

static void TestMaintenanceAndInactiveStartupGates(void)
{
    ResetFixture();
    RouteStartupStatusAndScheduleCoroutine0();
    assert(event_count == 1u && events[0] == 2u);
    assert(coroutine_count == 0u);

    ResetFixture();
    SetConfigurationWord(0x0100u);
    RouteStartupStatusAndScheduleCoroutine0();
    assert(event_count == 0u && coroutine_count == 0u);
}

int main(void)
{
    TestPendingStatusMappings();
    TestIndexedStateRouteNormalizesTransientState();
    TestStateSpecificCoroutineEntries();
    TestMaintenanceAndInactiveStartupGates();
    return 0;
}
