#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/radial_model_profile_coordinate.h"

static u8 memory[0x1000000ul];
static fw_addr_t reads[8];
static unsigned read_count;

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
    memory[address] = value;
}

static void StoreWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1ul] = (u8)value;
}

static void StoreDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1ul] = (u8)(value >> 16);
    memory[address + 2ul] = (u8)(value >> 8);
    memory[address + 3ul] = (u8)value;
}

static float Reference(u32 raw_scale, u16 span, u8 index, u16 endpoint)
{
    const float pi = 3.1415927f;
    float scale = (float)raw_scale;
    float radial;
    float root;
    s8 signed_index = (s8)index;

    scale = scale / 65.536f;
    radial = scale * (float)span;
    radial = radial / 1000000.0f;
    if(signed_index >= 96)
        signed_index = (s8)(signed_index - 100);
    radial = radial * (float)signed_index;
    radial = radial * 60.0f;
    radial = radial / pi;
    radial = radial + 625.0f;
    root = (float)sqrt((double)radial);
    root = root * 2.0f;
    root = root * pi;
    root = root * (float)endpoint;
    return root / (scale * 60.0f);
}

static void CheckCase(u32 raw_scale, u16 span, u8 index, u16 endpoint)
{
    float actual;
    float expected;

    memset(memory, 0, sizeof(memory));
    memset(reads, 0, sizeof(reads));
    read_count = 0u;
    StoreDword(0x000cf5ul, raw_scale);
    StoreWord(0x000cf9ul, span);
    expected = Reference(raw_scale, span, index, endpoint);
    actual = ComputeRadialModelProfileCoordinate(index, endpoint);
    assert(memcmp(&actual, &expected, sizeof(actual)) == 0);
    assert(read_count == 6u);
    assert(reads[0] == 0x000cf5ul && reads[1] == 0x000cf6ul && reads[2] == 0x000cf7ul && reads[3] == 0x000cf8ul && reads[4] == 0x000cf9ul && reads[5] == 0x000cfaul);
}

int main(void)
{
    CheckCase(0x00040000ul, 1200u, 0u, 5000u);
    CheckCase(0x00100000ul, 32768u, 95u, 0xffffu);
    CheckCase(0x00080000ul, 2400u, 96u, 32000u);
    CheckCase(0x00080000ul, 2400u, 99u, 32000u);
    CheckCase(0x00080000ul, 2400u, 0x80u, 32000u);
    puts("radial_model_profile_coordinate_tests: ok");
    return 0;
}
