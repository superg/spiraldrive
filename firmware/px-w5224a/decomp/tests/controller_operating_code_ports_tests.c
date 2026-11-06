#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_operating_code_ports.h"

#define MEMORY_SIZE 0x20000u
#define PROFILE_BASE 0xff8516ul

typedef struct
{
    fw_addr_t address;
    u8 value;
} PortWrite;

static u8 memory[MEMORY_SIZE];
static u8 profiles[10u * 8u];
static PortWrite writes[8];
static unsigned write_count;
static fw_addr_t strobe_address;
static u8 strobe_value;
static unsigned strobe_count;
static unsigned progress_update_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= PROFILE_BASE && address < PROFILE_BASE + sizeof(profiles))
        return profiles[address - PROFILE_BASE];
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
}

void ObserveOperatingCodePortsProgressUpdate(void)
{
    ++progress_update_count;
}

void ObserveOperatingCodePortsStrobe(fw_addr_t address, u8 value)
{
    ++strobe_count;
    strobe_address = address;
    strobe_value = value;
}

void ObserveOperatingCodePortsWrite(fw_addr_t address, u8 value)
{
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    writes[write_count].address = address;
    writes[write_count].value = value;
    ++write_count;
}

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0, sizeof(memory));
    memset(writes, 0, sizeof(writes));
    for(index = 0u; index != sizeof(profiles); ++index)
        profiles[index] = (u8)(0x20u + index);
    write_count = 0u;
    strobe_address = 0u;
    strobe_value = 0u;
    strobe_count = 0u;
    progress_update_count = 0u;
    memory[0x1302u] = 0xe5u;
    memory[0xfab7u] = 0xd7u;
}

static void AssertAppliedProfile(u8 operating_code, u8 enabled, u8 expected_profile, u8 expected_command)
{
    static const fw_addr_t ports[8] = { 0x18094ul, 0x18095ul, 0x18096ul, 0x18097ul, 0x18092ul, 0x18091ul, 0x18093ul, 0x18090ul };
    unsigned index;

    memory[0x1006u] = enabled;
    ConfigureControllerPortsForOperatingCode(operating_code);

    assert(strobe_count == 1u && strobe_address == 0x1804bul && strobe_value == expected_command);
    assert(progress_update_count == 0u);
    assert(memory[0x1302u] == 0x65u);
    assert(write_count == 8u);
    for(index = 0u; index != 8u; ++index)
    {
        u8 expected = profiles[(u16)expected_profile * 8u + index];

        if(index == 3u && operating_code >= 0x20u)
            expected = 0xd7u;
        assert(writes[index].address == ports[index]);
        assert(writes[index].value == expected);
    }
}

static void test_all_profile_dispatch_classes(void)
{
    static const struct
    {
        u8 code;
        u8 enabled;
        u8 profile;
        u8 command;
    } cases[] = {
        { 1u,    0u, 0u, 0x2au },
        { 2u,    0u, 0u, 0x2au },
        { 4u,    0u, 1u, 0x2au },
        { 8u,    0u, 2u, 0x2au },
        { 10u,   0u, 3u, 0x2au },
        { 12u,   0u, 3u, 0x2au },
        { 16u,   0u, 4u, 0x2au },
        { 16u,   1u, 5u, 0x2au },
        { 20u,   0u, 4u, 0x2au },
        { 20u,   1u, 5u, 0x2au },
        { 24u,   1u, 6u, 0x2au },
        { 32u,   1u, 6u, 0x2au },
        { 40u,   1u, 7u, 0x2au },
        { 52u,   1u, 8u, 0x3au },
        { 0xffu, 1u, 8u, 0x3au },
        { 0u,    1u, 8u, 0x3au }
    };
    unsigned index;

    for(index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetFixture();
        AssertAppliedProfile(cases[index].code, cases[index].enabled, cases[index].profile, cases[index].command);
    }
}

static void test_code_eight_alternate_profile_flag(void)
{
    ResetFixture();
    memory[0xf9e9u] = 0x04u;
    AssertAppliedProfile(8u, 0u, 9u, 0x2au);
}

static void test_disabled_high_codes_only_update_progress_and_strobe(void)
{
    static const u8 codes[] = { 24u, 32u, 40u, 52u, 0xffu, 0u };
    unsigned index;

    for(index = 0u; index != sizeof(codes); ++index)
    {
        ResetFixture();
        memory[0x1006u] = 0u;
        ConfigureControllerPortsForOperatingCode(codes[index]);
        assert(progress_update_count == 1u);
        assert(strobe_count == 1u && strobe_address == 0x1804bul && strobe_value == 0x2au);
        assert(write_count == 0u);
        assert(memory[0x1302u] == 0xe5u);
    }
}

int main(void)
{
    test_all_profile_dispatch_classes();
    test_code_eight_alternate_profile_flag();
    test_disabled_high_codes_only_update_progress_and_strobe();
    puts("controller operating-code port tests passed");
    return 0;
}
