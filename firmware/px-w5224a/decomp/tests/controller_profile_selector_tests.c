#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_profile_selector.h"

#define MEMORY_SIZE 0x20000u
#define FIXED_ROM_BASE 0xf8363dul
#define FIXED_ROM_SIZE 0x2000u

typedef struct
{
    u8 kind;
    fw_addr_t address;
    u8 value;
} Event;

static u8 memory[MEMORY_SIZE];
static u8 fixed_rom[FIXED_ROM_SIZE];
static Event events[160];
static unsigned event_count;
static u8 captured_r9;
static unsigned multiply_count;
static u32 multiply_left;
static u32 multiply_right;
static unsigned map_count;
static u8 map_inputs[8];
static u8 mutate_fca_after_main_profile;

static void RecordEvent(u8 kind, fw_addr_t address, u8 value)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].address = address;
    events[event_count].value = value;
    ++event_count;
}

u8 FirmwareRead8(fw_addr_t address)
{
    RecordEvent('r', address, 0u);
    if(address >= FIXED_ROM_BASE && address < FIXED_ROM_BASE + FIXED_ROM_SIZE)
        return fixed_rom[address - FIXED_ROM_BASE];
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    RecordEvent('w', address, value);
    assert(address < MEMORY_SIZE);
    memory[address] = value;
    if(address == 0x1808bu && mutate_fca_after_main_profile != 0u)
        memory[0xfcau] = mutate_fca_after_main_profile;
}

u8 ObserveCaptureInheritedR9ForControllerProfileSelector(void)
{
    RecordEvent('c', 9u, captured_r9);
    return captured_r9;
}

void ObserveControllerProfileSelectorClearEa(void)
{
    RecordEvent('b', 0x0547u, 0u);
}

void ObserveControllerProfileSelectorSetEa(void)
{
    RecordEvent('b', 0x0547u, 1u);
}

void ObserveControllerProfileSelectorSetEc(void)
{
    RecordEvent('b', 0x0546u, 1u);
}

void ObserveControllerProfileSelectorSetA9Bit2(void)
{
    RecordEvent('b', 0x05aau, 1u);
}

u32 MultiplyU32Low(u32 factor_a, u32 factor_b)
{
    ++multiply_count;
    multiply_left = factor_a;
    multiply_right = factor_b;
    return factor_a * factor_b;
}

u8 MapControllerRegisterIndexSkippingGap(u8 index)
{
    assert(map_count < sizeof(map_inputs));
    map_inputs[map_count++] = index;
    return (u8)(index ^ 0xa5u);
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(fixed_rom, 0, sizeof(fixed_rom));
    memset(events, 0, sizeof(events));
    memset(map_inputs, 0, sizeof(map_inputs));
    event_count = 0u;
    captured_r9 = 0u;
    multiply_count = 0u;
    multiply_left = 0u;
    multiply_right = 0u;
    map_count = 0u;
    mutate_fca_after_main_profile = 0u;
}

static u8 IsMappedIndex(u8 index)
{
    return (u8)(index == 6u || index == 9u || index == 13u || index == 16u || index == 20u || index == 23u);
}

static void AssertProfileOutputs(fw_addr_t source)
{
    static const u16 ports[24] = { 0x8082u, 0x8083u, 0x8094u, 0x8095u, 0x8084u, 0x8085u, 0x808eu, 0x8096u, 0x8097u, 0x808cu, 0x8086u, 0x8087u, 0x808fu, 0x8090u, 0x8098u, 0x8099u, 0x808du, 0x8088u,
        0x8089u, 0x8091u, 0x8092u, 0x809au, 0x809bu, 0x808bu };
    unsigned index;

    for(index = 0u; index != 24u; ++index)
    {
        u8 value;

        if(source >= FIXED_ROM_BASE)
            value = fixed_rom[source - FIXED_ROM_BASE + index];
        else
            value = memory[source + index];
        if(IsMappedIndex((u8)index) != 0u)
            value ^= 0xa5u;
        assert(memory[0x10000u + ports[index]] == value);
    }
}

static void AssertMappedInputs(fw_addr_t source)
{
    static const u8 offsets[6] = { 6u, 9u, 13u, 16u, 20u, 23u };
    unsigned index;

    assert(map_count == 6u);
    for(index = 0u; index != 6u; ++index)
    {
        u8 expected = source >= FIXED_ROM_BASE ? fixed_rom[source - FIXED_ROM_BASE + offsets[index]] : memory[source + offsets[index]];

        assert(map_inputs[index] == expected);
    }
}

static void AssertInterruptEnvelope(u8 first_shadow, u8 final_shadow)
{
    fw_addr_t write_addresses[4];
    u8 write_values[4];
    unsigned write_count = 0u;
    unsigned index;

    assert(events[0].kind == 'c');
    assert(events[1].kind == 'b' && events[1].address == 0x0547u && events[1].value == 0u);
    for(index = 0u; index != event_count; ++index)
    {
        if(events[index].kind == 'w' && (events[index].address == 0xfcau || events[index].address == 0x18002u))
        {
            assert(write_count < 4u);
            write_addresses[write_count] = events[index].address;
            write_values[write_count] = events[index].value;
            ++write_count;
        }
    }
    assert(write_count == 4u);
    assert(write_addresses[0] == 0xfcau && write_values[0] == first_shadow);
    assert(write_addresses[1] == 0x18002u && write_values[1] == first_shadow);
    assert(write_addresses[2] == 0xfcau && write_values[2] == final_shadow);
    assert(write_addresses[3] == 0x18002u && write_values[3] == final_shadow);
    assert(memory[0xfcau] == final_shadow);
    assert(memory[0x18002u] == final_shadow);
    assert(events[event_count - 3u].kind == 'b' && events[event_count - 3u].address == 0x0547u && events[event_count - 3u].value == 1u);
    assert(events[event_count - 2u].kind == 'b' && events[event_count - 2u].address == 0x0546u);
    assert(events[event_count - 1u].kind == 'b' && events[event_count - 1u].address == 0x05aau);
}

static void test_fixed_profile_uses_memory_index_and_inherited_auxiliary(void)
{
    fw_addr_t source = FIXED_ROM_BASE + 3u * 24u;
    fw_addr_t auxiliary = (fw_addr_t)(u16)(0xe823u + 7u * 34u);
    unsigned index;

    ResetFixture();
    captured_r9 = 7u;
    memory[0xfcau] = 0x52u;
    memory[0xf05fu] = 3u;
    for(index = 0u; index != 24u; ++index)
        fixed_rom[source - FIXED_ROM_BASE + index] = (u8)(0x20u + index);
    for(index = 0u; index != 4u; ++index)
        memory[auxiliary + index] = (u8)(0xd0u + index);

    ProgramControllerRegisterProfileSelector(0x13u);

    assert(multiply_count == 1u && multiply_left == 3u && multiply_right == 24u);
    AssertProfileOutputs(source);
    AssertMappedInputs(source);
    assert(memory[0x180a9u] == 0xd0u);
    assert(memory[0x180a6u] == 0xd1u);
    assert(memory[0x180a7u] == 0xd2u);
    assert(memory[0x180a8u] == 0xd3u);
    AssertInterruptEnvelope(0x5au, 0x42u);
}

static void test_generated_profile_derives_both_sources_from_selector(void)
{
    fw_addr_t source = (fw_addr_t)(u16)(0xe806u + 2u * 34u);
    fw_addr_t auxiliary = (fw_addr_t)(u16)(0xe823u + 2u * 34u);
    unsigned index;

    ResetFixture();
    captured_r9 = 0x7fu;
    memory[0xfcau] = 0x08u;
    for(index = 0u; index != 24u; ++index)
        memory[source + index] = (u8)(0x40u + index);
    for(index = 0u; index != 4u; ++index)
        memory[auxiliary + index] = (u8)(0xe0u + index);

    ProgramControllerRegisterProfileSelector(0x16u);

    assert(multiply_count == 0u);
    AssertProfileOutputs(source);
    AssertMappedInputs(source);
    assert(memory[0x180a9u] == 0xe0u);
    assert(memory[0x180a6u] == 0xe1u);
    assert(memory[0x180a7u] == 0xe2u);
    assert(memory[0x180a8u] == 0xe3u);
    AssertInterruptEnvelope(0x08u, 0x00u);
}

static void test_final_shadow_uses_fresh_fca_read(void)
{
    ResetFixture();
    memory[0xfcau] = 0x11u;
    mutate_fca_after_main_profile = 0xbfu;

    ProgramControllerRegisterProfileSelector(0x14u);

    AssertInterruptEnvelope(0x19u, 0xa7u);
}

int main(void)
{
    test_fixed_profile_uses_memory_index_and_inherited_auxiliary();
    test_generated_profile_derives_both_sources_from_selector();
    test_final_shadow_uses_fresh_fca_read();
    puts("controller profile selector tests passed");
    return 0;
}
