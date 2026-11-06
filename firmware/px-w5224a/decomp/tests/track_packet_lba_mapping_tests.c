#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/track_packet_lba_mapping.h"

struct Conversion
{
    fw_addr_t address;
    s32 result;
};

static u8 memory[0x20000ul];
static struct Conversion conversions[8];
static unsigned conversion_count;
static unsigned conversion_index;
static u8 bit_000b;
static u8 bit_000c;
static u8 bit_0021;
static u8 effective_last_track;
static unsigned last_track_call_count;
static unsigned divide_count;
static u32 divide_left;
static u32 divide_right;
static unsigned multiply_count;
static u32 multiply_left;
static u32 multiply_right;

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

u8 FirmwareReadBit000B(void)
{
    return bit_000b;
}
void FirmwareWriteBit000C(u8 value)
{
    bit_000c = value;
}
u8 FirmwareReadBit0021(void)
{
    return bit_0021;
}

s32 ObserveTrackPacketConvertMsf(fw_addr_t address)
{
    assert(conversion_index < conversion_count);
    assert(conversions[conversion_index].address == address);
    return conversions[conversion_index++].result;
}

u8 ObserveTrackPacketLastTrack(void)
{
    ++last_track_call_count;
    return effective_last_track;
}

u32 ObserveTrackPacketDivide(u32 dividend, u32 divisor)
{
    ++divide_count;
    divide_left = dividend;
    divide_right = divisor;
    return dividend / divisor;
}

u32 ObserveTrackPacketMultiply(u32 left, u32 right)
{
    ++multiply_count;
    multiply_left = left;
    multiply_right = right;
    return left * right;
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

static fw_addr_t Field(u8 track_number, fw_addr_t base)
{
    return base + (u16)track_number * 0x13u;
}

static void AddConversion(fw_addr_t address, s32 result)
{
    assert(conversion_count < sizeof(conversions) / sizeof(conversions[0]));
    conversions[conversion_count].address = address;
    conversions[conversion_count].result = result;
    ++conversion_count;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(conversions, 0, sizeof(conversions));
    conversion_count = 0u;
    conversion_index = 0u;
    bit_000b = 1u;
    bit_000c = 0xa5u;
    bit_0021 = 0u;
    effective_last_track = 3u;
    last_track_call_count = 0u;
    divide_count = 0u;
    divide_left = divide_right = 0ul;
    multiply_count = 0u;
    multiply_left = multiply_right = 0ul;
    memory[0x1149u] = 1u;
    memory[0x1147u] = 2u;
}

static void PrepareTrackTwo(s32 scan_end, s32 packet_end, s32 packet_start, u32 packet_size)
{
    memory[Field(2u, 0xdd89u)] = 1u;
    WriteU32(Field(2u, 0xdd8au), packet_size);
    AddConversion(Field(2u, 0xdd94u), scan_end);
    AddConversion(Field(2u, 0xdd84u), packet_end);
    AddConversion(Field(2u, 0xdd81u), packet_start);
}

static void TestDisabledLayoutReturnsBiasedAddressAndClearsFlag(void)
{
    ResetFixture();
    bit_000b = 0u;

    assert(MapLogicalBlockAddressThroughTrackPacketLayout(-200l) == -50l);
    assert(bit_000c == 0u && last_track_call_count == 0u);
}

static void TestValidPacketLayoutAddsSevenFramesPerCompletedPacket(void)
{
    ResetFixture();
    PrepareTrackTwo(1000l, 500l, 200l, 32ul);

    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 257l);
    assert(conversion_index == 3u);
    assert(divide_count == 1u && divide_left == 50ul && divide_right == 32ul);
    assert(multiply_count == 1u && multiply_left == 7ul && multiply_right == 1ul);
    assert(ReadU32(0x0f0cu) == 7ul);
    assert(bit_000c == 1u);
}

static void TestTrackScanUsesLeadInFirstTrackAndZeroEndFallback(void)
{
    ResetFixture();
    memory[0x12c9u] = 1u;
    memory[0x12bfu] = 1u;
    effective_last_track = 2u;
    WriteU32(0x1153u, 200l);
    AddConversion(Field(1u, 0xdd94u), 0l);
    AddConversion(Field(2u, 0xdd94u), 1000l);
    memory[Field(2u, 0xdd89u)] = 0u;

    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(conversion_index == 2u);
    assert(bit_000c == 0u);
}

static void TestPacketEndBeforeMappedAddressRejectsAdjustment(void)
{
    ResetFixture();
    PrepareTrackTwo(1000l, 249l, 0l, 32ul);

    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(conversion_index == 2u);
    assert(divide_count == 0u && bit_000c == 0u);
}

static void TestNegativeDeltaClearsOrPreservesExistingFlag(void)
{
    ResetFixture();
    PrepareTrackTwo(1000l, 500l, 300l, 32ul);
    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(bit_000c == 0u);

    ResetFixture();
    PrepareTrackTwo(1000l, 500l, 300l, 32ul);
    memory[0x15d7u] = 1u;
    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(bit_000c == 0xa5u);

    ResetFixture();
    PrepareTrackTwo(1000l, 500l, 300l, 32ul);
    bit_0021 = 1u;
    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(bit_000c == 0xa5u);
}

static void TestMissingPacketFlagOrSizeClearsOutputFlag(void)
{
    ResetFixture();
    AddConversion(Field(2u, 0xdd94u), 1000l);
    memory[Field(2u, 0xdd89u)] = 0u;
    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(bit_000c == 0u);

    ResetFixture();
    AddConversion(Field(2u, 0xdd94u), 1000l);
    memory[Field(2u, 0xdd89u)] = 1u;
    WriteU32(Field(2u, 0xdd8au), 0ul);
    assert(MapLogicalBlockAddressThroughTrackPacketLayout(100l) == 250l);
    assert(bit_000c == 0u);
}

int main(void)
{
    TestDisabledLayoutReturnsBiasedAddressAndClearsFlag();
    TestValidPacketLayoutAddsSevenFramesPerCompletedPacket();
    TestTrackScanUsesLeadInFirstTrackAndZeroEndFallback();
    TestPacketEndBeforeMappedAddressRejectsAdjustment();
    TestNegativeDeltaClearsOrPreservesExistingFlag();
    TestMissingPacketFlagOrSizeClearsOutputFlag();
    puts("track-packet LBA mapping tests passed");
    return 0;
}
