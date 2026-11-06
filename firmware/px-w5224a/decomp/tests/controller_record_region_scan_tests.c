#include <assert.h>
#include <string.h>
#include "platform/controller_record_region_scan.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static unsigned probe_count, copy_count;
static u8 expected_bank, expected_page;
static u8 probe_mode;
static s32 first_probe_position;
static u8 copied_markers[64];

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u32 MultiplyU32Low(u32 left, u32 right)
{
    return left * right;
}

u8 ProbeControllerRecordAtFrameAndMapStatus(s32 frame_position)
{
    if(probe_count == 0u)
        first_probe_position = frame_position;
    ++probe_count;
    if(probe_mode == 1u)
        return 5u;
    if(probe_mode == 2u && probe_count == 1u)
        return 0xffu;
    if(probe_mode == 3u && probe_count <= 10u)
        return 0x11u;
    memory[0x002bb91cul] = (u8)((expected_bank << 4) | expected_page);
    return 0u;
}

void ObserveCopyControllerRecordRegionPage(fw_addr_t source, u8 bank, u8 page, u8 marker)
{
    assert(source == 0x002bb919ul);
    assert(bank == expected_bank && page == expected_page);
    assert(copy_count < 64u);
    copied_markers[copy_count++] = marker;
    ++expected_page;
    if(expected_page == 8u)
    {
        expected_page = 0u;
        ++expected_bank;
    }
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
    u8 bank;

    memset(memory, 0, sizeof(memory));
    probe_count = copy_count = 0u;
    expected_bank = expected_page = 0u;
    probe_mode = 0u;
    first_probe_position = 0;
    memset(copied_markers, 0, sizeof(copied_markers));
    memory[0x002ba588ul] = 0x40u;
    WriteU32(0x0000dd3dul, 1000ul);
    for(bank = 0u; bank != 8u; ++bank)
    {
        WriteU32(0x00fb04ul + (u16)bank * 5u, (u32)(100 + (s32)bank * 10l));
        WriteU32(0x00fb2cul + (u16)bank * 5u, 64ul);
    }
}

static void TestPrimaryAndFallbackScans(void)
{
    ResetState();
    assert(ScanControllerRecordRegionGridAndCachePages(0u) == 1u);
    assert(probe_count == 64u && copy_count == 64u);
    assert(first_probe_position == 1100);
    assert(expected_bank == 8u && expected_page == 0u);
    assert((memory[0x000c8bul] & 2u) != 0u);

    ResetState();
    assert(ScanControllerRecordRegionGridAndCachePages(1u) == 1u);
    assert(probe_count == 64u && copy_count == 64u);
    assert(first_probe_position == 786);
    assert(copied_markers[0] == 0u && copied_markers[63] == 0u);
}

static void TestProbeFailure(void)
{
    ResetState();
    probe_mode = 1u;
    assert(ScanControllerRecordRegionGridAndCachePages(0u) == 0u);
    assert(probe_count == 1u && copy_count == 0u);
}

static void TestPrimarySlotFallback(void)
{
    ResetState();
    probe_mode = 2u;
    WriteU32(0x002ba577ul, 500ul);
    assert(ScanControllerRecordRegionGridAndCachePages(0u) == 1u);
    assert(memory[0x00fb03ul] == 1u);
    assert(ReadU32(0x00fb04ul) == 468ul);
    assert(ReadU32(0x002ba577ul) == 468ul);
    assert((memory[0x000c8bul] & 1u) != 0u);
}

static void TestSecondaryPositionSwitch(void)
{
    unsigned index;

    ResetState();
    probe_mode = 3u;
    assert(ScanControllerRecordRegionGridAndCachePages(0u) == 1u);
    assert(probe_count == 74u && copy_count == 64u);
    for(index = 0u; index != 8u; ++index)
        assert(copied_markers[index] == 1u);
    for(; index != 64u; ++index)
        assert(copied_markers[index] == 0u);
}

int main(void)
{
    TestPrimaryAndFallbackScans();
    TestProbeFailure();
    TestPrimarySlotFallback();
    TestSecondaryPositionSwitch();
    return 0;
}
