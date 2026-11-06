#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_track_information_free_blocks.h"

static u8 memory[1u << 16];
static unsigned divide_count;
static unsigned multiply_count;
static u32 divide_dividend;
static u32 divide_divisor;
static u32 multiply_left;
static u32 multiply_right;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[(u16)address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[(u16)address] = value;
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

u32 ObserveReadTrackInformationFreeDivide(u32 dividend, u32 divisor)
{
    divide_count++;
    divide_dividend = dividend;
    divide_divisor = divisor;
    return dividend / divisor;
}

u32 ObserveReadTrackInformationFreeMultiply(u32 factor_a, u32 factor_b)
{
    multiply_count++;
    multiply_left = factor_a;
    multiply_right = factor_b;
    return factor_a * factor_b;
}

static void WriteDword(u16 address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[(u16)(address + 1u)] = (u8)(value >> 16);
    memory[(u16)(address + 2u)] = (u8)(value >> 8);
    memory[(u16)(address + 3u)] = (u8)value;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    divide_count = 0u;
    multiply_count = 0u;
    divide_dividend = 0ul;
    divide_divisor = 0ul;
    multiply_left = 0ul;
    multiply_right = 0ul;
    WriteDword(0xdd3du, 1000ul);
    WriteDword(0xdd41u, 1500ul);
    WriteDword(0xdd45u, 1000ul);
    WriteDword(0xdd4du, 1000ul);
    WriteDword(0xdd57u, 32ul);
    WriteDword(0xdd65u, 1700ul);
    WriteDword(0xdd69u, 1600ul);
    WriteDword(0xdd6du, 1200ul);
    WriteDword(0x1153u, 2000ul);
}

static void AssertPacketGeometry(u32 span, u32 expected)
{
    assert(divide_count == 1u && multiply_count == 1u);
    assert(divide_dividend == span && divide_divisor == 39ul);
    assert(multiply_left == span / 39ul && multiply_right == 32ul);
    assert(expected == multiply_left * multiply_right);
}

static void TestClosedTrackTypesReturnZero(void)
{
    static const u8 types[] = { 4u, 5u, 6u, 7u };
    unsigned index;

    for(index = 0u; index < sizeof(types); ++index)
    {
        ResetFixture();
        memory[0xdd34u] = types[index];
        memory[0x0c84u] = 8u;
        memory[0x0c44u] = 3u;
        assert(ComputeReadTrackInformationFreeBlockCount() == 0ul);
    }
}

static void TestInactiveWriteRoutes(void)
{
    ResetFixture();
    memory[0xdd34u] = 3u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 493ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 384ul);
    AssertPacketGeometry(500ul, 384ul);

    ResetFixture();
    memory[0xdd34u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 993ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 800ul);

    ResetFixture();
    memory[0xdd34u] = 2u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 498ul);
}

static void TestInactiveTypeZeroWriteTypes(void)
{
    static const u32 expected[] = { 998ul, 998ul, 650ul, 400ul };
    u8 write_type;

    for(write_type = 0u; write_type < 4u; ++write_type)
    {
        ResetFixture();
        memory[0xdd34u] = 0u;
        memory[0x0c44u] = write_type;
        assert(ComputeReadTrackInformationFreeBlockCount() == expected[write_type]);
    }

    ResetFixture();
    memory[0xdd34u] = 0u;
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 800ul);
    AssertPacketGeometry(1005ul, 800ul);
}

static void TestActiveWriteTypeOneTwoAndThree(void)
{
    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0x0c44u] = 1u;
    memory[0xdd34u] = 0u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 998ul);
    memory[0xdd34u] = 2u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 498ul);

    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0x0c44u] = 2u;
    memory[0xdd34u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 650ul);

    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0x0c44u] = 3u;
    memory[0xdd34u] = 3u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 300ul);
}

static void TestActiveWriteTypeZeroRoutes(void)
{
    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0xdd34u] = 3u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 498ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 384ul);
    AssertPacketGeometry(505ul, 384ul);

    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0xdd34u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 998ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 800ul);
}

static void TestNegativeClampAndUndefinedStateNormalization(void)
{
    ResetFixture();
    memory[0xdd34u] = 2u;
    WriteDword(0xdd41u, 900ul);
    assert(ComputeReadTrackInformationFreeBlockCount() == 0ul);

    ResetFixture();
    memory[0x0c84u] = 8u;
    memory[0x0c44u] = 1u;
    memory[0xdd34u] = 1u;
    assert(ComputeReadTrackInformationFreeBlockCount() == 0ul);
}

int main(void)
{
    TestClosedTrackTypesReturnZero();
    TestInactiveWriteRoutes();
    TestInactiveTypeZeroWriteTypes();
    TestActiveWriteTypeOneTwoAndThree();
    TestActiveWriteTypeZeroRoutes();
    TestNegativeClampAndUndefinedStateNormalization();
    return 0;
}
