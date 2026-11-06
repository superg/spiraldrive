#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_cd_track_range_match.h"

static u8 memory[0x2bc000];
static u8 bits[0x20];
static fw_addr_t convert_addresses[16];
static s32 convert_results[16];
static u8 convert_calls;

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

u8 ObserveReadCdTrackRangeBit(u8 bit_address)
{
    assert(bit_address < sizeof(bits));
    return bits[bit_address];
}

void ObserveWriteReadCdTrackRangeBit(u8 bit_address, u8 value)
{
    assert(bit_address < sizeof(bits));
    bits[bit_address] = value;
}

s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t address)
{
    u8 index = convert_calls++;

    convert_addresses[index] = address;
    return convert_results[index];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 0, sizeof(bits));
    memset(convert_addresses, 0, sizeof(convert_addresses));
    memset(convert_results, 0, sizeof(convert_results));
    convert_calls = 0u;
    WriteDword(0x1153u, 5000ul);
}

static void TestDecodedTrackSelection(void)
{
    u8 result;

    ResetState();
    convert_results[0] = 100;
    convert_results[1] = 200;
    result = MatchReadCdFramePositionToTrackRange(150, 1u, 3u, 0x96u);
    assert(result == 1u);
    assert(convert_calls == 2u);
    assert(convert_addresses[0] == 0x00f2c5ul);
    assert(convert_addresses[1] == 0x00f2cbul);
    assert(memory[0x15e9] == 2u);
    assert(bits[0x11] == 1u);
}

static void TestContinuationThresholds(void)
{
    ResetState();
    bits[0x10] = bits[0x11] = 1u;
    memory[0x1037] = 9u;
    convert_results[0] = 1000;
    assert(MatchReadCdFramePositionToTrackRange(100, 1u, 1u, 0x96u) == 1u);
    assert(memory[0x15e9] == 1u);
    assert(bits[0x10] == 0u && bits[0x11] == 0u);
    assert(memory[0x1037] == 0u);

    ResetState();
    convert_results[0] = 1000;
    assert(MatchReadCdFramePositionToTrackRange(800, 1u, 1u, 0x96u) == 1u);
    assert(bits[0x11] == 1u);
}

static void TestLatchedAdjustmentAndBias95(void)
{
    ResetState();
    convert_results[0] = 100;
    convert_results[1] = 260;
    memory[0x00f2bdul + 2u * 6u] = 0x40u;
    assert(MatchReadCdFramePositionToTrackRange(200, 1u, 3u, 0x95u) == 1u);
    assert(convert_calls == 2u);
    assert(memory[0x15e9] == 0u && bits[0x11] == 0u);
}

static void TestRawTrackAndLeadOutFallback(void)
{
    ResetState();
    memory[0x1008] = 1u;
    memory[0x2bb047ul + 4u * 10u] = 4u;
    memory[0x2bb03dul + 4u * 10u] = 4u;
    assert(MatchReadCdFramePositionToTrackRange(4800, 4u, 4u, 0x96u) == 1u);
    assert(convert_calls == 0u);
    assert(memory[0x15e9] == 4u);

    ResetState();
    convert_results[0] = 0;
    assert(MatchReadCdFramePositionToTrackRange(4800, 1u, 1u, 0x96u) == 1u);
    assert(memory[0x15e9] == 1u);
}

static void TestNoMatchAndControlRejection(void)
{
    ResetState();
    convert_results[0] = 100;
    convert_results[1] = 200;
    assert(MatchReadCdFramePositionToTrackRange(1000, 1u, 2u, 0x95u) == 1u);
    assert(convert_calls == 2u);

    ResetState();
    convert_results[0] = 1000;
    memory[0x00f2b7ul + 3u * 6u] = 0x40u;
    assert(MatchReadCdFramePositionToTrackRange(10, 3u, 3u, 0x95u) == 0u);

    ResetState();
    memory[0x1008] = 1u;
    memory[0x2bb047ul + 3u * 10u] = 3u;
    memory[0x2bb03dul + 3u * 10u] = 3u;
    memory[0x2bb03bul + 3u * 10u] = 0x40u;
    assert(MatchReadCdFramePositionToTrackRange(10, 3u, 3u, 0x95u) == 0u);
}

int main(void)
{
    TestDecodedTrackSelection();
    TestContinuationThresholds();
    TestLatchedAdjustmentAndBias95();
    TestRawTrackAndLeadOutFallback();
    TestNoMatchAndControlRejection();
    puts("read CD track range matcher tests passed");
    return 0;
}
