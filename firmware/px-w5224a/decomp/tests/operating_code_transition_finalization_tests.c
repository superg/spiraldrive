#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/operating_code_transition_finalization.h"

struct PortWrite
{
    fw_addr_t address;
    u8 value;
};

static u8 memory[0x20000ul];
static u8 profile_table[32];
static struct PortWrite port_writes[16];
static unsigned port_write_count;
static unsigned synchronize_count;
static unsigned apply_count;
static u8 apply_direction;
static unsigned wait_count;
static u8 wait_advance;
static u8 wait_lower_only;
static unsigned map_count;
static u8 map_argument;
static u8 map_result;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= 0xff84feul && address < 0xff851eul)
        return profile_table[address - 0xff84feul];
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

void ObserveOperatingCodeTransitionSynchronize(void)
{
    ++synchronize_count;
}

void ObserveOperatingCodeTransitionWritePort(fw_addr_t address, u8 value)
{
    assert(port_write_count < sizeof(port_writes) / sizeof(port_writes[0]));
    port_writes[port_write_count].address = address;
    port_writes[port_write_count].value = value;
    ++port_write_count;
}

void ObserveOperatingCodeTransitionApplyProfile(u8 direction)
{
    ++apply_count;
    apply_direction = direction;
}

void ObserveOperatingCodeTransitionWait(u8 wait_for_initial_advance, u8 lower_bound_only)
{
    ++wait_count;
    wait_advance = wait_for_initial_advance;
    wait_lower_only = lower_bound_only;
}

u8 ObserveOperatingCodeTransitionMapProfileIndex(u8 operating_code)
{
    ++map_count;
    map_argument = operating_code;
    return map_result;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(u8 applied_code, u8 current_code)
{
    memset(memory, 0, sizeof(memory));
    memset(profile_table, 0, sizeof(profile_table));
    memset(port_writes, 0, sizeof(port_writes));
    memory[0x0e4du] = applied_code;
    memory[0x0e43u] = current_code;
    memory[0x0e4cu] = 0x5au;
    memory[0x125au] = 0xffu;
    memory[0x1306u] = 0xffu;
    port_write_count = 0u;
    synchronize_count = 0u;
    apply_count = 0u;
    apply_direction = 0u;
    wait_count = 0u;
    wait_advance = 0u;
    wait_lower_only = 0xffu;
    map_count = 0u;
    map_argument = 0u;
    map_result = 0u;
}

static void AssertCommonFinalState(u8 current_code)
{
    assert(memory[0x0e4du] == current_code);
    assert(memory[0x0e4eu] == 0x5au);
    assert(memory[0x1306u] == 0xefu);
}

static void TestUnchangedCodeOnlyPublishesFinalState(void)
{
    ResetFixture(8u, 8u);

    FinalizeControllerOperatingCodeTransition(77);

    assert(synchronize_count == 1u);
    assert(apply_count == 0u && wait_count == 0u && map_count == 0u);
    assert(port_write_count == 0u);
    assert(memory[0x125au] == 0xffu);
    AssertCommonFinalState(8u);
}

static void TestLowCodeTransitionUsesShortCountdown(void)
{
    ResetFixture(2u, 4u);

    FinalizeControllerOperatingCodeTransition(37);

    assert(synchronize_count == 2u);
    assert(ReadU32(0x125eu) == 37ul);
    assert(ReadU16(0x1200u) == 80u);
    assert(apply_count == 1u && apply_direction == 1u);
    assert(wait_count == 1u && wait_advance == 1u && wait_lower_only == 0u);
    assert(map_count == 0u && port_write_count == 0u);
    assert(memory[0x125au] == 0xfeu);
    AssertCommonFinalState(4u);
}

static void TestMidCodeTransitionUsesFallbackAndMediumCountdown(void)
{
    ResetFixture(8u, 12u);

    FinalizeControllerOperatingCodeTransition(-1);

    assert(ReadU32(0x125eu) == 150ul);
    assert(ReadU16(0x1200u) == 500u);
    assert(port_write_count == 0u && map_count == 0u);
    AssertCommonFinalState(12u);
}

static void TestHighCodeTransitionProgramsBothProfilePhases(void)
{
    static const u8 expected_values[] = { 8u, 0u, 4u, 1u, 0u, 8u, 0x34u, 4u, 0x56u, 0u };
    static const fw_addr_t expected_addresses[] = { 0x0180caul, 0x0180cbul, 0x0180caul, 0x0180cbul, 0x0180caul, 0x0180caul, 0x0180cbul, 0x0180caul, 0x0180cbul, 0x0180caul };
    unsigned index;

    ResetFixture(16u, 24u);
    memory[0x1006u] = 1u;
    map_result = 3u;
    profile_table[6] = 0x34u;
    profile_table[7] = 0x56u;

    FinalizeControllerOperatingCodeTransition(0);

    assert(ReadU32(0x125eu) == 150ul);
    assert(ReadU16(0x1200u) == 1000u);
    assert(map_count == 1u && map_argument == 24u);
    assert(port_write_count == 10u);
    for(index = 0u; index < port_write_count; ++index)
    {
        assert(port_writes[index].address == expected_addresses[index]);
        assert(port_writes[index].value == expected_values[index]);
    }
    AssertCommonFinalState(24u);
}

static void TestHighCodeDisabledSkipsPortProgramming(void)
{
    ResetFixture(12u, 32u);
    memory[0x1006u] = 2u;

    FinalizeControllerOperatingCodeTransition(9);

    assert(ReadU16(0x1200u) == 1000u);
    assert(map_count == 0u && port_write_count == 0u);
    AssertCommonFinalState(32u);
}

int main(void)
{
    TestUnchangedCodeOnlyPublishesFinalState();
    TestLowCodeTransitionUsesShortCountdown();
    TestMidCodeTransitionUsesFallbackAndMediumCountdown();
    TestHighCodeTransitionProgramsBothProfilePhases();
    TestHighCodeDisabledSkipsPortProgramming();
    puts("operating-code transition finalization tests passed");
    return 0;
}
