#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/mip_controller_record_snapshot.h"

static u8 memory[0x300000ul];
static unsigned divide_calls;
static s32 observed_dividend;
static s32 observed_divisor;

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

s32 ObserveMipControllerRecordSnapshotDivide(s32 dividend, s32 divisor)
{
    divide_calls++;
    observed_dividend = dividend;
    observed_divisor = divisor;
    return dividend / divisor;
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

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0, sizeof(memory));
    memset(memory + 0x2ba5dfu, 0xa5, 0x51u);
    divide_calls = 0u;
    observed_dividend = 0;
    observed_divisor = 0;

    WriteU32(0x2ba525u, 0x12345678ul);
    WriteU32(0x2ba529u, 0x89abcdeful);
    WriteU32(0x2ba52eu, 0xfffabcdeul);
    WriteU32(0x2ba532u, 0x00123456ul);
    memory[0x2ba536u] = 7u;
    for(index = 0u; index < 8u; ++index)
    {
        WriteU32(0x2ba537u + index * 4u, 0xab100000ul + (u32)index * 0x010203ul);
        WriteU32(0x2ba557u + index * 4u, 0xcd200000ul + (u32)index * 0x030201ul);
    }
}

static void AssertPacked20(unsigned offset, u32 value)
{
    assert(memory[0x2ba5dfu + offset] == (u8)((value >> 16) & 0x0fu));
    assert(memory[0x2ba5dfu + offset + 1u] == (u8)(value >> 8));
    assert(memory[0x2ba5dfu + offset + 2u] == (u8)value);
}

static void TestCompleteNonzeroSnapshot(void)
{
    static const u8 fixed_prefix[24] = { 0x4d, 0x49, 0x50, 0x01, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x00, 0xab, 0xcd, 0xef, 0x00, 0x20, 0x00, 0x08, 0x00, 0x88, 0x00, 0x00 };
    u32 expected_derived;
    unsigned index;

    ResetFixture();
    memory[0x2ba5afu] = 1u;
    memory[0x000c9eu] = 3u;
    WriteU32(0x2ba5a3u, 1000ul);

    BuildMipControllerRecordSnapshotBlock();

    assert(memcmp(memory + 0x2ba5dfu, fixed_prefix, sizeof(fixed_prefix)) == 0);
    assert(memory[0x2ba5f7u] == 0x90u);
    AssertPacked20(25u, 0xfffabcdeul);
    assert(divide_calls == 1u);
    assert(observed_dividend == (s32)((1000ul + 0xa7ul) << 5));
    assert(observed_divisor == 39l);
    expected_derived = (u32)(observed_dividend / observed_divisor);
    assert(ReadU32(0x2ba532u) == expected_derived);
    AssertPacked20(28u, expected_derived);
    assert(memory[0x2ba5feu] == 7u);
    for(index = 0u; index < 8u; ++index)
    {
        AssertPacked20(32u + index * 3u, 0xab100000ul + (u32)index * 0x010203ul);
        AssertPacked20(56u + index * 3u, 0xcd200000ul + (u32)index * 0x030201ul);
    }
    assert(memory[0x2ba62eu] == (u8)(0xcd200000ul + 7ul * 0x030201ul));
    assert(memory[0x2ba62fu] == 0xa5u);
}

static void TestZeroSourceClearsDerivedPosition(void)
{
    ResetFixture();
    memory[0x2ba5afu] = 0u;
    WriteU32(0x2ba5a3u, 0ul);
    WriteU32(0x2ba532u, 0ul);

    BuildMipControllerRecordSnapshotBlock();

    assert(memory[0x2ba5f7u] == 0x40u);
    assert(divide_calls == 0u);
    assert(ReadU32(0x2ba532u) == 0ul);
    AssertPacked20(28u, 0ul);
}

static void TestFlagRoutesAndMatchingPosition(void)
{
    ResetFixture();
    memory[0x2ba5afu] = 1u;
    memory[0x000c9eu] = 2u;
    WriteU32(0x2ba532u, 0xfffabcdeul);
    WriteU32(0x2ba5a3u, 1ul);

    BuildMipControllerRecordSnapshotBlock();

    assert(memory[0x2ba5f7u] == 0x40u);
}

static void TestWrappedSignedNumerator(void)
{
    u32 source = 0x03ffff59ul;
    u32 wrapped = (source + 0xa7ul) << 5;

    ResetFixture();
    WriteU32(0x2ba5a3u, source);
    BuildMipControllerRecordSnapshotBlock();

    assert(divide_calls == 1u);
    assert(observed_dividend == (s32)wrapped);
    assert(observed_dividend < 0);
    assert(ReadU32(0x2ba532u) == (u32)(observed_dividend / observed_divisor));
}

int main(void)
{
    TestCompleteNonzeroSnapshot();
    TestZeroSourceClearsDerivedPosition();
    TestFlagRoutesAndMatchingPosition();
    TestWrappedSignedNumerator();
    puts("MIP controller record snapshot tests passed");
    return 0;
}
