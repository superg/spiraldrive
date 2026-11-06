#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/synchronize_cache10.h"

static u8 memory[1u << 24];
static u8 validate_result;
static fw_addr_t validate_mask;
static u8 unsupported_count;
static u8 no_sense_count;
static u8 complete_count;
static u8 clear_bit30_count;
static u8 clear_bit35_count;
static u8 close_operation_count;
static u8 sequence_error_count;
static u8 finalize_working_count;
static u8 finalize_active_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveSynchronizeCacheValidate(fw_addr_t mask)
{
    validate_mask = mask;
    return validate_result;
}

void ObserveSynchronizeCacheUnsupported(void)
{
    unsupported_count++;
}

void ObserveSynchronizeCacheNoSense(void)
{
    no_sense_count++;
}

void ObserveSynchronizeCacheComplete(void)
{
    complete_count++;
}

void ObserveSynchronizeCacheClearBit30(void)
{
    clear_bit30_count++;
}

void ObserveSynchronizeCacheClearBit35(void)
{
    clear_bit35_count++;
}

void ObserveSynchronizeCacheCloseOperation(void)
{
    close_operation_count++;
}

void ObserveSynchronizeCacheSequenceError(void)
{
    sequence_error_count++;
}

void ObserveSynchronizeCacheFinalizeWorkingTrack(void)
{
    finalize_working_count++;
}

void ObserveSynchronizeCacheFinalizeActiveWrite(void)
{
    finalize_active_count++;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    validate_result = 1u;
    validate_mask = 0u;
    unsupported_count = 0u;
    no_sense_count = 0u;
    complete_count = 0u;
    clear_bit30_count = 0u;
    clear_bit35_count = 0u;
    close_operation_count = 0u;
    sequence_error_count = 0u;
    finalize_working_count = 0u;
    finalize_active_count = 0u;
}

static void TestValidationAndIdleFastPath(void)
{
    ResetState();
    validate_result = 0u;
    memory[0x000c84u] = 0x88u;
    HandleSynchronizeCache10Command();
    assert(validate_mask == 0xff8da7ul);
    assert(unsupported_count == 1u);
    assert(clear_bit30_count == 0u && finalize_active_count == 0u);

    ResetState();
    memory[0x000c84u] = 0u;
    memory[0x000c85u] = 0xa5u;
    HandleSynchronizeCache10Command();
    assert(validate_mask == 0xff8da7ul);
    assert(no_sense_count == 1u);
    assert(memory[0x000c85u] == 0xa5u);
    assert(finalize_active_count == 0u);
}

static void TestNegativeStateCloseWithImmediateCompletion(void)
{
    ResetState();
    memory[0x000c84u] = 0x88u;
    memory[0x000c85u] = 0xaau;
    memory[0x000b0bu] = 2u;
    memory[0x000c44u] = 1u;

    HandleSynchronizeCache10Command();

    assert(complete_count == 1u);
    assert(clear_bit30_count == 1u && clear_bit35_count == 1u);
    assert(close_operation_count == 1u);
    assert(sequence_error_count == 0u && finalize_working_count == 0u);
    assert(finalize_active_count == 0u);
    assert(memory[0x000c85u] == 0xaau);
    assert(memory[0x000c84u] == 0u);
}

static void TestNegativeStateWriteTypeRoutes(void)
{
    u8 write_type;

    for(write_type = 2u; write_type <= 3u; write_type++)
    {
        ResetState();
        memory[0x000c84u] = 0x80u;
        memory[0x000c44u] = write_type;
        HandleSynchronizeCache10Command();
        assert(sequence_error_count == 1u);
        assert(finalize_working_count == 0u);
        assert(clear_bit30_count == 1u && clear_bit35_count == 1u);
    }

    ResetState();
    memory[0x000c84u] = 0x80u;
    memory[0x000c44u] = 0u;
    HandleSynchronizeCache10Command();
    assert(sequence_error_count == 0u);
    assert(finalize_working_count == 1u);
}

static void TestActiveWriteTypeSpeedThreshold(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 2u;
    memory[0x000c39u] = 0x34u;
    memory[0x001592u] = 0x50u;
    HandleSynchronizeCache10Command();
    assert(finalize_active_count == 1u);
    assert(memory[0x000c38u] == 0u && memory[0x000e47u] == 0u);

    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 3u;
    memory[0x000c39u] = 0x56u;
    memory[0x001592u] = 0x51u;
    HandleSynchronizeCache10Command();
    assert(finalize_active_count == 1u);
    assert(memory[0x000c38u] == 0x56u && memory[0x000e47u] == 0x56u);
}

static void TestActiveOrdinaryWriteLeadoutThreshold(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 1u;
    memory[0x000c39u] = 0x67u;
    WriteU32(0x001153u, 0x00058fd3ul);
    HandleSynchronizeCache10Command();
    assert(memory[0x000c38u] == 0u && memory[0x000e47u] == 0u);

    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 1u;
    memory[0x000c39u] = 0x78u;
    WriteU32(0x001153u, 0x00058fd4ul);
    HandleSynchronizeCache10Command();
    assert(memory[0x000c38u] == 0x78u && memory[0x000e47u] == 0x78u);

    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 1u;
    memory[0x000c39u] = 0x89u;
    WriteU32(0x001153u, 0xfffffffful);
    HandleSynchronizeCache10Command();
    assert(memory[0x000c38u] == 0u && memory[0x000e47u] == 0u);
}

static void PrepareCountdownRoute(u8 state)
{
    memory[0x000c84u] = 0x80u;
    memory[0x000c44u] = 0xffu;
    memory[0x000c9eu] = state;
}

static void TestCountdownRoutes(void)
{
    ResetState();
    PrepareCountdownRoute(1u);
    HandleSynchronizeCache10Command();
    assert(memory[0x000c89u] == 8u);
    assert(memory[0x00122bu] == 20u);

    ResetState();
    PrepareCountdownRoute(2u);
    memory[0x00e750u] = 1u;
    HandleSynchronizeCache10Command();
    assert(memory[0x000c89u] == 8u);
    assert(memory[0x00122bu] == 60u);

    ResetState();
    PrepareCountdownRoute(1u);
    memory[0x2ba5afu] = 1u;
    memory[0x000c89u] = 0x80u;
    HandleSynchronizeCache10Command();
    assert(memory[0x000c8au] == 1u);
    assert(memory[0x00122cu] == 10u);

    ResetState();
    PrepareCountdownRoute(3u);
    memory[0x2ba5b1u] = 1u;
    memory[0x000c89u] = 0x80u;
    HandleSynchronizeCache10Command();
    assert(memory[0x000c8au] == 1u);
    assert(memory[0x00122cu] == 10u);
}

int main(void)
{
    TestValidationAndIdleFastPath();
    TestNegativeStateCloseWithImmediateCompletion();
    TestNegativeStateWriteTypeRoutes();
    TestActiveWriteTypeSpeedThreshold();
    TestActiveOrdinaryWriteLeadoutThreshold();
    TestCountdownRoutes();
    return 0;
}
