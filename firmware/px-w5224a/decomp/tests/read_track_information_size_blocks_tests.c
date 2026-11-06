#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_track_information_size_blocks.h"

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

u32 ObserveReadTrackInformationSizeDivide(u32 dividend, u32 divisor)
{
    divide_count++;
    divide_dividend = dividend;
    divide_divisor = divisor;
    return dividend / divisor;
}

u32 ObserveReadTrackInformationSizeMultiply(u32 factor_a, u32 factor_b)
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
    WriteDword(0xdd41u, 1502ul);
    WriteDword(0xdd51u, 1600ul);
    WriteDword(0x1153u, 2002ul);
    WriteDword(0xdd57u, 32ul);
}

static void AssertPacketGeometry(u32 expected_span_plus_five, u32 expected_result)
{
    assert(divide_count == 1u && multiply_count == 1u);
    assert(divide_dividend == expected_span_plus_five);
    assert(divide_divisor == 39ul);
    assert(multiply_left == expected_span_plus_five / 39ul);
    assert(multiply_right == 32ul);
    assert(expected_result == multiply_left * multiply_right);
}

static void TestTypesThreeFourFiveUseWorkingEnd(void)
{
    static const u8 types[] = { 3u, 4u, 5u };
    unsigned index;

    for(index = 0u; index < sizeof(types); ++index)
    {
        ResetFixture();
        memory[0xdd34u] = types[index];
        assert(ComputeReadTrackInformationTrackSizeBlocks() == 500ul);
        assert(divide_count == 0u && multiply_count == 0u);

        memory[0xdd56u] = 1u;
        assert(ComputeReadTrackInformationTrackSizeBlocks() == 416ul);
        AssertPacketGeometry(507ul, 416ul);
    }
}

static void TestTypeZeroSelectionAndPacketGate(void)
{
    ResetFixture();
    memory[0xdd34u] = 0u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 1000ul);

    ResetFixture();
    memory[0xdd34u] = 0u;
    memory[0x0c46u] = 1u;
    memory[0xe4ecu] = 8u;
    memory[0xdd35u] = 7u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 500ul);

    ResetFixture();
    memory[0xdd34u] = 0u;
    memory[0x0c46u] = 3u;
    memory[0xe4ecu] = 7u;
    memory[0xdd35u] = 7u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 800ul);
    AssertPacketGeometry(1007ul, 800ul);
}

static void TestTypesOneTwoSixAndSeven(void)
{
    ResetFixture();
    memory[0xdd34u] = 1u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 1000ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 800ul);

    ResetFixture();
    memory[0xdd34u] = 2u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 500ul);

    ResetFixture();
    memory[0xdd34u] = 6u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 600ul);
    memory[0xdd56u] = 1u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 480ul);

    ResetFixture();
    memory[0xdd34u] = 7u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 0ul);
    memory[0xdd5fu] = 1u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 500ul);
}

static void TestSignedClampAndInvalidTypeFallback(void)
{
    ResetFixture();
    memory[0xdd34u] = 2u;
    WriteDword(0xdd41u, 1002ul);
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 0ul);
    WriteDword(0xdd41u, 0x80000400ul);
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 0ul);

    ResetFixture();
    memory[0xdd34u] = 8u;
    assert(ComputeReadTrackInformationTrackSizeBlocks() == 0ul);
}

int main(void)
{
    TestTypesThreeFourFiveUseWorkingEnd();
    TestTypeZeroSelectionAndPacketGate();
    TestTypesOneTwoSixAndSeven();
    TestSignedClampAndInvalidTypeFallback();
    return 0;
}
