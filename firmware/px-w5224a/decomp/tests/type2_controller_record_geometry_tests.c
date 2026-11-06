#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/type2_controller_record_geometry.h"

enum ArithmeticKind
{
    ARITHMETIC_SIGNED_DIVIDE,
    ARITHMETIC_UNSIGNED_DIVIDE,
    ARITHMETIC_UNSIGNED_REMAINDER,
    ARITHMETIC_MULTIPLY
};

struct ArithmeticCall
{
    enum ArithmeticKind kind;
    u32 left;
    u32 right;
};

static u8 memory[0x300000ul];
static fw_addr_t write_addresses[512];
static u8 write_values[512];
static unsigned write_count;
static struct ArithmeticCall arithmetic_calls[32];
static unsigned arithmetic_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    assert(write_count < sizeof(write_addresses) / sizeof(write_addresses[0]));
    write_addresses[write_count] = address;
    write_values[write_count] = value;
    ++write_count;
    memory[address] = value;
}

static void RecordArithmetic(enum ArithmeticKind kind, u32 left, u32 right)
{
    assert(arithmetic_count < sizeof(arithmetic_calls) / sizeof(arithmetic_calls[0]));
    arithmetic_calls[arithmetic_count].kind = kind;
    arithmetic_calls[arithmetic_count].left = left;
    arithmetic_calls[arithmetic_count].right = right;
    ++arithmetic_count;
}

s32 ObserveType2GeometryDivideS32(s32 dividend, s32 divisor)
{
    RecordArithmetic(ARITHMETIC_SIGNED_DIVIDE, (u32)dividend, (u32)divisor);
    return dividend / divisor;
}

u32 ObserveType2GeometryDivideU32Quotient(u32 dividend, u32 divisor)
{
    RecordArithmetic(ARITHMETIC_UNSIGNED_DIVIDE, dividend, divisor);
    return dividend / divisor;
}

u32 ObserveType2GeometryDivideU32Remainder(u32 dividend, u32 divisor)
{
    RecordArithmetic(ARITHMETIC_UNSIGNED_REMAINDER, dividend, divisor);
    return dividend % divisor;
}

u32 ObserveType2GeometryMultiplyU32(u32 factor_a, u32 factor_b)
{
    RecordArithmetic(ARITHMETIC_MULTIPLY, factor_a, factor_b);
    return factor_a * factor_b;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(u32 lead_out)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(memory + 0xfb03u, 0, 40u);
    memset(memory + 0xfb2bu, 0, 40u);
    memory[0x1008u] = 2u;
    WriteU32(0x1153u, lead_out);
    write_count = 0u;
    arithmetic_count = 0u;
}

static void SetSlot(fw_addr_t state_address, u8 state, s32 frame)
{
    memory[state_address] = state;
    WriteU32(state_address + 1u, (u32)frame);
}

static void AssertCommonGeometry(u8 active_segments, u32 high_frame, u32 maximum_lba, u8 group_count)
{
    u8 index;

    assert(ReadU32(0xfc44u) == 0x00800000ul);
    assert(memory[0x2ba588u] == active_segments);
    assert(ReadU32(0x2ba58du) == high_frame);
    assert(ReadU32(0x2ba529u) == maximum_lba);
    assert(memory[0x2ba536u] == group_count);
    assert(ReadU32(0x2ba591u) == (u32)-39l);
    assert(ReadU32(0x2ba595u) == high_frame - 39ul);
    assert(ReadU32(0x2ba599u) == high_frame - 39ul - 0x138ul);
    assert(ReadU32(0x2ba589u) == ReadU32(0x2ba599u));
    for(index = group_count; index < 8u; ++index)
    {
        assert(ReadU32(0x2ba537u + (u16)index * 4u) == 0x00fffffful);
        assert(ReadU32(0x2ba557u + (u16)index * 4u) == 0x00fffffful);
    }
}

static void TestMediaGatePreservesAllState(void)
{
    ResetFixture(50000ul);
    memory[0x1008u] = 1u;

    InitializeType2ControllerRecordGeometry();

    assert(write_count == 0u);
    assert(arithmetic_count == 0u);
}

static void TestLowRemainderBuildsOneDefaultGroup(void)
{
    u32 lead_out = 0x0a78ul + 8ul * 0x15f0ul + 100ul;
    u32 high_frame = 8ul * 0x15f0ul + 0x0a78ul;
    s32 secondary_seed = (s32)(((high_frame - 39ul - 0x91ul) << 5) / 39ul);

    ResetFixture(lead_out);
    SetSlot(0xfb03u, 0u, 0x12345678l);
    SetSlot(0xfb2bu, 0u, 0x23456789l);
    InitializeType2ControllerRecordGeometry();

    AssertCommonGeometry(8u, high_frame, 34815ul, 1u);
    assert(ReadU32(0x2ba537u) == (u32)-214l);
    assert(ReadU32(0xfb04u) == (u32)-214l);
    assert(ReadU32(0x2ba577u) == (u32)-214l);
    assert(ReadU32(0x2ba557u) == (u32)(secondary_seed - 32l));
    assert(ReadU32(0xfb2cu) == (u32)(secondary_seed - 32l));
    assert(ReadU32(0x2ba57bu) == (u32)(secondary_seed - 32l));
    assert(ReadU16(0x2ba5d9u) == (u16)((s32)(high_frame - 39ul - 0x138ul - 0x91ul) / 39l));

    assert(arithmetic_count == 5u);
    assert(arithmetic_calls[0].kind == ARITHMETIC_SIGNED_DIVIDE);
    assert(arithmetic_calls[0].left == lead_out - 0x0a78ul);
    assert(arithmetic_calls[1].kind == ARITHMETIC_UNSIGNED_REMAINDER);
    assert(arithmetic_calls[2].kind == ARITHMETIC_MULTIPLY);
    assert(arithmetic_calls[2].left == 7u && arithmetic_calls[2].right == 0x88u);
    assert(arithmetic_calls[3].kind == ARITHMETIC_SIGNED_DIVIDE);
    assert(arithmetic_calls[4].kind == ARITHMETIC_SIGNED_DIVIDE);
}

static void TestMiddleRemainderRoundsToTwoGroups(void)
{
    u32 lead_out = 0x0a78ul + 9ul * 0x15f0ul + 320ul;
    u32 high_frame = 9ul * 0x15f0ul + 0x0bb0ul;

    ResetFixture(lead_out);
    InitializeType2ControllerRecordGeometry();

    AssertCommonGeometry(10u, high_frame, ((((9ul - 1ul) * 0x88ul) + 0x88ul) << 5) - 1ul, 2u);
    assert(ReadU32(0x2ba537u) == (u32)-214l);
    assert(ReadU32(0x2ba53bu) == (u32)-246l);
    assert(ReadU32(0x2ba577u) == (u32)-246l);
    assert(memory[0xfb03u] == 0u);
    assert(memory[0xfb08u] == 0u);
    assert(arithmetic_count == 5u);
    assert(arithmetic_calls[2].kind == ARITHMETIC_MULTIPLY);
}

static void TestResidualBranchHonorsExistingSlotFrames(void)
{
    u32 lead_out = 0x0a78ul + 15ul * 0x15f0ul + 390ul;
    u32 high_frame = 15ul * 0x15f0ul + 0x0bb0ul + 2ul * 39ul;
    s32 secondary_seed = (s32)(((high_frame - 39ul - 0x91ul) << 5) / 39ul);

    ResetFixture(lead_out);
    SetSlot(0xfb03u, 1u, -100l);
    SetSlot(0xfb08u, 0u, 1234l);
    SetSlot(0xfb2bu, 2u, -50000l);
    SetSlot(0xfb30u, 0u, 5678l);
    InitializeType2ControllerRecordGeometry();

    AssertCommonGeometry(16u, high_frame, ((((16ul - 1ul) * 0x88ul) + 2ul) << 5) - 1ul, 2u);
    assert(ReadU32(0x2ba537u) == (u32)-100l);
    assert(ReadU32(0x2ba53bu) == (u32)-246l);
    assert(ReadU32(0xfb04u) == (u32)-100l);
    assert(ReadU32(0xfb09u) == (u32)-246l);
    assert(ReadU32(0x2ba577u) == (u32)-246l);
    assert(ReadU32(0x2ba557u) == (u32)-50000l);
    assert(ReadU32(0x2ba55bu) == (u32)(secondary_seed - 64l));
    assert(ReadU32(0xfb2cu) == (u32)-50000l);
    assert(ReadU32(0xfb31u) == (u32)(secondary_seed - 64l));
    assert(ReadU32(0x2ba57bu) == (u32)-50000l);

    assert(arithmetic_count == 8u);
    assert(arithmetic_calls[2].kind == ARITHMETIC_UNSIGNED_DIVIDE);
    assert(arithmetic_calls[2].left == 78ul && arithmetic_calls[2].right == 39ul);
    assert(arithmetic_calls[3].kind == ARITHMETIC_MULTIPLY);
    assert(arithmetic_calls[3].left == 15ul);
    assert(arithmetic_calls[4].kind == ARITHMETIC_MULTIPLY);
    assert(arithmetic_calls[4].left == 2ul);
    assert(arithmetic_calls[5].kind == ARITHMETIC_MULTIPLY);
    assert(arithmetic_calls[5].left == 15ul && arithmetic_calls[5].right == 0x88ul);
}

static void TestLeadOutClampBuildsAllEightGroups(void)
{
    u32 high_frame = 63ul * 0x15f0ul + 0x0bb0ul + 82ul * 39ul;
    u8 index;

    ResetFixture(360001ul);
    InitializeType2ControllerRecordGeometry();

    AssertCommonGeometry(64u, high_frame, ((((64ul - 1ul) * 0x88ul) + 82ul) << 5) - 1ul, 8u);
    for(index = 0u; index < 8u; ++index)
        assert(ReadU32(0x2ba537u + (u16)index * 4u) == (u32)(-214l - (s32)index * 32l));
    assert(ReadU32(0x2ba577u) == (u32)-438l);
}

int main(void)
{
    TestMediaGatePreservesAllState();
    TestLowRemainderBuildsOneDefaultGroup();
    TestMiddleRemainderRoundsToTwoGroups();
    TestResidualBranchHonorsExistingSlotFrames();
    TestLeadOutClampBuildsAllEightGroups();
    puts("type-two controller-record geometry tests passed");
    return 0;
}
