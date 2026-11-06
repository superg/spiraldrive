#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_measurement_transition.h"
#include "platform/firmware_memory.h"

static u8 memory_bytes[0x1000000ul];
static u8 mapped_profile;
static u32 forced_root;
static unsigned signed_divide_count;
static unsigned unsigned_divide_count;
static unsigned multiply_count;
static unsigned square_root_count;
static unsigned map_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory_bytes[address] = (u8)(value >> 8);
    memory_bytes[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

s32 DivideS32Quotient(s32 dividend, s32 divisor)
{
    ++signed_divide_count;
    assert(divisor != 0l);
    return dividend / divisor;
}

u32 DivideU32Quotient(u32 dividend, u32 divisor)
{
    ++unsigned_divide_count;
    assert(divisor != 0ul);
    return dividend / divisor;
}

u32 MultiplyU32Low(u32 factor_a, u32 factor_b)
{
    ++multiply_count;
    return factor_a * factor_b;
}

u32 IntegerSquareRootU32(u32 value)
{
    u32 root;

    ++square_root_count;
    if(forced_root != 0ul)
        return forced_root;
    root = 0ul;
    while((root + 1ul) <= value / (root + 1ul))
        ++root;
    return root;
}

u8 MapOperatingCodeToProfileIndex(u8 operating_code)
{
    (void)operating_code;
    ++map_count;
    return mapped_profile;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    mapped_profile = 1u;
    forced_root = 0ul;
    signed_divide_count = 0u;
    unsigned_divide_count = 0u;
    multiply_count = 0u;
    square_root_count = 0u;
    map_count = 0u;
    memory_bytes[0x1006u] = 1u;
    WriteU16(0x0cf9u, 100u);
    WriteU16(0xff89e6ul, 60u);
}

static void AssertNoDynamicArithmetic(void)
{
    assert(signed_divide_count == 0u);
    assert(unsigned_divide_count == 0u);
    assert(multiply_count == 0u);
    assert(square_root_count == 0u);
    assert(map_count == 0u);
}

static void TestFixedReferenceSelection(void)
{
    static const u8 codes[] = { 0x18u, 0x20u, 0x28u };
    static const fw_addr_t addresses[] = { 0x0cebul, 0x0cedul, 0x0ceful };
    unsigned index;

    for(index = 0u; index < sizeof(codes); ++index)
    {
        ResetFixture();
        memory_bytes[0x1006u] = 0u;
        WriteU16(addresses[index], (u16)(0x1100u + index));
        WriteU16(0xfc25u, 0xffffu);
        assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, codes[index]) == (u32)(0x1100u + index));
        assert(ReadU16(0xfc25u) == 0u);
        AssertNoDynamicArithmetic();
    }

    ResetFixture();
    memory_bytes[0x103au] = 0x40u;
    WriteU16(0x0ce9u, 0x2222u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 0x0au) == 0x2222ul);
    AssertNoDynamicArithmetic();
}

static void TestHighAndErasedCodeReferenceSelection(void)
{
    ResetFixture();
    memory_bytes[0x1006u] = 0u;
    memory_bytes[0x0e43u] = 0x33u;
    WriteU16(0x0cf1u, 0x3333u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 0x34u) == 0x3333ul);

    ResetFixture();
    memory_bytes[0x0e43u] = 0x34u;
    WriteU16(0x0cf3u, 0x4444u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 0xffu) == 0x4444ul);
    AssertNoDynamicArithmetic();
}

static void TestDynamicReferenceArithmetic(void)
{
    ResetFixture();
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 1u) == 10000ul);
    assert(ReadU16(0xfc25u) == 0u);
    assert(signed_divide_count == 3u);
    assert(unsigned_divide_count == 2u);
    assert(multiply_count == 7u);
    assert(square_root_count == 1u);
    assert(map_count == 1u);
}

static void TestAuxiliaryWordDivisorsAndProfileZero(void)
{
    ResetFixture();
    forced_root = 451ul;
    WriteU16(0xff89e6ul, 40u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 1u) == 10000ul);
    assert(ReadU16(0xfc25u) == 3u);

    ResetFixture();
    forced_root = 416ul;
    mapped_profile = 0u;
    WriteU16(0xff89e4ul, 64u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 2u) == 8000ul);
    assert(ReadU16(0xfc25u) == 2u);
}

static void TestEnabledHighCodeUsesDynamicPath(void)
{
    ResetFixture();
    forced_root = 250ul;
    WriteU16(0xff89e6ul, 72u);
    assert(ComputeControllerMeasurementReferenceForOperatingCode(750ul, 10u, 0x18u) == 500ul);
    assert(signed_divide_count == 3u);
}

int main(void)
{
    TestFixedReferenceSelection();
    TestHighAndErasedCodeReferenceSelection();
    TestDynamicReferenceArithmetic();
    TestAuxiliaryWordDivisorsAndProfileZero();
    TestEnabledHighCodeUsesDynamicPath();
    puts("controller measurement reference tests passed");
    return 0;
}
