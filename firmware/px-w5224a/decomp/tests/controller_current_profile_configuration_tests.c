#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_current_profile_configuration.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 events[16];
static unsigned event_count;
static u8 fixed_code;
static unsigned fixed_count;
static unsigned map_count;
static unsigned multiply_count;
static u32 multiply_left[4];
static u32 multiply_right[4];

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

void ObserveCurrentProfileClearEa(void)
{
    events[event_count++] = 1u;
}
void ObserveCurrentProfileSetEa(void)
{
    events[event_count++] = 2u;
}
void ObserveCurrentProfileSetEc(void)
{
    events[event_count++] = 3u;
}
void ObserveCurrentProfileSetA9Bit2(void)
{
    events[event_count++] = 4u;
}

void ObserveCurrentProfileApplyFixed(u8 operating_code)
{
    events[event_count++] = 5u;
    fixed_code = operating_code;
    ++fixed_count;
}

u32 ObserveCurrentProfileMultiply(u32 factor_a, u32 factor_b)
{
    assert(multiply_count < 4u);
    multiply_left[multiply_count] = factor_a;
    multiply_right[multiply_count] = factor_b;
    ++multiply_count;
    return factor_a * factor_b;
}

u8 MapControllerRegisterIndexSkippingGap(u8 index)
{
    ++map_count;
    return (u8)(index + 0x40u);
}

static void Reset(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    fixed_code = 0u;
    fixed_count = 0u;
    map_count = 0u;
    multiply_count = 0u;
}

static void CheckGates(void)
{
    assert(events[0] == 1u);
    assert(events[event_count - 3u] == 2u);
    assert(events[event_count - 2u] == 3u);
    assert(events[event_count - 1u] == 4u);
    assert(memory[0x000fcaul] == 0u);
    assert(memory[0x018002ul] == 0u);
}

static void Fill(fw_addr_t address, unsigned count, u8 first)
{
    unsigned index;

    for(index = 0u; index < count; ++index)
        memory[address + index] = (u8)(first + index);
}

static void CheckIndexedMaskedRecord(void)
{
    fw_addr_t source = 0xf8395dul + 28u;
    unsigned index;

    Reset();
    memory[0x000fcaul] = 0x10u;
    memory[0x001008ul] = 1u;
    memory[0x000e47ul] = 2u;
    memory[0x000e43ul] = 0x10u;
    memory[0x00f969ul] = 2u;
    memory[0x00fb5eul] = 0u;
    Fill(source, 28u, 0x20u);
    Fill(0xf83ea2ul, 5u, 0xd0u);
    ConfigureControllerRegisterProfileForCurrentMediaState();

    assert(multiply_count == 1u && multiply_left[0] == 14u && multiply_right[0] == 2u);
    assert(memory[0x00131cul] == 0x82u && memory[0x018080ul] == 0x82u);
    for(index = 0u; index < 28u; ++index)
    {
        u8 expected = index < 12u ? (u8)(0x20u + index) : index == 18u ? 0x2cu : index == 19u ? 0x2du : 0u;
        assert(memory[0x018081ul + index] == expected);
    }
    assert(memory[0x0180a9ul] == 0xd0u);
    assert(memory[0x0180a6ul] == 0xd1u);
    assert(memory[0x0180a7ul] == 0xd2u);
    assert(memory[0x0180a8ul] == 0xd3u);
    assert(memory[0x0180aaul] == 0xd4u);
    CheckGates();
}

static void CheckIndexedFixedRecord(void)
{
    Reset();
    memory[0x000fcaul] = 0x10u;
    memory[0x001008ul] = 1u;
    memory[0x000e47ul] = 0xffu;
    memory[0x000e43ul] = 0x20u;
    ConfigureControllerRegisterProfileForCurrentMediaState();

    assert(fixed_count == 1u && fixed_code == 0x34u);
    assert(memory[0x0180b6ul] == 4u);
    assert(events[0] == 1u && events[1] == 5u);
    CheckGates();
}

static void CheckOrdinaryPrimaryRecord(void)
{
    Reset();
    memory[0x000fcaul] = 0x10u;
    memory[0x001008ul] = 0u;
    memory[0x000e43ul] = 4u;
    memory[0x00103bul] = 0u;
    memory[0x00fb88ul] = 0u;
    memory[0x00fb80ul] = 2u;
    memory[0x00f05ful] = 3u;
    memory[0x00ef72ul] = 8u;
    Fill(0xf83447ul, 3u, 0x61u);
    Fill(0xf83451ul, 4u, 0x71u);
    ConfigureControllerRegisterProfileForCurrentMediaState();

    assert(multiply_count == 1u && multiply_left[0] == 3u && multiply_right[0] == 3u);
    assert(memory[0x00131cul] == 0xa8u && memory[0x018080ul] == 0xa8u);
    assert(memory[0x0180b2ul] == 0x61u);
    assert(memory[0x0180b3ul] == 0x62u);
    assert(memory[0x0180b4ul] == 0x63u);
    assert(memory[0x0180a9ul] == 0x71u);
    assert(memory[0x0180a6ul] == 0x72u);
    assert(memory[0x0180a7ul] == 0x73u);
    assert(memory[0x0180a8ul] == 0x74u);
    assert(memory[0x0180aaul] == 0u && map_count == 0u);
    CheckGates();
}

static void CheckOrdinaryMappedRecord(void)
{
    static const fw_addr_t mapped_destinations[24] = { 0x018082ul, 0x018083ul, 0x018094ul, 0x018095ul, 0x018084ul, 0x018085ul, 0x01808eul, 0x018096ul, 0x018097ul, 0x01808cul, 0x018086ul, 0x018087ul,
        0x01808ful, 0x018090ul, 0x018098ul, 0x018099ul, 0x01808dul, 0x018088ul, 0x018089ul, 0x018091ul, 0x018092ul, 0x01809aul, 0x01809bul, 0x01808bul };
    fw_addr_t source = 0xf8372dul + 24u;
    unsigned index;

    Reset();
    memory[0x000fcaul] = 0x10u;
    memory[0x001008ul] = 0u;
    memory[0x000e43ul] = 0x14u;
    memory[0x00103bul] = 1u;
    memory[0x00fb88ul] = 2u;
    memory[0x00f05ful] = 1u;
    Fill(source, 24u, 1u);
    Fill(0xf83461ul, 4u, 0x81u);
    ConfigureControllerRegisterProfileForCurrentMediaState();

    assert(multiply_count == 1u && multiply_left[0] == 24u && multiply_right[0] == 1u);
    assert(memory[0x00131cul] == 0x98u && memory[0x018080ul] == 0x98u);
    for(index = 0u; index < 24u; ++index)
    {
        u8 expected = (index == 6u || index == 9u || index == 13u || index == 16u || index == 20u || index == 23u) ? (u8)(0x41u + index) : (u8)(1u + index);
        assert(memory[mapped_destinations[index]] == expected);
    }
    assert(map_count == 6u);
    assert(memory[0x01808aul] == 8u && memory[0x018093ul] == 0u);
    assert(memory[0x0180a9ul] == 0x81u);
    assert(memory[0x0180a6ul] == 0x82u);
    assert(memory[0x0180a7ul] == 0x83u);
    assert(memory[0x0180a8ul] == 0x84u);
    CheckGates();
}

int main(void)
{
    CheckIndexedMaskedRecord();
    CheckIndexedFixedRecord();
    CheckOrdinaryPrimaryRecord();
    CheckOrdinaryMappedRecord();
    puts("controller_current_profile_configuration_tests: ok");
    return 0;
}
