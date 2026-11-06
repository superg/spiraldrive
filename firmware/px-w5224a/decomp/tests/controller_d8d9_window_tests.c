#include <assert.h>
#include <string.h>
#include "platform/controller_d8d9_window.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 16];
static u8 sfr[256];
static u8 d8_samples[10];
static u8 d9_samples[10];
static unsigned d8_index;
static unsigned d9_index;
static unsigned f1_read_count;
static u16 wait_durations[16];
static u8 wait_yields[16];
static unsigned wait_count;
static fw_addr_t sort_addresses[4];
static u16 sort_counts[4];
static unsigned sort_count;
static u8 bit_events[8];
static unsigned bit_event_count;
static u32 multiply_left;
static u32 multiply_right;

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
    if(address == 0xf1u)
    {
        ++f1_read_count;
        return 3u;
    }
    if(address == 0xd8u)
        return d8_samples[d8_index++];
    if(address == 0xd9u)
        return d9_samples[d9_index++];
    return sfr[address];
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr[address] = value;
}

void SetBit0719Atomic(void)
{
    bit_events[bit_event_count++] = 1u;
}

void ClearBit0719Atomic(void)
{
    bit_events[bit_event_count++] = 4u;
}

void SetBit070FAtomic(void)
{
    bit_events[bit_event_count++] = 2u;
}

void ClearBit070FAtomic(void)
{
    bit_events[bit_event_count++] = 3u;
}

void WaitTimer1ScaledDuration(u16 duration_units, u8 yield_while_waiting)
{
    wait_durations[wait_count] = duration_units;
    wait_yields[wait_count] = yield_while_waiting;
    ++wait_count;
}

void SortByteArrayAscendingInPlace(fw_addr_t address, u16 count)
{
    u16 outer;

    sort_addresses[sort_count] = address;
    sort_counts[sort_count] = count;
    ++sort_count;
    for(outer = 0u; outer < count; ++outer)
    {
        u16 inner;

        for(inner = 1u; inner < count; ++inner)
        {
            u16 left = (u16)(address + inner - 1u);
            u16 right = (u16)(address + inner);

            if(memory[right] < memory[left])
            {
                u8 temporary = memory[left];

                memory[left] = memory[right];
                memory[right] = temporary;
            }
        }
    }
}

u32 MultiplyU32Low(u32 left, u32 right)
{
    multiply_left = left;
    multiply_right = right;
    return left * right;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[(u16)address] = (u8)(value >> 8);
    memory[(u16)(address + 1u)] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[(u16)address] << 8) | memory[(u16)(address + 1u)]);
}

static void ResetFixture(void)
{
    static const u8 default_d8[10] = { 10u, 9u, 8u, 7u, 6u, 5u, 4u, 3u, 2u, 1u };
    static const u8 default_d9[10] = { 20u, 19u, 18u, 17u, 16u, 15u, 14u, 13u, 12u, 11u };

    memset(memory, 0, sizeof(memory));
    memset(sfr, 0, sizeof(sfr));
    memcpy(d8_samples, default_d8, sizeof(d8_samples));
    memcpy(d9_samples, default_d9, sizeof(d9_samples));
    memset(wait_durations, 0, sizeof(wait_durations));
    memset(wait_yields, 0, sizeof(wait_yields));
    memset(sort_addresses, 0, sizeof(sort_addresses));
    memset(sort_counts, 0, sizeof(sort_counts));
    memset(bit_events, 0, sizeof(bit_events));
    d8_index = 0u;
    d9_index = 0u;
    f1_read_count = 0u;
    wait_count = 0u;
    sort_count = 0u;
    bit_event_count = 0u;
    multiply_left = 0u;
    multiply_right = 0u;
}

static void AssertCaptureProtocol(u16 initial_delay, u16 sample_interval)
{
    unsigned index;

    assert(wait_count == 10u);
    assert(wait_durations[0] == initial_delay && wait_yields[0] == 0u);
    for(index = 1u; index < 10u; ++index)
    {
        assert(wait_durations[index] == sample_interval);
        assert(wait_yields[index] == 0u);
    }
    assert(d8_index == 10u && d9_index == 10u && f1_read_count == 20u);
    assert(bit_event_count == 4u);
    assert(bit_events[0] == 1u && bit_events[1] == 2u && bit_events[2] == 3u && bit_events[3] == 4u);
    assert(sort_count == 2u);
    assert(sort_addresses[0] == 0x00f9b7ul && sort_counts[0] == 10u);
    assert(sort_addresses[1] == 0x00f9cbul && sort_counts[1] == 10u);
}

static void TestCapturePublishesAveragesMetricAndSignedRatio(void)
{
    ResetFixture();
    memory[0xe43u] = 4u;
    memory[0xfafu] = 6u;
    memory[0xfedu] = 2u;
    WriteU16(0xf9a0u, 100u);
    WriteU16(0xf9a2u, 50u);

    assert(CaptureControllerD8D9WindowAndUpdateAdjustment() == 162);
    AssertCaptureProtocol(190u, 2u);
    assert(ReadU16(0xefc9u) == 125u);
    assert(ReadU16(0xefe7u) == 32u);
    assert(memory[0xf9a4u] == 43u);
    assert((s32)multiply_left == 7 && multiply_right == 1000ul);
    assert(sfr[0xe1u] == 0u);
}

static void TestSmallOrNonpositiveMetricUsesFallbackRatio(void)
{
    ResetFixture();
    memory[0xe43u] = 2u;
    WriteU16(0xf9a0u, 125u);
    WriteU16(0xf9a2u, 32u);

    assert(CaptureControllerD8D9WindowAndUpdateAdjustment() == -500);
    AssertCaptureProtocol(400u, 15u);
    assert(memory[0xf9a4u] == 0u);
    assert((s32)multiply_left == -2 && multiply_right == 1000ul);
}

static void TestLargePositiveMetricSaturates(void)
{
    ResetFixture();
    memory[0xe43u] = 9u;
    WriteU16(0xf9a0u, 0u);
    WriteU16(0xf9a2u, 200u);

    assert(CaptureControllerD8D9WindowAndUpdateAdjustment() == -146);
    AssertCaptureProtocol(80u, 2u);
    assert(memory[0xf9a4u] == 0xffu);
}

static void TestControlFlagSelectsOneUnitDelay(void)
{
    ResetFixture();
    memory[0xe43u] = 12u;
    memory[0x1098u] = 1u;
    WriteU16(0xf9a0u, 125u);
    WriteU16(0xf9a2u, 32u);

    assert(CaptureControllerD8D9WindowAndUpdateAdjustment() == -500);
    AssertCaptureProtocol(1u, 2u);
}

int main(void)
{
    TestCapturePublishesAveragesMetricAndSignedRatio();
    TestSmallOrNonpositiveMetricUsesFallbackRatio();
    TestLargePositiveMetricSaturates();
    TestControlFlagSelectsOneUnitDelay();
    return 0;
}
