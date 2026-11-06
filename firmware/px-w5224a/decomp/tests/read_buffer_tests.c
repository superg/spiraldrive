#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_buffer.h"

static u8 memory[0x20000];
static u8 validate_result;
static u8 unsupported_calls;
static u8 sequence_error_calls;
static u8 no_sense_calls;
static u8 prepare_calls;
static u8 snapshot_calls;
static u16 snapshot_length;
static u8 response_calls;
static fw_addr_t response_source[4];
static u32 response_length[4];
static u8 response_result[4];
static u8 transfer_calls;
static u32 transfer_address[4];
static u32 transfer_length[4];
static u8 transfer_result[4];

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

u8 ValidateScsiCdbFieldMasks(fw_addr_t mask)
{
    assert(mask == 0xff8d9cul);
    return validate_result;
}

void HandleUnsupportedScsiCommand(void)
{
    unsupported_calls++;
}
void SetCommandSequenceErrorSense(void)
{
    sequence_error_calls++;
}
void SetNoSenseData(void)
{
    no_sense_calls++;
}
void PLEXTOR_NEAR_FUNCTION PrepareBufferDataTransfer(void)
{
    prepare_calls++;
}

u16 PLEXTOR_NEAR_FUNCTION BuildReadBufferMode1Id2Snapshot(void)
{
    snapshot_calls++;
    return snapshot_length;
}

u32 MinU32(u32 value, u32 limit)
{
    return value < limit ? value : limit;
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 length)
{
    response_source[response_calls] = source;
    response_length[response_calls] = length;
    return response_result[response_calls++];
}

u8 ExecuteBufferDataTransfer(u32 address, u32 length, u8 opcode)
{
    assert(opcode == 0x3cu);
    transfer_address[transfer_calls] = address;
    transfer_length[transfer_calls] = length;
    return transfer_result[transfer_calls++];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    u32 value = (u32)memory[address] << 24;

    value |= (u32)memory[address + 1u] << 16;
    value |= (u32)memory[address + 2u] << 8;
    value |= memory[address + 3u];
    return value;
}

static void SetCdb(u8 mode, u8 id, u32 offset, u32 length)
{
    memory[0x0b0b] = mode;
    memory[0x0b0c] = id;
    memory[0x0b0d] = (u8)(offset >> 16);
    memory[0x0b0e] = (u8)(offset >> 8);
    memory[0x0b0f] = (u8)offset;
    memory[0x0b10] = (u8)(length >> 16);
    memory[0x0b11] = (u8)(length >> 8);
    memory[0x0b12] = (u8)length;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    validate_result = 1u;
    unsupported_calls = sequence_error_calls = no_sense_calls = 0u;
    prepare_calls = snapshot_calls = response_calls = transfer_calls = 0u;
    snapshot_length = 0x28e2u;
    memset(response_source, 0, sizeof(response_source));
    memset(response_length, 0, sizeof(response_length));
    memset(response_result, 0, sizeof(response_result));
    memset(transfer_address, 0, sizeof(transfer_address));
    memset(transfer_length, 0, sizeof(transfer_length));
    memset(transfer_result, 0, sizeof(transfer_result));
    WriteDword(0x1556u, 0x001b0000ul);
    WriteDword(0x155au, 0x00160000ul);
    memory[0x0c85] = 2u;
}

static void TestAdmissionAndInvalidModes(void)
{
    ResetState();
    validate_result = 0u;
    memory[0x0c83] = 0x41u;
    SetCdb(0u, 0u, 0ul, 8ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u);
    assert(memory[0x0c83] == 0x41u);
    assert(response_calls == 0u && transfer_calls == 0u);

    ResetState();
    SetCdb(4u, 0u, 0ul, 8ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u && no_sense_calls == 0u);
    assert((memory[0x0c83] & 0x20u) == 0u);
}

static void TestMode0(void)
{
    ResetState();
    SetCdb(0u, 0u, 0ul, 10ul);
    HandleReadBufferCommand();
    assert(ReadDword(0x0b2au) == 0x00200000ul);
    assert(response_calls == 1u && response_source[0] == 0x0b2aul);
    assert(response_length[0] == 4ul);
    assert(transfer_calls == 1u && transfer_address[0] == 0ul);
    assert(transfer_length[0] == 6ul);
    assert(no_sense_calls == 1u);

    ResetState();
    SetCdb(0u, 0u, 0ul, 3ul);
    response_result[0] = 4u;
    HandleReadBufferCommand();
    assert(response_calls == 1u && response_length[0] == 3ul);
    assert(transfer_calls == 0u && no_sense_calls == 0u);

    ResetState();
    SetCdb(0u, 1u, 0ul, 3ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u && response_calls == 0u);
}

static void TestMode1Ordinary(void)
{
    ResetState();
    SetCdb(1u, 0u, 0x100ul, 12ul);
    HandleReadBufferCommand();
    assert(ReadDword(0x0b2au) == 0x001aff00ul);
    assert(response_calls == 1u && response_length[0] == 4ul);
    assert(transfer_calls == 1u && transfer_address[0] == 0x100ul);
    assert(transfer_length[0] == 8ul && no_sense_calls == 1u);

    ResetState();
    SetCdb(1u, 0u, 0x1afffcul, 16ul);
    HandleReadBufferCommand();
    assert(ReadDword(0x0b2au) == 4ul);
    assert(transfer_length[0] == 12ul);

    ResetState();
    SetCdb(1u, 0u, 0x1b0000ul, 4ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u && response_calls == 0u);
}

static void TestMode1Snapshot(void)
{
    ResetState();
    SetCdb(1u, 2u, 0ul, 0x3000ul);
    HandleReadBufferCommand();
    assert(prepare_calls == 1u && snapshot_calls == 1u);
    assert(memory[0x0c2a] == 4u);
    assert(transfer_calls == 1u);
    assert(transfer_address[0] == 0x00160000ul);
    assert(transfer_length[0] == 0x28e2ul);
    assert(no_sense_calls == 1u);

    ResetState();
    memory[0x0fae] = 1u;
    SetCdb(1u, 2u, 0ul, 1ul);
    HandleReadBufferCommand();
    assert(sequence_error_calls == 1u && prepare_calls == 0u);

    ResetState();
    memory[0x0c85] = 0u;
    SetCdb(1u, 2u, 0ul, 1ul);
    HandleReadBufferCommand();
    assert(sequence_error_calls == 1u && prepare_calls == 0u);

    ResetState();
    SetCdb(1u, 2u, 1ul, 1ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u && snapshot_calls == 0u);
}

static void TestMode2AndMode3(void)
{
    ResetState();
    SetCdb(2u, 0u, 0x1afff8ul, 20ul);
    HandleReadBufferCommand();
    assert(transfer_calls == 1u);
    assert(transfer_address[0] == 0x1afff8ul);
    assert(transfer_length[0] == 8ul && no_sense_calls == 1u);

    ResetState();
    transfer_result[0] = 4u;
    SetCdb(2u, 0u, 0ul, 1ul);
    HandleReadBufferCommand();
    assert(no_sense_calls == 0u);

    ResetState();
    SetCdb(3u, 0u, 0ul, 9ul);
    HandleReadBufferCommand();
    assert(ReadDword(0x0b2au) == 0x001b0000ul);
    assert(response_length[0] == 4ul && no_sense_calls == 1u);

    ResetState();
    SetCdb(3u, 7u, 0ul, 2ul);
    HandleReadBufferCommand();
    assert(ReadDword(0x0b2au) == 0ul);
    assert(response_length[0] == 2ul);

    ResetState();
    SetCdb(3u, 0u, 1ul, 4ul);
    HandleReadBufferCommand();
    assert(unsupported_calls == 1u && response_calls == 0u);
}

int main(void)
{
    TestAdmissionAndInvalidModes();
    TestMode0();
    TestMode1Ordinary();
    TestMode1Snapshot();
    TestMode2AndMode3();
    puts("read buffer tests passed");
    return 0;
}
