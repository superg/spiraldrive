#include <assert.h>
#include <string.h>
#include "media/pma_acquisition.h"
#include "platform/firmware_memory.h"

#define RESULT_CAPACITY 32u

static u8 memory[1u << 24];
static u8 acquire_results[RESULT_CAPACITY];
static u8 reset_results[RESULT_CAPACITY];
static u8 parse_results[RESULT_CAPACITY];
static u8 process_results[RESULT_CAPACITY];
static u8 accumulate_results[RESULT_CAPACITY];
static u8 pulse_results[RESULT_CAPACITY];
static u8 complete_results[RESULT_CAPACITY];
static unsigned acquire_count, acquire_index;
static unsigned reset_count, reset_index;
static unsigned parse_count, parse_index;
static unsigned process_count, process_index;
static unsigned accumulate_count, accumulate_index;
static unsigned pulse_count, pulse_index;
static unsigned complete_count, complete_index;
static unsigned reset_tables_calls;
static unsigned operation_calls;
static unsigned apply_calls;
static unsigned arm_calls;
static unsigned rearm_calls;
static unsigned wait_calls;
static unsigned finalize_calls;
static unsigned yield_calls;
static unsigned reset_drive_calls;
static u8 operation_result;
static u16 divide_result;
static u16 waited_quotient;
static u8 expire_budget_after_acquire;
static u8 expire_budget_after_window_reset;
static fw_addr_t convert_addresses[4];
static s32 convert_results[4];
static unsigned convert_count, convert_index;
static fw_addr_t copy_sources[8];
static fw_addr_t copy_destinations[8];
static u16 copy_counts[8];
static unsigned copy_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadBe32(fw_addr_t address)
{
    return (u32)memory[address] << 24 | (u32)memory[address + 1u] << 16 | (u32)memory[address + 2u] << 8 | memory[address + 3u];
}

void ObservePmaResetTables(void)
{
    reset_tables_calls++;
}
u8 ObservePmaOperationInTocSet(void)
{
    operation_calls++;
    return operation_result;
}
u8 ObservePmaAcquirePosition(void)
{
    assert(acquire_index < acquire_count);
    if(expire_budget_after_acquire != 0u)
        memory[0x001203u] = 0u;
    return acquire_results[acquire_index++];
}
u8 ObservePmaResetAcquisition(u8 reinitialize_controller)
{
    assert(reset_index < reset_count);
    if(reinitialize_controller == 0u && expire_budget_after_window_reset != 0u)
        memory[0x001203u] = 0u;
    return reset_results[reset_index++];
}
void ObservePmaApplyOperatingCode(void)
{
    apply_calls++;
}
void ObservePmaArmPacket(void)
{
    arm_calls++;
}
u8 ObservePmaParsePacket(void)
{
    assert(parse_index < parse_count);
    return parse_results[parse_index++];
}
void ObservePmaRearmPacket(void)
{
    rearm_calls++;
}
u8 ObservePmaProcessPacket(void)
{
    assert(process_index < process_count);
    return process_results[process_index++];
}
u8 ObservePmaAccumulatePacket(void)
{
    assert(accumulate_index < accumulate_count);
    return accumulate_results[accumulate_index++];
}
u16 ObservePmaDivideTimer(u16 dividend, u8 divisor)
{
    assert(dividend == 0x1733u && divisor == 4u);
    return divide_result;
}
void ObservePmaWaitTimer(u16 quotient)
{
    wait_calls++;
    waited_quotient = quotient;
}
u8 ObservePmaPulseThreshold(u8 limit)
{
    assert(limit == 10u && pulse_index < pulse_count);
    return pulse_results[pulse_index++];
}
u8 ObservePmaStateComplete(void)
{
    assert(complete_index < complete_count);
    return complete_results[complete_index++];
}
void ObservePmaFinalizeStart(void)
{
    finalize_calls++;
}
void ObservePmaYield(void)
{
    yield_calls++;
}
void ObservePmaResetDrive(void)
{
    reset_drive_calls++;
}
s32 ObservePmaConvertMsf(fw_addr_t tuple_address)
{
    assert(convert_index < convert_count);
    convert_addresses[convert_index] = tuple_address;
    return convert_results[convert_index++];
}
void ObservePmaCopyBytes(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    assert(copy_count < 8u);
    copy_sources[copy_count] = source;
    copy_destinations[copy_count] = destination;
    copy_counts[copy_count++] = byte_count;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(acquire_results, 0, sizeof(acquire_results));
    memset(reset_results, 0, sizeof(reset_results));
    memset(parse_results, 0, sizeof(parse_results));
    memset(process_results, 0, sizeof(process_results));
    memset(accumulate_results, 0, sizeof(accumulate_results));
    memset(pulse_results, 0, sizeof(pulse_results));
    memset(complete_results, 0, sizeof(complete_results));
    acquire_count = acquire_index = reset_count = reset_index = 0u;
    parse_count = parse_index = process_count = process_index = 0u;
    accumulate_count = accumulate_index = pulse_count = pulse_index = 0u;
    complete_count = complete_index = 0u;
    reset_tables_calls = operation_calls = apply_calls = arm_calls = 0u;
    rearm_calls = wait_calls = finalize_calls = yield_calls = 0u;
    reset_drive_calls = 0u;
    operation_result = expire_budget_after_acquire = 0u;
    expire_budget_after_window_reset = 0u;
    memset(convert_addresses, 0, sizeof(convert_addresses));
    memset(convert_results, 0, sizeof(convert_results));
    memset(copy_sources, 0, sizeof(copy_sources));
    memset(copy_destinations, 0, sizeof(copy_destinations));
    memset(copy_counts, 0, sizeof(copy_counts));
    convert_count = convert_index = copy_count = 0u;
    divide_result = 0x1234u;
    waited_quotient = 0u;
    WriteBe32(0x00114fu, 10000ul);
}

static void TestActiveOperationShortcut(void)
{
    ResetState();
    memory[0x001008u] = 2u;
    memory[0x001308u] = 1u;
    memory[0x00130au] = 13u;
    assert(AcquirePmaQRecordState() == 0u);
    assert(operation_calls == 0u && reset_tables_calls == 1u);
    assert(ReadBe32(0x001143u) == 9000ul);
    assert((memory[0x001132u] & 0x80u) != 0u);
    assert(ReadBe32(0x000e3bu) == 8985ul);

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x001308u] = 1u;
    memory[0x00130au] = 12u;
    operation_result = 1u;
    assert(AcquirePmaQRecordState() == 0u);
    assert(operation_calls == 1u);
}

static void TestExpiredBudgetRoutes(void)
{
    ResetState();
    acquire_results[0] = 0u;
    acquire_count = reset_count = 1u;
    reset_results[0] = 1u;
    expire_budget_after_acquire = 1u;
    assert(AcquirePmaQRecordState() == 0u);
    assert(reset_tables_calls == 2u && reset_drive_calls == 0u);
    assert(ReadBe32(0x001143u) == 9000ul);

    ResetState();
    acquire_results[0] = 0u;
    acquire_count = reset_count = 1u;
    reset_results[0] = 1u;
    expire_budget_after_acquire = 1u;
    memory[0x0012beu] = 4u;
    memory[0x00f553u] = 0u;
    memory[0x001008u] = 1u;
    assert(AcquirePmaQRecordState() == 4u);
    assert(reset_drive_calls == 1u && reset_tables_calls == 1u);
}

static void TestAcquisitionAndAccumulationFailures(void)
{
    ResetState();
    acquire_results[0] = 0u;
    acquire_count = reset_count = 1u;
    reset_results[0] = 0u;
    assert(AcquirePmaQRecordState() == 4u);

    ResetState();
    acquire_results[0] = 1u;
    acquire_count = 1u;
    parse_results[0] = 0u;
    parse_count = 1u;
    process_results[0] = 0u;
    process_count = 1u;
    accumulate_results[0] = 0u;
    accumulate_count = 1u;
    reset_results[0] = 0u;
    reset_count = 1u;
    WriteBe32(0x001235u, 8998ul);
    assert(AcquirePmaQRecordState() == 4u);
    assert(apply_calls == 1u && arm_calls == 2u && rearm_calls == 1u);
}

static void TestPmaCompletionAfterPulseFailures(void)
{
    unsigned i;

    ResetState();
    acquire_results[0] = 1u;
    acquire_count = 1u;
    parse_count = process_count = accumulate_count = pulse_count = 11u;
    for(i = 0u; i < 11u; ++i)
    {
        parse_results[i] = 0u;
        process_results[i] = 0u;
        accumulate_results[i] = 1u;
        pulse_results[i] = 0u;
    }
    complete_results[0] = 1u;
    complete_count = 1u;
    WriteBe32(0x001235u, 8998ul);
    assert(AcquirePmaQRecordState() == 0u);
    assert(wait_calls == 11u && waited_quotient == 0x1234u);
    assert(finalize_calls == 1u && complete_index == 1u);
    assert(ReadBe32(0x001143u) == 8998ul);
}

static void TestFrameWindowReset(void)
{
    ResetState();
    acquire_results[0] = 1u;
    acquire_count = 1u;
    parse_results[0] = 0u;
    parse_count = 1u;
    reset_results[0] = 1u;
    reset_count = 1u;
    expire_budget_after_window_reset = 1u;
    WriteBe32(0x001235u, 8960ul);
    assert(AcquirePmaQRecordState() == 0u);
    assert(reset_index == 1u && reset_tables_calls == 2u);
}

static void TestAccumulateAllAdrRecords(void)
{
    static const fw_addr_t expected_destinations[6] = { 0x001245ul, 0x002bb5dbul, 0x002bb441ul, 0x002bb51dul, 0x002bb603ul, 0x002bb79dul };
    unsigned i;

    ResetState();
    memory[0x00125bu] = 0x3fu;
    memory[0x001247u] = 3u;
    memory[0x001147u] = 0xffu;
    memory[0x001149u] = 2u;
    memory[0x001132u] = 0x40u;
    memory[0x002bb05cu] = 0x5fu;
    convert_results[0] = 100l;
    convert_results[1] = 100l;
    convert_count = 2u;

    assert(AccumulateCurrentQPacketIntoPmaState() == 1u);
    assert(convert_index == 2u);
    assert(convert_addresses[0] == 0x002bb060ul && convert_addresses[1] == 0x00124cul);
    assert(copy_count == 6u);
    assert(copy_sources[0] == 0x002bb059ul);
    for(i = 0u; i < 6u; ++i)
    {
        assert(copy_destinations[i] == expected_destinations[i]);
        assert(copy_counts[i] == 10u);
    }
    for(i = 1u; i < 6u; ++i)
        assert(copy_sources[i] == 0x001245ul);
    assert(memory[0x001147u] == 3u && memory[0x001148u] == 3u && memory[0x001149u] == 3u);
    assert(memory[0x00114au] == 3u && memory[0x00114bu] == 3u && memory[0x00114cu] == 3u && memory[0x00114du] == 3u);
    assert(memory[0x00114eu] == 1u && memory[0x00102eu] == 1u);
    assert((memory[0x0012beu] & 1u) != 0u);
}

static void TestAdr1CopyGateAndSummary(void)
{
    ResetState();
    memory[0x00125bu] = 1u;
    memory[0x001247u] = 2u;
    memory[0x001147u] = 1u;
    memory[0x001148u] = 9u;
    memory[0x001149u] = 9u;
    memory[0x001132u] = 0x40u;
    memory[0x001245u] = 0x40u;
    convert_results[0] = 101l;
    convert_results[1] = 100l;
    convert_count = 2u;

    assert(AccumulateCurrentQPacketIntoPmaState() == 1u);
    assert(copy_count == 0u);
    assert(memory[0x001147u] == 1u && memory[0x001148u] == 9u && memory[0x001149u] == 9u);
    assert((memory[0x0012beu] & 2u) != 0u);
    assert(memory[0x00102eu] == 0u);

    ResetState();
    assert(AccumulateCurrentQPacketIntoPmaState() == 1u);
    assert(convert_index == 0u && copy_count == 0u);
}

int main(void)
{
    TestActiveOperationShortcut();
    TestExpiredBudgetRoutes();
    TestAcquisitionAndAccumulationFailures();
    TestPmaCompletionAfterPulseFailures();
    TestFrameWindowReset();
    TestAccumulateAllAdrRecords();
    TestAdr1CopyGateAndSummary();
    return 0;
}
