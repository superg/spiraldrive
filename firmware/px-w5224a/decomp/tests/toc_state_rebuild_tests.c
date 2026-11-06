#include <assert.h>
#include <string.h>
#include "media/toc_state_rebuild.h"
#include "platform/firmware_memory.h"

#define TRACE_CAPACITY 2400u

static u8 low_memory[1u << 16];
static u8 far_2b_memory[1u << 16];
static fw_addr_t read_trace[32];
static fw_addr_t write_trace[TRACE_CAPACITY];
static unsigned read_count;
static unsigned write_count;
static unsigned parse_count;
static unsigned parse_write_count;
static u16 first_descriptor_count;
static u16 second_descriptor_count;
static unsigned descriptor_count_reads;

u8 FirmwareRead8(fw_addr_t address)
{
    u8 value;

    assert(read_count < sizeof(read_trace) / sizeof(read_trace[0]));
    read_trace[read_count++] = address;
    if(address == 0x008010ul)
    {
        value = (u8)((descriptor_count_reads == 0u ? first_descriptor_count : second_descriptor_count) >> 8);
        descriptor_count_reads++;
        return value;
    }
    if(address == 0x008011ul)
        return (u8)(descriptor_count_reads == 1u ? first_descriptor_count : second_descriptor_count);
    if((address & 0xff0000ul) == 0x2b0000ul)
        return far_2b_memory[(u16)address];
    return low_memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(write_count < TRACE_CAPACITY);
    write_trace[write_count++] = address;
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

void ObserveParseRawReadQRecordTableIntoTocState(void)
{
    parse_count++;
    parse_write_count = write_count;
}

static void WriteU32(u16 address, u32 value)
{
    low_memory[address] = (u8)(value >> 24);
    low_memory[(u16)(address + 1u)] = (u8)(value >> 16);
    low_memory[(u16)(address + 2u)] = (u8)(value >> 8);
    low_memory[(u16)(address + 3u)] = (u8)value;
}

static u32 ReadU32(u16 address)
{
    return ((u32)low_memory[address] << 24) | ((u32)low_memory[(u16)(address + 1u)] << 16) | ((u32)low_memory[(u16)(address + 2u)] << 8) | low_memory[(u16)(address + 3u)];
}

static void ResetFixture(void)
{
    memset(low_memory, 0xa5, sizeof(low_memory));
    memset(far_2b_memory, 0xa5, sizeof(far_2b_memory));
    read_count = 0u;
    write_count = 0u;
    parse_count = 0u;
    parse_write_count = 0u;
    first_descriptor_count = 2u;
    second_descriptor_count = 2u;
    descriptor_count_reads = 0u;
}

static void AssertRecordWriteOrder(unsigned trace_index, fw_addr_t address)
{
    static const u8 offsets[10] = { 0u, 1u, 2u, 6u, 5u, 4u, 3u, 9u, 8u, 7u };
    unsigned index;

    for(index = 0u; index < 10u; ++index)
        assert(write_trace[trace_index + index] == address + offsets[index]);
}

static void TestRejectedWriteTypeTwoState(void)
{
    ResetFixture();
    low_memory[0x0c44u] = 2u;
    low_memory[0x0c45u] = 0xbfu;

    ResetAndRebuildTocStateFromRawReadQRecords();

    assert(read_count == 2u);
    assert(read_trace[0] == 0x000c44ul && read_trace[1] == 0x000c45ul);
    assert(write_count == 0u);
    assert(parse_count == 0u);
}

static void TestFullClearAndNonzeroPublication(void)
{
    unsigned index;

    ResetFixture();
    low_memory[0x0c44u] = 2u;
    low_memory[0x0c45u] = 0xc0u;
    low_memory[0x1132u] = 0x80u;
    WriteU32(0x114fu, 4000ul);

    ResetAndRebuildTocStateFromRawReadQRecords();

    assert(parse_count == 1u && parse_write_count == 2277u);
    assert(write_count == 2284u);
    AssertRecordWriteOrder(0u, 0x2bb03bul);
    AssertRecordWriteOrder(990u, 0x2bb419ul);
    AssertRecordWriteOrder(1000u, 0x2bb5dbul);
    AssertRecordWriteOrder(1010u, 0x2bb423ul);
    AssertRecordWriteOrder(1020u, 0x2bb4fful);
    AssertRecordWriteOrder(1430u, 0x2bb4f5ul);
    AssertRecordWriteOrder(1440u, 0x2bb5d1ul);
    AssertRecordWriteOrder(1450u, 0x2bb5e5ul);
    AssertRecordWriteOrder(1460u, 0x2bb77ful);
    AssertRecordWriteOrder(2250u, 0x2bb775ul);
    AssertRecordWriteOrder(2260u, 0x2bb90ful);
    for(index = 0u; index < 7u; ++index)
        assert(write_trace[2270u + index] == 0x001148ul + index);
    assert(far_2b_memory[0xb03au] == 0xa5u);
    assert(far_2b_memory[0xb03bu] == 0u);
    assert(far_2b_memory[0xb422u] == 0u);
    assert(far_2b_memory[0xb90fu] == 0u);
    assert(far_2b_memory[0xb919u] == 0xa5u);
    assert(ReadU32(0x1143u) == 3002ul);
    assert(low_memory[0x1147u] == 1u);
    assert(low_memory[0x1132u] == 0x20u);
    assert(descriptor_count_reads == 2u);
}

static void TestFreshDescriptorCountSelectsEmptyPublication(void)
{
    ResetFixture();
    low_memory[0x0c44u] = 1u;
    low_memory[0x0c45u] = 0x55u;
    low_memory[0x1132u] = 0x3fu;
    WriteU32(0x114fu, 1500ul);
    first_descriptor_count = 7u;
    second_descriptor_count = 0u;

    ResetAndRebuildTocStateFromRawReadQRecords();

    assert(read_trace[0] == 0x000c44ul);
    assert(read_trace[1] == 0x00114ful);
    assert(ReadU32(0x1143u) == 507ul);
    assert(low_memory[0x1147u] == 0xffu);
    assert(low_memory[0x1132u] == 0x9fu);
    assert(write_trace[2281u] == 0x001147ul);
    assert(write_trace[2282u] == 0x001132ul);
    assert(write_trace[2283u] == 0x001132ul);
}

int main(void)
{
    TestRejectedWriteTypeTwoState();
    TestFullClearAndNonzeroPublication();
    TestFreshDescriptorCountSelectsEmptyPublication();
    return 0;
}
