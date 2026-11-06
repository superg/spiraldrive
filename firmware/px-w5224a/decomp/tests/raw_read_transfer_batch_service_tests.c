#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/raw_read_transfer_batch_service.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x1000000];
static u8 active_results[32], active_count, active_index;
static u8 should_results[32], should_count, should_index;
static u8 release_results[16], release_count, release_index;
static u8 bit28, bit36, recover_result, fail_result;
static u8 prepare_result, compare_result, decode_result, finalize_result;
static u8 advance_packet_result;
static u8 configure_activate_failure;
static u16 computed_count;
static u32 resume_address;
static u8 yield_calls, restart_calls, recover_calls, fail_calls;
static u8 prepare_calls, service_host_calls, sequence_error_calls;
static u8 build_tdi_calls, advance_track_calls, refresh_isrc_calls;
static u8 decode_calls, resume_calls, configure_calls, enqueue_calls;
static u8 finalize_calls, advance_packet_calls, queue_sense_calls;
static u8 set_sense_calls, sense_key;
static u16 sense_code;
static u32 sense_information;
static fw_addr_t configure_address, finalize_address;
static u16 configure_count, finalize_record;
static u8 finalize_completed;

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_image[address] = (u8)(value >> 8);
    memory_image[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(active_results, 0, sizeof(active_results));
    memset(should_results, 0, sizeof(should_results));
    memset(release_results, 0, sizeof(release_results));
    active_count = active_index = should_count = should_index = 0u;
    release_count = release_index = 0u;
    bit28 = bit36 = 0u;
    recover_result = prepare_result = finalize_result = 1u;
    fail_result = compare_result = decode_result = 0u;
    advance_packet_result = 1u;
    configure_activate_failure = 0u;
    computed_count = 3u;
    resume_address = 0x00112233ul;
    yield_calls = restart_calls = recover_calls = fail_calls = 0u;
    prepare_calls = service_host_calls = sequence_error_calls = 0u;
    build_tdi_calls = advance_track_calls = refresh_isrc_calls = 0u;
    decode_calls = resume_calls = configure_calls = enqueue_calls = 0u;
    finalize_calls = advance_packet_calls = queue_sense_calls = 0u;
    set_sense_calls = sense_key = 0u;
    sense_code = 0u;
    sense_information = 0u;
    configure_address = finalize_address = 0u;
    configure_count = finalize_record = 0u;
    finalize_completed = 0u;
    release_results[0] = 1u;
    release_count = 1u;
    WriteU16(0x1292u, 1u);
    WriteU16(0x129cu, 0x20u);
    WriteU16(0x146du, 2352u);
    WriteU32(0x126eu, 0x00445566ul);
    WriteU32(0x1455u, 3u);
    WriteU32(0x1459u, 0u);
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x1462ul && active_index < active_count)
        return active_results[active_index++];
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

u8 ObserveTransferBatchReadBit0028(void)
{
    return bit28;
}
u8 ObserveTransferBatchReadBit0036(void)
{
    return bit36;
}
void ObserveTransferBatchYield(void)
{
    ++yield_calls;
}
void ObserveTransferBatchSetSense(u8 key, u16 code)
{
    ++set_sense_calls;
    sense_key = key;
    sense_code = code;
}
void ObserveTransferBatchSetInformation(u32 information)
{
    sense_information = information;
}
u8 ObserveTransferBatchShouldWait(void)
{
    if(should_index < should_count)
        return should_results[should_index++];
    return 0u;
}
u8 ObserveTransferBatchTryRecover(void)
{
    ++recover_calls;
    return recover_result;
}
u8 ObserveTransferBatchFailPreparation(void)
{
    ++fail_calls;
    return fail_result;
}
void ObserveTransferBatchRestartProducer(void)
{
    ++restart_calls;
}
u8 ObserveTransferBatchTryRelease(void)
{
    if(release_index < release_count)
        return release_results[release_index++];
    return 1u;
}
u16 ObserveTransferBatchComputeCount(void)
{
    return computed_count;
}
u8 ObserveTransferBatchPrepareFixedPacket(void)
{
    ++prepare_calls;
    return prepare_result;
}
void ObserveTransferBatchServiceHost(void)
{
    ++service_host_calls;
}
u8 ObserveTransferBatchComparePosition(s32 position, u16 record_index)
{
    assert(position == (s32)ReadU32(0x1598u));
    assert(record_index == ReadU16(0x149bu));
    return compare_result;
}
void ObserveTransferBatchSequenceError(void)
{
    ++sequence_error_calls;
}
void ObserveTransferBatchBuildTdi(fw_addr_t address)
{
    ++build_tdi_calls;
    assert(address == 0x143cul);
}
void ObserveTransferBatchAdvanceTrack(void)
{
    ++advance_track_calls;
    WriteU16(0x149bu, 4u);
}
void ObserveTransferBatchRefreshIsrc(void)
{
    ++refresh_isrc_calls;
}
u8 ObserveTransferBatchDecodeQType(u16 record_index)
{
    ++decode_calls;
    assert(record_index == 2u);
    return decode_result;
}
fw_addr_t ObserveTransferBatchResumeRing(void)
{
    ++resume_calls;
    return resume_address;
}
void ObserveTransferBatchConfigure(fw_addr_t fallback, u16 count)
{
    ++configure_calls;
    configure_address = fallback;
    configure_count = count;
    WriteU32(0x1459u, ReadU32(0x1459u) + count);
    if(configure_activate_failure != 0u)
    {
        memory_image[0x127du] = 1u;
        memory_image[0xfb9au] = 1u;
    }
}
void ObserveTransferBatchEnqueue(u16 record_index)
{
    ++enqueue_calls;
    assert(record_index == finalize_record);
}
u8 ObserveTransferBatchFinalize(fw_addr_t address, u16 record_index, u8 completed_count)
{
    ++finalize_calls;
    finalize_address = address;
    finalize_record = record_index;
    finalize_completed = completed_count;
    return finalize_result;
}
u8 ObserveTransferBatchAdvancePacket(void)
{
    ++advance_packet_calls;
    return advance_packet_result;
}
void ObserveTransferBatchQueueSense(void)
{
    ++queue_sense_calls;
}

static void TestLatchedFailureMapping(void)
{
    ResetHarness();
    memory_image[0xfaeu] = 3u;
    active_results[0] = 1u;
    active_results[1] = 0u;
    active_count = 2u;
    memory_image[0xf84457u] = 4u;
    WriteU16(0xf8446eu, 0x1234u);
    WriteU32(0x0b02u, 0x1000ul);
    WriteU32(0x1459u, 7ul);
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(yield_calls == 1u && set_sense_calls == 1u);
    assert(sense_key == 4u && sense_code == 0x1234u);
    assert(sense_information == 0x1007ul);
}

static void TestInitialProducerWaitRoutes(void)
{
    ResetHarness();
    should_results[0] = 1u;
    should_count = 1u;
    assert(ServiceRawReadTransferBatch() == 0xfeu);
    assert(restart_calls == 1u);

    ResetHarness();
    should_results[0] = 1u;
    should_count = 1u;
    memory_image[0x127du] = 1u;
    recover_result = 0u;
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(recover_calls == 1u && fail_calls == 0u);

    ResetHarness();
    should_results[0] = 1u;
    should_count = 1u;
    memory_image[0x127du] = 1u;
    memory_image[0xfb9au] = 1u;
    fail_result = 1u;
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(recover_calls == 0u && fail_calls == 1u);
}

static void TestReleaseAndCapacityGates(void)
{
    ResetHarness();
    memory_image[0x127du] = 1u;
    fail_result = 1u;
    assert(ServiceRawReadTransferBatch() == 0u);

    ResetHarness();
    release_results[0] = 0u;
    assert(ServiceRawReadTransferBatch() == 0xfeu);

    ResetHarness();
    memory_image[0xfb9au] = 1u;
    WriteU16(0x1463u, 2u);
    WriteU16(0x1292u, 0x1eu);
    assert(ServiceRawReadTransferBatch() == 0xfeu);

    ResetHarness();
    memory_image[0xfb9au] = 1u;
    bit28 = bit36 = 1u;
    assert(ServiceRawReadTransferBatch() == 0xfeu);
}

static void TestFixedPacketPreparation(void)
{
    ResetHarness();
    memory_image[0x0c46u] = 2u;
    prepare_result = 0u;
    assert(ServiceRawReadTransferBatch() == 0u);

    ResetHarness();
    memory_image[0x0c46u] = 2u;
    release_results[0] = 1u;
    release_results[1] = 0u;
    release_results[2] = 1u;
    release_count = 3u;
    should_results[0] = 0u;
    should_results[1] = 1u;
    should_results[2] = 0u;
    should_count = 3u;
    assert(ServiceRawReadTransferBatch() == 1u);
    assert(prepare_calls == 1u && restart_calls == 1u);
    assert(service_host_calls == 1u && yield_calls == 2u);
    assert(advance_packet_calls == 1u);
}

static void SeedType1Record(u16 index)
{
    memory_image[0xb4bdu + (u16)(index << 3)] = 1u;
}

static void TestModeTwoTrackRoutes(void)
{
    ResetHarness();
    memory_image[0x0c44u] = 2u;
    WriteU16(0x149bu, 3u);
    compare_result = 2u;
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(sequence_error_calls == 1u && resume_calls == 0u);

    ResetHarness();
    memory_image[0x0c44u] = 2u;
    WriteU16(0x149bu, 3u);
    compare_result = 0u;
    SeedType1Record(2u);
    assert(ServiceRawReadTransferBatch() == 1u);
    assert(build_tdi_calls == 1u && advance_track_calls == 1u);
    assert(refresh_isrc_calls == 1u && finalize_record == 2u);

    ResetHarness();
    memory_image[0x0c44u] = 2u;
    WriteU16(0x149bu, 3u);
    compare_result = 1u;
    decode_result = 8u;
    SeedType1Record(2u);
    assert(ServiceRawReadTransferBatch() == 1u);
    assert(decode_calls == 1u && memory_image[0x1285u] == 8u);
    assert(build_tdi_calls == 0u && finalize_record == 2u);
}

static void TestBatchAndActiveTransferRoutes(void)
{
    unsigned index;

    ResetHarness();
    computed_count = 0u;
    assert(ServiceRawReadTransferBatch() == 0xfeu);
    assert(resume_calls == 0u);

    ResetHarness();
    for(index = 0u; index < 6u; ++index)
        active_results[index] = 1u;
    active_results[6] = 0u;
    active_count = 7u;
    memory_image[0x18016u] = 1u;
    memory_image[0x18017u] = 0u;
    assert(ServiceRawReadTransferBatch() == 1u);
    assert(configure_calls == 1u && configure_address == resume_address && configure_count == 3u);
    assert(enqueue_calls == 1u && finalize_completed == 1u);
    assert(finalize_address == 0x00445566ul);

    ResetHarness();
    configure_activate_failure = 1u;
    fail_result = 1u;
    active_results[0] = 1u;
    active_results[1] = 1u;
    active_results[2] = 0u;
    active_count = 3u;
    should_results[0] = 0u;
    should_results[1] = 1u;
    should_count = 2u;
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(service_host_calls == 1u && yield_calls == 1u);
}

static void TestTerminalRoutes(void)
{
    ResetHarness();
    finalize_result = 0u;
    assert(ServiceRawReadTransferBatch() == 0u);

    ResetHarness();
    memory_image[0x0c46u] = 2u;
    advance_packet_result = 0u;
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(advance_packet_calls == 1u);

    ResetHarness();
    memory_image[0x0c83u] = 0x10u;
    assert(ServiceRawReadTransferBatch() == 0u);

    ResetHarness();
    WriteU16(0x1292u, 0u);
    assert(ServiceRawReadTransferBatch() == 0u);
    assert(queue_sense_calls == 1u);

    ResetHarness();
    WriteU32(0x1455u, 4u);
    assert(ServiceRawReadTransferBatch() == 0xfeu);

    ResetHarness();
    assert(ServiceRawReadTransferBatch() == 1u);
    assert(memory_image[0x1590u] == 2u);
}

int main(void)
{
    TestLatchedFailureMapping();
    TestInitialProducerWaitRoutes();
    TestReleaseAndCapacityGates();
    TestFixedPacketPreparation();
    TestModeTwoTrackRoutes();
    TestBatchAndActiveTransferRoutes();
    TestTerminalRoutes();
    puts("raw-read transfer-batch service tests passed");
    return 0;
}
