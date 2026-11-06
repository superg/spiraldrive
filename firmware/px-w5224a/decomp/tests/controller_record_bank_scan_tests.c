#include <assert.h>
#include <string.h>
#include "platform/controller_record_bank_scan.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static s32 compressed_inputs[8];
static s32 expanded_inputs[256];
static unsigned compress_count;
static unsigned expand_count;
static unsigned remainder_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

s32 CompressThirtyNineFramePositionToLogicalBlockAddress(s32 value)
{
    compressed_inputs[compress_count++] = value;
    return value;
}

s32 ExpandLogicalBlockAddressToThirtyNineFramePosition(s32 value)
{
    expanded_inputs[expand_count++] = value;
    return value + 150l;
}

u32 DivideU32RemainderNative(u32 dividend, u32 divisor)
{
    ++remainder_count;
    return dividend % divisor;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteU24(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 16);
    memory[address + 1u] = (u8)(value >> 8);
    memory[address + 2u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static fw_addr_t BankAddress(u8 index)
{
    return 0x194000ul + (fw_addr_t)index * 0x800ul;
}

static void SetBank(u8 index, u32 header_first, u32 header_second, u16 record_count, u16 alternate_offset)
{
    fw_addr_t bank = BankAddress(index);

    WriteU16(bank + 8u, record_count);
    WriteU16(bank + 0x0cu, alternate_offset);
    WriteU24(bank + 0x11u, header_first);
    WriteU24(bank + 0x15u, header_second);
}

static void SetRecordAtOffset(u8 bank_index, u16 offset, u32 first, u32 second)
{
    fw_addr_t record = BankAddress(bank_index) + offset;

    WriteU24(record, first);
    WriteU24(record + 3u, second);
}

static void SetRecord(u8 bank_index, u16 record_index, u32 first, u32 second)
{
    SetRecordAtOffset(bank_index, (u16)(0x20u + record_index * 6u), first, second);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(compressed_inputs, 0, sizeof(compressed_inputs));
    memset(expanded_inputs, 0, sizeof(expanded_inputs));
    compress_count = 0u;
    expand_count = 0u;
    remainder_count = 0u;
}

static void TestEmptyScanClearsSummariesAndScratchCount(void)
{
    unsigned index;

    ResetFixture();
    memset(&memory[0x2b9700u], 0xa5, 0x1cu);
    memory[0xfc40u] = 0x12u;
    memory[0xfc41u] = 0x34u;
    memory[0xfc42u] = 0x56u;
    memory[0xfc43u] = 0x78u;

    assert(ScanControllerRecordBanksForFrameRange(0u, 100, 20ul) == 0u);
    assert(compress_count == 1u && compressed_inputs[0] == 100);
    assert(ReadU32(0x2b9714u) == 100ul);
    assert(ReadU32(0x2b9718u) == 119ul);
    assert(ReadU16(0xfc40u) == 0u && ReadU16(0xfc42u) == 0u);
    for(index = 0u; index < 0x10u; ++index)
        assert(memory[0x2b9700u + index] == 0u);
}

static void TestNormalRecordsPublishExtremaAndDirectContinuity(void)
{
    ResetFixture();
    SetBank(0u, 0ul, 0ul, 3u, 0u);
    SetRecord(0u, 0u, 100ul, 200ul);
    SetRecord(0u, 1u, 101ul, 201ul);
    SetRecord(0u, 2u, 90ul, 190ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 90, 20ul) == 3u);
    assert(ReadU32(0x2b9704u) == 240ul);
    assert(ReadU32(0x2b9700u) == 340ul);
    assert(ReadU32(0x2b970cu) == 251ul);
    assert(ReadU32(0x2b9708u) == 351ul);
    assert(ReadU16(0xfc42u) == 1u);
}

static void TestThirtyNineAlignedEightFrameContinuity(void)
{
    ResetFixture();
    SetBank(0u, 0ul, 0ul, 2u, 0u);
    SetRecord(0u, 0u, 39ul, 78ul);
    SetRecord(0u, 1u, 47ul, 86ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 0, 100ul) == 2u);
    assert(ReadU16(0xfc42u) == 2u);
    assert(remainder_count == 2u);
}

static void TestDiscontinuityStopsLaterContinuityGrowth(void)
{
    ResetFixture();
    SetBank(0u, 0ul, 0ul, 4u, 0u);
    SetRecord(0u, 0u, 100ul, 200ul);
    SetRecord(0u, 1u, 101ul, 201ul);
    SetRecord(0u, 2u, 110ul, 210ul);
    SetRecord(0u, 3u, 111ul, 211ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 95, 30ul) == 4u);
    assert(ReadU16(0xfc42u) == 2u);
}

static void TestSentinelBankUsesSpecialAlternateOffset(void)
{
    ResetFixture();
    memory[0xca1u] = 1u;
    SetBank(2u, 0xfffffful, 0ul, 2u, 0x26u);
    SetRecordAtOffset(2u, 0x20u, 50ul, 60ul);
    SetRecordAtOffset(2u, 0x26u, 100ul, 200ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 100, 1ul) == 1u);
    assert(ReadU32(0x2b9704u) == 250ul);
    assert(ReadU32(0x2b9700u) == 350ul);
}

static void TestForceScanOverridesRejectedBankHeader(void)
{
    ResetFixture();
    SetBank(1u, 0xfffffful, 0ul, 1u, 0u);
    SetRecord(1u, 0u, 100ul, 200ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 100, 1ul) == 0u);
    assert(ScanControllerRecordBanksForFrameRange(1u, 100, 1ul) == 1u);
}

static void TestSpecialTypeThreeInitializesSinglePositionSummary(void)
{
    ResetFixture();
    memory[0xc89u] = 1u;
    SetBank(0u, 0ul, 0ul, 1u, 0u);
    SetRecord(0u, 0u, 0x300001ul, 120ul);

    assert(ScanControllerRecordBanksForFrameRange(0u, 120, 1ul) == 1u);
    assert(ReadU32(0x2b9700u) == 270ul);
    assert(ReadU32(0x2b9704u) == 270ul);
    assert(ReadU32(0x2b9708u) == 270ul);
    assert(ReadU32(0x2b970cu) == 270ul);
    assert(ReadU16(0xfc42u) == 0u);
}

int main(void)
{
    TestEmptyScanClearsSummariesAndScratchCount();
    TestNormalRecordsPublishExtremaAndDirectContinuity();
    TestThirtyNineAlignedEightFrameContinuity();
    TestDiscontinuityStopsLaterContinuityGrowth();
    TestSentinelBankUsesSpecialAlternateOffset();
    TestForceScanOverridesRejectedBankHeader();
    TestSpecialTypeThreeInitializesSinglePositionSummary();
    return 0;
}
