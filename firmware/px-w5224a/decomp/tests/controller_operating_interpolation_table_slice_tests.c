#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_operating_interpolation_table_slice.h"
#include "platform/firmware_memory.h"

#define DESTINATION_BASE 0x00e823ul

static u8 memory[0x1000000ul];
static fw_addr_t reads[128];
static fw_addr_t writes[128];
static unsigned read_count;
static unsigned write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    assert(read_count < sizeof(reads) / sizeof(reads[0]));
    reads[read_count++] = address;
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    writes[write_count++] = address;
    memory[address] = value;
}

static u8 ReferencePart(u8 first, u8 last, u8 code)
{
    float slope = ((float)last - (float)first) / ((float)memory[0x001059ul] - (float)memory[0x001058ul]);
    float intercept = (float)first - slope * (float)memory[0x001058ul];
    float value = slope * (float)code;

    value = value + intercept;
    return (u8)(s32)floor((double)value);
}

static void CheckRange(u8 first_code, u8 last_code, u8 column, fw_addr_t first_source, fw_addr_t last_source, u8 first_packed, u8 last_packed)
{
    u8 code;

    memset(memory, 0xa5, sizeof(memory));
    memset(reads, 0, sizeof(reads));
    memset(writes, 0, sizeof(writes));
    read_count = 0u;
    write_count = 0u;
    memory[0x001058ul] = first_code;
    memory[0x001059ul] = last_code;
    memory[0x001057ul] = column;
    memory[first_source + column] = first_packed;
    memory[last_source + column] = last_packed;

    BuildControllerOperatingInterpolationTableSlice();

    for(code = first_code; code <= last_code; ++code)
    {
        u8 index = code >= 0x14u ? (u8)(code - 0x14u) : code;
        fw_addr_t destination = DESTINATION_BASE + (fw_addr_t)((u16)index * 34u) + column;
        u8 high = ReferencePart((u8)(first_packed >> 5), (u8)(last_packed >> 5), code);
        u8 low = ReferencePart((u8)(first_packed & 0x1fu), (u8)(last_packed & 0x1fu), code);
        assert(memory[destination] == (u8)(((high & 7u) << 5) | low));
    }
    assert(write_count == (unsigned)(16u + 2u * (u8)(last_code - first_code + 1u)));
}

static void CheckInvalidRangeDoesNothing(void)
{
    memset(memory, 0xa5, sizeof(memory));
    read_count = 0u;
    write_count = 0u;
    memory[0x001058ul] = 5u;
    memory[0x001059ul] = 9u;
    BuildControllerOperatingInterpolationTableSlice();
    assert(write_count == 0u);
}

int main(void)
{
    CheckRange(4u, 10u, 2u, 0x00f83451ul, 0x00f83459ul, 0x22u, 0xb2u);
    CheckRange(0x14u, 0x18u, 3u, 0x00f83461ul, 0x00f83465ul, 0xe1u, 0x1fu);
    CheckInvalidRangeDoesNothing();
    puts("controller_operating_interpolation_table_slice_tests: ok");
    return 0;
}
