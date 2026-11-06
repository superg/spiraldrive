#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_state6_calibration_positioning.h"
#include "platform/firmware_memory.h"

static u8 memory_bytes[0x20000ul];
static u8 acquisition_results[16];
static size_t acquisition_count;
static size_t acquisition_index;
static u8 initialization_results[8];
static size_t initialization_count;
static size_t initialization_index;
static u8 tolerance_results[8];
static size_t tolerance_count;
static size_t tolerance_index;
static u8 expire_countdown_on_acquisition;
static unsigned reset_count;
static unsigned enable_count;
static u8 enable_argument;
static unsigned tolerance_call_count;
static u8 tolerance_arguments[8];

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

u8 ObserveState6PositioningAcquire(void)
{
    assert(acquisition_index < acquisition_count);
    if(expire_countdown_on_acquisition != 0u && acquisition_index != 0u)
        memory_bytes[0x1217u] = 0u;
    return acquisition_results[acquisition_index++];
}

void ObserveState6PositioningReset(void)
{
    ++reset_count;
}

u8 ObserveState6PositioningInitialize(void)
{
    assert(initialization_index < initialization_count);
    return initialization_results[initialization_index++];
}

void ObserveState6PositioningEnable(u8 enabled)
{
    ++enable_count;
    enable_argument = enabled;
}

u8 ObserveState6PositioningTolerance(u8 restart_timeout)
{
    assert(tolerance_call_count < sizeof(tolerance_arguments));
    assert(tolerance_index < tolerance_count);
    tolerance_arguments[tolerance_call_count++] = restart_timeout;
    return tolerance_results[tolerance_index++];
}

static void SetAcquisitionResults(const u8 *values, size_t count)
{
    memcpy(acquisition_results, values, count);
    acquisition_count = count;
    acquisition_index = 0u;
}

static void SetInitializationResults(const u8 *values, size_t count)
{
    memcpy(initialization_results, values, count);
    initialization_count = count;
    initialization_index = 0u;
}

static void SetToleranceResults(const u8 *values, size_t count)
{
    memcpy(tolerance_results, values, count);
    tolerance_count = count;
    tolerance_index = 0u;
}

static void ResetFixture(void)
{
    static const u8 success[] = { 1u };
    static const u8 initialized[] = { 1u };
    static const u8 tolerated[] = { 1u };

    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(acquisition_results, 0, sizeof(acquisition_results));
    memset(initialization_results, 0, sizeof(initialization_results));
    memset(tolerance_results, 0, sizeof(tolerance_results));
    memset(tolerance_arguments, 0, sizeof(tolerance_arguments));
    expire_countdown_on_acquisition = 0u;
    reset_count = enable_count = tolerance_call_count = 0u;
    enable_argument = 0xffu;
    WriteDword(0xfe9u, 10000ul);
    memory_bytes[0xfeeu] = 2u;
    memory_bytes[0xe47u] = 4u;
    memory_bytes[0xe3bu] = 0xa5u;
    memory_bytes[0xe3cu] = 0xa5u;
    memory_bytes[0xe3du] = 0xa5u;
    memory_bytes[0xe3eu] = 0xa5u;
    memory_bytes[0xe40u] = 0xa5u;
    memory_bytes[0xed6u] = 0xa5u;
    memory_bytes[0xfafu] = 6u;
    memory_bytes[0xe3fu] = 1u;
    SetAcquisitionResults(success, sizeof(success));
    SetInitializationResults(initialized, sizeof(initialized));
    SetToleranceResults(tolerated, sizeof(tolerated));
}

static void AssertSuccessfulTarget(u8 mode, u8 expected_code, u32 target)
{
    assert(PositionReadAcquisitionForCalibrationMode(mode, 0u) == 1u);
    assert(memory_bytes[0xe44u] == expected_code);
    assert(ReadDword(0xe3bu) == target);
    assert(memory_bytes[0xe40u] == 0u);
    assert(memory_bytes[0xed6u] == 0u);
}

static void TestAllTargetModes(void)
{
    ResetFixture();
    AssertSuccessfulTarget(0u, 1u, 10000ul - 0x047eul);

    ResetFixture();
    memory_bytes[0xe47u] = 16u;
    AssertSuccessfulTarget(1u, 16u, 10000ul - 30ul - 0x0494ul);

    ResetFixture();
    memory_bytes[0xe47u] = 12u;
    AssertSuccessfulTarget(2u, 1u, 10000ul - 30ul - 0x0488ul - 1344ul);

    ResetFixture();
    memory_bytes[0xe47u] = 7u;
    AssertSuccessfulTarget(3u, 1u, 10000ul - 30ul - 0x0488ul - 126ul);

    ResetFixture();
    memory_bytes[0xe47u] = 8u;
    AssertSuccessfulTarget(3u, 8u, 10000ul - 30ul - 0x05b4ul);

    ResetFixture();
    memory_bytes[0xe47u] = 7u;
    AssertSuccessfulTarget(4u, 1u, 10000ul - 0x0406ul - 2ul - 525ul);

    ResetFixture();
    memory_bytes[0xe47u] = 8u;
    AssertSuccessfulTarget(4u, 8u, 10000ul - 0x0406ul - 2ul - 600ul);
}

static void TestMode2OperatingCodeSelection(void)
{
    static const u16 flags[] = { 0x0000u, 0x1000u, 0x1800u, 0x1400u, 0x1200u, 0x1100u };
    static const u8 codes[] = { 16u, 16u, 40u, 32u, 24u, 20u };
    size_t index;

    for(index = 0u; index < sizeof(codes); ++index)
    {
        ResetFixture();
        memory_bytes[0xe47u] = 16u;
        WriteWord(0xff5u, flags[index]);
        if((flags[index] & 0x1000u) != 0u)
        {
            static const u8 repeated_success[] = { 1u, 1u };
            expire_countdown_on_acquisition = 1u;
            SetAcquisitionResults(repeated_success, sizeof(repeated_success));
        }
        assert(PositionReadAcquisitionForCalibrationMode(2u, 0u) == 1u);
        assert(memory_bytes[0xe44u] == codes[index]);
    }

    ResetFixture();
    WriteWord(0xff5u, 0x1080u);
    {
        static const u8 repeated_success[] = { 1u, 1u };
        expire_countdown_on_acquisition = 1u;
        SetAcquisitionResults(repeated_success, sizeof(repeated_success));
    }
    assert(PositionReadAcquisitionForCalibrationMode(2u, 0u) == 1u);
    assert(memory_bytes[0xe44u] == 16u);
}

static void TestToleranceRetryUsesOneThenZero(void)
{
    static const u8 acquisitions[] = { 1u, 1u };
    static const u8 tolerances[] = { 2u, 1u };

    ResetFixture();
    SetAcquisitionResults(acquisitions, sizeof(acquisitions));
    SetToleranceResults(tolerances, sizeof(tolerances));
    assert(PositionReadAcquisitionForCalibrationMode(0u, 1u) == 1u);
    assert(tolerance_call_count == 2u);
    assert(tolerance_arguments[0] == 1u);
    assert(tolerance_arguments[1] == 0u);
}

static void TestAcquisitionSentinelRepeats(void)
{
    static const u8 acquisitions[] = { 0xffu, 1u };

    ResetFixture();
    SetAcquisitionResults(acquisitions, sizeof(acquisitions));
    assert(PositionReadAcquisitionForCalibrationMode(0u, 0u) == 1u);
    assert(acquisition_index == 2u);
    assert(memory_bytes[0xe3fu] == 0u);
}

static void TestInitializationFailurePublishesSeven(void)
{
    static const u8 failed_acquisition[] = { 0u };
    static const u8 failed_initialization[] = { 0u };

    ResetFixture();
    SetAcquisitionResults(failed_acquisition, sizeof(failed_acquisition));
    SetInitializationResults(failed_initialization, sizeof(failed_initialization));
    assert(PositionReadAcquisitionForCalibrationMode(1u, 0u) == 0u);
    assert(reset_count == 1u);
    assert(enable_count == 0u);
    assert(memory_bytes[0xe40u] == 0u);
    assert(memory_bytes[0xe41u] == 7u);
    assert(memory_bytes[0xfaabu] == 7u);
    assert(memory_bytes[0xfafu] == 0u);
}

static void TestRecoveryRetryLimitAndFc3cShortcut(void)
{
    static const u8 failed_acquisitions[] = { 0u, 0u, 0u };
    static const u8 initialized[] = { 1u, 1u, 1u };
    static const u8 one_failed_acquisition[] = { 0u };
    static const u8 one_initialization[] = { 1u };

    ResetFixture();
    memory_bytes[0xe41u] = 0x55u;
    SetAcquisitionResults(failed_acquisitions, sizeof(failed_acquisitions));
    SetInitializationResults(initialized, sizeof(initialized));
    assert(PositionReadAcquisitionForCalibrationMode(0u, 0u) == 0u);
    assert(reset_count == 3u && enable_count == 3u);
    assert(enable_argument == 1u);
    assert(memory_bytes[0xfaabu] == 0x55u);
    assert(memory_bytes[0xe40u] == 0u && memory_bytes[0xfafu] == 0u);

    ResetFixture();
    memory_bytes[0xfc3cu] = 1u;
    memory_bytes[0xe41u] = 0x66u;
    SetAcquisitionResults(one_failed_acquisition, sizeof(one_failed_acquisition));
    SetInitializationResults(one_initialization, sizeof(one_initialization));
    assert(PositionReadAcquisitionForCalibrationMode(0u, 0u) == 0u);
    assert(reset_count == 1u && enable_count == 1u);
    assert(memory_bytes[0xfaabu] == 0x66u);
}

static void TestMode4TerminalFailurePreservesFaab(void)
{
    static const u8 failed_acquisitions[] = { 0u, 0u, 0u };
    static const u8 initialized[] = { 1u, 1u, 1u };

    ResetFixture();
    memory_bytes[0xe41u] = 0x55u;
    memory_bytes[0xfaabu] = 0xaau;
    SetAcquisitionResults(failed_acquisitions, sizeof(failed_acquisitions));
    SetInitializationResults(initialized, sizeof(initialized));
    assert(PositionReadAcquisitionForCalibrationMode(4u, 0u) == 0u);
    assert(memory_bytes[0xfaabu] == 0xaau);
}

static void TestUnsupportedModeLeavesTargetUntouched(void)
{
    ResetFixture();
    assert(PositionReadAcquisitionForCalibrationMode(5u, 0u) == 1u);
    assert(ReadDword(0xe3bu) == 0xa5a5a5a5ul);
}

int main(void)
{
    TestAllTargetModes();
    TestMode2OperatingCodeSelection();
    TestToleranceRetryUsesOneThenZero();
    TestAcquisitionSentinelRepeats();
    TestInitializationFailurePublishesSeven();
    TestRecoveryRetryLimitAndFc3cShortcut();
    TestMode4TerminalFailurePreservesFaab();
    TestUnsupportedModeLeavesTargetUntouched();
    puts("controller state-6 calibration positioning tests passed");
    return 0;
}
