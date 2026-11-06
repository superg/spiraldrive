#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/raw_sector_range.h"

#define MEMORY_SIZE 0x20000u
#define ROM_BASE 0x00ff95baul
#define ROM_SIZE 0x40u

static u8 memory[MEMORY_SIZE];
static u8 rom[ROM_SIZE];
static u8 bits[0x600];
static u8 range_valid;
static s32 validated_start;
static s32 validated_end;
static unsigned validate_count;
static unsigned progress_count;
static unsigned position_count;
static s32 positioned_target;
static unsigned reset_count;
static unsigned initialize_count;
static u32 initialized_entry;
static unsigned yield_count;
static u8 poll_values[8];
static unsigned poll_count;
static unsigned poll_index;
static unsigned no_sense_count;
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;
static unsigned information_count;
static u32 sense_information;
static u16 descriptor_address;
static unsigned dispatch_wait_count;
static unsigned dispatch_request_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= ROM_BASE && address < ROM_BASE + ROM_SIZE)
        return rom[address - ROM_BASE];
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
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

#define DEFINE_BIT_READER(name, address)                                                                                                                                                               \
    u8 name(void)                                                                                                                                                                                      \
    {                                                                                                                                                                                                  \
        return bits[(address)];                                                                                                                                                                        \
    }
#define DEFINE_BIT_WRITER(name, address)                                                                                                                                                               \
    void name(u8 value)                                                                                                                                                                                \
    {                                                                                                                                                                                                  \
        bits[(address)] = value;                                                                                                                                                                       \
    }

DEFINE_BIT_READER(FirmwareReadBit000D, 0x000d)
DEFINE_BIT_READER(FirmwareReadBit000E, 0x000e)
DEFINE_BIT_READER(FirmwareReadBit0013, 0x0013)
DEFINE_BIT_WRITER(FirmwareWriteBit0001, 0x0001)
DEFINE_BIT_WRITER(FirmwareWriteBit000C, 0x000c)
DEFINE_BIT_WRITER(FirmwareWriteBit0013, 0x0013)
DEFINE_BIT_WRITER(FirmwareWriteBit001B, 0x001b)
DEFINE_BIT_WRITER(FirmwareWriteBit001C, 0x001c)
DEFINE_BIT_WRITER(FirmwareWriteBit001D, 0x001d)
DEFINE_BIT_WRITER(FirmwareWriteBit0045, 0x0045)
DEFINE_BIT_WRITER(FirmwareWriteBit0580, 0x0580)
DEFINE_BIT_WRITER(FirmwareWriteBit0581, 0x0581)

u8 ValidateRawSectorReadFrameRangeAgainstMediaBounds(s32 start, s32 end)
{
    ++validate_count;
    validated_start = start;
    validated_end = end;
    return range_valid;
}

void UpdateRawReadProgressCountersAndPhaseFlags(void)
{
    ++progress_count;
}

void PositionRawReadPipelineAtFramePositionForTest(s32 target)
{
    ++position_count;
    positioned_target = target;
}

void ResetRawReadQueueAndTransferState(void)
{
    ++reset_count;
}

void InitializeCoroutine2(u32 entry_point)
{
    ++initialize_count;
    initialized_entry = entry_point;
}

void YieldCoroutine0(void)
{
    ++yield_count;
}

u8 PollRawSectorReadHostTransferStatus(void)
{
    assert(poll_index < poll_count);
    return poll_values[poll_index++];
}

void SetNoSenseData(void)
{
    ++no_sense_count;
}

void SetSenseData(u8 key, u16 code)
{
    ++sense_count;
    sense_key = key;
    sense_code = code;
}

void SetSenseInformation(u32 information)
{
    ++information_count;
    sense_information = information;
}

u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    return descriptor_address;
}

u8 DispatchAndWaitForReadAcquisitionResult(void)
{
    ++dispatch_wait_count;
    return 0u;
}

void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    ++dispatch_request_count;
}

static void StoreU16(u32 address, u16 value)
{
    if(address >= ROM_BASE)
    {
        rom[address - ROM_BASE] = (u8)(value >> 8);
        rom[address - ROM_BASE + 1u] = (u8)value;
    }
    else
    {
        memory[address] = (u8)(value >> 8);
        memory[address + 1u] = (u8)value;
    }
}

static u16 LoadU16(u32 address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void StoreU32(u32 address, u32 value)
{
    u8 *base;
    u32 offset;

    if(address >= ROM_BASE)
    {
        base = rom;
        offset = address - ROM_BASE;
    }
    else
    {
        base = memory;
        offset = address;
    }
    base[offset] = (u8)(value >> 24);
    base[offset + 1u] = (u8)(value >> 16);
    base[offset + 2u] = (u8)(value >> 8);
    base[offset + 3u] = (u8)value;
}

static u32 LoadU32(u32 address)
{
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(rom, 0, sizeof(rom));
    memset(bits, 0, sizeof(bits));
    range_valid = 1u;
    validated_start = 0;
    validated_end = 0;
    validate_count = 0u;
    progress_count = 0u;
    position_count = 0u;
    positioned_target = 0;
    reset_count = 0u;
    initialize_count = 0u;
    initialized_entry = 0ul;
    yield_count = 0u;
    poll_values[0] = 1u;
    poll_count = 1u;
    poll_index = 0u;
    no_sense_count = 0u;
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    information_count = 0u;
    sense_information = 0ul;
    descriptor_address = 0x4000u;
    dispatch_wait_count = 0u;
    dispatch_request_count = 0u;
    StoreU16(ROM_BASE + 0u, 0x0900u);
    StoreU16(ROM_BASE + 0x0au, 0x0800u);
    StoreU32(0x00ff95ceul, 10ul);
}

static void AssertEntryPublications(s32 start, s32 end)
{
    assert(LoadU32(0x0b02u) == (u32)((u32)start - 150ul));
    assert(LoadU32(0x0f14u) == (u32)end);
    assert(bits[0x001bu] == 0u);
    assert(bits[0x001cu] == 0u);
    assert(bits[0x001du] == 0u);
}

static void TestContinuationRejections(void)
{
    Reset();
    bits[0x000du] = 1u;
    memory[0x0f0au] = 2u;
    memory[0x0f09u] = 7u;
    assert(ExecuteRawSectorReadRange(100, 101, 0u) == 1u);
    assert(memory[0x0f09u] == 0u);
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x6400u);
    AssertEntryPublications(100, 101);

    Reset();
    bits[0x000du] = 1u;
    memory[0x0f09u] = 2u;
    StoreU32(0x0f14u, 96ul);
    assert(ExecuteRawSectorReadRange(100, 120, 0u) == 1u);
    assert(sense_count == 1u && sense_key == 3u && sense_code == 0x1500u);
    assert(memory[0x0f09u] == 2u);

    Reset();
    bits[0x000du] = 1u;
    memory[0x0f09u] = 2u;
    StoreU32(0x0f14u, 95ul);
    poll_values[0] = 1u;
    assert(ExecuteRawSectorReadRange(100, 120, 0u) == 1u);
    assert(sense_code != 0x1500u && memory[0x0f09u] == 1u);
}

static void TestRangeAdmissionAndZeroLength(void)
{
    Reset();
    assert(ExecuteRawSectorReadRange(-20000, -19999, 0u) == 1u);
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x6400u);

    Reset();
    assert(ExecuteRawSectorReadRange(-999, -998, 0u) == 1u);
    assert(sense_count == 1u && sense_code == 0x6400u);

    Reset();
    poll_values[0] = 1u;
    assert(ExecuteRawSectorReadRange(-19999, -19998, 0u) == 1u);
    assert(bits[0x0013u] == 1u && validate_count == 0u);

    Reset();
    poll_values[0] = 1u;
    assert(ExecuteRawSectorReadRange(-1000, -999, 0u) == 1u);
    assert(bits[0x0013u] == 1u && validate_count == 0u);

    Reset();
    range_valid = 0u;
    assert(ExecuteRawSectorReadRange(75, 91, 0u) == 1u);
    assert(validate_count == 1u && validated_start == 75 && validated_end == 91);
    assert(progress_count == 0u && poll_index == 0u);

    Reset();
    assert(ExecuteRawSectorReadRange(75, 75, 0u) == 1u);
    assert(dispatch_wait_count == 1u);
    assert(memory[0x0f4au] == 0u && memory[0x0c8cu] == 0u);
    assert(LoadU32(0x0c8eu) == 75ul);
    assert(progress_count == 0u);
}

static void TestFreshPipelineAndPolling(void)
{
    Reset();
    StoreU32(0x00ff95ceul + 4u * 4u, (u32)-7);
    StoreU16(ROM_BASE + 4u * 2u, 0x1234u);
    bits[0x000eu] = 0u;
    bits[0x0045u] = 1u;
    poll_values[0] = 0xfeu;
    poll_values[1] = 0xfeu;
    poll_values[2] = 1u;
    poll_count = 3u;

    assert(ExecuteRawSectorReadRange(100, 140, 8u) == 1u);
    assert(LoadU32(0x1455u) == 40ul && LoadU32(0x1459u) == 0ul);
    assert(progress_count == 1u && bits[0x000cu] == 0u);
    assert(LoadU32(0x0f32u) == 100ul);
    assert(reset_count == 1u && memory[0x0c2au] == 3u);
    assert(initialize_count == 1u && initialized_entry == 0x00f929abul);
    assert(memory[0x0f06u] == 4u && LoadU32(0x1598u) == 93ul);
    assert(LoadU16(0x145du) == 0x1234u);
    assert(poll_index == 3u && yield_count == 3u);
    assert(bits[0x0045u] == 0u && bits[0x0001u] == 0u);
    assert(memory[0x1226u] == 4u);
    assert(bits[0x0580u] == 1u && bits[0x0581u] == 1u);
    assert(no_sense_count == 1u);
}

static void TestPipelineReuseAndAlternateLength(void)
{
    Reset();
    memory[0x0c2au] = 3u;
    memory[0x0f06u] = 0u;
    StoreU32(0x00ff95ceul, 12ul);
    bits[0x000du] = 1u;
    memory[0x0f0bu] = 0u;
    StoreU16(0x00ff95c4ul, 0x0200u);
    poll_values[0] = 1u;
    assert(ExecuteRawSectorReadRange(100, 101, 0u) == 1u);
    assert(position_count == 1u && positioned_target == 112);
    assert(reset_count == 0u && initialize_count == 0u);
    assert(LoadU16(0x145du) == 0x0200u);

    Reset();
    bits[0x000du] = 1u;
    memory[0x0f0bu] = 0u;
    bits[0x0045u] = 0u;
    StoreU16(0x00ff95c4ul, 0u);
    assert(ExecuteRawSectorReadRange(100, 101, 0u) == 1u);
    assert(no_sense_count == 1u && poll_index == 0u);
    assert(bits[0x0045u] == 0u);

    Reset();
    bits[0x000du] = 1u;
    bits[0x000eu] = 1u;
    memory[0x0f0bu] = 2u;
    StoreU16(0x00ff95c4ul, 0x0100u);
    poll_values[0] = 1u;
    assert(ExecuteRawSectorReadRange(100, 101, 0u) == 1u);
    assert(LoadU16(0x145du) == 0x0a26u);
    assert(memory[0x0c2au] == 7u && memory[0x1226u] == 0u);
    assert(bits[0x0580u] == 0u && bits[0x0581u] == 0u);
}

static void PrepareFailureContext(u16 high_word)
{
    StoreU16(0x1457u, 0x0100u);
    StoreU16(0x0100u, high_word);
    StoreU16((u16)(descriptor_address + 0x0eu), 0x4100u);
    memory[0x4110u] = 0x0bu;
    poll_values[0] = 0u;
}

static void TestFailureContextAndRestart(void)
{
    Reset();
    PrepareFailureContext(0x8000u);
    assert(ExecuteRawSectorReadRange(100, 356, 0u) == 0u);
    assert(sense_count == 1u && sense_key == 0x0bu);
    assert(sense_code == descriptor_address);
    assert(information_count == 1u);
    assert(sense_information == (0x80001455ul - 150ul));
    assert(reset_count == 2u && initialize_count == 2u);
    assert(initialized_entry == 0x00f84a00ul);
    assert(dispatch_request_count == 1u && yield_count == 2u);

    Reset();
    PrepareFailureContext(0u);
    StoreU32(0x12c1u, 0x10000000ul);
    assert(ExecuteRawSectorReadRange(100, 356, 0u) == 0u);
    assert(sense_count == 0u && information_count == 0u);
    assert(dispatch_request_count == 1u);
}

int main(void)
{
    TestContinuationRejections();
    TestRangeAdmissionAndZeroLength();
    TestFreshPipelineAndPolling();
    TestPipelineReuseAndAlternateLength();
    TestFailureContextAndRestart();
    puts("raw sector range tests passed");
    return 0;
}
