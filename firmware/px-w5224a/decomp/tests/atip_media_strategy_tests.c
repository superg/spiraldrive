#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/atip_media_strategy.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 validation_result;
static unsigned validation_count;
static unsigned reset_count;
static unsigned restart_count;
static u32 restart_entry;
static unsigned initialize_count;

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

u8 ObserveAtipStrategyValidate(void)
{
    ++validation_count;
    return validation_result;
}

void ObserveAtipStrategyResetDrive(void)
{
    ++reset_count;
}

void ObserveAtipStrategyRestart(u32 entry_point)
{
    ++restart_count;
    restart_entry = entry_point;
}

void ObserveAtipStrategyInitialize(void)
{
    ++initialize_count;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void SetIdentifier(u16 identifier)
{
    u16 delta = (u16)(identifier - 0xa914u);
    u8 second;
    u8 group;

    for(second = 0u; second < 60u; ++second)
    {
        for(group = 0u; group < 8u; ++group)
        {
            if((u16)((u16)second * 75u + (u16)group * 10u) == delta)
            {
                memory[0x001140ul] = second;
                memory[0x001141ul] = (u8)(group * 10u);
                return;
            }
        }
    }
    assert(!"unreachable ATIP strategy identifier");
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    validation_result = 1u;
    validation_count = reset_count = restart_count = initialize_count = 0u;
    restart_entry = 0ul;
    memory[0x00fad7ul] = 0x12u;
    memory[0x00fad8ul] = 0x7fu;
    memory[0x001301ul] = 0xffu;
    memory[0x001008ul] = 1u;
}

static void CheckCase(u16 identifier, u8 bcd_second, u8 bcd_frame, s32 lead_in, u32 lead_out, u8 expected_index, u8 expected_alternate)
{
    ResetFixture();
    SetIdentifier(identifier);
    memory[0x00fb54ul] = bcd_second;
    memory[0x00fb55ul] = bcd_frame;
    WriteU32(0x00114ful, (u32)lead_in);
    WriteU32(0x001153ul, lead_out);
    memset(&memory[0xff9491ul], expected_alternate, 256u);

    SelectAndApplyMediaStrategyFromAtipLeadInTime();

    assert(memory[0x00fb5dul] == expected_index);
    assert(memory[0x00fb5eul] == expected_alternate);
    assert(memory[0x00fad7ul] == 0x12u);
    assert(memory[0x00fad8ul] == 0x6fu);
    assert(validation_count == 1u);
    assert(reset_count == 0u && restart_count == 0u);
    assert(initialize_count == 1u);
}

static void TestIdentityRoutes(void)
{
    CheckCase(0xa914u, 0u, 0u, 0, 0ul, 0x30u, 0u);
    CheckCase(0xad79u, 0u, 7u, 0, 0ul, 0x37u, 1u);
    CheckCase(0xad83u, 0u, 2u, 0, 0x57e3eul, 0x48u, 0u);
    CheckCase(0xb229u, 0u, 7u, 0, 0x521f9ul, 0x4bu, 1u);
    CheckCase(0xb70bu, 0x15u, 0x17u, 0, 0ul, 0x28u, 1u);
    CheckCase(0xaf9au, 0u, 6u, 0, 0x57e38ul, 0x0fu, 1u);
    CheckCase(0xb68eu, 0u, 0u, (s32)-0x2db4L, 0ul, 0x06u, 0u);
    CheckCase(0xb698u, 0u, 0x15u, 0, 0ul, 0x3fu, 1u);
    CheckCase(0xb701u, 0u, 2u, 0, 0x57e3eul, 0x46u, 0u);
    CheckCase(0xb0c6u, 0u, 0x25u, 0, 0x57e3eul, 0x01u, 1u);
    CheckCase(0xb760u, 0u, 0x54u, 0, 0ul, 0x20u, 0u);
    CheckCase(0xb0fdu, 0u, 0u, 0, 0ul, 0x01u, 1u);
    CheckCase(0xb779u, 0u, 7u, 0, 0x57e3dul, 0x17u, 1u);
    CheckCase(0xb27eu, 0u, 9u, 0, 0x57e3cul, 0x4fu, 1u);
    CheckCase(0xb7ceu, 0u, 2u, 0, 0ul, 0x25u, 0u);
    CheckCase(0xb5cbu, 0u, 3u, 0, 0ul, 0x19u, 1u);
    CheckCase(0xb756u, 0u, 7u, 0, 0ul, 0x32u, 1u);
}

static void TestRecoveryAndStateTwoBypass(void)
{
    ResetFixture();
    validation_result = 0u;
    memory[0x001008ul] = 2u;
    SelectAndApplyMediaStrategyFromAtipLeadInTime();
    assert(memory[0x01802bul] == 0x80u);
    assert(memory[0x001301ul] == 0xfdu);
    assert(reset_count == 1u && restart_count == 1u);
    assert(restart_entry == 0x00f84f6aul);
    assert(initialize_count == 1u);
}

static void TestRecordFamilyValidation(void)
{
    ResetFixture();
    SetIdentifier(0xa914u);
    memory[0xff9491ul + 0x30u] = 1u;
    SelectAndApplyMediaStrategyFromAtipLeadInTime();
    assert(memory[0x00fb5dul] == 0u);

    ResetFixture();
    SetIdentifier(0xad79u);
    memory[0x00fb55ul] = 7u;
    memory[0xff9491ul + 0x37u] = 0u;
    SelectAndApplyMediaStrategyFromAtipLeadInTime();
    assert(memory[0x00fb5dul] == 1u);
}

int main(void)
{
    TestIdentityRoutes();
    TestRecoveryAndStateTwoBypass();
    TestRecordFamilyValidation();
    puts("atip_media_strategy_tests: ok");
    return 0;
}
