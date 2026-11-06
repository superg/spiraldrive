#include <assert.h>
#include <string.h>
#include "platform/controller_buffer_setup.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 bits[256];
static unsigned current_loads;
static unsigned lookahead_loads;
static unsigned increment_msf_calls;
static unsigned bcd_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveBufferSetupReadBit(u8 bit_address)
{
    return bits[bit_address];
}

void ObserveBufferSetupWriteBit(u8 bit_address, u8 value)
{
    bits[bit_address] = (u8)(value != 0u);
}

void ObserveBufferSetupLoadCurrentDescriptor(void)
{
    current_loads++;
}

void ObserveBufferSetupLoadLookaheadDescriptor(void)
{
    lookahead_loads++;
}

void ObserveBufferSetupIncrementMsf(u16 address)
{
    assert(address == 0x1529u);
    increment_msf_calls++;
    memory[0x001529u] = 12u;
    memory[0x00152au] = 34u;
    memory[0x00152bu] = 56u;
}

u8 ObserveBufferSetupBinaryToBcd(u8 value)
{
    bcd_calls++;
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 0, sizeof(bits));
    current_loads = 0u;
    lookahead_loads = 0u;
    increment_msf_calls = 0u;
    bcd_calls = 0u;
    WriteU16(0x001292u, 10u);
    WriteU16(0x001294u, 2u);
    WriteU16(0x00129au, 7u);
    WriteU32(0x00156au, 8u);
    WriteU32(0x00fbaau, 0x00123456ul);
    WriteU32(0x00fbb0u, 0x10203040ul);
    WriteU32(0x0014bcu, 0xfffffffful);
    memory[0x00fbb6u] = 0x5au;
    memory[0x000efeu] = 0x10u;
    memory[0x000effu] = 0xffu;
}

static void AssertConsumed(void)
{
    assert(ReadU16(0x001294u) == 3u);
    assert(ReadU16(0x001292u) == 9u);
    assert(current_loads == 1u);
}

static void TestInitialModes(void)
{
    ResetState();
    memory[0x00fb9eu] = 1u;
    WriteU16(0x00fbaeu, 0x0123u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 3u);
    assert(memory[0x000efbu] == 2u && memory[0x000efcu] == 0x47u);
    assert(memory[0x018007u] == 0x56u && memory[0x018008u] == 0x34u && memory[0x018009u] == 0x12u);
    assert(memory[0x01801cu] == 0x23u && memory[0x01801du] == 1u);
    assert(memory[0x00fbc2u] == 0x5au);
    assert(ReadU32(0x00fbbeu) == 0x10203040ul);
    assert(memory[0x018035u] == 0x10u && memory[0x00fbd6u] == 1u);
    AssertConsumed();

    ResetState();
    memory[0x00fb9eu] = 2u;
    WriteU16(0x00fbaeu, 0x2345u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 11u);
    assert(memory[0x000efeu] == 0xf4u && memory[0x018018u] == 0xf4u);
    assert(memory[0x01801cu] == 0x45u && memory[0x01801du] == 0x23u);
    assert(current_loads == 0u);

    ResetState();
    memory[0x00fb9eu] = 3u;
    memory[0x00fb9du] = 3u;
    WriteU16(0x00fbaeu, 7u);
    ServiceControllerBufferSetupStateMachine();
    assert(ReadU16(0x00fbaeu) == 1u);
    assert(memory[0x01801cu] == 1u && memory[0x01801du] == 0u);
    assert(memory[0x018034u] == 8u && memory[0x00fba1u] == 5u);
    AssertConsumed();

    ResetState();
    memory[0x00fb9eu] = 13u;
    memory[0x00fbb7u] = 1u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x000efbu] == 0x12u && memory[0x000efcu] == 0x43u);
    assert(memory[0x01800au] == 0x62u && memory[0x01800bu] == 0x34u && memory[0x01800cu] == 0x12u);
    assert(memory[0x00fba1u] == 6u && current_loads == 0u);

    ResetState();
    memory[0x00fb9eu] = 14u;
    WriteU16(0x00fbaeu, 0x5678u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 16u);
    assert(memory[0x000efeu] == 0x94u && memory[0x000effu] == 0xffu);
    assert(memory[0x018018u] == 0x94u && memory[0x018019u] == 0xffu);
    assert(memory[0x01801cu] == 0x78u && memory[0x01801du] == 0x56u);
    AssertConsumed();
}

static void TestActiveModeRoutesAndLookahead(void)
{
    ResetState();
    memory[0x00fba1u] = 1u;
    memory[0x00fb9eu] = 8u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 10u);
    assert(memory[0x000efbu] == 0x52u && memory[0x000efcu] == 0x40u);
    assert(memory[0x01800au] == 0x62u);
    assert(bits[0x2bu] == 1u && lookahead_loads == 1u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 3u;
    memory[0x00fb9eu] = 9u;
    memory[0x00fbbau] = 2u;
    WriteU16(0x00fbbbu, 0x1234u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 11u);
    assert(memory[0x01801cu] == 0x34u && memory[0x01801du] == 0x12u);
    assert(memory[0x018018u] == 0xf4u && lookahead_loads == 1u);

    ResetState();
    memory[0x00fba1u] = 5u;
    memory[0x00fb9eu] = 1u;
    memory[0x00fbbau] = 2u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 15u);

    ResetState();
    memory[0x00fba1u] = 10u;
    memory[0x00fb9eu] = 12u;
    memory[0x00fbbau] = 2u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x000efbu] == 0x12u && memory[0x000efcu] == 0x43u);
    assert(memory[0x00fba1u] == 13u);
    assert(memory[0x01800au] == 0u && memory[0x01800bu] == 0u && memory[0x01800cu] == 0u);

    ResetState();
    memory[0x00fba1u] = 14u;
    memory[0x00fb9eu] = 3u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 4u && lookahead_loads == 0u);
}

static void TestCompletionRoutes(void)
{
    ResetState();
    memory[0x00fba1u] = 3u;
    memory[0x00fb9eu] = 2u;
    bits[0x28u] = 1u;
    bits[0x30u] = 1u;
    WriteU16(0x00fbb4u, 2u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fbd6u] == 12u && memory[0x00fba1u] == 15u);
    assert(bits[0x32u] == 1u && memory[0x018035u] == 0u);

    ResetState();
    memory[0x00fba1u] = 10u;
    memory[0x0014c0u] = 1u;
    bits[0x30u] = bits[0x32u] = bits[0x33u] = 1u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fbd6u] == 10u && memory[0x00fba1u] == 15u);
    assert(bits[0x30u] == 0u && bits[0x32u] == 0u && bits[0x33u] == 0u);
    assert(lookahead_loads == 0u);

    ResetState();
    memory[0x00fba1u] = 16u;
    WriteU32(0x00fba2u, 0x01020304ul);
    WriteU32(0x0014bcu, 0x01020304ul);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x0014c0u] == 1u);
    assert(ReadU32(0x0014deu) == 0x01020304ul);
    assert(lookahead_loads == 0u);
}

static void TestIntermediateAndShutdownPhases(void)
{
    ResetState();
    memory[0x00fba1u] = 2u;
    WriteU16(0x00fbaeu, 0x4567u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 3u);
    assert(memory[0x01801cu] == 0x67u && memory[0x01801du] == 0x45u);
    assert(memory[0x000effu] == 0xefu && memory[0x018019u] == 0xefu);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 4u;
    WriteU16(0x00fbaeu, 7u);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 5u && ReadU16(0x00fbaeu) == 1u);
    assert(memory[0x01801cu] == 1u && memory[0x018018u] == 0x94u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 6u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 10u && memory[0x00fbd6u] == 1u);
    assert(memory[0x018035u] == 0x10u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 11u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 9u && current_loads == 0u);

    ResetState();
    memory[0x00fba1u] = 9u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 14u);
    assert(memory[0x018007u] == 0x56u && memory[0x018009u] == 0x12u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 13u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 14u && memory[0x000efbu] == 2u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 12u;
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 14u);
    AssertConsumed();

    ResetState();
    memory[0x00fba1u] = 16u;
    memory[0x00fb9eu] = 14u;
    memory[0x00fbbau] = 2u;
    WriteU16(0x00fbbbu, 0x89abu);
    ServiceControllerBufferSetupStateMachine();
    assert(memory[0x00fba1u] == 10u && lookahead_loads == 1u);
    assert(memory[0x01801cu] == 0xabu && memory[0x01801du] == 0x89u);

    ResetState();
    memory[0x00fba1u] = 15u;
    bits[0x2bu] = bits[0x32u] = bits[0x43u] = 1u;
    WriteU32(0x001235u, 100ul);
    ServiceControllerBufferSetupStateMachine();
    assert(bits[0x2bu] == 0u && bits[0x43u] == 0u);
    assert(memory[0x000efbu] == 2u && memory[0x000efcu] == 0u);
    assert(ReadU32(0x0014deu) == 98ul);
    assert(current_loads == 0u && lookahead_loads == 0u);
}

static void TestEntryAccounting(void)
{
    ResetState();
    memory[0x00fba1u] = 8u;
    bits[0x2bu] = 1u;
    WriteU32(0x00150bu, 0xfffffffful);
    WriteU16(0x00fbc7u, 0xffffu);
    ServiceControllerBufferSetupStateMachine();
    assert(ReadU32(0x00150bu) == 0u);
    assert(ReadU16(0x00fbc7u) == 0u);
    assert(memory[0x00154cu] == 8u);

    ResetState();
    memory[0x000c46u] = 1u;
    WriteU32(0x00150bu, 5u);
    ServiceControllerBufferSetupStateMachine();
    assert(ReadU32(0x00150bu) == 5u);
}

static void TestAdvanceSetupSelector(void)
{
    ResetState();
    WriteU32(0x001525u, 0x00abcdeful);
    AdvanceControllerBufferSetupPhase();
    assert(memory[0x000efbu] == 0x52u && memory[0x000efcu] == 0x40u);
    assert(memory[0x01800au] == 0xefu && memory[0x01800bu] == 0xcdu && memory[0x01800cu] == 0xabu);
    assert(memory[0x001531u] == 1u);

    ResetState();
    memory[0x001531u] = 1u;
    memory[0x01802bu] = 0x11u;
    memory[0x01802cu] = 0x22u;
    memory[0x01802du] = 0x33u;
    AdvanceControllerBufferSetupPhase();
    assert(memory[0x001531u] == 2u);
    assert(memory[0x018035u] == 0x10u && memory[0x00fbd6u] == 1u);
    assert(memory[0x018036u] == 0x41u && memory[0x018037u] == 1u && memory[0x018038u] == 1u);
    assert(memory[0x018039u] == 0u && memory[0x01803cu] == 0u);
    assert(memory[0x01803du] == 0x11u && memory[0x01803eu] == 0x22u && memory[0x01803fu] == 0x33u);
    AssertConsumed();

    ResetState();
    memory[0x001531u] = 2u;
    memory[0x01802bu] = 0xa1u;
    memory[0x01802cu] = 0xb2u;
    memory[0x01802du] = 0xc3u;
    AdvanceControllerBufferSetupPhase();
    assert(increment_msf_calls == 1u && bcd_calls == 3u);
    assert(memory[0x00152du] == 0x12u && memory[0x00152eu] == 0x34u && memory[0x00152fu] == 0x56u);
    assert(memory[0x018039u] == 0x12u && memory[0x01803au] == 0x34u && memory[0x01803bu] == 0x56u);
    assert(memory[0x01803du] == 0xa1u && memory[0x01803eu] == 0xb2u && memory[0x01803fu] == 0xc3u);
    AssertConsumed();

    ResetState();
    memory[0x001531u] = 3u;
    AdvanceControllerBufferSetupPhase();
    assert(current_loads == 0u && memory[0x001531u] == 3u);
}

int main(void)
{
    TestInitialModes();
    TestActiveModeRoutesAndLookahead();
    TestCompletionRoutes();
    TestIntermediateAndShutdownPhases();
    TestEntryAccounting();
    TestAdvanceSetupSelector();
    return 0;
}
