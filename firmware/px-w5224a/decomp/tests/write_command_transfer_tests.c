#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_command_transfer.h"

#define MEMORY_SIZE (1u << 24)

static u8 memory[MEMORY_SIZE];
static unsigned normalize_count;
static unsigned reset_count;
static unsigned initialize_count;
static u32 initialized_entry;
static unsigned yield_count;
static unsigned sense_status_count;
static u8 sense_status;
static unsigned unsupported_count;
static u32 mapped_result;
static unsigned map_count;
static s32 expanded_result;
static unsigned expand_count;
static u8 opc_result;
static unsigned opc_count;
static u8 prepare_result;
static unsigned prepare_count;
static s32 prepared_start;
static u8 range_result;
static unsigned range_count;
static u32 range_blocks;
static unsigned readback_count;
static u8 pipeline_result;
static unsigned pipeline_count;
static u8 pipeline_initialize;
static u8 lowest_speed;
static unsigned failure_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void NormalizeWriteSpeedForCurrentMedia(void)
{
    ++normalize_count;
}
void ResetRawReadQueueAndTransferState(void)
{
    ++reset_count;
}
void InitializeCoroutine2(u32 entry_point)
{
    ++initialize_count;
    initialized_entry = entry_point;
}
void YieldCoroutine0(void)
{
    ++yield_count;
}
void SetSenseDataFromStatusCode(u8 status)
{
    ++sense_status_count;
    sense_status = status;
}
void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_count;
}
u32 MapFixedPacketLogicalToPhysicalAddress(u32 address)
{
    ++map_count;
    (void)address;
    return mapped_result;
}
s32 ExpandWriteLogicalAddressForRecordOverhead(s32 address)
{
    ++expand_count;
    (void)address;
    return expanded_result;
}
u8 EnsureOpcForSelectedWriteSpeed(void)
{
    ++opc_count;
    return opc_result;
}
u8 PrepareWritePipelineForStartAddress(s32 start_address)
{
    ++prepare_count;
    prepared_start = start_address;
    return prepare_result;
}
u8 ExecuteWriteTransferBlockRange(u32 block_count)
{
    ++range_count;
    range_blocks = block_count;
    return range_result;
}
void ExecuteWriteAndVerifyReadback(void)
{
    ++readback_count;
}
u8 ExecuteWriteCommandTransferPipeline(u8 initialize_transfer)
{
    ++pipeline_count;
    pipeline_initialize = initialize_transfer;
    return pipeline_result;
}
u8 SelectStateEligibleLowestWriteSpeed(void)
{
    return lowest_speed;
}
void UpdateWorkingTrackAfterWriteFailure(void)
{
    ++failure_count;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    normalize_count = reset_count = initialize_count = yield_count = 0u;
    sense_status_count = unsupported_count = map_count = expand_count = 0u;
    opc_count = prepare_count = range_count = readback_count = 0u;
    pipeline_count = failure_count = 0u;
    initialized_entry = 0ul;
    sense_status = 0u;
    mapped_result = 0x200ul;
    expanded_result = 0x300;
    opc_result = prepare_result = range_result = pipeline_result = 1u;
    prepared_start = 0;
    range_blocks = 0ul;
    pipeline_initialize = 0u;
    lowest_speed = 8u;
    WriteU32(0x002ba529ul, 0x10000ul);
    memory[0x000c46ul] = 1u;
    memory[0x000e51ul] = 7u;
    memory[0x000e47ul] = 9u;
}

static void TestInitialSuccess(void)
{
    ResetState();
    memory[0x000c2bul] = 3u;
    memory[0x018050ul] = 1u;
    memory[0x000faful] = 2u;
    assert(StartOrContinueWriteCommandTransfer(100u, 4u) == 1u);
    assert(normalize_count == 1u && reset_count == 1u);
    assert(initialize_count == 1u && initialized_entry == 0x00f84a00ul);
    assert(yield_count == 1u);
    assert(memory[0x000c86ul] == 0x80u);
    assert(memory[0x000e3ful] == 1u && memory[0x000faful] == 3u);
    assert(expand_count == 1u && ReadU32(0x000b02ul) == 0x300ul);
    assert(prepare_count == 1u && prepared_start == 100);
    assert((memory[0x00fb69ul] & 1u) != 0u);
    assert(ReadU32(0x000b06ul) == 4u);
    assert(ReadU32(0x002bc703ul) == 100u);
    assert(pipeline_count == 1u && pipeline_initialize == 1u);
    assert(memory[0x000c7cul] == 7u);
    assert(memory[0x000c7aul] == 8u);
    assert(memory[0x000c7bul] == 9u);
    assert(memory[0x00122dul] == 5u);
}

static void TestInitialFailures(void)
{
    ResetState();
    memory[0x000faeul] = 3u;
    assert(StartOrContinueWriteCommandTransfer(1u, 1u) == 0u);
    assert(sense_status_count == 1u && sense_status == 3u);
    assert((memory[0x000c84ul] & 8u) == 0u);
    assert(prepare_count == 0u && pipeline_count == 0u);

    ResetState();
    memory[0x000c88ul] = 0x80u;
    mapped_result = 0x4e0u;
    assert(StartOrContinueWriteCommandTransfer(1u, 1u) == 0u);
    assert(unsupported_count == 1u);
    assert(prepare_count == 0u);
}

static void TestContinuationAdmissionAndSkip(void)
{
    ResetState();
    memory[0x000c84ul] = 0x18u;
    assert(StartOrContinueWriteCommandTransfer(10u, 0u) == 0u);
    assert(unsupported_count == 1u);

    ResetState();
    memory[0x000c84ul] = 0x18u;
    WriteU32(0x002bc703ul, 100u);
    WriteU32(0x000b06ul, 4u);
    assert(StartOrContinueWriteCommandTransfer(104u, 2u) == 1u);
    assert(range_count == 0u && prepare_count == 0u);
    assert(expand_count == 1u);
    assert(pipeline_initialize == 0u);
    assert(ReadU32(0x000b06ul) == 2u);
    assert(ReadU32(0x002bc703ul) == 104u);
}

static void TestContinuationFlushAndVerify(void)
{
    ResetState();
    memory[0x000c84ul] = 0x18u;
    memory[0x000b0aul] = 0x2eu;
    WriteU32(0x002bc703ul, 100u);
    WriteU32(0x000b06ul, 4u);
    assert(StartOrContinueWriteCommandTransfer(200u, 3u) == 1u);
    assert(range_count == 1u && range_blocks == 4u);
    assert(readback_count == 1u);
    assert((memory[0x000c8aul] & 0x40u) != 0u);
    assert(ReadU32(0x002bc707ul) == 200u);
    assert(prepare_count == 1u && prepared_start == 200);
    assert(pipeline_initialize == 1u);
}

static void TestPipelineFailure(void)
{
    ResetState();
    pipeline_result = 0u;
    memory[0x000c85ul] = 0xffu;
    assert(StartOrContinueWriteCommandTransfer(100u, 4u) == 0u);
    assert(failure_count == 1u);
    assert(memory[0x000c7cul] == 7u);
    assert(memory[0x000c7aul] == 8u);
    assert(memory[0x000c7bul] == 9u);
    assert((memory[0x000c84ul] & 8u) == 0u);
    assert((memory[0x000c85ul] & 4u) == 0u);
}

static void TestSuccessStateRouting(void)
{
    ResetState();
    memory[0x000c9eul] = 2u;
    memory[0x00e750ul] = 1u;
    assert(StartOrContinueWriteCommandTransfer(100u, 4u) == 1u);
    assert((memory[0x000c89ul] & 8u) != 0u);
    assert(memory[0x00122bul] == 60u);

    ResetState();
    memory[0x000c9eul] = 2u;
    memory[0x002ba5aful] = 1u;
    WriteU32(0x002ba59ful, 99u);
    WriteU32(0x001598ul, 100u);
    assert(StartOrContinueWriteCommandTransfer(100u, 4u) == 1u);
    assert(memory[0x002ba5aful] == 0u);
    assert(memory[0x000c9eul] == 2u);
    assert(memory[0x00122bul] == 20u);

    ResetState();
    memory[0x000c9eul] = 3u;
    memory[0x000c89ul] = 0x80u;
    assert(StartOrContinueWriteCommandTransfer(100u, 4u) == 1u);
    assert((memory[0x000c8aul] & 1u) != 0u);
    assert(memory[0x00122cul] == 20u);
}

int main(void)
{
    TestInitialSuccess();
    TestInitialFailures();
    TestContinuationAdmissionAndSkip();
    TestContinuationFlushAndVerify();
    TestPipelineFailure();
    TestSuccessStateRouting();
    return 0;
}
