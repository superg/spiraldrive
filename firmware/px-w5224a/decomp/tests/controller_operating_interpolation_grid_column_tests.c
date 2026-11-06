#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_operating_interpolation_grid_column.h"
#include "platform/firmware_memory.h"

#define SOURCE_BASE 0x00f8372dul
#define DESTINATION_BASE 0x00e806ul

static u8 memory[0x1000000ul];
static fw_addr_t reads[32];
static fw_addr_t writes[32];
static unsigned read_count;
static unsigned write_count;
static u8 row_reads[2];
static u8 column_reads[7];
static unsigned row_read_count;
static unsigned column_read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    assert(read_count < sizeof(reads) / sizeof(reads[0]));
    reads[read_count++] = address;
    if(address == 0x00f05ful && row_read_count < 2u)
        return row_reads[row_read_count++];
    if(address == 0x001057ul && column_read_count < 7u)
        return column_reads[column_read_count++];
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    writes[write_count++] = address;
    memory[address] = value;
}

static void Reset(u8 row, u8 column)
{
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    memset(reads, 0, sizeof(reads));
    memset(writes, 0, sizeof(writes));
    read_count = 0u;
    write_count = 0u;
    row_read_count = 0u;
    column_read_count = 0u;
    row_reads[0] = row;
    row_reads[1] = row;
    for(index = 0u; index < 7u; ++index)
        column_reads[index] = column;
}

static float LoadStoredFloat(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } decoded;

    decoded.bits = ((u32)memory[address] << 24) | ((u32)memory[address + 1ul] << 16) | ((u32)memory[address + 2ul] << 8) | memory[address + 3ul];
    return decoded.value;
}

static void CheckFractionalInterpolation(void)
{
    static const u8 expected[5] = { 1u, 1u, 2u, 3u, 4u };
    unsigned index;

    Reset(2u, 7u);
    memory[SOURCE_BASE + 2u * 24u + 7u] = 1u;
    memory[SOURCE_BASE + 240u + 2u * 24u + 7u] = 4u;
    BuildControllerOperatingInterpolationGridColumn();

    for(index = 0u; index < 5u; ++index)
    {
        fw_addr_t address = DESTINATION_BASE + index * 34u + 7u;
        assert(memory[address] == expected[index]);
    }
    assert(LoadStoredFloat(0x0012deul) == 0.75f);
    assert(LoadStoredFloat(0x0012e2ul) == -14.0f);
    assert(read_count == 11u);
    assert(write_count == 13u);
    assert(reads[0] == 0x00f05ful && reads[1] == 0x00f05ful);
    assert(reads[2] == 0x001057ul);
    assert(reads[3] == SOURCE_BASE + 2u * 24u + 7u);
    assert(reads[4] == 0x001057ul);
    assert(reads[5] == SOURCE_BASE + 240u + 2u * 24u + 7u);
}

static void CheckFreshSelectorsAndClamp(void)
{
    unsigned index;

    Reset(1u, 2u);
    row_reads[1] = 3u;
    column_reads[1] = 5u;
    for(index = 0u; index < 5u; ++index)
        column_reads[index + 2u] = (u8)(9u + index);
    memory[SOURCE_BASE + 1u * 24u + 2u] = 40u;
    memory[SOURCE_BASE + 240u + 3u * 24u + 5u] = 80u;

    BuildControllerOperatingInterpolationGridColumn();

    for(index = 0u; index < 5u; ++index)
    {
        fw_addr_t address = DESTINATION_BASE + index * 34u + 9u + index;
        assert(memory[address] == 31u);
    }
    assert(reads[3] == SOURCE_BASE + 1u * 24u + 2u);
    assert(reads[5] == SOURCE_BASE + 240u + 3u * 24u + 5u);
    assert(memory[DESTINATION_BASE + 5u * 34u + 9u] == 0xa5u);
}

int main(void)
{
    CheckFractionalInterpolation();
    CheckFreshSelectorsAndClamp();
    puts("controller_operating_interpolation_grid_column_tests: ok");
    return 0;
}
