#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/state5_position_window.h"
#include "platform/firmware_memory.h"

static u8 memory_bytes[0x14000ul];

s16 AbsoluteValueI16(s16 value)
{
    return value < 0 ? (s16)(u16)(0u - (u16)value) : value;
}

u32 MultiplyU32Low(u32 factor_a, u32 factor_b)
{
    return factor_a * factor_b;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static void ResetFixture(u8 speed, s32 target, s32 current)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    memory_bytes[0x0e47u] = speed;
    memory_bytes[0xfb9bu] = 1u;
    WriteDword(0x0e3bu, (u32)target);
    WriteDword(0x1235u, (u32)current);
}

static void TestLowSpeedAndSignedDivision(void)
{
    ResetFixture(2u, 0l, 750l);
    assert(ComputeMediaState5PositionWindowFrames() == 150l);
    assert(memory_bytes[0xfaeau] == 2u);
    assert(memory_bytes[0xfafcu] == 1u);

    ResetFixture(2u, 0l, -74l);
    assert(ComputeMediaState5PositionWindowFrames() == 150l);

    ResetFixture(1u, 0l, 0l);
    assert(ComputeMediaState5PositionWindowFrames() == 150l);
}

static void TestEffectiveSpeedCapAndMediaTypeOverride(void)
{
    ResetFixture(52u, 30000l, 30000l);
    memory_bytes[0xfb9bu] = 0u;
    assert(ComputeMediaState5PositionWindowFrames() == 1680l);
    assert(memory_bytes[0xfaeau] == 52u);
    assert(memory_bytes[0xfafcu] == 6u);

    ResetFixture(10u, 44999l, 44999l);
    memory_bytes[0x1008u] = 2u;
    assert(ComputeMediaState5PositionWindowFrames() == 900l);
}

static void TestNegativeTargetMultiplier(void)
{
    ResetFixture(10u, -1200l, -1200l);
    assert(ComputeMediaState5PositionWindowFrames() == 1270l);
    assert(memory_bytes[0xfafcu] == 1u);

    ResetFixture(10u, -1199l, -1199l);
    assert(ComputeMediaState5PositionWindowFrames() == 750l);
    assert(memory_bytes[0xfafcu] == 6u);
}

static void TestDistanceBucketBoundaries(void)
{
    ResetFixture(20u, 30000l, 30000l + 1199l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 1800l);

    ResetFixture(20u, 30000l, 30000l + 1200l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 1800l);

    ResetFixture(20u, 30000l, 30000l + 2400l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 3000l);

    ResetFixture(40u, 30000l, 30000l + 2399l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 3600l);

    ResetFixture(40u, 30000l, 30000l + 2400l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 6000l);

    ResetFixture(40u, 30000l, 30000l + 4200l * 75l);
    assert(ComputeMediaState5PositionWindowFrames() == 9000l);
}

static void TestSpeed52BucketsAndFinalClamp(void)
{
    ResetFixture(52u, 30000l, 30000l + 1200l * 75l);
    memory_bytes[0xfb9bu] = 1u;
    assert(ComputeMediaState5PositionWindowFrames() == 9360l);

    ResetFixture(40u, 22500l, 22500l + 4200l * 75l);
    memory_bytes[0xfb9bu] = 1u;
    assert(ComputeMediaState5PositionWindowFrames() == 0x816l);

    ResetFixture(40u, 22501l, 22501l + 4200l * 75l);
    memory_bytes[0xfb9bu] = 1u;
    assert(ComputeMediaState5PositionWindowFrames() == 9000l);
}

int main(void)
{
    TestLowSpeedAndSignedDivision();
    TestEffectiveSpeedCapAndMediaTypeOverride();
    TestNegativeTargetMultiplier();
    TestDistanceBucketBoundaries();
    TestSpeed52BucketsAndFinalClamp();
    puts("media state-5 position-window tests passed");
    return 0;
}
