#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_toc_cd_text_acquisition.h"

#define MEMORY_SIZE 0x20000u
#define MAX_EVENTS 64u

static u8 memory[MEMORY_SIZE];
static u8 bits[0x600u];
static unsigned bit_writes[0x600u];
static s8 validation_results[MAX_EVENTS];
static unsigned validation_count;
static unsigned validation_index;
static u8 vary_response_length;
static unsigned reset_count;
static u32 coroutine_entries[MAX_EVENTS];
static unsigned coroutine_count;
static u8 acquisition_attempts[MAX_EVENTS];
static unsigned acquisition_count;
static unsigned yield_count;
static u16 descriptor_address;
static unsigned descriptor_count;
static unsigned empty_response_count;
static u16 empty_response_allocation;
static unsigned transfer_count;
static fw_addr_t transfer_address;
static u32 transfer_length;
static unsigned no_sense_count;
static unsigned dispatch_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
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
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

#define DEFINE_BIT_WRITER(name, address)                                                                                                                                                               \
    void name(u8 value)                                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        bits[(address)] = (u8)(value != 0u);                                                                                                                                                           \
        bit_writes[(address)]++;                                                                                                                                                                       \
    }

DEFINE_BIT_WRITER(FirmwareWriteBit0013, 0x0013u)
DEFINE_BIT_WRITER(FirmwareWriteBit001E, 0x001eu)
DEFINE_BIT_WRITER(FirmwareWriteBit0580, 0x0580u)
DEFINE_BIT_WRITER(FirmwareWriteBit0581, 0x0581u)

void ResetRawReadQueueAndTransferState(void)
{
    reset_count++;
}

void InitializeCoroutine2(u32 entry_point)
{
    assert(coroutine_count < MAX_EVENTS);
    coroutine_entries[coroutine_count++] = entry_point;
}

void InitializeCdTextRawReadAcquisition(u8 start_acquisition)
{
    assert(acquisition_count < MAX_EVENTS);
    acquisition_attempts[acquisition_count++] = start_acquisition;
}

void YieldCoroutine0(void)
{
    yield_count++;
}

s8 ValidateLatestCdTextRawSectorPackBatch(void)
{
    s8 result;

    assert(validation_index < validation_count);
    result = validation_results[validation_index++];
    if(vary_response_length != 0u && result == 1)
        WriteU16(0x000f50ul, (u16)(5u + validation_index));
    return result;
}

u16 GetLatestQueuedRawSectorDescriptorAddress(void)
{
    descriptor_count++;
    return descriptor_address;
}

void ReturnEmptyCdTextResponse(u16 allocation_length)
{
    empty_response_count++;
    empty_response_allocation = allocation_length;
}

void ExecuteControllerDataInTransfer(fw_addr_t source_address, u32 byte_count)
{
    transfer_count++;
    transfer_address = source_address;
    transfer_length = byte_count;
}

void SetNoSenseData(void)
{
    no_sense_count++;
}

void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    dispatch_count++;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 0xff, sizeof(bits));
    memset(bit_writes, 0, sizeof(bit_writes));
    memset(validation_results, 0, sizeof(validation_results));
    validation_count = 0u;
    validation_index = 0u;
    vary_response_length = 0u;
    reset_count = 0u;
    coroutine_count = 0u;
    acquisition_count = 0u;
    yield_count = 0u;
    descriptor_address = 0x1300u;
    descriptor_count = 0u;
    empty_response_count = 0u;
    empty_response_allocation = 0u;
    transfer_count = 0u;
    transfer_address = 0u;
    transfer_length = 0u;
    no_sense_count = 0u;
    dispatch_count = 0u;
    memory[0x1008u] = 1u;
    memory[0x12c9u] = 1u;
    WriteU32(0x1556u, 0x00123450ul);
}

static void AssertNoAllowedPathEffects(u16 allocation_length)
{
    assert(empty_response_count == 1u);
    assert(empty_response_allocation == allocation_length);
    assert(reset_count == 0u && coroutine_count == 0u);
    assert(acquisition_count == 0u && dispatch_count == 0u);
    assert(transfer_count == 0u && no_sense_count == 0u);
    assert(bit_writes[0x0580u] == 0u && bit_writes[0x0581u] == 0u);
}

static void test_admission_rejections_use_only_empty_response(void)
{
    ResetFixture();
    memory[0x1008u] = 0u;
    memory[0x12beu] = 2u;
    ExecuteReadTocFormat5CdTextResponse(0x1234u);
    AssertNoAllowedPathEffects(0x1234u);

    ResetFixture();
    memory[0x12c9u] = 0u;
    memory[0x1149u] = 0u;
    ExecuteReadTocFormat5CdTextResponse(7u);
    AssertNoAllowedPathEffects(7u);

    ResetFixture();
    memory[0x0c9eu] = 1u;
    ExecuteReadTocFormat5CdTextResponse(4u);
    AssertNoAllowedPathEffects(4u);
}

static void test_cached_response_is_clipped_and_cleanup_runs(void)
{
    ResetFixture();
    memory[0x0c2au] = 10u;
    WriteU16(0x0f50u, 0x20u);

    ExecuteReadTocFormat5CdTextResponse(0x10u);

    assert(transfer_count == 1u && transfer_length == 0x10u);
    assert(transfer_address == 0x0011a450ul);
    assert(no_sense_count == 1u && empty_response_count == 0u);
    assert(reset_count == 1u && coroutine_count == 1u);
    assert(coroutine_entries[0] == 0x00f84a00ul);
    assert(dispatch_count == 1u && ReadU32(0x1598u) == 150ul);
    assert(memory[0x0f06u] == 0u && memory[0x1226u] == 4u);
    assert(bits[0x0013u] == 0u);
    assert(bits[0x0580u] == 1u && bits[0x0581u] == 1u);
    assert(bit_writes[0x0580u] == 2u && bit_writes[0x0581u] == 2u);
}

static void test_complete_pack_set_transfers_after_polling(void)
{
    unsigned index;

    ResetFixture();
    WriteU16(0x0f50u, 40u);
    WriteU16(0xf798u, 1u);
    WriteU16(0xf796u, 1u);
    validation_results[0] = (s8)-2;
    validation_results[1] = 1;
    validation_count = 2u;

    ExecuteReadTocFormat5CdTextResponse(30u);

    assert(validation_index == 2u && yield_count == 2u);
    assert(acquisition_count == 1u && acquisition_attempts[0] == 1u);
    assert(reset_count == 2u && coroutine_count == 3u);
    assert(coroutine_entries[0] == 0x00f84a00ul);
    assert(coroutine_entries[1] == 0x00f929abul);
    assert(coroutine_entries[2] == 0x00f84a00ul);
    assert(transfer_count == 1u && transfer_length == 30u);
    assert(memory[0x0c2au] == 10u && no_sense_count == 1u);
    assert(memory[0x15eeu] == 1u && bits[0x001eu] == 0u);
    for(index = 0u; index < 5u; ++index)
        assert(ReadU16(0xf58cu + index * 2u) == 0xffffu);
}

static void test_stable_length_is_accepted_on_attempt_eight(void)
{
    unsigned index;

    ResetFixture();
    WriteU16(0x0f50u, 5u);
    for(index = 0u; index < 8u; ++index)
        validation_results[index] = 1;
    validation_count = 8u;

    ExecuteReadTocFormat5CdTextResponse(0xffffu);

    assert(acquisition_count == 8u && validation_index == 8u);
    assert(memory[0x15eeu] == 8u);
    assert(transfer_count == 1u && transfer_length == 5u);
    assert(reset_count == 9u);
}

static void test_changing_length_is_accepted_on_attempt_thirteen(void)
{
    unsigned index;

    ResetFixture();
    for(index = 0u; index < 13u; ++index)
        validation_results[index] = 1;
    validation_count = 13u;
    vary_response_length = 1u;

    ExecuteReadTocFormat5CdTextResponse(0xffffu);

    assert(acquisition_count == 13u && validation_index == 13u);
    assert(memory[0x15eeu] == 13u);
    assert(transfer_count == 1u && transfer_length == 18u);
}

static void test_three_nonzero_error_descriptors_return_empty_response(void)
{
    ResetFixture();
    WriteU16((u16)(descriptor_address + 0x0eu), 1u);
    validation_results[0] = 0x63;
    validation_results[1] = 0x63;
    validation_results[2] = 0x63;
    validation_count = 3u;

    ExecuteReadTocFormat5CdTextResponse(0x55aau);

    assert(acquisition_count == 3u && descriptor_count == 3u);
    assert(memory[0x15eeu] == 3u);
    assert(empty_response_count == 1u);
    assert(empty_response_allocation == 0x55aau);
    assert(transfer_count == 0u && reset_count == 4u);
    assert(ReadU32(0x0f52u) == 0u);
}

static void test_full_toc_error_descriptor_stops_immediately(void)
{
    ResetFixture();
    memory[0x12beu] = 2u;
    WriteU16((u16)(descriptor_address + 0x0eu), 1u);
    validation_results[0] = 0x63;
    validation_count = 1u;

    ExecuteReadTocFormat5CdTextResponse(9u);

    assert(acquisition_count == 1u && descriptor_count == 1u);
    assert(empty_response_count == 1u && transfer_count == 0u);
}

static void test_zero_error_descriptors_reach_exact_retry_boundary(void)
{
    unsigned index;

    ResetFixture();
    for(index = 0u; index < 14u; ++index)
        validation_results[index] = 0x63;
    validation_count = 14u;

    ExecuteReadTocFormat5CdTextResponse(4u);

    assert(acquisition_count == 14u && validation_index == 14u);
    assert(descriptor_count == 14u && memory[0x15eeu] == 15u);
    assert(empty_response_count == 1u && transfer_count == 0u);
    assert(reset_count == 15u && ReadU32(0x0f52u) == 0u);
}

static void test_ordinary_failure_stops_on_second_attempt(void)
{
    ResetFixture();
    validation_results[0] = 0;
    validation_results[1] = 0;
    validation_count = 2u;

    ExecuteReadTocFormat5CdTextResponse(4u);

    assert(acquisition_count == 2u && validation_index == 2u);
    assert(memory[0x15eeu] == 2u);
    assert(empty_response_count == 0u && transfer_count == 0u);
    assert(reset_count == 3u && dispatch_count == 1u);
}

int main(void)
{
    test_admission_rejections_use_only_empty_response();
    test_cached_response_is_clipped_and_cleanup_runs();
    test_complete_pack_set_transfers_after_polling();
    test_stable_length_is_accepted_on_attempt_eight();
    test_changing_length_is_accepted_on_attempt_thirteen();
    test_three_nonzero_error_descriptors_return_empty_response();
    test_full_toc_error_descriptor_stops_immediately();
    test_zero_error_descriptors_reach_exact_retry_boundary();
    test_ordinary_failure_stops_on_second_attempt();
    puts("read TOC CD-TEXT acquisition tests passed");
    return 0;
}
