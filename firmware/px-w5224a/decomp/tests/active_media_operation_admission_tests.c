#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/mcs251.h"
#include "scsi/active_media_operation_admission.h"

static u8 memory[0x1000000ul];
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;
static unsigned information_count;
static u32 sense_information;
static unsigned sequence_error_count;
static unsigned tray_sense_count;
static unsigned finalize_count;
static u8 finalize_result;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void SetSenseData(u8 key, u16 code)
{
    sense_count++;
    sense_key = key;
    sense_code = code;
}

void SetSenseInformation(u32 information)
{
    information_count++;
    sense_information = information;
}

void SetCommandSequenceErrorSense(void)
{
    sequence_error_count++;
}

void SetTrayOpenOrNoSenseForCurrentCommand(void)
{
    tray_sense_count++;
}

u8 FinalizeActiveWritePipelineAndTrackState(void)
{
    finalize_count++;
    return finalize_result;
}

static void WriteBigEndianU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetCase(void)
{
    memset(memory, 0, 0x2000u);
    memset(&memory[0xe600u], 0, 0x200u);
    memset(&memory[0x2ba5a0u], 0, 0x20u);
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    information_count = 0u;
    sense_information = 0u;
    sequence_error_count = 0u;
    tray_sense_count = 0u;
    finalize_count = 0u;
    finalize_result = 1u;
    memory[0x0c84u] = 0x08u;
}

static void SetOpcode(u8 opcode)
{
    memory[0x0b0au] = opcode;
}

static u8 IsBusyAllowlistOpcode(u8 opcode)
{
    return (u8)(opcode == 0x03u || opcode == 0x12u || opcode == 0x46u || opcode == 0x4au || opcode == 0xebu || opcode == 0xf5u);
}

static u8 IsImmediateOpcode(u8 opcode)
{
    switch(opcode)
    {
    case 0x00u:
    case 0x03u:
    case 0x0au:
    case 0x12u:
    case 0x2au:
    case 0x35u:
    case 0x46u:
    case 0x4au:
    case 0x52u:
    case 0x5bu:
    case 0x5cu:
    case 0xaau:
    case 0xebu:
    case 0xf5u:
    case 0xf7u:
    case 0xf8u:
        return 1u;
    default:
        return 0u;
    }
}

static u8 IsFinalizationOpcode(u8 opcode)
{
    return (u8)(opcode == 0x08u || opcode == 0x28u || opcode == 0xa8u || opcode == 0xb9u || opcode == 0xbeu || opcode == 0xd8u || opcode == 0xd9u);
}

static u8 IsMediaStatusGatedOpcode(u8 opcode)
{
    return (u8)(opcode == 0x1au || opcode == 0x23u || opcode == 0x25u || opcode == 0x2eu || opcode == 0x51u || opcode == 0x55u || opcode == 0x5au);
}

static void TestInactiveAndBusyAdmission(void)
{
    unsigned opcode;

    ResetCase();
    memory[0x0c84u] = 0u;
    assert(ValidateCommandDuringActiveMediaOperation() == 1u);

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0x0c88u] = 0x40u;
    assert(ValidateCommandDuringActiveMediaOperation() == 1u);

    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        u8 expected = IsBusyAllowlistOpcode((u8)opcode);

        ResetCase();
        memory[0x0c85u] = 1u;
        SetOpcode((u8)opcode);
        assert(ValidateCommandDuringActiveMediaOperation() == expected);
        assert(sense_count == (unsigned)!expected);
        if(expected == 0u)
            assert(sense_key == 2u && sense_code == 0x0408u);
    }

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0x0c9eu] = 1u;
    SetOpcode(0x52u);
    assert(ValidateCommandDuringActiveMediaOperation() == 1u);

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0x0c87u] = 0x20u;
    SetOpcode(0x10u);
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(sense_key == 2u && sense_code == 0x0404u);
}

static void TestBusySenseInformation(void)
{
    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0xe622u] = 1u;
    SetOpcode(0x10u);
    WriteBigEndianU32(0xe606u, 105u);
    WriteBigEndianU32(0x1235u, 100u);
    WriteBigEndianU32(0xe60au, 10u);
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(ReadBigEndianU32(0xe60eu) == 5u);
    assert(information_count == 1u && sense_information == 5u);
    assert(ReadBigEndianU32(0xe60au) == 5u);

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0xe622u] = 1u;
    SetOpcode(0x10u);
    WriteBigEndianU32(0xe606u, 110u);
    WriteBigEndianU32(0x1235u, 100u);
    WriteBigEndianU32(0xe60au, 5u);
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(ReadBigEndianU32(0xe60eu) == 10u);
    assert(sense_information == 5u && ReadBigEndianU32(0xe60au) == 5u);

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0xe622u] = 1u;
    SetOpcode(0x10u);
    WriteBigEndianU32(0xe606u, 0u);
    WriteBigEndianU32(0x1235u, 1u);
    WriteBigEndianU32(0xe60au, 1u);
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(sense_information == 0xfffffffful);
    assert(ReadBigEndianU32(0xe60au) == 0xfffffffful);

    ResetCase();
    memory[0x0c85u] = 1u;
    memory[0xe622u] = 1u;
    memory[0x0e40u] = 1u;
    SetOpcode(0x10u);
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(information_count == 0u);
}

static void TestNormalOpcodeMatrix(void)
{
    unsigned opcode;

    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        u8 immediate = IsImmediateOpcode((u8)opcode);
        u8 finalization = IsFinalizationOpcode((u8)opcode);
        u8 gated = IsMediaStatusGatedOpcode((u8)opcode);
        u8 special = (u8)(opcode == 0x1eu);
        u8 expected = (u8)(immediate || finalization || gated || special);

        ResetCase();
        memory[0x0c9eu] = 1u;
        SetOpcode((u8)opcode);
        assert(ValidateCommandDuringActiveMediaOperation() == expected);
        assert(finalize_count == (unsigned)(finalization || special));
        assert(sequence_error_count == (unsigned)!expected);
        assert(tray_sense_count == 0u);
        if(finalization || special)
            assert((memory[0x0c84u] & 0x08u) == 0u);
    }
}

static void TestNormalRejectionAndFinalizationGates(void)
{
    static const u8 gated_opcodes[] = { 0x1au, 0x23u, 0x25u, 0x2eu, 0x51u, 0x55u, 0x5au };
    unsigned index;

    for(index = 0u; index < sizeof(gated_opcodes); ++index)
    {
        ResetCase();
        SetOpcode(gated_opcodes[index]);
        assert(ValidateCommandDuringActiveMediaOperation() == 0u);
        assert(sequence_error_count == 1u);
    }

    ResetCase();
    SetOpcode(0x28u);
    memory[0x0c44u] = 1u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(sequence_error_count == 1u && finalize_count == 0u);
    assert(memory[0x0c84u] == 0x08u);

    ResetCase();
    SetOpcode(0x28u);
    finalize_result = 0u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(finalize_count == 1u && memory[0x0c84u] == 0u);

    ResetCase();
    SetOpcode(0x1eu);
    memory[0x0c8bu] = 0x20u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(tray_sense_count == 1u && finalize_count == 0u);

    ResetCase();
    SetOpcode(0x1eu);
    memory[0x0c44u] = 1u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(sequence_error_count == 1u && finalize_count == 0u);

    ResetCase();
    SetOpcode(0x1eu);
    memory[0x0b0eu] = 1u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(sequence_error_count == 1u && finalize_count == 0u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c8bu] = 0x20u;
    assert(ValidateCommandDuringActiveMediaOperation() == 0u);
    assert(tray_sense_count == 1u && sequence_error_count == 0u);
}

static void RunCountdownCase(u8 status, u8 suppress, u8 branch, u8 c89, u8 e750)
{
    ResetCase();
    SetOpcode(0x28u);
    memory[0x0c9eu] = status;
    memory[0x2ba5b1u] = suppress;
    memory[0x2ba5afu] = branch;
    memory[0x0c89u] = c89;
    memory[0xe750u] = e750;
    assert(ValidateCommandDuringActiveMediaOperation() == 1u);
    assert(finalize_count == 1u);
}

static void TestCompletionCountdown(void)
{
    RunCountdownCase(1u, 0u, 0u, 0x20u, 0u);
    assert(memory[0x0c89u] == 0x28u && memory[0x122bu] == 20u);

    RunCountdownCase(2u, 0u, 0u, 0u, 1u);
    assert(memory[0x0c89u] == 0x08u && memory[0x122bu] == 60u);

    RunCountdownCase(1u, 0u, 1u, 0x80u, 0u);
    assert(memory[0x0c8au] == 1u && memory[0x122cu] == 10u);

    RunCountdownCase(1u, 0u, 1u, 0u, 0u);
    assert(memory[0x0c8au] == 0u && memory[0x122cu] == 0u);

    RunCountdownCase(2u, 1u, 0u, 0x80u, 1u);
    assert(memory[0x0c8au] == 0u && memory[0x122bu] == 0u);

    RunCountdownCase(3u, 1u, 1u, 0x80u, 0u);
    assert(memory[0x0c8au] == 1u && memory[0x122cu] == 10u);

    RunCountdownCase(4u, 0u, 0u, 0x80u, 0u);
    assert(memory[0x0c8au] == 0u && memory[0x122cu] == 0u);
}

int main(void)
{
    TestInactiveAndBusyAdmission();
    TestBusySenseInformation();
    TestNormalOpcodeMatrix();
    TestNormalRejectionAndFinalizationGates();
    TestCompletionCountdown();
    puts("active media operation admission tests passed");
    return 0;
}
