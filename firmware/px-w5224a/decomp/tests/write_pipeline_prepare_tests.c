#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_pipeline_prepare.h"

static u8 memory[1u << 24];
static u32 host_target;
static unsigned clear_count, reset_count, acquire_index_count;
static u16 acquire_first, acquire_last;
static u8 acquire_mode;
static u8 requires_record, preservation_result;
static unsigned initialize_track_count, fill_count, preservation_count;
static fw_addr_t fill_address;
static u32 fill_length;
static u8 fill_value;
static s32 preservation_address;
static s32 map_result;
static unsigned map_count, enqueue_count;
static s32 enqueue_address;
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u32 InitializeHostTransferRange(void)
{
    return host_target;
}
void ClearRawReadControllerBufferRegions(void)
{
    ++clear_count;
}
void ResetRawReadPipelineBuffersAndState(void)
{
    ++reset_count;
}
u8 AcquireThirtyNineFrameControllerRecordIndexRange(u16 first, u16 last, u8 mode)
{
    ++acquire_index_count;
    acquire_first = first;
    acquire_last = last;
    acquire_mode = mode;
    return 1u;
}
u8 DoesWriteStartRequireControllerRecordPreparation(s32 address, u8 option)
{
    (void)address;
    assert(option == 0u);
    return requires_record;
}
void InitializeReadVerifyWorkingTrackState(void)
{
    ++initialize_track_count;
}
void FillControllerBufferRange(fw_addr_t address, u32 length, u8 value)
{
    ++fill_count;
    fill_address = address;
    fill_length = length;
    fill_value = value;
}
u8 AcquireWritePreservationRawReadAtAddress(s32 address, u8 mode)
{
    assert(mode == 0u);
    ++preservation_count;
    preservation_address = address;
    return preservation_result;
}
s32 ObserveMapWritePipelineLogicalBlockAddress(s32 address)
{
    ++map_count;
    (void)address;
    return map_result;
}
void EnqueueThirtyTwoSyntheticWriteDescriptorsForStartAddress(s32 address)
{
    ++enqueue_count;
    enqueue_address = address;
}
void SetSenseData(u8 key, u16 code)
{
    ++sense_count;
    sense_key = key;
    sense_code = code;
}

static void WriteU32(fw_addr_t a, u32 v)
{
    memory[a] = (u8)(v >> 24);
    memory[a + 1u] = (u8)(v >> 16);
    memory[a + 2u] = (u8)(v >> 8);
    memory[a + 3u] = (u8)v;
}
static u32 ReadU32(fw_addr_t a)
{
    return ((u32)memory[a] << 24) | ((u32)memory[a + 1u] << 16) | ((u32)memory[a + 2u] << 8) | memory[a + 3u];
}
static u16 ReadU16(fw_addr_t a)
{
    return (u16)(((u16)memory[a] << 8) | memory[a + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    host_target = 145u;
    clear_count = reset_count = acquire_index_count = 0u;
    acquire_first = acquire_last = 0u;
    acquire_mode = 0xffu;
    requires_record = 0u;
    preservation_result = 1u;
    initialize_track_count = fill_count = preservation_count = 0u;
    fill_address = fill_length = 0ul;
    fill_value = 0xffu;
    preservation_address = 0;
    map_result = 1000;
    map_count = enqueue_count = sense_count = 0u;
    enqueue_address = 0;
    sense_key = 0u;
    sense_code = 0u;
    WriteU32(0x000b02ul, 0x12345678ul);
    WriteU32(0x00156aul, 16u);
    WriteU32(0x001562ul, 0x00abc000ul);
    WriteU32(0x00fafful, 0x00120000ul);
}

static void TestAlignedAndMissingIndex(void)
{
    ResetState();
    memory[0x000c8aul] = 0xffu;
    assert(PrepareWritePipelineForStartAddress(64) == 1u);
    assert((memory[0x000c8aul] & 0x10u) == 0u);
    assert(ReadU32(0x001598ul) == 145u);
    assert(fill_count == 0u);

    ResetState();
    host_target = 145u + 39u * 3u;
    WriteU32(0x002ba59ful, 0x89abcdeful);
    assert(PrepareWritePipelineForStartAddress(64) == 1u);
    assert(clear_count == 1u && reset_count == 1u);
    assert(acquire_index_count == 1u);
    assert(acquire_first == 2u && acquire_last == 2u && acquire_mode == 0u);
    assert(ReadU32(0x002ba59ful) == 0x89abcdeful);
    assert(ReadU16(0x00129aul) == 15u);
    assert(ReadU16(0x00129cul) == 10u);
    assert(ReadU32(0x00129eul) == 0x000088bbul);
    assert(ReadU32(0x00126aul) == 0x00abc000ul);
}

static void TestPreservationPaths(void)
{
    ResetState();
    requires_record = 1u;
    memory[0x000c3cul] = 0xa5u;
    assert(PrepareWritePipelineForStartAddress(33) == 1u);
    assert(initialize_track_count == 1u);
    assert(fill_count == 1u && fill_address == 0x00019940ul);
    assert(fill_length == 0u && fill_value == 0u);
    assert(preservation_count == 1u && preservation_address == 32);
    assert(memory[0x000c3cul] == 0xa5u);
    assert(memory[0x000ca1ul] == 0u && memory[0x0015d7ul] == 0u);
    assert(ReadU32(0x000b02ul) == 0x12345678ul);
    assert((memory[0x000c8aul] & 0x10u) != 0u);

    ResetState();
    requires_record = 1u;
    preservation_result = 0u;
    assert(PrepareWritePipelineForStartAddress(33) == 0u);
    assert(sense_count == 1u && sense_key == 3u && sense_code == 0x0c07u);
    assert(ReadU32(0x000b02ul) == 0x12345678ul);
}

static void TestSyntheticPath(void)
{
    ResetState();
    requires_record = 0u;
    map_result = 1000;
    assert(PrepareWritePipelineForStartAddress(33) == 1u);
    assert(fill_count == 1u && fill_address == 0x0000cca0ul);
    assert(map_count == 1u && enqueue_count == 1u && enqueue_address == 32);
    assert(ReadU32(0x000b02ul) == 0x12345678ul);
    assert(ReadU16(0x00129aul) == 15u);
    assert((memory[0x000c8aul] & 0x10u) != 0u);
}

int main(void)
{
    TestAlignedAndMissingIndex();
    TestPreservationPaths();
    TestSyntheticPath();
    return 0;
}
