#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/mcs251.h"
#include "scsi/command_admission.h"

static u8 memory[0x1000000ul];
static u8 supported_opcodes[256];
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;
static unsigned unsupported_count;
static u8 eligibility_result;
static unsigned eligibility_count;
static unsigned mapped_sense_count;
static u8 mapped_status;
static u8 incompatible_result;
static unsigned incompatible_count;
static u8 active_result;
static unsigned active_count;

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

void HandleUnsupportedScsiCommand(void)
{
    unsupported_count++;
}

u8 IsSupportedScsiOpcode(u8 opcode)
{
    return supported_opcodes[opcode];
}

u8 ScsiCommandRequiresMedia(u8 opcode)
{
    return (u8)(memory[0xf842f7ul + opcode] == 1u);
}

u8 IsDriveOperationStatusScsiEligible(void)
{
    eligibility_count++;
    return eligibility_result;
}

void SetSenseDataFromStatusCode(u8 status)
{
    mapped_sense_count++;
    mapped_status = status;
}

u8 RejectIncompatibleMediumWriteCommand(void)
{
    incompatible_count++;
    return incompatible_result;
}

u8 ValidateCommandDuringActiveMediaOperation(void)
{
    active_count++;
    return active_result;
}

static void InitializeRomTables(void)
{
    unsigned opcode;

    memory[0xff9691u] = 0x00u;
    memory[0xff9692u] = 0xfau;
    memory[0xff9693u] = 0xffu;
    memory[0xff9694u] = 0x96u;
    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        fw_addr_t entry = 0xf83ef7ul + (fw_addr_t)opcode * 4ul;

        memory[entry] = 0x00u;
        memory[entry + 1u] = 0x12u;
        memory[entry + 2u] = 0x34u;
        memory[entry + 3u] = 0x56u;
        memory[0xf842f7ul + opcode] = 0u;
        supported_opcodes[opcode] = 1u;
    }
}

static void ResetCase(void)
{
    memset(memory, 0, 0x2000u);
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    unsupported_count = 0u;
    eligibility_result = 1u;
    eligibility_count = 0u;
    mapped_sense_count = 0u;
    mapped_status = 0u;
    incompatible_result = 1u;
    incompatible_count = 0u;
    active_result = 1u;
    active_count = 0u;
}

static void SetOpcode(u8 opcode)
{
    memory[0x0b0au] = opcode;
}

static void SetDispatchUnsupported(u8 opcode, u8 unsupported)
{
    fw_addr_t entry = 0xf83ef7ul + (fw_addr_t)opcode * 4ul;

    if(unsupported != 0u)
    {
        supported_opcodes[opcode] = 0u;
        memcpy(&memory[entry], &memory[0xff9691u], 4u);
    }
    else
    {
        supported_opcodes[opcode] = 1u;
        memory[entry] = 0x00u;
        memory[entry + 1u] = 0x12u;
        memory[entry + 2u] = 0x34u;
        memory[entry + 3u] = 0x56u;
    }
}

static u8 IsReadFamilyOpcode(u8 opcode)
{
    return (u8)(opcode == 0x08u || opcode == 0x28u || opcode == 0x44u || opcode == 0xa8u || opcode == 0xb9u || opcode == 0xbeu || opcode == 0xd8u || opcode == 0xd9u || opcode == 0xdfu);
}

static u8 IsNoMediaAllowlistOpcode(u8 opcode)
{
    return (u8)(opcode == 0x00u || opcode == 0x03u || opcode == 0x12u || opcode == 0x1au || opcode == 0x1bu || opcode == 0x1eu || opcode == 0x35u || opcode == 0x4au || opcode == 0x52u
                || opcode == 0x58u || opcode == 0x5au || opcode == 0x5bu || opcode == 0x5cu || opcode == 0xf4u || opcode == 0xf5u || opcode == 0xeeu);
}

static void TestInitialAndDispatchGates(void)
{
    unsigned opcode;

    ResetCase();
    SetOpcode(0x10u);
    memory[0x12f7u] = 1u;
    assert(ValidateScsiCommandAdmission() == 0u);
    assert(sense_count == 1u && sense_key == 4u && sense_code == 0x4000u);

    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        ResetCase();
        SetOpcode((u8)opcode);
        SetDispatchUnsupported((u8)opcode, 1u);
        assert(ValidateScsiCommandAdmission() == 0u);
        assert(unsupported_count == 1u && sense_count == 0u);
        SetDispatchUnsupported((u8)opcode, 0u);
    }

    ResetCase();
    SetOpcode(0xeeu);
    assert(ValidateScsiCommandAdmission() == 1u);
}

static void TestUnitAttentionRoutes(void)
{
    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c85u] = 0x08u;
    assert(ValidateScsiCommandAdmission() == 0u && memory[0x0c85u] == 0u && sense_count == 0u);

    ResetCase();
    SetOpcode(0x03u);
    memory[0x0c85u] = 0x08u;
    assert(ValidateScsiCommandAdmission() == 1u && memory[0x0c85u] == 0x08u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c85u] = 0x40u;
    memory[0x0c89u] = 0x80u;
    memory[0x0c8bu] = 0x11u;
    assert(ValidateScsiCommandAdmission() == 0u);
    assert(memory[0x0c85u] == 0u && memory[0x0c8bu] == 0x19u && sense_key == 6u && sense_code == 0x2900u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c85u] = 0x80u;
    assert(ValidateScsiCommandAdmission() == 0u && memory[0x0c85u] == 0u && sense_code == 0x2800u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c86u] = 0x01u;
    assert(ValidateScsiCommandAdmission() == 0u && memory[0x0c86u] == 0u && sense_code == 0x2a01u);

    ResetCase();
    SetOpcode(0xe9u);
    memory[0x0c85u] = 0xc0u;
    assert(ValidateScsiCommandAdmission() == 1u && memory[0x0c85u] == 0xc0u);
}

static void TestReadFamilyModeGate(void)
{
    unsigned opcode;

    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        u8 expected = IsReadFamilyOpcode((u8)opcode);

        ResetCase();
        SetOpcode((u8)opcode);
        memory[0x0c86u] = 0x02u;
        assert(ValidateScsiCommandAdmission() == (u8)!expected);
        assert(sense_count == expected);
        if(expected != 0u)
            assert(sense_key == 5u && sense_code == 0x6400u);
    }
}

static void TestDriveStatusAndMediaRequirementGates(void)
{
    static const u8 rejected_statuses[] = { 4u, 5u, 0x0du, 0x0eu };
    unsigned index;

    ResetCase();
    SetOpcode(0x10u);
    memory[0x1009u] = 1u;
    eligibility_result = 0u;
    assert(ValidateScsiCommandAdmission() == 0u && eligibility_count == 1u && sense_key == 4u && sense_code == 0x4000u);

    ResetCase();
    SetOpcode(0x1cu);
    memory[0x1009u] = 1u;
    eligibility_result = 0u;
    assert(ValidateScsiCommandAdmission() == 1u);

    for(index = 0u; index < sizeof(rejected_statuses); ++index)
    {
        ResetCase();
        SetOpcode(0x10u);
        memory[0xf842f7u + 0x10u] = 1u;
        memory[0x1009u] = rejected_statuses[index];
        memory[0x0c3eu] = 1u;
        memory[0x0c85u] = 0x02u;
        assert(ValidateScsiCommandAdmission() == 0u && sense_key == 2u && sense_code == 0x5700u);
        memory[0xf842f7u + 0x10u] = 0u;
    }

    ResetCase();
    SetOpcode(0x10u);
    memory[0xf842f7u + 0x10u] = 1u;
    memory[0x1009u] = 3u;
    memory[0x0c3eu] = 1u;
    memory[0x0c85u] = 0x02u;
    assert(ValidateScsiCommandAdmission() == 1u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0xf842f7u + 0x10u] = 1u;
    memory[0x0faeu] = 7u;
    assert(ValidateScsiCommandAdmission() == 0u && mapped_sense_count == 1u && mapped_status == 7u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0xf842f7u + 0x10u] = 1u;
    assert(ValidateScsiCommandAdmission() == 0u && sense_key == 2u && sense_code == 0x0401u);
    memory[0xf842f7u + 0x10u] = 0u;
}

static void TestNestedPolicyGates(void)
{
    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c9eu] = 1u;
    memory[0x0c8bu] = 0x02u;
    incompatible_result = 0u;
    assert(ValidateScsiCommandAdmission() == 0u && incompatible_count == 1u && active_count == 0u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c9eu] = 1u;
    memory[0x0c8bu] = 0x02u;
    assert(ValidateScsiCommandAdmission() == 1u && incompatible_count == 1u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c84u] = 0x08u;
    active_result = 0u;
    assert(ValidateScsiCommandAdmission() == 0u && active_count == 1u);

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c84u] = 0x08u;
    assert(ValidateScsiCommandAdmission() == 1u && active_count == 1u);
}

static void TestNoMediaAllowlist(void)
{
    unsigned opcode;

    for(opcode = 0u; opcode < 256u; ++opcode)
    {
        u8 allowed = IsNoMediaAllowlistOpcode((u8)opcode);

        ResetCase();
        SetOpcode((u8)opcode);
        memory[0x0c84u] = 0x80u;
        assert(ValidateScsiCommandAdmission() == allowed);
        assert(sense_count == (unsigned)!allowed);
        if(allowed == 0u)
            assert(sense_key == 3u && sense_code == 0x0c07u);
    }

    ResetCase();
    SetOpcode(0x10u);
    memory[0x0c84u] = 0x80u;
    memory[0x0faeu] = 1u;
    assert(ValidateScsiCommandAdmission() == 1u);
}

int main(void)
{
    InitializeRomTables();
    TestInitialAndDispatchGates();
    TestUnitAttentionRoutes();
    TestReadFamilyModeGate();
    TestDriveStatusAndMediaRequirementGates();
    TestNestedPolicyGates();
    TestNoMediaAllowlist();
    puts("SCSI command-admission tests passed");
    return 0;
}
