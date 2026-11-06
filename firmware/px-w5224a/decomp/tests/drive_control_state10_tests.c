#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/drive_control_state10.h"

static u8 memory[1u << 16];
static u16 submitted_words[32];
static unsigned submitted_count;
static unsigned yield_count;
static unsigned clear_count;
static unsigned clear_phase_count;
static u8 phase_bit;
static u8 bit7_low;
static u8 bit5_low;
static u8 state_flag_result;
static u8 countdown_after_yield[16];
static unsigned countdown_result_count;

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

u8 ObserveDriveState10PhaseBit(void)
{
    return phase_bit;
}

void ObserveDriveState10ClearPhaseBit(void)
{
    phase_bit = 0u;
    ++clear_phase_count;
}

void ObserveDriveState10SubmitControllerWord(u16 controller_value)
{
    submitted_words[submitted_count++] = controller_value;
}

u8 ObserveDriveState10DebouncedP1Bit7Low(void)
{
    return bit7_low;
}

u8 ObserveDriveState10DebouncedP1Bit5Low(void)
{
    return bit5_low;
}

void ObserveDriveState10YieldCoroutine1(void)
{
    if(yield_count < countdown_result_count)
        memory[0x1208u] = countdown_after_yield[yield_count];
    if(countdown_result_count != 0u && yield_count + 1u >= countdown_result_count)
    {
        bit7_low = 0u;
        bit5_low = 1u;
    }
    memory[0x1214u] = 0u;
    ++yield_count;
}

u8 ObserveDriveState10TakeStateFlag11EFBit5(void)
{
    return state_flag_result;
}

void ObserveDriveState10ClearControllerWord0252(void)
{
    ++clear_count;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(submitted_words, 0, sizeof(submitted_words));
    memset(countdown_after_yield, 0, sizeof(countdown_after_yield));
    submitted_count = 0u;
    yield_count = 0u;
    clear_count = 0u;
    clear_phase_count = 0u;
    phase_bit = 0u;
    bit7_low = 0u;
    bit5_low = 1u;
    state_flag_result = 0u;
    countdown_result_count = 0u;
}

static void SetConfigurationWord(u16 value)
{
    memory[0xfaacu] = (u8)(value >> 8);
    memory[0xfaadu] = (u8)value;
}

static u16 GetConfigurationWord(void)
{
    return (u16)(((u16)memory[0xfaacu] << 8) | memory[0xfaadu]);
}

static void AssertCommonSetupAndCleanup(void)
{
    assert((memory[0x1303u] & 1u) == 0u);
    assert((memory[0x1588u] & 8u) != 0u);
    assert(memory[0x1300u] == 8u);
    assert(clear_count == 1u);
}

static void TestImmediateInputCompletionReturnsOne(void)
{
    ResetFixture();
    memory[0x1303u] = 0xffu;
    memory[0x1588u] = 0x80u;
    memory[0x1302u] = 0xffu;
    memory[0x1301u] = 0xffu;

    assert(RunDriveControlState10Sequence() == 1u);
    AssertCommonSetupAndCleanup();
    assert(submitted_count == 1u && submitted_words[0] == 0xb000u);
    assert(yield_count == 0u && clear_phase_count == 0u);
    assert(memory[0x1302u] == 0xfdu);
    assert(memory[0x1301u] == 0xdfu);
}

static void TestCountdownExpiryRunsSettlingTailAndReturnsZero(void)
{
    ResetFixture();
    memory[0x1302u] = 2u;
    bit7_low = 1u;
    bit5_low = 0u;
    countdown_after_yield[0] = 0u;
    countdown_result_count = 1u;

    assert(RunDriveControlState10Sequence() == 0u);
    AssertCommonSetupAndCleanup();
    assert(submitted_count == 2u && submitted_words[0] == 0xb000u && submitted_words[1] == 0xda00u);
    assert(yield_count == 2u && clear_phase_count == 1u && phase_bit == 0u);
}

static void TestPhaseBitRunsBothStagedTransitions(void)
{
    ResetFixture();
    memory[0x1302u] = 2u;
    phase_bit = 1u;
    bit7_low = 1u;
    bit5_low = 0u;
    countdown_after_yield[0] = 87u;
    countdown_after_yield[1] = 81u;
    countdown_after_yield[2] = 80u;
    countdown_result_count = 3u;

    assert(RunDriveControlState10Sequence() == 1u);
    assert(submitted_count == 5u);
    assert(submitted_words[0] == 0xb000u);
    assert(submitted_words[1] == 0x9000u);
    assert(submitted_words[2] == 0x9000u);
    assert(submitted_words[3] == 0xc800u);
    assert(submitted_words[4] == 0xda00u);
    assert(memory[0x1302u] == 0u);
    AssertCommonSetupAndCleanup();
}

static void TestExactStateFlagOneAborts(void)
{
    ResetFixture();
    memory[0x1302u] = 2u;
    bit7_low = 1u;
    bit5_low = 0u;
    state_flag_result = 1u;

    assert(RunDriveControlState10Sequence() == 2u);
    AssertCommonSetupAndCleanup();
    assert(yield_count == 1u);
}

static void TestConfigurationAbortClearsPendingStateAndBit14(void)
{
    ResetFixture();
    memory[0x1302u] = 2u;
    memory[0xf976u] = 1u;
    SetConfigurationWord(0x4123u);
    bit7_low = 1u;
    bit5_low = 0u;
    countdown_after_yield[0] = 86u;
    countdown_result_count = 1u;

    assert(RunDriveControlState10Sequence() == 2u);
    AssertCommonSetupAndCleanup();
    assert(memory[0xf976u] == 0u);
    assert(GetConfigurationWord() == 0x0123u);
}

static void TestLowCountdownWithoutAbortFlagSetsFailureFlags(void)
{
    ResetFixture();
    memory[0x1302u] = 0x40u;
    bit7_low = 1u;
    bit5_low = 0u;
    countdown_after_yield[0] = 60u;
    countdown_result_count = 1u;

    assert(RunDriveControlState10Sequence() == 2u);
    AssertCommonSetupAndCleanup();
    assert(memory[0x1302u] == 2u);
}

int main(void)
{
    TestImmediateInputCompletionReturnsOne();
    TestCountdownExpiryRunsSettlingTailAndReturnsZero();
    TestPhaseBitRunsBothStagedTransitions();
    TestExactStateFlagOneAborts();
    TestConfigurationAbortClearsPendingStateAndBit14();
    TestLowCountdownWithoutAbortFlagSetsFailureFlags();
    return 0;
}
