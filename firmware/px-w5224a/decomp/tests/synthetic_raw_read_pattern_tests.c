#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/synthetic_raw_read_pattern.h"
#include "platform/firmware_memory.h"

static u8 memory[0x20000ul];
static u8 bit_0030;
static u8 bit_0035;
static unsigned busy_reads_remaining;
static fw_addr_t resumed_cursor;
static u8 capacity_result;
static unsigned resume_count;
static unsigned yield_count;
static unsigned capacity_count;
static unsigned failure_count;
static u8 failure_code;
static unsigned fill_count;
static fw_addr_t fill_addresses[8];
static u16 fill_lengths[8];
static u8 fill_values[8];
static unsigned auxiliary_count;
static unsigned enqueue_count;
static unsigned advance_msf_count;
static unsigned advance_ring_count;

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

u8 FirmwareReadBit0036(void)
{
    if(busy_reads_remaining != 0u)
    {
        --busy_reads_remaining;
        return 1u;
    }
    return 0u;
}

void FirmwareWriteBit0030(u8 value)
{
    bit_0030 = value;
}
void FirmwareWriteBit0035(u8 value)
{
    bit_0035 = value;
}

void ObserveSyntheticPatternYield(void)
{
    ++yield_count;
}

fw_addr_t ObserveSyntheticPatternResumeRing(void)
{
    ++resume_count;
    return resumed_cursor;
}

u8 ObserveSyntheticPatternWaitForCapacity(void)
{
    ++capacity_count;
    return capacity_result;
}

void ObserveSyntheticPatternReportFailure(u8 code)
{
    ++failure_count;
    failure_code = code;
}

u32 ObserveSyntheticPatternRemainder(s32 dividend, s32 divisor)
{
    u32 magnitude = dividend < 0 ? 0ul - (u32)dividend : (u32)dividend;
    u32 divisor_magnitude = divisor < 0 ? 0ul - (u32)divisor : (u32)divisor;

    return magnitude % divisor_magnitude;
}

void ObserveSyntheticPatternFill(fw_addr_t address, u16 count, u8 value)
{
    assert(fill_count < sizeof(fill_addresses) / sizeof(fill_addresses[0]));
    fill_addresses[fill_count] = address;
    fill_lengths[fill_count] = count;
    fill_values[fill_count] = value;
    ++fill_count;
}

void ObserveSyntheticPatternCopyAuxiliary(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    assert(destination == resumed_cursor + auxiliary_count * 0x930ul);
    assert(source == 0xff967aul && byte_count == 12u);
    ++auxiliary_count;
}

void ObserveSyntheticPatternEnqueue(void)
{
    ++enqueue_count;
}

void ObserveSyntheticPatternAdvanceMsf(u8 reverse_relative)
{
    assert(reverse_relative == 0u);
    ++advance_msf_count;
}

u32 ObserveSyntheticPatternAdvanceRing(u32 cursor)
{
    ++advance_ring_count;
    return cursor + 0x930ul;
}

void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t address)
{
    assert(frame_position == (s32)(((u32)memory[0x1598u] << 24) | ((u32)memory[0x1599u] << 16) | ((u32)memory[0x159au] << 8) | memory[0x159bu]));
    memory[address] = 1u;
    memory[address + 1u] = 2u;
    memory[address + 2u] = 3u;
}

void ConvertThreeBinaryBytesToPackedBcd(u16 address)
{
    memory[address] = 0x11u;
    memory[address + 1u] = 0x22u;
    memory[address + 2u] = 0x33u;
}

void CopyThreeBytes16(u16 destination, u16 source)
{
    memory[destination] = memory[source];
    memory[destination + 1u] = memory[source + 1u];
    memory[destination + 2u] = memory[source + 2u];
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
    memset(memory, 0, sizeof(memory));
    bit_0030 = bit_0035 = 0xa5u;
    busy_reads_remaining = 0u;
    resumed_cursor = 0x4000ul;
    capacity_result = 1u;
    resume_count = yield_count = capacity_count = 0u;
    failure_count = 0u;
    failure_code = 0xffu;
    fill_count = auxiliary_count = enqueue_count = 0u;
    advance_msf_count = advance_ring_count = 0u;
}

static void TestEmptyRangeInitializesThenCompletes(void)
{
    ResetFixture();
    WriteU32(0x1598u, 1234ul);

    assert(GenerateSyntheticRawReadPatternRange(10l, 10l) == 1u);
    assert(resume_count == 1u);
    assert(ReadU32(0x1455u) == 50ul);
    assert(bit_0030 == 0u && bit_0035 == 0u);
    assert(enqueue_count == 0u && fill_count == 0u);
}

static void TestTwoFilledDescriptorsAndSignedMagnitudeRemainder(void)
{
    fw_addr_t second_cursor;

    ResetFixture();
    WriteU32(0x156au, 0x200ul);
    WriteU32(0x1598u, (u32)-106l);

    assert(GenerateSyntheticRawReadPatternRange(10l, 12l) == 1u);
    assert(fill_count == 2u && auxiliary_count == 2u);
    assert(fill_addresses[0] == 0x4000ul && fill_addresses[1] == 0x4930ul);
    assert(fill_lengths[0] == 0x930u && fill_lengths[1] == 0x930u);
    assert(fill_values[0] == 0u && fill_values[1] == 0xffu);
    assert(memory[0x1279u] == 0x11u && memory[0x127au] == 0x22u && memory[0x127bu] == 0x33u);
    assert(ReadU32(0x401cul) == 0x11223301ul);
    assert(ReadU32(0x4020ul) == (u32)-256l);
    assert(ReadU32(0x4024ul) == 0x11223301ul);
    second_cursor = 0x4930ul;
    assert(ReadU32(second_cursor + 0x1cul) == 0x11223301ul);
    assert(ReadU32(second_cursor + 0x20ul) == (u32)-255l);
    assert(enqueue_count == 2u && advance_msf_count == 2u && advance_ring_count == 2u);
    assert(ReadU32(0x1598u) == (u32)-104l);
    assert(bit_0030 == 0u && bit_0035 == 0u);
}

static void TestFirstCapacityFailureLeavesActiveBitsSet(void)
{
    ResetFixture();
    WriteU32(0x156au, 0ul);
    WriteU32(0x1598u, 1000ul);
    capacity_result = 0u;

    assert(GenerateSyntheticRawReadPatternRange(0l, 8l) == 0u);
    assert(enqueue_count == 5u && capacity_count == 1u);
    assert(failure_count == 1u && failure_code == 0u);
    assert(bit_0030 == 1u && bit_0035 == 1u);
    assert(ReadU32(0x1598u) == 1005ul);
}

static void TestBusyAndSuccessfulCapacityWaitYield(void)
{
    ResetFixture();
    WriteU32(0x156au, 0ul);
    WriteU32(0x1598u, 1000ul);
    busy_reads_remaining = 1u;

    assert(GenerateSyntheticRawReadPatternRange(0l, 6l) == 1u);
    assert(capacity_count == 1u && enqueue_count == 6u);
    assert(yield_count == 2u);
    assert(bit_0030 == 0u && bit_0035 == 0u);
}

int main(void)
{
    TestEmptyRangeInitializesThenCompletes();
    TestTwoFilledDescriptorsAndSignedMagnitudeRemainder();
    TestFirstCapacityFailureLeavesActiveBitsSet();
    TestBusyAndSuccessfulCapacityWaitYield();
    puts("synthetic raw-read pattern tests passed");
    return 0;
}
