#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_cd_engine.h"

static u8 memory[0x20000];
static u8 bits[0x40];
static u8 unsupported_calls;
static u8 prepare_calls;
static s32 prepare_lba;
static u8 reset_progress_calls;
static u8 coverage_calls;
static s32 coverage_start;
static s32 coverage_end;
static u8 coverage_result;
static u8 format_transfer_calls;
static u8 format_transfer_selection;
static s8 format_transfer_result;
static u8 raw_range_calls;
static s32 raw_range_start;
static s32 raw_range_end;
static u8 raw_range_format;
static u8 raw_range_result;
static u16 descriptor_address;
static u8 sense_calls;
static u8 sense_key;
static u16 sense_code;
static u8 information_calls;
static u32 sense_information;
static u8 reset_queue_calls;
static u8 initialize_calls;
static u32 initialize_entry;
static u8 dispatch_calls;
static u8 yield_calls;

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

u8 ObserveReadCdEngineBit(u8 bit_address)
{
    assert(bit_address < sizeof(bits));
    return bits[bit_address];
}

void ObserveWriteReadCdEngineBit(u8 bit_address, u8 value)
{
    assert(bit_address < sizeof(bits));
    bits[bit_address] = value;
}

void HandleUnsupportedScsiCommand(void)
{
    unsupported_calls++;
}
void PrepareAndDispatchReadOrSeekAcquisitionAtLba(s32 lba)
{
    prepare_calls++;
    prepare_lba = lba;
}
void ResetRawReadProgressCountersAndPhaseFlags(void)
{
    reset_progress_calls++;
}
u8 ClassifyReadCdFrameRangeTrackCoverage(s32 start, s32 end)
{
    coverage_calls++;
    coverage_start = start;
    coverage_end = end;
    return coverage_result;
}
s8 ExecuteReadCdRawSectorTransferForFormat(u8 selection)
{
    format_transfer_calls++;
    format_transfer_selection = selection;
    return format_transfer_result;
}
u8 ExecuteRawSectorReadRange(s32 start, s32 end, u8 format)
{
    raw_range_calls++;
    raw_range_start = start;
    raw_range_end = end;
    raw_range_format = format;
    return raw_range_result;
}
u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    return descriptor_address;
}
void SetSenseData(u8 key, u16 code)
{
    sense_calls++;
    sense_key = key;
    sense_code = code;
}
void SetSenseInformation(u32 information)
{
    information_calls++;
    sense_information = information;
}
void ResetRawReadQueueAndTransferState(void)
{
    reset_queue_calls++;
}
void InitializeCoroutine2(u32 entry)
{
    initialize_calls++;
    initialize_entry = entry;
}
void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    dispatch_calls++;
}
void YieldCoroutine0(void)
{
    yield_calls++;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 0, sizeof(bits));
    unsupported_calls = prepare_calls = reset_progress_calls = 0u;
    coverage_calls = format_transfer_calls = raw_range_calls = 0u;
    sense_calls = information_calls = reset_queue_calls = 0u;
    initialize_calls = dispatch_calls = yield_calls = 0u;
    coverage_result = 1u;
    format_transfer_result = 1;
    raw_range_result = 0u;
    descriptor_address = 0x1800u;
    memory[0x0b14] = 1u;
    WriteDword(0x0b02u, 1000ul);
    WriteDword(0x0b06u, 10ul);
}

static void TestValidationAndZeroCount(void)
{
    ResetState();
    memory[0x0b15] = 4u;
    ExecuteReadCdCommand();
    assert(unsupported_calls == 1u && coverage_calls == 0u);

    ResetState();
    memory[0x0b14] = 3u;
    ExecuteReadCdCommand();
    assert(unsupported_calls == 1u);

    ResetState();
    bits[0x0e] = bits[0x18] = 1u;
    WriteDword(0x0b02u, 0xffffff9cul);
    WriteDword(0x0b06u, 0ul);
    ExecuteReadCdCommand();
    assert(bits[0x0e] == 0u && bits[0x18] == 0u);
    assert(prepare_calls == 1u && prepare_lba == -100);
    assert(coverage_calls == 0u && format_transfer_calls == 0u);
}

static void TestOrdinarySetupAndFormatTransfer(void)
{
    ResetState();
    memory[0x0b0b] = 0x1cu;
    memory[0x0b13] = 2u;
    memory[0x0f0b] = 0u;
    bits[0x21] = 1u;
    ExecuteReadCdCommand();
    assert(memory[0x0f0a] == 7u);
    assert(memory[0x0c2a] == 7u && bits[0x0d] == 1u);
    assert(reset_progress_calls == 1u && memory[0x0f0b] == 2u);
    assert(coverage_calls == 1u);
    assert(coverage_start == 1150 && coverage_end == 1160);
    assert(format_transfer_calls == 1u && format_transfer_selection == 1u);
    assert(bits[0x21] == 0u);

    ResetState();
    memory[0x0b13] = 0xf0u;
    memory[0x0f0b] = 0xf8u;
    bits[0x0d] = 1u;
    ExecuteReadCdCommand();
    assert(memory[0x0c2a] == 0u);
    assert(reset_progress_calls == 1u);
}

static void TestNegativeModesAndCoverageFallback(void)
{
    ResetState();
    WriteDword(0x0b02u, (u32)-1000l);
    ExecuteReadCdCommand();
    assert(bits[0x13] == 1u && coverage_calls == 1u);

    ResetState();
    WriteDword(0x0b02u, (u32)-20000l);
    ExecuteReadCdCommand();
    assert(bits[0x1c] == 1u && coverage_calls == 0u);

    ResetState();
    WriteDword(0x0b02u, (u32)-30000l);
    ExecuteReadCdCommand();
    assert(bits[0x1d] == 1u && coverage_calls == 0u);

    ResetState();
    WriteDword(0x0b02u, (u32)-30000l);
    bits[0x21] = 1u;
    ExecuteReadCdCommand();
    assert(bits[0x13] == 0u && bits[0x1c] == 0u && bits[0x1d] == 0u);

    ResetState();
    memory[0x0b14] = 4u;
    coverage_result = 2u;
    ExecuteReadCdCommand();
    assert(raw_range_calls == 1u && raw_range_format == 2u);
    assert(raw_range_start == 1150 && raw_range_end == 1160);
    assert(format_transfer_calls == 0u);
}

static void TestContinuationAndTransferFailure(void)
{
    ResetState();
    bits[0x11] = bits[0x10] = 1u;
    WriteDword(0x15eau, 1150ul);
    ExecuteReadCdCommand();
    assert(raw_range_calls == 1u && raw_range_format == 4u);
    assert(ReadDword(0x15eau) == 1160ul);
    assert(format_transfer_calls == 0u);

    ResetState();
    bits[0x11] = bits[0x10] = 1u;
    WriteDword(0x15eau, 99ul);
    format_transfer_result = -1;
    ExecuteReadCdCommand();
    assert(bits[0x10] == 0u);
    assert(format_transfer_calls == 1u && raw_range_calls == 0u);
    assert(sense_calls == 0u);
}

static void TestDescriptorFailures(void)
{
    ResetState();
    bits[0x11] = 1u;
    format_transfer_result = 0;
    memory[descriptor_address + 0x13u] = 4u;
    WriteDword(0x1459u, 7ul);
    ExecuteReadCdCommand();
    assert(sense_calls == 1u && sense_key == 3u && sense_code == 0x0600u);
    assert(information_calls == 1u && sense_information == 1007ul);
    assert(reset_queue_calls == 1u && initialize_calls == 1u);
    assert(initialize_entry == 0x00f84a00ul);
    assert(dispatch_calls == 1u && yield_calls == 1u);

    ResetState();
    bits[0x11] = 1u;
    format_transfer_result = 0;
    WriteDword(0x1459u, 2ul);
    ExecuteReadCdCommand();
    assert(sense_calls == 1u && sense_code == 0x1500u);
    assert(sense_information == 1002ul && reset_queue_calls == 0u);
}

static void TestF8FaContinuationGate(void)
{
    ResetState();
    bits[0x11] = 1u;
    format_transfer_result = 0;
    memory[0x0b13] = 0xf8u;
    memory[descriptor_address + 0x14u] = 0u;
    ExecuteReadCdCommand();
    assert(raw_range_calls == 0u && sense_calls == 0u);

    ResetState();
    bits[0x11] = 1u;
    format_transfer_result = 0;
    raw_range_result = 1u;
    memory[0x0b13] = 0xfau;
    memory[descriptor_address + 0x14u] = 0x40u;
    ExecuteReadCdCommand();
    assert(raw_range_calls == 1u && raw_range_start == 1150);
    assert(raw_range_end == 1160 && raw_range_format == 4u);
    assert(bits[0x10] == 1u && ReadDword(0x15eau) == 1160ul);

    ResetState();
    bits[0x11] = 1u;
    format_transfer_result = 0;
    raw_range_result = 0u;
    memory[0x0b13] = 0xf8u;
    memory[descriptor_address + 0x14u] = 0x40u;
    ExecuteReadCdCommand();
    assert(bits[0x10] == 0u && ReadDword(0x15eau) == 0ul);
}

int main(void)
{
    TestValidationAndZeroCount();
    TestOrdinarySetupAndFormatTransfer();
    TestNegativeModesAndCoverageFallback();
    TestContinuationAndTransferFailure();
    TestDescriptorFailures();
    TestF8FaContinuationGate();
    puts("read CD engine tests passed");
    return 0;
}
