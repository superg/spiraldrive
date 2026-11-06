#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/high_write_speed_calibration_flags.h"

static u8 memory[0x10000ul];

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

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memory[0xe47u] = 16u;
    WriteWord(0xffeu, 0u);
    WriteWord(0xfd9u, 1000u);
    WriteWord(0xfacfu, 0u);
    WriteWord(0xe752u, 0u);
    WriteWord(0xe754u, 0x0100u);
    WriteWord(0xe756u, 0x0200u);
    WriteWord(0xe758u, 0x0300u);
    WriteWord(0xf9fau, 0u);
    WriteWord(0xfad7u, 3u);
}

static void TestLowSpeedLeavesStateUntouched(void)
{
    ResetFixture();
    memory[0xe47u] = 15u;
    WriteWord(0xfad7u, 0xa503u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0xa503u);
}

static void TestEnableGateVariants(void)
{
    ResetFixture();
    WriteWord(0xfd9u, 1u);
    WriteWord(0xfad7u, 1u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 1u);

    ResetFixture();
    WriteWord(0xfd9u, 1u);
    WriteWord(0xfad7u, 1u);
    memory[0x1008u] = 2u;
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0201u);

    ResetFixture();
    WriteWord(0xfd9u, 1u);
    WriteWord(0xfad7u, 0x1000u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x1200u);
}

static void TestFirstMatchPriorityAndStrictThresholds(void)
{
    ResetFixture();
    WriteWord(0xfd9u, 0xffu);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0203u);

    ResetFixture();
    WriteWord(0xfd9u, 0x0100u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0103u);

    ResetFixture();
    WriteWord(0xfd9u, 0x0200u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0083u);

    ResetFixture();
    WriteWord(0xfd9u, 0x0300u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0043u);

    ResetFixture();
    WriteWord(0xfd9u, 0x0400u);
    memory[0xf99fu] = 200u;
    WriteWord(0xf9fau, 2000u);
    WriteWord(0xffeu, 50u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0023u);
}

static void TestWrappedThresholdAndSignedDifference(void)
{
    ResetFixture();
    WriteWord(0xe752u, 0xff80u);
    WriteWord(0xfd9u, 100u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0203u);

    ResetFixture();
    WriteWord(0xfd9u, 0u);
    WriteWord(0xfacfu, 1u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 3u);
}

static void TestNegativeDistanceClamp(void)
{
    ResetFixture();
    memory[0xf99fu] = 200u;
    WriteWord(0xffeu, 100u);
    WriteWord(0xfd9u, 1u);
    UpdateHighWriteSpeedFlagsFromCalibrationThresholds();
    assert(ReadWord(0xfad7u) == 0x0203u);
}

int main(void)
{
    TestLowSpeedLeavesStateUntouched();
    TestEnableGateVariants();
    TestFirstMatchPriorityAndStrictThresholds();
    TestWrappedThresholdAndSignedDifference();
    TestNegativeDistanceClamp();
    puts("high-write-speed calibration flag tests passed");
    return 0;
}
