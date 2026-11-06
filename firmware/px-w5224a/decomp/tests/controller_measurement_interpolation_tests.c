#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_interpolation.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static float ReadFloat(fw_addr_t address)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.bits = ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
    return converted.value;
}

static float AbsoluteFloat(float value)
{
    return value < 0.0f ? -value : value;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
}

static void CheckDegenerateState(void)
{
    Reset();
    WriteDword(0x0012e6ul, 12u);
    WriteDword(0x0012eaul, 6u);
    WriteDword(0x0012eeul, 9u);
    WriteDword(0x0012f2ul, 20u);
    memory[0x0012f6ul] = 3u;
    WriteDword(0x0012deul, 0x12345678ul);
    WriteDword(0x0012e2ul, 0x9abcdef0ul);
    SolveControllerMeasurementInterpolationAtSelectedInterval();
    assert(memory[0x00ef80ul] == 2u);
    assert(((u32)memory[0x0012deul] << 24 | (u32)memory[0x0012dfu] << 16 | (u32)memory[0x0012e0u] << 8 | memory[0x0012e1u]) == 0x12345678ul);
    assert(((u32)memory[0x0012e2ul] << 24 | (u32)memory[0x0012e3u] << 16 | (u32)memory[0x0012e4u] << 8 | memory[0x0012e5u]) == 0x9abcdef0ul);
}

static void CheckCoefficients(void)
{
    float denominator;
    float numerator;
    float coefficient_e2;
    float coefficient_de;

    Reset();
    WriteDword(0x0012e6ul, 14u);
    WriteDword(0x0012eaul, 6u);
    WriteDword(0x0012eeul, 9u);
    WriteDword(0x0012f2ul, 20u);
    memory[0x0012f6ul] = 3u;

    denominator = (float)3 * (float)14;
    denominator = denominator / (float)6;
    denominator = denominator - (float)6;
    numerator = (float)9 * (float)14;
    numerator = numerator / (float)6;
    numerator = numerator - (float)20;
    coefficient_e2 = numerator / denominator;
    numerator = (float)6 * coefficient_e2;
    numerator = (float)20 - numerator;
    coefficient_de = numerator / (float)14;

    SolveControllerMeasurementInterpolationAtSelectedInterval();
    assert(memory[0x00ef80ul] == 0u);
    assert(AbsoluteFloat(ReadFloat(0x0012e2ul) - coefficient_e2) < 0.000001f);
    assert(AbsoluteFloat(ReadFloat(0x0012deul) - coefficient_de) < 0.000001f);
}

int main(void)
{
    CheckDegenerateState();
    CheckCoefficients();
    return 0;
}
