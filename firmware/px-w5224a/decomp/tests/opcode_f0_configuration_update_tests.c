#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/mcs251.h"
#include "scsi/opcode_f0_configuration_update.h"

#define LOW_MEMORY_SIZE 0x10000u

static u8 memory[LOW_MEMORY_SIZE];
static u8 identity[8];
static u8 readback[0x100];
static u8 busy_reads;
static u8 data_out_result;
static u8 update_result;
static u8 yield_count;
static u8 unsupported_count;
static u8 sense_count;
static u8 sense_key;
static u16 sense_code;
static u8 no_sense_count;
static u8 initialize_count;
static u32 initialize_entry;
static fw_addr_t update_source;
static u16 update_offset;
static u16 update_count;
static fw_addr_t read_destination;
static u16 read_offset;
static u16 read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x000c86ul && busy_reads != 0u)
    {
        --busy_reads;
        return 4u;
    }
    if(address >= 0xff95f0ul && address < 0xff95f8ul)
        return identity[(u8)(address - 0xff95f0ul)];
    assert(address < LOW_MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < LOW_MEMORY_SIZE);
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

void YieldCoroutine0(void)
{
    ++yield_count;
}

u8 ExecuteDataOutAndCopyFromControllerBuffer(fw_addr_t destination, u32 byte_count)
{
    assert(destination == 0x000b2aul);
    assert(byte_count == (u32)(((u16)memory[0x0b12u] << 8) | memory[0x0b13u]));
    return data_out_result;
}

u8 UpdateAndProgramDriveConfigurationRange(fw_addr_t source, u16 destination_offset, u16 byte_count)
{
    update_source = source;
    update_offset = destination_offset;
    update_count = byte_count;
    return update_result;
}

void ReadDriveConfigurationBytes(fw_addr_t offset, fw_addr_t destination, u16 count)
{
    u16 index;

    read_offset = (u16)offset;
    read_destination = destination;
    read_count = count;
    for(index = 0u; index < count; ++index)
        memory[destination + index] = readback[index];
}

void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_count;
}

void SetSenseData(u8 key, u16 code)
{
    ++sense_count;
    sense_key = key;
    sense_code = code;
}

void SetNoSenseData(void)
{
    ++no_sense_count;
}

void InitializeCoroutine1(u32 entry_point)
{
    ++initialize_count;
    initialize_entry = entry_point;
}

static void StoreWord(u16 address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void Reset(u16 offset, u16 parameter_length)
{
    u8 index;

    memset(memory, 0, sizeof(memory));
    memset(readback, 0, sizeof(readback));
    busy_reads = 0u;
    data_out_result = 0u;
    update_result = 1u;
    yield_count = 0u;
    unsupported_count = 0u;
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    no_sense_count = 0u;
    initialize_count = 0u;
    initialize_entry = 0ul;
    update_source = 0ul;
    update_offset = 0u;
    update_count = 0u;
    read_destination = 0ul;
    read_offset = 0u;
    read_count = 0u;
    StoreWord(0x0b0eu, offset);
    StoreWord(0x0b12u, parameter_length);
    for(index = 0u; index < 8u; ++index)
    {
        identity[index] = (u8)(0xa0u + index);
        memory[0x0b2au + index] = identity[index];
    }
}

static void MatchReadback(u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        readback[index] = memory[0x0b32u + index];
}

static void TestBusyWaitAndSuccessfulProgramming(void)
{
    Reset(0x20u, 12u);
    memory[0x0b32u] = 0x11u;
    memory[0x0b33u] = 0x22u;
    memory[0x0b34u] = 0x33u;
    memory[0x0b35u] = 0x44u;
    MatchReadback(3u);
    busy_reads = 2u;
    HandleScsiOpcodeF0();
    assert(yield_count == 2u);
    assert(update_source == 0x000b32ul);
    assert(update_offset == 0x20u);
    assert(update_count == 4u);
    assert(read_offset == 0x20u);
    assert(read_destination == 0x00e4faul);
    assert(read_count == 3u);
    assert(no_sense_count == 1u);
    assert(sense_count == 0u && initialize_count == 0u);
}

static void TestRangeAndTransferRejections(void)
{
    Reset(0u, 7u);
    HandleScsiOpcodeF0();
    assert(unsupported_count == 1u);
    assert(update_source == 0ul);

    Reset(0xa1u, 8u);
    HandleScsiOpcodeF0();
    assert(unsupported_count == 1u);

    Reset(0u, 8u);
    data_out_result = 4u;
    HandleScsiOpcodeF0();
    assert(sense_count == 0u && no_sense_count == 0u);
    assert(update_source == 0ul);
}

static void TestIdentityAndProtectedFieldValidation(void)
{
    Reset(0x20u, 12u);
    memory[0x0b2du] ^= 1u;
    HandleScsiOpcodeF0();
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x2600u);
    assert(update_source == 0ul);

    Reset(0x40u, 10u);
    memory[0xe750u] = 2u;
    memory[0x0b33u] = 1u;
    HandleScsiOpcodeF0();
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x2600u);

    Reset(0x4bu, 10u);
    memory[0xe750u] = 2u;
    memory[0x0b33u] = 0x3eu;
    HandleScsiOpcodeF0();
    assert(sense_count == 1u && sense_key == 5u && sense_code == 0x2600u);

    Reset(0x4bu, 10u);
    memory[0xe750u] = 2u;
    memory[0x0b33u] = 0x7fu;
    MatchReadback(1u);
    HandleScsiOpcodeF0();
    assert(no_sense_count == 1u && sense_count == 0u);
}

static void TestProgrammingAndVerificationFailures(void)
{
    Reset(0x20u, 12u);
    update_result = 0u;
    HandleScsiOpcodeF0();
    assert(initialize_count == 1u && initialize_entry == 0x00f853cful);
    assert(sense_count == 1u && sense_key == 4u && sense_code == 0x4000u);
    assert(read_destination == 0ul);

    Reset(0x20u, 12u);
    memory[0x0b32u] = 0x11u;
    memory[0x0b33u] = 0x22u;
    memory[0x0b34u] = 0x33u;
    MatchReadback(3u);
    readback[1] ^= 1u;
    HandleScsiOpcodeF0();
    assert(memory[0x1009u] == 1u);
    assert(initialize_count == 1u && initialize_entry == 0x00f853cful);
    assert(sense_count == 0u && no_sense_count == 0u);
}

static void TestSkippedAndFinalBytesAreNotCompared(void)
{
    Reset(0x5du, 12u);
    memory[0x0b32u] = 0x10u;
    memory[0x0b33u] = 0x20u;
    memory[0x0b34u] = 0x30u;
    memory[0x0b35u] = 0x40u;
    MatchReadback(3u);
    readback[1] ^= 1u;
    readback[2] ^= 1u;
    HandleScsiOpcodeF0();
    assert(read_count == 3u);
    assert(memory[0x1009u] == 0u);
    assert(initialize_count == 0u && no_sense_count == 1u);
}

int main(void)
{
    TestBusyWaitAndSuccessfulProgramming();
    TestRangeAndTransferRejections();
    TestIdentityAndProtectedFieldValidation();
    TestProgrammingAndVerificationFailures();
    TestSkippedAndFinalBytesAreNotCompared();
    puts("opcode F0 configuration update tests passed");
    return 0;
}
