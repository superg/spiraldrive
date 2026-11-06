#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/close_sync_raw_read_prepare.h"
#include "platform/firmware_memory.h"

static u8 memory_image[0x20000];
static u8 acquire_results[8], prefix_results[8], queue_results[8];
static u8 plan_results[8], sequence_results[8], opc_results[8];
static u8 acquire_count, acquire_index, prefix_count, prefix_index;
static u8 queue_count, queue_index, plan_count, plan_index;
static u8 sequence_count, sequence_index, opc_count, opc_index;
static u8 reset_count, sector_class, target_sense_count, sense_count;
static u8 last_sense_key;
static u16 last_sense_code, sequence_expected;
static s32 last_acquire_count, last_msf_relative, last_msf_absolute;
static s32 last_tuple_frame;
static u16 last_tuple_destination;

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_image[address] = (u8)(value >> 24);
    memory_image[address + 1u] = (u8)(value >> 16);
    memory_image[address + 2u] = (u8)(value >> 8);
    memory_image[address + 3u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_image[address] << 8) | memory_image[address + 1u]);
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_image[address] << 24) | ((u32)memory_image[address + 1u] << 16) | ((u32)memory_image[address + 2u] << 8) | memory_image[address + 3u];
}

static void ResetHarness(void)
{
    memset(memory_image, 0, sizeof(memory_image));
    memset(acquire_results, 0, sizeof(acquire_results));
    memset(prefix_results, 0, sizeof(prefix_results));
    memset(queue_results, 0, sizeof(queue_results));
    memset(plan_results, 0, sizeof(plan_results));
    memset(sequence_results, 0, sizeof(sequence_results));
    memset(opc_results, 0, sizeof(opc_results));
    acquire_count = acquire_index = prefix_count = prefix_index = 0u;
    queue_count = queue_index = plan_count = plan_index = 0u;
    sequence_count = sequence_index = opc_count = opc_index = 0u;
    reset_count = target_sense_count = sense_count = 0u;
    sector_class = 9u;
    last_sense_key = 0u;
    last_sense_code = sequence_expected = 0u;
    last_acquire_count = last_msf_relative = last_msf_absolute = 0l;
    last_tuple_frame = 0l;
    last_tuple_destination = 0u;
    WriteDword(0x156au, 32ul);
    WriteDword(0x1562u, 0x00123456ul);
    WriteDword(0xdd3du, 1000ul);
    WriteDword(0xdd41u, 2000ul);
    WriteDword(0xdd45u, 1100ul);
    WriteDword(0xdd4du, 1200ul);
    WriteDword(0xdd51u, 1100ul);
    memory_image[0xc48u] = 1u;
    memory_image[0xc44u] = 1u;
    memory_image[0xe47u] = 8u;
    memory_image[0xc38u] = 16u;
}

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_image));
    return memory_image[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_image));
    memory_image[address] = value;
}

void FillControllerBufferRange(fw_addr_t address, u32 count, u8 value)
{
    assert(address == 0ul && count == 0x1b0000ul && value == 0u);
}

void ObserveCloseSyncPrepareReset(void)
{
    ++reset_count;
}
u8 ObserveCloseSyncPrepareSectorClass(void)
{
    return sector_class;
}
void ObserveCloseSyncPrepareBuildQ(s32 relative, fw_addr_t destination)
{
    last_msf_relative = relative;
    assert(destination == 0x1272ul);
}
void ObserveCloseSyncPrepareSetMsf(s32 relative, s32 absolute)
{
    last_msf_relative = relative;
    last_msf_absolute = absolute;
}
u8 ObserveCloseSyncPrepareAcquire(s32 count)
{
    assert(acquire_index < acquire_count);
    last_acquire_count = count;
    return acquire_results[acquire_index++];
}
u8 ObserveCloseSyncPreparePrefix(u8 omit, u8 wait)
{
    (void)omit;
    (void)wait;
    assert(prefix_index < prefix_count);
    return prefix_results[prefix_index++];
}
u8 ObserveCloseSyncPrepareSequence(u8 first)
{
    assert(first == 1u && sequence_index < sequence_count);
    sequence_expected = ReadWord(0x8010u);
    return sequence_results[sequence_index++];
}
u8 ObserveCloseSyncPrepareQueue(void)
{
    assert(queue_index < queue_count);
    return queue_results[queue_index++];
}
u8 ObserveCloseSyncPrepareOpc(void)
{
    assert(opc_index < opc_count);
    return opc_results[opc_index++];
}
u8 ObserveCloseSyncPreparePlan(u8 alternate)
{
    assert(plan_index < plan_count);
    (void)alternate;
    return plan_results[plan_index++];
}
void ObserveCloseSyncPrepareSense030c08(void)
{
    ++target_sense_count;
}
void ObserveCloseSyncPrepareSense(u8 key, u16 asc_ascq)
{
    ++sense_count;
    last_sense_key = key;
    last_sense_code = asc_ascq;
}
void ObserveCloseSyncPrepareTuple(s32 frame, u8 tuple[3])
{
    last_tuple_frame = frame;
    tuple[0] = 1u;
    tuple[1] = 2u;
    tuple[2] = 3u;
}
void ObserveCloseSyncPreparePublishTuple(u16 destination, u8 tuple[3])
{
    last_tuple_destination = destination;
    memory_image[destination] = tuple[0];
    memory_image[destination + 1u] = tuple[1];
    memory_image[destination + 2u] = tuple[2];
}

static void TestTrackZeroInitialization(void)
{
    ResetHarness();
    assert(PrepareCloseTrackOrSyncRawReadState() == 1u);
    assert(reset_count == 1u);
    assert(ReadWord(0x129au) == 31u && ReadWord(0x129cu) == 26u);
    assert(ReadDword(0x129eu) == 0x88bbul);
    assert(ReadDword(0x126au) == 0x123456ul);
    assert(memory_image[0x1281u] == 2u);
    assert(memory_image[0x142bu] == sector_class);
    assert(ReadDword(0x1598u) == 1200ul);
}

static void TestTrackOneFillFailure(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 1u;
    memory_image[0xc44u] = 0u;
    acquire_results[0] = 0u;
    acquire_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 0u);
    assert(last_acquire_count == 200l);
    assert(last_msf_relative == 100l && last_msf_absolute == 1100l);
}

static void TestTrackOneSpecialSequence(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 1u;
    memory_image[0xc44u] = 0u;
    memory_image[0x1008u] = 2u;
    memory_image[0x114eu] = 1u;
    memory_image[0x1149u] = 1u;
    WriteDword(0xdd45u, 1400ul);
    sequence_results[0] = 1u;
    sequence_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 1u);
    assert(sequence_expected == 20u);
    assert(memory_image[0x88b9u] == 0u);
}

static void TestTrackTwoPlanFailure(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 2u;
    plan_results[0] = 0u;
    plan_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 0u);
    assert(memory_image[0x128bu] == 0u);
}

static void TestTrackSixAlternatePlanFailureReportsSense(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 6u;
    memory_image[0xc84u] = 0x40u;
    WriteDword(0xdd51u, 900ul);
    plan_results[0] = 0u;
    plan_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 0u);
    assert(target_sense_count == 1u);
}

static void TestNegativeEndRangeReportsSense(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 3u;
    memory_image[0xc44u] = 0u;
    WriteDword(0xdd41u, 1101ul);
    assert(PrepareCloseTrackOrSyncRawReadState() == 0u);
    assert(target_sense_count == 1u);
    assert(last_tuple_frame == 1100l && last_tuple_destination == 0x1279u);
}

static void TestHighSpeedQueueOpcFailure(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 4u;
    memory_image[0xc84u] = 0x40u;
    memory_image[0xe47u] = 16u;
    prefix_results[0] = 1u;
    prefix_count = 1u;
    queue_results[0] = 0u;
    queue_count = 1u;
    opc_results[0] = 0u;
    opc_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 0u);
    assert(memory_image[0xe47u] == 4u);
    assert(sense_count == 1u && last_sense_key == 3u);
    assert(last_sense_code == 0x7304u);
}

static void TestTrackFiveFinalQueue(void)
{
    ResetHarness();
    memory_image[0xdd34u] = 5u;
    memory_image[0xc84u] = 0x40u;
    WriteDword(0xdd41u, 1500ul);
    acquire_results[0] = 1u;
    acquire_count = 1u;
    prefix_results[0] = 1u;
    prefix_count = 1u;
    queue_results[0] = 1u;
    queue_count = 1u;
    assert(PrepareCloseTrackOrSyncRawReadState() == 1u);
    assert(last_acquire_count == 398l);
    assert(memory_image[0x128du] == 3u);
}

int main(void)
{
    TestTrackZeroInitialization();
    TestTrackOneFillFailure();
    TestTrackOneSpecialSequence();
    TestTrackTwoPlanFailure();
    TestTrackSixAlternatePlanFailureReportsSense();
    TestNegativeEndRangeReportsSense();
    TestHighSpeedQueueOpcFailure();
    TestTrackFiveFinalQueue();
    puts("close/sync raw-read preparation tests passed");
    return 0;
}
