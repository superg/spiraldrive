#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/media_state6_calibration_threshold.h"

static u8 memory[0x10000ul];
static fw_addr_t decode_addresses[4];
static u8 decode_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

s8 DecodeControllerEncodedSignedOffset(u8 encoded_offset)
{
    assert(decode_count < sizeof(decode_addresses) / sizeof(decode_addresses[0]));
    decode_addresses[decode_count++] = encoded_offset;
    if(encoded_offset == 0xffu)
        return 0;
    if((encoded_offset & 0x80u) != 0u)
        return (s8)(128 - (s16)encoded_offset);
    return (s8)encoded_offset;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(decode_addresses, 0, sizeof(decode_addresses));
    decode_count = 0u;
    memory[0xe47u] = 12u;
    memory[0xf963u] = 10u;
    memory[0xf964u] = 20u;
    memory[0xf965u] = 30u;
    memory[0xf966u] = 40u;
    memory[0xfadcu] = 3u;
    memory[0xfaddu] = 4u;
    memory[0xfadeu] = 5u;
    memory[0xfadfu] = 6u;
    memory[0xfabfu] = 0xffu;
}

static void TestOperatingCodeRoutes(void)
{
    static const u8 codes[] = { 1u, 4u, 8u, 16u, 20u, 24u, 32u, 40u };
    static const u8 expected[] = { 13u, 24u, 30u, 32u, 32u, 32u, 32u, 32u };
    static const u8 expected_class[] = { 0u, 0u, 3u, 3u, 3u, 3u, 3u, 3u };
    static const u8 decoded[] = { 3u, 4u, 5u, 6u, 6u, 6u, 6u, 6u };
    u8 index;

    for(index = 0u; index != sizeof(codes); ++index)
    {
        ResetFixture();
        memory[0xe43u] = codes[index];
        DeriveMediaState6CalibrationThresholdState();
        assert(memory[0xf962u] == expected[index]);
        assert(memory[0xfadbu] == decoded[index]);
        assert(memory[0xff3u] == expected_class[index]);
        assert(decode_count == 1u && decode_addresses[0] == decoded[index]);
    }
}

static void TestOptionalAdjustmentAndConfigurationOverride(void)
{
    ResetFixture();
    memory[0xe43u] = 16u;
    memory[0xfabfu] = 5u;
    DeriveMediaState6CalibrationThresholdState();
    assert(decode_count == 2u && decode_addresses[1] == 5u);
    assert(memory[0xff3u] == 3u && memory[0xf962u] == 37u);

    ResetFixture();
    memory[0xe43u] = 16u;
    memory[0xfabfu] = 5u;
    WriteWord(0xfaacu, 2u);
    DeriveMediaState6CalibrationThresholdState();
    assert(memory[0xff3u] == 3u && memory[0xf962u] == 30u);
}

static void TestLowSpeedThresholdClass(void)
{
    ResetFixture();
    memory[0xe43u] = 8u;
    memory[0xe47u] = 11u;
    DeriveMediaState6CalibrationThresholdState();
    assert(memory[0xff3u] == 3u && memory[0xf962u] == 0x21u);

    ResetFixture();
    memory[0xe43u] = 4u;
    memory[0xe47u] = 11u;
    DeriveMediaState6CalibrationThresholdState();
    assert(memory[0xff3u] == 0u && memory[0xf962u] == 24u);
}

static void TestSignedClampUnsupportedAndWrappingBase(void)
{
    ResetFixture();
    memory[0xe43u] = 1u;
    memory[0xf963u] = 0u;
    memory[0xfadcu] = 0x81u;
    DeriveMediaState6CalibrationThresholdState();
    assert(memory[0xfadbu] == 0xffu);
    assert(memory[0xf962u] == 0u && memory[0xff3u] == 2u);

    ResetFixture();
    memory[0xe43u] = 7u;
    memory[0xfadbu] = 0xa5u;
    DeriveMediaState6CalibrationThresholdState();
    assert(decode_count == 0u && memory[0xfadbu] == 0xa5u);
    assert(memory[0xf962u] == 0u && memory[0xff3u] == 2u);

    ResetFixture();
    memory[0xe43u] = 32u;
    memory[0xf966u] = 250u;
    memory[0xfadfu] = 0u;
    DeriveMediaState6CalibrationThresholdState();
    assert(memory[0xf962u] == 4u && memory[0xff3u] == 0u);
}

int main(void)
{
    TestOperatingCodeRoutes();
    TestOptionalAdjustmentAndConfigurationOverride();
    TestLowSpeedThresholdClass();
    TestSignedClampUnsupportedAndWrappingBase();
    puts("media-state-6 calibration threshold tests passed");
    return 0;
}
