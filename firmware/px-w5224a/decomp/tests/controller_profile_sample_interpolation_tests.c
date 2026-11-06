#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_profile_sample_interpolation.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static fw_addr_t reads[32];
static unsigned read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    assert(read_count < sizeof(reads) / sizeof(reads[0]));
    reads[read_count++] = address;
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

static s8 Reference(s16 start, s16 end, float value, u8 encode_20)
{
    float coefficient = (float)(s16)(end - start) / 2048.0f;
    float offset = (float)start - coefficient * 256.0f;
    u8 result = (u8)(s32)floor((double)(coefficient * value * value + offset));

    if((encode_20 != 0u) && ((s8)result < 0))
        result = (u8)((u8)(0u - result) | 0x20u);
    return (s8)result;
}

static fw_addr_t LowRow(u8 row)
{
    return 0x00f83b1dul + (fw_addr_t)((u16)row * 28u);
}

static fw_addr_t HighRow(u8 row)
{
    return 0x00f83cddul + (fw_addr_t)((u16)row * 28u);
}

static void ResetFixture(u8 row)
{
    memset(memory, 0, sizeof(memory));
    memset(reads, 0, sizeof(reads));
    read_count = 0u;
    memory[0x00f96bul] = row;
}

static void TestBasePlusByteFamily(void)
{
    fw_addr_t low;
    fw_addr_t high;

    ResetFixture(2u);
    low = LowRow(2u);
    high = HighRow(2u);
    memory[low] = 0x43u;
    memory[high] = 0x04u;
    memory[low + 1u] = 0xfeu;
    memory[high + 1u] = 1u;
    assert(InterpolateControllerProfileSampleByIndex(32.0f, 0u) == Reference(-5, 5, 32.0f, 0u));
    assert(reads[0] == 0x00f96bul);
    assert(reads[1] == low && reads[2] == low + 1u);
    assert(reads[3] == high && reads[4] == high + 1u);

    read_count = 0u;
    memory[low + 9u] = 3u;
    memory[high + 9u] = 7u;
    assert(InterpolateControllerProfileSampleByIndex(48.0f, 8u) == Reference(0, 11, 48.0f, 0u));
}

static void TestSignedMagnitude20Family(void)
{
    fw_addr_t low;
    fw_addr_t high;

    ResetFixture(1u);
    low = LowRow(1u);
    high = HighRow(1u);
    memory[low + 10u] = 0x25u;
    memory[high + 10u] = 7u;
    assert(InterpolateControllerProfileSampleByIndex(0.0f, 9u) == Reference(-5, 7, 0.0f, 1u));

    memory[low + 18u] = 2u;
    memory[high + 18u] = 0x23u;
    assert(InterpolateControllerProfileSampleByIndex(64.0f, 17u) == Reference(2, -3, 64.0f, 1u));
}

static void TestHighNibbleFamily(void)
{
    fw_addr_t low;
    fw_addr_t high;

    ResetFixture(3u);
    low = LowRow(3u);
    high = HighRow(3u);
    memory[low + 19u] = 0xabu;
    memory[high + 19u] = 0xe1u;
    assert(InterpolateControllerProfileSampleByIndex(80.0f, 18u) == Reference(10, 14, 80.0f, 0u));

    memory[low + 27u] = 0x30u;
    memory[high + 27u] = 0x90u;
    assert(InterpolateControllerProfileSampleByIndex(12.0f, 26u) == Reference(3, 9, 12.0f, 0u));
}

static void TestBaseMinusNibbleFamily(void)
{
    fw_addr_t low;
    fw_addr_t high;

    ResetFixture(4u);
    low = LowRow(4u);
    high = HighRow(4u);
    memory[low] = 0x43u;
    memory[high] = 4u;
    memory[low + 19u] = 0xb5u;
    memory[high + 19u] = 0xa2u;
    assert(InterpolateControllerProfileSampleByIndex(128.0f, 27u) == Reference(-8, 2, 128.0f, 0u));

    memory[low + 27u] = 1u;
    memory[high + 27u] = 0x0fu;
    assert(InterpolateControllerProfileSampleByIndex(255.5f, 35u) == Reference(-4, -11, 255.5f, 0u));
}

static void TestInvalidIndexReturnsZeroWithoutReads(void)
{
    ResetFixture(0u);
    assert(InterpolateControllerProfileSampleByIndex(1.0f, 36u) == 0);
    assert(InterpolateControllerProfileSampleByIndex(1.0f, 0xffu) == 0);
    assert(read_count == 0u);
}

int main(void)
{
    TestBasePlusByteFamily();
    TestSignedMagnitude20Family();
    TestHighNibbleFamily();
    TestBaseMinusNibbleFamily();
    TestInvalidIndexReturnsZeroWithoutReads();
    puts("controller_profile_sample_interpolation_tests: ok");
    return 0;
}
