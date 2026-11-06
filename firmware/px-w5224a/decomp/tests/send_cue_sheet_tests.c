#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/send_cue_sheet.h"

static u8 memory[1u << 16];
static u8 validate_result;
static u8 receive_result;
static unsigned unsupported_count;
static unsigned sequence_error_count;
static unsigned receive_count;
static fw_addr_t receive_destination;
static u32 receive_length;
static unsigned no_sense_count;
static u8 payload[0x40];
static u16 payload_length;

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

u8 ObserveSendCueValidateCdb(fw_addr_t mask_descriptor)
{
    assert(mask_descriptor == 0xff8cdful);
    return validate_result;
}

void ObserveSendCueUnsupported(void)
{
    unsupported_count++;
}

void ObserveSendCueSequenceError(void)
{
    sequence_error_count++;
}

u8 ObserveSendCueReceive(fw_addr_t destination, u32 byte_count)
{
    unsigned index;

    receive_count++;
    receive_destination = destination;
    receive_length = byte_count;
    for(index = 0u; index < payload_length; ++index)
        memory[(u16)(destination + index)] = payload[index];
    return receive_result;
}

void ObserveSendCueNoSense(void)
{
    no_sense_count++;
}

static void WriteTransferLength(u32 value)
{
    memory[0x0b0fu] = (u8)(value >> 24);
    memory[0x0b10u] = (u8)(value >> 16);
    memory[0x0b11u] = (u8)(value >> 8);
    memory[0x0b12u] = (u8)value;
}

static void SetPayloadRecord(u16 index, u8 control, u8 point, u8 pmin, u8 psec, u8 pframe)
{
    u16 address = (u16)(index * 8u);

    payload[address] = control;
    payload[(u16)(address + 1u)] = point;
    payload[(u16)(address + 5u)] = pmin;
    payload[(u16)(address + 6u)] = psec;
    payload[(u16)(address + 7u)] = pframe;
}

static void ResetFixture(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(payload, 0, sizeof(payload));
    validate_result = 1u;
    receive_result = 0u;
    unsupported_count = 0u;
    sequence_error_count = 0u;
    receive_count = 0u;
    receive_destination = 0ul;
    receive_length = 0ul;
    no_sense_count = 0u;
    payload_length = 0u;
    memory[0x0c44u] = 2u;
}

static void TestValidationAndLengthFailures(void)
{
    ResetFixture();
    validate_result = 0u;
    HandleSendCueSheetCommand();
    assert(unsupported_count == 1u && receive_count == 0u);

    ResetFixture();
    memory[0x0c44u] = 1u;
    HandleSendCueSheetCommand();
    assert(sequence_error_count == 1u && receive_count == 0u);

    ResetFixture();
    WriteTransferLength(9ul);
    HandleSendCueSheetCommand();
    assert(unsupported_count == 1u && receive_count == 0u);

    ResetFixture();
    WriteTransferLength(0x2808ul);
    HandleSendCueSheetCommand();
    assert(unsupported_count == 1u && receive_count == 0u);
}

static void TestTransferResultFourReturnsBeforeParsing(void)
{
    ResetFixture();
    WriteTransferLength(8ul);
    receive_result = 4u;
    HandleSendCueSheetCommand();
    assert(receive_count == 1u);
    assert(receive_destination == 0x00b4bdul && receive_length == 8ul);
    assert(memory[0xb4bdu] == 0u && memory[0xdcbcu] == 0u);
    assert(no_sense_count == 0u);
    assert(memory[0x0c85u] == 0xa5u);
}

static void TestCueParsingAndPositiveControlRoutes(void)
{
    ResetFixture();
    WriteTransferLength(24ul);
    payload_length = 24u;
    SetPayloadRecord(0u, 0x01u, 1u, 0u, 0u, 0u);
    SetPayloadRecord(1u, 0x41u, 2u, 0u, 0u, 0u);
    SetPayloadRecord(2u, 0x01u, 0xaau, 0x12u, 0x34u, 0x56u);
    memory[0x158fu] = 1u;
    memory[0x158eu] = 1u;
    memory[0x1008u] = 1u;
    memory[0x0c38u] = 8u;
    memory[0x0c87u] = 0x55u;

    HandleSendCueSheetCommand();
    assert(memory[0x0c85u] == 0xa5u);
    assert(memory[0xb4bbu] == 0u && memory[0xb4bcu] == 3u);
    assert(memory[0x1592u] == 0x12u);
    assert(memory[0x1593u] == 0x34u);
    assert(memory[0x1594u] == 0x56u);
    assert(memory[0x0c87u] == 0xd5u);
    assert(memory[0x128eu] == 0u && memory[0x128fu] == 0u);
    assert(memory[0x158cu] == 8u);
    assert((memory[0x158eu] & 2u) == 0u);
    assert(no_sense_count == 1u);
}

static void TestSpeedRestoreAndMissingBitSixEntry(void)
{
    ResetFixture();
    WriteTransferLength(16ul);
    payload_length = 16u;
    SetPayloadRecord(0u, 0x01u, 1u, 0u, 0u, 0u);
    SetPayloadRecord(1u, 0x03u, 2u, 0u, 0u, 0u);
    memory[0x158eu] = 3u;
    memory[0x158cu] = 12u;
    memory[0x1008u] = 1u;

    HandleSendCueSheetCommand();
    assert(memory[0x0c38u] == 12u && memory[0x0e47u] == 12u);
    assert((memory[0x158eu] & 2u) != 0u);
    assert(no_sense_count == 1u);
}

int main(void)
{
    TestValidationAndLengthFailures();
    TestTransferResultFourReturnsBeforeParsing();
    TestCueParsingAndPositiveControlRoutes();
    TestSpeedRestoreAndMissingBitSixEntry();
    return 0;
}
