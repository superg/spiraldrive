#include <assert.h>
#include <string.h>
#include "platform/controller_threshold_sampling.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 sample;
static u8 indexed;
static unsigned read_count;
static unsigned write_count;
static fw_addr_t write_addresses[12];
static u8 write_values[12];
static unsigned interrupt_count;
static u8 interrupt_events[8];
static unsigned accumulate_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
void WaitControllerPortReadyAndWriteByte(fw_addr_t address, u8 value)
{
    write_addresses[write_count] = address;
    write_values[write_count++] = value;
}
u8 WaitControllerPortReadyAndReadByte(fw_addr_t address)
{
    assert(address == 0x0180c7ul);
    ++read_count;
    return sample;
}
u8 IsIndexedDriveStateActive(u8 state)
{
    assert(state == 4u);
    return indexed;
}
void AccumulateWriteSpeedFlagsFromPositionAndOperatingCode(void)
{
    ++accumulate_count;
}
void ObserveControllerThresholdSamplingInterrupt(u8 event)
{
    interrupt_events[interrupt_count++] = event;
}

static void WritePosition(s32 value)
{
    u32 bits = (u32)value;

    memory[0x001235ul] = (u8)(bits >> 24);
    memory[0x001236ul] = (u8)(bits >> 16);
    memory[0x001237ul] = (u8)(bits >> 8);
    memory[0x001238ul] = (u8)bits;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(write_addresses, 0, sizeof(write_addresses));
    memset(write_values, 0, sizeof(write_values));
    memset(interrupt_events, 0, sizeof(interrupt_events));
    sample = indexed = 0u;
    read_count = write_count = interrupt_count = accumulate_count = 0u;
    memory[0x000e43ul] = 0x20u;
    memory[0x00121cul] = 0u;
    memory[0x001008ul] = 1u;
    WritePosition(200000l);
}

static void AssertInterruptTail(void)
{
    assert(interrupt_count == 4u);
    assert(interrupt_events[0] == 0u && interrupt_events[1] == 1u && interrupt_events[2] == 2u && interrupt_events[3] == 3u);
    assert(memory[0x00121cul] == 10u);
}

static void TestSignedCountdownGate(void)
{
    Reset();
    memory[0x00121cul] = 1u;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert(interrupt_count == 0u && write_count == 0u);
    Reset();
    memory[0x00121cul] = 0xffu;
    memory[0x000fd0ul] = 0u;
    ServiceControllerOperatingThresholdSamplingCycle();
    AssertInterruptTail();
    assert(write_count == 0u);
}

static void TestPhaseZeroStartsCommand14(void)
{
    Reset();
    memory[0x000fd0ul] = 7u;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert((memory[0x000fd0ul] & 7u) == 1u);
    assert(write_count == 2u && write_values[0] == 3u && write_values[1] == 0x14u);
    AssertInterruptTail();
}

static void TestPhaseOneTracksMaximumAndStartsCommand16(void)
{
    Reset();
    memory[0x000fd0ul] = 1u;
    sample = 0x28u;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert(memory[0x000fceul] == 1u && memory[0x000fcbul] == 0x28u);
    assert(read_count == 1u && write_count == 5u);
    assert(write_values[3] == 3u && write_values[4] == 0x16u);
    AssertInterruptTail();
}

static void TestPhaseTwoLatchAccumulatesOrIsSuppressed(void)
{
    Reset();
    memory[0x000fd0ul] = 3u;
    memory[0x000fcful] = 2u;
    memory[0x000fad7ul] = 0x10u;
    sample = 0x50u;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert((memory[0x00fad6ul] & 2u) != 0u);
    assert(memory[0x000fcful] == 0u && memory[0x000fceul] == 0u);
    assert((memory[0x000fd0ul] & 8u) != 0u && accumulate_count == 1u);

    Reset();
    memory[0x000fd0ul] = 3u;
    memory[0x000fcful] = 2u;
    memory[0x000fad7ul] = 0x10u;
    sample = 0x50u;
    indexed = 1u;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert(accumulate_count == 0u && memory[0x000fcful] == 3u);
}

static void TestPhaseThreeCapturesStatus(void)
{
    Reset();
    memory[0x000fd0ul] = 5u;
    sample = 0x9au;
    ServiceControllerOperatingThresholdSamplingCycle();
    assert(memory[0x000fcdul] == 0x9au && read_count == 1u);
    assert(write_count == 3u && write_values[0] == 0u && write_values[1] == 0u && write_values[2] == 0u);
}

int main(void)
{
    TestSignedCountdownGate();
    TestPhaseZeroStartsCommand14();
    TestPhaseOneTracksMaximumAndStartsCommand16();
    TestPhaseTwoLatchAccumulatesOrIsSuppressed();
    TestPhaseThreeCapturesStatus();
    return 0;
}
