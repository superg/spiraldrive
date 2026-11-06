#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_state6_position_window.h"
#include "platform/firmware_memory.h"

static u8 memory_bytes[0x20000ul];
static u8 poll_results[16];
static size_t poll_count;
static size_t poll_index;
static u8 operating_result;
static u8 readiness_result;
static u8 clear_phase_on_yield;
static unsigned arm_count;
static unsigned e8e9_count;
static u8 e8e9_value;
static unsigned operating_count;
static unsigned restore_count;
static u8 restore_argument;
static unsigned readiness_count;
static unsigned yield_count;
static unsigned apply_count;

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

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

void ObserveState6PositionWindowArm(void)
{
    ++arm_count;
}

void ObserveState6PositionWindowWriteE8E9(u8 value)
{
    ++e8e9_count;
    e8e9_value = value;
}

u8 ObserveState6PositionWindowPoll(void)
{
    assert(poll_index < poll_count);
    return poll_results[poll_index++];
}

u8 ObserveState6PositionWindowOperatingWindow(u8 wide)
{
    assert(wide == 1u);
    ++operating_count;
    return operating_result;
}

void ObserveState6PositionWindowRestore(u8 refresh)
{
    ++restore_count;
    restore_argument = refresh;
    memory_bytes[0xefcu] = 0u;
    memory_bytes[0x18005u] = 0u;
}

u8 ObserveState6PositionWindowReadiness(void)
{
    ++readiness_count;
    return readiness_result;
}

void ObserveState6PositionWindowYield(void)
{
    ++yield_count;
    if(clear_phase_on_yield != 0u)
        WriteWord(0xff5u, (u16)(ReadWord(0xff5u) & 0xfff9u));
}

void ObserveState6PositionWindowApplyParameters(void)
{
    ++apply_count;
}

static s32 PrimaryTarget(void)
{
    return 8803l;
}

static s32 AlternateBase(void)
{
    return 9998l;
}

static void SetPolls(const u8 *values, size_t count)
{
    memcpy(poll_results, values, count);
    poll_count = count;
    poll_index = 0u;
}

static void ResetFixture(void)
{
    static const u8 default_poll[] = { 1u };

    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(poll_results, 0, sizeof(poll_results));
    poll_count = poll_index = 0u;
    operating_result = 1u;
    readiness_result = 1u;
    clear_phase_on_yield = 0u;
    arm_count = e8e9_count = operating_count = 0u;
    restore_count = readiness_count = yield_count = apply_count = 0u;
    restore_argument = 0xffu;
    e8e9_value = 0u;
    memory_bytes[0x1301u] = 0xa0u;
    memory_bytes[0xfab5u] = 0x5au;
    WriteDword(0xfe9u, 10000ul);
    memory_bytes[0xfeeu] = 2u;
    memory_bytes[0xfefu] = 4u;
    SetPolls(default_poll, sizeof(default_poll));
}

static void AssertInitialization(void)
{
    assert(arm_count == 1u);
    assert(e8e9_count == 1u && e8e9_value == 0x5au);
    assert(ReadDword(0xfe5u) == 0x10000000ul);
    assert(memory_bytes[0xfa04u] == 0x28u);
}

static void TestInitialPollFailureRetainsControllerMode(void)
{
    static const u8 polls[] = { 0u };

    ResetFixture();
    SetPolls(polls, sizeof(polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
    AssertInitialization();
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    assert(memory_bytes[0xefcu] == 0x40u);
    assert(memory_bytes[0x18005u] == 0x40u);
}

static void TestPrimaryWindowCompletionRoutes(void)
{
    static const u8 immediate_polls[] = { 1u, 0u };
    static const u8 completed_polls[] = { 1u, 1u, 1u };

    ResetFixture();
    WriteWord(0xff5u, 1u);
    WriteDword(0x1235u, (u32)PrimaryTarget());
    SetPolls(immediate_polls, sizeof(immediate_polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 1u);
    assert((ReadWord(0xff5u) & 2u) != 0u);
    assert(memory_bytes[0xfedu] == 0u);
    assert(memory_bytes[0xefcu] == 0u && memory_bytes[0x18005u] == 0u);
    assert((memory_bytes[0x1301u] & 1u) == 0u);
    assert(apply_count == 0u);

    ResetFixture();
    WriteWord(0xff5u, 1u);
    WriteDword(0x1235u, (u32)PrimaryTarget());
    clear_phase_on_yield = 1u;
    SetPolls(completed_polls, sizeof(completed_polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 1u);
    assert(yield_count == 1u && apply_count == 1u);
}

static void TestPrimaryWindowRejections(void)
{
    static const u8 polls[] = { 1u };

    ResetFixture();
    WriteWord(0xff5u, 1u);
    WriteDword(0x1235u, (u32)PrimaryTarget() + 1ul);
    SetPolls(polls, sizeof(polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);

    ResetFixture();
    WriteWord(0xff5u, 1u);
    WriteDword(0x1235u, (u32)PrimaryTarget() + 8ul);
    SetPolls(polls, sizeof(polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
}

static void TestAlternateReadinessCheckpoint(void)
{
    static const u8 one_poll[] = { 1u };
    static const u8 retry_polls[] = { 1u, 0u };

    ResetFixture();
    WriteDword(0x1235u, 10000ul - 0x486ul);
    operating_result = 0u;
    readiness_result = 0u;
    SetPolls(one_poll, sizeof(one_poll));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
    assert(operating_count == 1u && restore_count == 1u);
    assert(restore_argument == 0u && readiness_count == 1u);

    ResetFixture();
    WriteDword(0x1235u, 10000ul - 0x486ul);
    operating_result = 0u;
    readiness_result = 1u;
    SetPolls(retry_polls, sizeof(retry_polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
    assert(restore_count == 1u && readiness_count == 1u);
    assert(memory_bytes[0xefcu] == 0x40u);
    assert(memory_bytes[0x18005u] == 0x40u);
}

static void TestAlternateWindowRoutes(void)
{
    static const u8 one_poll[] = { 1u };
    static const u8 phase_polls[] = { 1u, 0u };

    ResetFixture();
    WriteDword(0x1235u, (u32)AlternateBase() - 0x409ul);
    operating_result = 0u;
    SetPolls(one_poll, sizeof(one_poll));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
    assert(operating_count == 1u);

    ResetFixture();
    WriteDword(0x1235u, (u32)AlternateBase() - 0x408ul);
    SetPolls(phase_polls, sizeof(phase_polls));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 1u);
    assert((ReadWord(0xff5u) & 4u) != 0u);
    assert(memory_bytes[0xfedu] == 0u);

    ResetFixture();
    WriteDword(0x1235u, (u32)AlternateBase() - 0x407ul);
    SetPolls(one_poll, sizeof(one_poll));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);

    ResetFixture();
    WriteDword(0x1235u, (u32)AlternateBase() - 0x402ul);
    SetPolls(one_poll, sizeof(one_poll));
    assert(RunMediaState6ControllerCalibrationPositionWindow() == 0u);
}

int main(void)
{
    TestInitialPollFailureRetainsControllerMode();
    TestPrimaryWindowCompletionRoutes();
    TestPrimaryWindowRejections();
    TestAlternateReadinessCheckpoint();
    TestAlternateWindowRoutes();
    puts("controller state-six position-window tests passed");
    return 0;
}
