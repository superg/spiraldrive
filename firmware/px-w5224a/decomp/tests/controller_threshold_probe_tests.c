#include <assert.h>
#include <string.h>
#include "platform/controller_threshold_probe.h"
#include "platform/firmware_memory.h"

typedef struct
{
    fw_addr_t address;
    u8 value;
} PortWrite;

static u8 memory[0x1000000ul];
static u8 bit31;
static u8 indexed_state_result;
static u8 read_results[2];
static unsigned read_result_index;
static PortWrite port_writes[10];
static unsigned port_write_count;
static unsigned wait_count;
static unsigned indexed_state_count;
static unsigned accumulate_count;
static unsigned reduce_count;
static u8 reduce_argument;
static u8 clear_flags_on_accumulate;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObserveControllerThresholdProbeBit31(void)
{
    return bit31;
}
void WaitControllerPortReadyAndWriteByte(fw_addr_t address, u8 value)
{
    port_writes[port_write_count].address = address;
    port_writes[port_write_count].value = value;
    ++port_write_count;
}
u8 WaitControllerPortReadyAndReadByte(fw_addr_t address)
{
    assert(address == 0x0180c7ul);
    return read_results[read_result_index++];
}
void WaitTimer1ScaledDuration(u16 duration, u8 yield)
{
    assert(duration == 2000u && yield == 1u);
    ++wait_count;
}
u8 IsIndexedDriveStateActive(u8 state)
{
    assert(state == 4u);
    ++indexed_state_count;
    return indexed_state_result;
}
void AccumulateWriteSpeedFlagsFromPositionAndOperatingCode(void)
{
    ++accumulate_count;
    if(clear_flags_on_accumulate != 0u)
    {
        memory[0x000fad7ul] = 0u;
        memory[0x000fad8ul] = 0u;
    }
}
u8 TryReduceWriteSpeedForCurrentMediaState(u8 enabled)
{
    ++reduce_count;
    reduce_argument = enabled;
    return 0u;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(port_writes, 0, sizeof(port_writes));
    bit31 = indexed_state_result = 0u;
    read_results[0] = read_results[1] = 0u;
    read_result_index = port_write_count = wait_count = 0u;
    indexed_state_count = accumulate_count = reduce_count = 0u;
    reduce_argument = clear_flags_on_accumulate = 0u;
    memory[0x000fb9bul] = 1u;
    memory[0x000e43ul] = 0x14u;
    memory[0x001008ul] = 1u;
}

static void AssertPortWrite(unsigned index, fw_addr_t address, u8 value)
{
    assert(port_writes[index].address == address);
    assert(port_writes[index].value == value);
}

static void TestInitialGateSetsContinueState(void)
{
    Reset();
    memory[0x000c46ul] = 1u;
    memory[0x000fd0ul] = 0x81u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 1u);
    assert(memory[0x000fd0ul] == 0x91u);
    assert(port_write_count == 0u && read_result_index == 0u);

    Reset();
    bit31 = 1u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 1u);
    assert(memory[0x000fd0ul] == 0x10u && port_write_count == 0u);
}

static void TestFirstThresholdTriggersReduction(void)
{
    Reset();
    read_results[0] = 0x50u;
    memory[0x000fad8ul] = 3u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 0u);
    assert(memory[0x000f9a9ul] == 0x50u);
    assert(read_result_index == 1u && wait_count == 1u);
    assert(port_write_count == 5u);
    AssertPortWrite(0u, 0x0180c5ul, 3u);
    AssertPortWrite(1u, 0x0180c4ul, 0x14u);
    AssertPortWrite(2u, 0x0180c4ul, 0x40u);
    AssertPortWrite(3u, 0x0180c5ul, 0u);
    AssertPortWrite(4u, 0x0180c4ul, 0u);
    assert(indexed_state_count == 1u && accumulate_count == 1u);
    assert(reduce_count == 1u && reduce_argument == 1u);
}

static void TestSecondStrategyThresholdAndIndexedSuppression(void)
{
    Reset();
    memory[0x000fb5dul] = 0x37u;
    read_results[0] = 0x4fu;
    read_results[1] = 0x67u;
    memory[0x000fad7ul] = 0x10u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 1u);
    assert(memory[0x000f9a9ul] == 0x4fu && memory[0x000f9aaul] == 0x67u);
    assert(read_result_index == 2u && wait_count == 2u);
    assert(port_write_count == 8u && accumulate_count == 0u);

    Reset();
    read_results[0] = 0x4fu;
    read_results[1] = 0x50u;
    indexed_state_result = 1u;
    memory[0x000fad7ul] = 0x10u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 1u);
    assert(accumulate_count == 0u && reduce_count == 0u);
}

static void TestEligibilityIsRecomputedAfterAccumulation(void)
{
    Reset();
    read_results[0] = 0x50u;
    memory[0x000fad7ul] = 0x10u;
    clear_flags_on_accumulate = 1u;
    assert(ProbeControllerThresholdsAndMaybeReduceWriteSpeed() == 0u);
    assert(accumulate_count == 1u && reduce_count == 1u);
    assert(reduce_argument == 0u);
}

int main(void)
{
    TestInitialGateSetsContinueState();
    TestFirstThresholdTriggersReduction();
    TestSecondStrategyThresholdAndIndexedSuppression();
    TestEligibilityIsRecomputedAfterAccumulation();
    return 0;
}
