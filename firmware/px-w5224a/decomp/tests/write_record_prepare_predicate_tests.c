#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_record_prepare_predicate.h"

static u8 memory[1u << 24];
static unsigned fixed_map_count, overhead_map_count, marked_count;
static u32 fixed_map_input, fixed_map_result;
static s32 overhead_map_input, overhead_map_result;
static u8 marked_result;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u32 MapFixedPacketLogicalToPhysicalAddress(u32 address)
{
    ++fixed_map_count;
    fixed_map_input = address;
    return fixed_map_result;
}
s32 ExpandWriteLogicalAddressForRecordOverhead(s32 address)
{
    ++overhead_map_count;
    overhead_map_input = address;
    return overhead_map_result;
}
u8 IsCurrentWriteControllerRecordIndexMarked(void)
{
    ++marked_count;
    return marked_result;
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
    fixed_map_count = overhead_map_count = marked_count = 0u;
    fixed_map_input = fixed_map_result = 0ul;
    overhead_map_input = overhead_map_result = 0;
    marked_result = 0u;
    WriteU32(0x002ba59ful, 0ul);
}

static void TestAlternateFixedPacketUsesActiveBoundary(void)
{
    ResetState();
    memory[0x000c8aul] = 0x04u;
    fixed_map_result = 100ul;
    WriteU32(0x002bae2ful, 246ul);
    assert(DoesWriteStartRequireControllerRecordPreparation(0x1234, 1u) == 1u);
    assert(fixed_map_count == 1u && fixed_map_input == 0x1234ul);
    assert(overhead_map_count == 0u && marked_count == 0u);
    assert(ReadU32(0x000b02ul) == 100ul);
}

static void TestAlternateOverheadUsesIndexedBoundary(void)
{
    ResetState();
    overhead_map_result = 1000;
    WriteU32(0x002bae33ul + 2ul * 4ul, 1146ul);
    assert(DoesWriteStartRequireControllerRecordPreparation(0x2200, 1u) == 1u);
    assert(overhead_map_count == 1u && overhead_map_input == 0x2200);
    assert(fixed_map_count == 0u && marked_count == 0u);
    assert(ReadU32(0x000b02ul) == 1000ul);
}

static void TestExistingStartBoundarySelection(void)
{
    ResetState();
    WriteU32(0x000b02ul, 100ul);
    WriteU32(0x002bae2ful, 246ul);
    memory[0x000c88ul] = 0x80u;
    assert(DoesWriteStartRequireControllerRecordPreparation(0, 2u) == 1u);
    assert(fixed_map_count == 0u && overhead_map_count == 0u);

    ResetState();
    WriteU32(0x000b02ul, 100ul);
    WriteU32(0x002bae33ul + 1ul * 4ul, 246ul);
    assert(DoesWriteStartRequireControllerRecordPreparation((s32)(0x1100ul * 257ul), 0u) == 1u);
    assert(marked_count == 0u);
}

static void TestRetainedBoundaryAndMarkedFallback(void)
{
    ResetState();
    WriteU32(0x000b02ul, 100ul);
    WriteU32(0x002bae33ul, 245ul);
    WriteU32(0x002ba59ful, 246ul);
    assert(DoesWriteStartRequireControllerRecordPreparation(-1, 0u) == 1u);
    assert(marked_count == 0u);

    ResetState();
    WriteU32(0x000b02ul, 100ul);
    WriteU32(0x002bae33ul, 245ul);
    WriteU32(0x002ba59ful, 245ul);
    marked_result = 2u;
    assert(DoesWriteStartRequireControllerRecordPreparation(0, 0u) == 0u);
    assert(marked_count == 1u);
    marked_result = 1u;
    assert(DoesWriteStartRequireControllerRecordPreparation(0, 0u) == 1u);
    assert(marked_count == 2u);
}

int main(void)
{
    TestAlternateFixedPacketUsesActiveBoundary();
    TestAlternateOverheadUsesIndexedBoundary();
    TestExistingStartBoundarySelection();
    TestRetainedBoundaryAndMarkedFallback();
    return 0;
}
