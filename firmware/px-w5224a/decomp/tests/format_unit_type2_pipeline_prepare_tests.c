#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/format_unit_type2_pipeline.h"

static u8 firmware_memory[0x1000000ul];
static fw_addr_t write_addresses[256];
static u8 write_values[256];
static unsigned write_count;
static u8 events[16];
static unsigned event_count;
static u8 primary_result;
static u8 base_window_result;
static u8 tdi_result;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    assert(write_count < sizeof(write_addresses) / sizeof(write_addresses[0]));
    write_addresses[write_count] = address;
    write_values[write_count] = value;
    ++write_count;
    firmware_memory[address] = value;
}

static void RecordEvent(u8 event)
{
    assert(event_count < sizeof(events));
    events[event_count++] = event;
}

void ObserveResetRawReadPipelineBuffersForFormatUnit(void)
{
    RecordEvent(1u);
}
void ObserveClearRawReadControllerBufferRegionsForFormatUnit(void)
{
    RecordEvent(2u);
}
void ObserveInitializeRawReadSectorControlStateForFormatUnit(void)
{
    RecordEvent(3u);
}
void ObserveClearRawReadPipelineStatusStateForFormatUnit(void)
{
    RecordEvent(4u);
}
void ObserveInitializeType2ControllerRecordGeometryForFormatUnit(void)
{
    RecordEvent(5u);
}
void ObserveBuildType2LeadInTocForFormatUnit(u8 write_full_toc)
{
    assert(write_full_toc == 0u);
    RecordEvent(6u);
}
u8 ObserveAcquireRequiredControllerRecordRunsForFormatUnit(u8 operation_mode)
{
    (void)operation_mode;
    return 0u;
}
void ObserveRecalculateType2ControllerRecordGeometryForFormatUnit(void) {}
void ObserveBuildActiveControllerRecordBoundaryTableForFormatUnit(void) {}
void ObserveInitializeControllerRecordBoundaryDefaultsForFormatUnit(void)
{
    RecordEvent(10u);
}
u8 ObservePreparePrimaryType2ControllerRecordSlotsForFormatUnit(u8 acquisition_mode)
{
    assert(acquisition_mode == 0u);
    RecordEvent(11u);
    return primary_result;
}
u8 ObserveAcquireFortyFrameType2ControllerRecordBaseWindowForFormatUnit(void)
{
    RecordEvent(12u);
    return base_window_result;
}
u8 ObserveAcquireType2TdiControllerRecordSequenceForFormatUnit(void)
{
    RecordEvent(13u);
    return tdi_result;
}
void ObserveBuildAllControllerRecordWorkspaceBankPagesForFormatUnit(void)
{
    RecordEvent(14u);
}
void ObserveCompleteScsiCommandWithNoSenseForFormatUnit(void)
{
    RecordEvent(15u);
}

void ObservePrepareWritePipelineNormalizeSpeed(void) {}
void ObservePrepareWritePipelineResetQueue(void) {}
void ObservePrepareWritePipelineInitializeCoroutine2(fw_addr_t entry_address)
{
    (void)entry_address;
}
void ObservePrepareWritePipelineYieldCoroutine0(void) {}
u8 ObservePrepareWritePipelineEnsureOpc(void)
{
    return 0u;
}
void ObservePrepareWritePipelineResetBuffers(void) {}
void ObservePrepareWritePipelineClearControllerRegions(void) {}
void ObservePrepareWritePipelineInitializeSectorControl(void) {}
void ObservePrepareWritePipelineClearStatus(void) {}
void ObservePrepareWritePipelineRecalculateGeometry(void) {}
void ObservePrepareWritePipelineAcquireRecordWindow(void) {}

static void WriteU32(fw_addr_t address, u32 value)
{
    firmware_memory[address] = (u8)(value >> 24);
    firmware_memory[address + 1u] = (u8)(value >> 16);
    firmware_memory[address + 2u] = (u8)(value >> 8);
    firmware_memory[address + 3u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)firmware_memory[address] << 8) | firmware_memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)firmware_memory[address] << 24) | ((u32)firmware_memory[address + 1u] << 16) | ((u32)firmware_memory[address + 2u] << 8) | firmware_memory[address + 3u];
}

static void PrepareFixture(u8 next_primary_result, u8 next_base_window_result, u8 next_tdi_result, u8 status_c85)
{
    memset(firmware_memory, 0xa5, sizeof(firmware_memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    write_count = 0u;
    primary_result = next_primary_result;
    base_window_result = next_base_window_result;
    tdi_result = next_tdi_result;
    WriteU32(0x00156aul, 10ul);
    WriteU32(0x001562ul, 0x00123456ul);
    firmware_memory[0x2ba5d9ul] = 0u;
    firmware_memory[0x2ba5daul] = 3u;
    firmware_memory[0x000c8bul] = 0xffu;
    firmware_memory[0x000c84ul] = 0xafu;
    firmware_memory[0x000c85ul] = status_c85;
    firmware_memory[0x000c8aul] = 0x41u;
}

static void AssertInitializedState(void)
{
    unsigned index;
    unsigned c8b_count = 0u;
    static const u8 expected_c8b_values[] = { 0xfbu, 0xf9u, 0xf8u };

    assert(ReadU16(0x001298ul) == 0u);
    assert(ReadU16(0x001294ul) == 0u);
    assert(ReadU16(0x001296ul) == 0u);
    assert(ReadU16(0x001292ul) == 0u);
    assert(ReadU16(0x00129aul) == 9u);
    assert(ReadU16(0x00129cul) == 4u);
    assert(ReadU32(0x00129eul) == 0x000088bbul);
    assert(ReadU32(0x001262ul) == 0ul);
    assert(ReadU32(0x001266ul) == 0ul);
    assert(ReadU32(0x00126aul) == 0x00123456ul);
    assert(ReadU32(0x00126eul) == 0ul);
    assert(ReadU32(0x001488ul) == 0ul);
    assert(ReadU32(0x00148cul) == 0ul);
    assert(firmware_memory[0x001490ul] == 0u);
    assert(firmware_memory[0x2ba5aful] == 0u);
    assert(ReadU32(0x00fafful) == 0x0000b4bbul);
    assert(firmware_memory[0x00b4baul] == 0xa5u);
    assert(firmware_memory[0x00b4bbul] == 0u);
    assert(firmware_memory[0x00b4bcul] == 0u);
    assert(firmware_memory[0x00b4bdul] == 0u);
    assert(firmware_memory[0x00b4beul] == 0xa5u);
    assert(ReadU16(0x2ba5dbul) == 0u);
    assert(ReadU32(0x2baf37ul) == 0ul);
    assert(ReadU32(0x2baf33ul) == 0ul);
    assert(ReadU32(0x2ba59ful) == 0ul);
    assert(ReadU32(0x2ba5abul) == 0ul);
    assert(ReadU32(0x2ba5a7ul) == 0ul);
    assert(ReadU32(0x2ba52eul) == 0ul);
    assert(ReadU32(0x2ba532ul) == 0ul);
    assert(ReadU32(0x2ba5a3ul) == 0ul);
    assert(ReadU32(0x000ce5ul) == 0ul);
    assert(ReadU32(0x000ce1ul) == 0ul);
    assert(firmware_memory[0x000c8bul] == 0xf8u);
    assert(firmware_memory[0x0088b7ul] == 1u);

    for(index = 0u; index != write_count; ++index)
    {
        if(write_addresses[index] == 0x000c8bul)
        {
            assert(c8b_count < sizeof(expected_c8b_values));
            assert(write_values[index] == expected_c8b_values[c8b_count]);
            ++c8b_count;
        }
    }
    assert(c8b_count == sizeof(expected_c8b_values));
}

static void AssertEvents(const u8 *expected, unsigned count)
{
    assert(event_count == count);
    assert(memcmp(events, expected, count) == 0);
}

int main(void)
{
    static const u8 primary_failure[] = { 1u, 2u, 3u, 4u, 5u, 10u, 6u, 11u };
    static const u8 base_failure[] = { 1u, 2u, 3u, 4u, 5u, 10u, 6u, 11u, 12u };
    static const u8 tdi_failure[] = { 1u, 2u, 3u, 4u, 5u, 10u, 6u, 11u, 12u, 13u };
    static const u8 success[] = { 1u, 2u, 3u, 4u, 5u, 10u, 6u, 11u, 12u, 13u, 14u, 15u };
    static const u8 busy_success[] = { 1u, 2u, 3u, 4u, 5u, 10u, 6u, 11u, 12u, 13u, 14u };

    PrepareFixture(0u, 1u, 1u, 0u);
    assert(PrepareFormatUnitCode11Type2ControllerRecordPipeline() == 0u);
    AssertInitializedState();
    assert(firmware_memory[0x2ba524ul] == 0xa5u);
    assert(firmware_memory[0x000c84ul] == 0xafu);
    assert(firmware_memory[0x000c8aul] == 0x41u);
    AssertEvents(primary_failure, sizeof(primary_failure));

    PrepareFixture(1u, 0u, 1u, 0u);
    assert(PrepareFormatUnitCode11Type2ControllerRecordPipeline() == 0u);
    AssertInitializedState();
    assert(firmware_memory[0x2ba524ul] == 0u);
    AssertEvents(base_failure, sizeof(base_failure));

    PrepareFixture(1u, 1u, 0u, 0u);
    assert(PrepareFormatUnitCode11Type2ControllerRecordPipeline() == 0u);
    AssertInitializedState();
    AssertEvents(tdi_failure, sizeof(tdi_failure));

    PrepareFixture(1u, 1u, 1u, 0u);
    assert(PrepareFormatUnitCode11Type2ControllerRecordPipeline() == 1u);
    AssertInitializedState();
    assert(firmware_memory[0x000c84ul] == 0xa7u);
    assert(firmware_memory[0x000c8aul] == 0x49u);
    AssertEvents(success, sizeof(success));

    PrepareFixture(1u, 1u, 1u, 1u);
    assert(PrepareFormatUnitCode11Type2ControllerRecordPipeline() == 1u);
    AssertInitializedState();
    AssertEvents(busy_success, sizeof(busy_success));

    puts("FORMAT UNIT Type-2 preparation pipeline tests passed");
    return 0;
}
