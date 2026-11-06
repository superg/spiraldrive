#include <assert.h>
#include <string.h>
#include "media/toc_state_parse.h"
#include "platform/firmware_memory.h"

static u8 low_memory[1u << 16];
static u8 far_2b_memory[1u << 16];
static fw_addr_t copy_source[8];
static fw_addr_t copy_destination[8];
static unsigned copy_count;
static unsigned bcd_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        return far_2b_memory[(u16)address];
    return low_memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if((address & 0xff0000ul) == 0x2b0000ul)
        far_2b_memory[(u16)address] = value;
    else
        low_memory[(u16)address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

u8 ObserveParseTocPackedBcdToBinary(u8 value)
{
    bcd_count++;
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}

void ObserveParseTocCopyDecode(fw_addr_t source, fw_addr_t destination)
{
    static const u8 decode_offsets[7] = { 2u, 3u, 4u, 5u, 7u, 8u, 9u };
    unsigned index;

    copy_source[copy_count] = source;
    copy_destination[copy_count] = destination;
    copy_count++;
    FirmwareWrite8(destination, FirmwareRead8(source));
    for(index = 0u; index < 7u; ++index)
    {
        u8 offset = decode_offsets[index];

        FirmwareWrite8(destination + offset, ObserveParseTocPackedBcdToBinary(FirmwareRead8(source + offset)));
    }
}

static void WriteWord(u16 address, u16 value)
{
    low_memory[address] = (u8)(value >> 8);
    low_memory[(u16)(address + 1u)] = (u8)value;
}

static void SetRecord(u16 index, u8 control_adr, u8 point, u8 minute, u8 second, u8 frame, u8 pminute, u8 psecond, u8 pframe)
{
    u16 address = (u16)(0x8012u + index * 10u);

    low_memory[address] = control_adr;
    low_memory[(u16)(address + 2u)] = point;
    low_memory[(u16)(address + 3u)] = minute;
    low_memory[(u16)(address + 4u)] = second;
    low_memory[(u16)(address + 5u)] = frame;
    low_memory[(u16)(address + 7u)] = pminute;
    low_memory[(u16)(address + 8u)] = psecond;
    low_memory[(u16)(address + 9u)] = pframe;
}

static void ResetFixture(void)
{
    memset(low_memory, 0, sizeof(low_memory));
    memset(far_2b_memory, 0xa5, sizeof(far_2b_memory));
    copy_count = 0u;
    bcd_count = 0u;
}

static void TestEmptyTableRetainsPublishedRanges(void)
{
    ResetFixture();
    low_memory[0x1149u] = 4u;
    low_memory[0x114au] = 3u;
    low_memory[0x114bu] = 2u;
    low_memory[0x114cu] = 1u;
    low_memory[0x114du] = 5u;

    ParseRawReadQRecordTableIntoTocState();

    assert(low_memory[0x1148u] == 4u && low_memory[0x1149u] == 4u);
    assert(low_memory[0x114au] == 3u && low_memory[0x114bu] == 2u);
    assert(low_memory[0x114cu] == 1u && low_memory[0x114du] == 5u);
    assert(copy_count == 0u);
}

static void TestAllAdrRoutesAndTrackThresholds(void)
{
    fw_addr_t first_track = 0x2bb045ul;
    fw_addr_t second_track = 0x2bb04ful;

    ResetFixture();
    WriteWord(0x800cu, 7u);
    SetRecord(0u, 0x02u, 0x00u, 0x12u, 0x34u, 0x56u, 0x00u, 0xabu, 0x00u);
    SetRecord(1u, 0x03u, 0x03u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u);
    SetRecord(2u, 0x04u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u);
    SetRecord(3u, 0x05u, 0x05u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u);
    SetRecord(4u, 0x06u, 0x06u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u);
    SetRecord(5u, 0x01u, 0x01u, 0x99u, 0x59u, 0x74u, 0x10u, 0x20u, 0x30u);
    SetRecord(6u, 0x41u, 0x02u, 0x9au, 0x5au, 0x75u, 0x11u, 0x22u, 0x33u);

    ParseRawReadQRecordTableIntoTocState();

    assert(far_2b_memory[0xb5dbu] == 0x02u);
    assert(far_2b_memory[0xb5deu] == 12u);
    assert(far_2b_memory[0xb5dfu] == 34u);
    assert(far_2b_memory[0xb5e0u] == 56u);
    assert(far_2b_memory[0xb5e3u] == 0xabu);
    assert(low_memory[0x114eu] == 1u);
    assert(copy_count == 4u);
    assert(copy_source[0] == 0x00801cul && copy_destination[0] == 0x2bb42dul);
    assert(copy_source[1] == 0x008026ul && copy_destination[1] == 0x2bb509ul);
    assert(copy_source[2] == 0x008030ul && copy_destination[2] == 0x2bb5eful);
    assert(copy_source[3] == 0x00803aul && copy_destination[3] == 0x2bb789ul);
    assert(FirmwareRead8(first_track) == 0x01u);
    assert(FirmwareRead8(first_track + 2u) == 1u);
    assert(FirmwareRead8(first_track + 3u) == 99u);
    assert(FirmwareRead8(first_track + 4u) == 59u);
    assert(FirmwareRead8(first_track + 5u) == 74u);
    assert(FirmwareRead8(first_track + 7u) == 10u);
    assert(FirmwareRead8(first_track + 8u) == 20u);
    assert(FirmwareRead8(first_track + 9u) == 30u);
    assert(FirmwareRead8(second_track) == 0x41u);
    assert(FirmwareRead8(second_track + 2u) == 2u);
    assert(FirmwareRead8(second_track + 3u) == 0x9au);
    assert(FirmwareRead8(second_track + 4u) == 0x5au);
    assert(FirmwareRead8(second_track + 5u) == 0x75u);
    assert(low_memory[0x12beu] == 3u);
    assert(low_memory[0x1148u] == 2u && low_memory[0x1149u] == 2u);
    assert(low_memory[0x114au] == 1u && low_memory[0x114bu] == 1u);
    assert(low_memory[0x114cu] == 1u && low_memory[0x114du] == 1u);
}

static void TestOptionFlagSuppressesTrackSummaryClass(void)
{
    ResetFixture();
    low_memory[0x0c46u] = 1u;
    low_memory[0x12beu] = 0x80u;
    WriteWord(0x800cu, 1u);
    SetRecord(0u, 0x41u, 0x01u, 0u, 0u, 0u, 0u, 0u, 0u);

    ParseRawReadQRecordTableIntoTocState();

    assert(low_memory[0x12beu] == 0x80u);
    assert(low_memory[0x1149u] == 1u && low_memory[0x1148u] == 1u);
}

int main(void)
{
    TestEmptyTableRetainsPublishedRanges();
    TestAllAdrRoutesAndTrackThresholds();
    TestOptionFlagSuppressesTrackSummaryClass();
    return 0;
}
