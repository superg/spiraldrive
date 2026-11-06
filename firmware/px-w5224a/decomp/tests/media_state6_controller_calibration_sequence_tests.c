#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/media_state6_controller_calibration_sequence.h"

static u8 memory_bytes[0x20000ul];
static u8 bit_0039;
static u8 position_results[32];
static size_t position_count;
static size_t position_index;
static u8 position_modes[32];
static u8 position_tolerances[32];
static u8 atip_results[8];
static size_t atip_count;
static size_t atip_index;
static u8 near_results[8];
static size_t near_count;
static size_t near_index;
static u8 read_path_results[16];
static size_t read_path_count;
static size_t read_path_index;
static s32 read_path_targets[16];
static u8 window_results[16];
static size_t window_count;
static size_t window_index;
static u8 sample_results[16];
static size_t sample_count;
static size_t sample_index;
static u8 evaluate_result;
static u8 fixed_success_result;
static u8 expire_atip_countdown;
static u8 expire_near_countdown;
static u8 set_fef_one_on_sample;
static unsigned enable_count;
static unsigned configure_count;
static unsigned apply_count;
static unsigned derive_count;
static unsigned strobe_count;
static unsigned restore_count;
static unsigned e2_count;
static u8 e2_values[16];
static unsigned write_d4_e2_count;
static unsigned raise_f4_count;
static unsigned restore_d4_e2_count;
static unsigned lower_f4_count;
static unsigned evaluate_count;

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

void FirmwareWriteBit0039(u8 value)
{
    bit_0039 = (u8)(value != 0u);
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

void ObserveState6SequenceEnable(u8 enabled)
{
    assert(enabled == 1u);
    ++enable_count;
}

u8 ObserveState6SequencePosition(u8 mode, u8 require_tolerance)
{
    assert(position_index < position_count);
    position_modes[position_index] = mode;
    position_tolerances[position_index] = require_tolerance;
    return position_results[position_index++];
}

void ObserveState6SequenceConfigure(u8 mode_selector)
{
    assert(mode_selector == 0u);
    ++configure_count;
}

void ObserveState6SequenceApply(void)
{
    ++apply_count;
}

u8 ObserveState6SequenceAtipWindow(void)
{
    assert(atip_index < atip_count);
    if(expire_atip_countdown != 0u)
        memory_bytes[0x1203u] = 0u;
    return atip_results[atip_index++];
}

u8 ObserveState6SequenceNearTarget(s32 target)
{
    assert(near_index < near_count);
    assert(target == (s32)(10000ul - 0x488ul - 30ul));
    if(expire_near_countdown != 0u)
        memory_bytes[0x1203u] = 0u;
    return near_results[near_index++];
}

u8 ObserveState6SequenceReadPath(s32 target)
{
    assert(read_path_index < read_path_count);
    read_path_targets[read_path_index] = target;
    return read_path_results[read_path_index++];
}

void ObserveState6SequenceStrobe(fw_addr_t address, u8 value)
{
    assert(address == 0x018080ul && value == 0u);
    ++strobe_count;
}

void ObserveState6SequenceDeriveThreshold(void)
{
    ++derive_count;
}

void ObserveState6SequenceWriteE2(u8 value)
{
    assert(e2_count < sizeof(e2_values));
    e2_values[e2_count++] = value;
}

u8 ObserveState6SequencePositionWindow(void)
{
    assert(window_index < window_count);
    return window_results[window_index++];
}

void ObserveState6SequenceRestore(u8 refresh)
{
    assert(refresh == 1u);
    ++restore_count;
}

void ObserveState6SequenceWriteD4E2(u8 value)
{
    assert(value == 0x74u);
    ++write_d4_e2_count;
}

void ObserveState6SequenceRaiseF4(void)
{
    ++raise_f4_count;
}

u8 ObserveState6SequenceAcquireSamples(void)
{
    assert(sample_index < sample_count);
    if(set_fef_one_on_sample != 0u)
        memory_bytes[0xfefu] = 1u;
    return sample_results[sample_index++];
}

void ObserveState6SequenceRestoreD4E2(void)
{
    ++restore_d4_e2_count;
}

void ObserveState6SequenceLowerF4(void)
{
    ++lower_f4_count;
}

u8 ObserveState6SequenceFixedSuccess(void)
{
    return fixed_success_result;
}

u8 ObserveState6SequenceEvaluateResult(void)
{
    ++evaluate_count;
    return evaluate_result;
}

static void SetResults(u8 *destination, size_t *count, size_t *index, const u8 *values, size_t value_count)
{
    memcpy(destination, values, value_count);
    *count = value_count;
    *index = 0u;
}

static void SetPositionResults(const u8 *values, size_t count)
{
    SetResults(position_results, &position_count, &position_index, values, count);
}

static void SetAtipResults(const u8 *values, size_t count)
{
    SetResults(atip_results, &atip_count, &atip_index, values, count);
}

static void SetNearResults(const u8 *values, size_t count)
{
    SetResults(near_results, &near_count, &near_index, values, count);
}

static void SetReadPathResults(const u8 *values, size_t count)
{
    SetResults(read_path_results, &read_path_count, &read_path_index, values, count);
}

static void SetWindowResults(const u8 *values, size_t count)
{
    SetResults(window_results, &window_count, &window_index, values, count);
}

static void SetSampleResults(const u8 *values, size_t count)
{
    SetResults(sample_results, &sample_count, &sample_index, values, count);
}

static void ResetFixture(void)
{
    static const u8 positions[] = { 1u, 1u, 1u, 1u };
    static const u8 atip[] = { 0u };
    static const u8 near_target[] = { 0u };
    static const u8 read_path[] = { 0u };
    static const u8 window[] = { 1u };
    static const u8 samples[] = { 1u };
    u8 index;

    memset(memory_bytes, 0, sizeof(memory_bytes));
    memset(position_modes, 0, sizeof(position_modes));
    memset(position_tolerances, 0, sizeof(position_tolerances));
    memset(read_path_targets, 0, sizeof(read_path_targets));
    memset(e2_values, 0, sizeof(e2_values));
    bit_0039 = 0u;
    expire_atip_countdown = expire_near_countdown = 0u;
    set_fef_one_on_sample = 0u;
    enable_count = configure_count = apply_count = derive_count = 0u;
    strobe_count = restore_count = e2_count = 0u;
    write_d4_e2_count = raise_f4_count = 0u;
    restore_d4_e2_count = lower_f4_count = evaluate_count = 0u;
    evaluate_result = 0x77u;
    fixed_success_result = 1u;
    WriteDword(0xfe9u, 10000ul);
    memory_bytes[0xfeeu] = 2u;
    memory_bytes[0xf962u] = 0x35u;
    WriteWord(0xfacfu, 0x2468u);
    for(index = 0u; index < 32u; ++index)
        memory_bytes[0x100au + index] = 0xa5u;
    SetPositionResults(positions, sizeof(positions));
    SetAtipResults(atip, sizeof(atip));
    SetNearResults(near_target, sizeof(near_target));
    SetReadPathResults(read_path, sizeof(read_path));
    SetWindowResults(window, sizeof(window));
    SetSampleResults(samples, sizeof(samples));
}

static void TestSuccessfulSequence(void)
{
    u8 index;

    ResetFixture();
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 1u);
    assert(enable_count == 1u);
    assert(position_index == 4u);
    assert(position_modes[0] == 0u && position_tolerances[0] == 1u);
    assert(position_modes[1] == 1u && position_tolerances[1] == 0u);
    assert(position_modes[2] == 2u && position_tolerances[2] == 1u);
    assert(position_modes[3] == 3u && position_tolerances[3] == 1u);
    assert(read_path_index == 1u);
    assert(read_path_targets[0] == (s32)(10000ul - 0x488ul - 30ul));
    assert(derive_count == 1u && e2_count == 2u);
    assert(e2_values[0] == 0x50u && e2_values[1] == 0x58u);
    assert(memory_bytes[0xff4u] == 0x35u);
    assert((ReadWord(0xff5u) & 1u) == 0u);
    assert(strobe_count == 1u && restore_count == 1u);
    assert(write_d4_e2_count == 1u && raise_f4_count == 1u);
    assert(restore_d4_e2_count == 1u && lower_f4_count == 1u);
    assert(ReadWord(0x1589u) == 0x2468u);
    assert(ReadWord(0xef85u) == 0x2468u);
    assert(bit_0039 == 1u && evaluate_count == 0u);
    for(index = 0u; index < 32u; ++index)
        assert(memory_bytes[0x100au + index] == 0u);
}

static void TestInitialPositionAndAtipTimeoutFailures(void)
{
    static const u8 failed_position[] = { 0u };
    static const u8 atip_retry[] = { 2u };

    ResetFixture();
    SetPositionResults(failed_position, sizeof(failed_position));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0u);
    assert(bit_0039 == 0u && configure_count == 0u);

    ResetFixture();
    SetAtipResults(atip_retry, sizeof(atip_retry));
    expire_atip_countdown = 1u;
    memory_bytes[0xe40u] = 1u;
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0u);
    assert(ReadDword(0xe3bu) == 0x96ul);
    assert(memory_bytes[0xe40u] == 0u);
    assert(memory_bytes[0xe41u] == 5u && memory_bytes[0xfaabu] == 5u);
    assert(memory_bytes[0xfafu] == 3u);
}

static void TestNearTargetTimeoutPreservesBusy(void)
{
    static const u8 near_retry[] = { 2u };

    ResetFixture();
    SetNearResults(near_retry, sizeof(near_retry));
    expire_near_countdown = 1u;
    memory_bytes[0xe40u] = 0x5au;
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0u);
    assert(memory_bytes[0xe40u] == 0x5au);
    assert(memory_bytes[0xe41u] == 5u && memory_bytes[0xfaabu] == 5u);
}

static void TestReadPathExhaustion(void)
{
    static const u8 positions[] = { 1u, 1u, 1u, 1u, 1u };
    static const u8 failures[] = { 2u, 2u, 2u };

    ResetFixture();
    SetPositionResults(positions, sizeof(positions));
    SetReadPathResults(failures, sizeof(failures));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0u);
    assert(read_path_index == 3u && strobe_count == 3u);
    assert(ReadDword(0xe3bu) == 0x96ul);
    assert(memory_bytes[0xe41u] == 0x1eu);
    assert(memory_bytes[0xfaabu] == 0x1eu);
}

static void TestPositionWindowRetry(void)
{
    static const u8 positions[] = { 1u, 1u, 1u, 1u, 1u };
    static const u8 read_paths[] = { 0u, 0u };
    static const u8 windows[] = { 0u, 1u };

    ResetFixture();
    SetPositionResults(positions, sizeof(positions));
    SetReadPathResults(read_paths, sizeof(read_paths));
    SetWindowResults(windows, sizeof(windows));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 1u);
    assert(derive_count == 2u && window_index == 2u);
    assert(strobe_count == 2u && restore_count == 2u);
}

static void TestSampleFailureClasses(void)
{
    static const u8 status_eleven[] = { 0x0bu };
    static const u8 status_five[] = { 5u };
    static const u8 zero_twice[] = { 0u, 0u };
    static const u8 repeated_two[] = { 2u, 2u };
    static const u8 positions_five[] = { 1u, 1u, 1u, 1u, 1u };

    ResetFixture();
    SetSampleResults(status_eleven, sizeof(status_eleven));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0x0bu);
    assert(ReadDword(0xe3bu) == 0x96ul);
    assert(memory_bytes[0xe41u] == 0x1bu);
    assert(memory_bytes[0xfaabu] == 0x1bu);
    assert(bit_0039 == 0u);

    ResetFixture();
    SetSampleResults(status_five, sizeof(status_five));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0x77u);
    assert(memory_bytes[0xff3u] == 3u && memory_bytes[0xff2u] == 3u);
    assert(bit_0039 == 1u && evaluate_count == 1u);

    ResetFixture();
    SetPositionResults(positions_five, sizeof(positions_five));
    SetSampleResults(zero_twice, sizeof(zero_twice));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0x77u);
    assert(memory_bytes[0xff2u] == 4u);

    ResetFixture();
    SetPositionResults(positions_five, sizeof(positions_five));
    WriteWord(0xff5u, 0x0010u);
    SetSampleResults(repeated_two, sizeof(repeated_two));
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 0x77u);
    assert(memory_bytes[0xff2u] == 2u);
}

static void TestTerminalMode4Calibration(void)
{
    static const u8 positions[] = { 1u, 1u, 1u, 1u, 1u };
    static const u8 read_paths[] = { 0u, 0u };
    static const u8 windows[] = { 1u, 1u };

    ResetFixture();
    SetPositionResults(positions, sizeof(positions));
    SetReadPathResults(read_paths, sizeof(read_paths));
    SetWindowResults(windows, sizeof(windows));
    set_fef_one_on_sample = 1u;
    assert(ExecuteMediaState6ControllerCalibrationSequence() == 1u);
    assert(position_modes[4] == 4u && position_tolerances[4] == 1u);
    assert(read_path_index == 2u);
    assert(read_path_targets[1] == (s32)(10000ul - 0x406ul - 2ul));
    assert(window_index == 2u && restore_count == 2u);
    assert(strobe_count == 2u && bit_0039 == 1u);
}

int main(void)
{
    TestSuccessfulSequence();
    TestInitialPositionAndAtipTimeoutFailures();
    TestNearTargetTimeoutPreservesBusy();
    TestReadPathExhaustion();
    TestPositionWindowRetry();
    TestSampleFailureClasses();
    TestTerminalMode4Calibration();
    puts("media state-6 controller calibration sequence tests passed");
    return 0;
}
