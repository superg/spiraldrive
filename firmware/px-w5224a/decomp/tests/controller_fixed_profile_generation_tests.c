#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_fixed_profile_generation.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static const float expected_values[5] = { 20.0f, 24.0f, 32.0f, 40.0f, 52.0f };
static const s8 baselines[5] = { -3, 4, -5, 6, -7 };
static float normalized_inputs[5];
static float scaled_inputs[5];
static unsigned initialize_count;
static unsigned generated_count;
static unsigned normalized_count;
static unsigned scaled_count;
static unsigned event_count;
static u8 events[12];

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

void InitializeFixedControllerProfileStorage(void)
{
    unsigned profile_index;

    events[event_count++] = 1u;
    ++initialize_count;
    for(profile_index = 0u; profile_index < 5u; ++profile_index)
    {
        memset(&memory[0x00e75cul + profile_index * 34u], 0, 33u);
        memory[0x00e75cul + profile_index * 34u + 33u] = 0xffu;
        memory[0x00e752ul + profile_index * 2u] = 0u;
        memory[0x00e752ul + profile_index * 2u + 1u] = 0u;
    }
}

void GenerateControllerRegisterProfiles(u16 profile_input)
{
    events[event_count++] = 2u;
    assert(profile_input == 0x4321u);
    ++generated_count;
}

s8 GenerateAndNormalizeControllerProfileSamples(float profile_value)
{
    unsigned index;

    assert(normalized_count < 5u);
    assert(profile_value == expected_values[normalized_count]);
    events[event_count++] = (u8)(3u + normalized_count * 2u);
    normalized_inputs[normalized_count] = profile_value;
    for(index = 0u; index < 18u; ++index)
    {
        memory[0x00f9b7ul + index] = (u8)(0x10u + normalized_count * 0x20u + index);
        memory[0x00f9cbul + index] = (u8)(0x80u + normalized_count * 0x10u + index);
    }
    return baselines[normalized_count++];
}

u16 ComputeControllerProfileScaledWord(float profile_value)
{
    assert(scaled_count < 5u);
    assert(profile_value == expected_values[scaled_count]);
    events[event_count++] = (u8)(4u + scaled_count * 2u);
    scaled_inputs[scaled_count] = profile_value;
    return (u16)(0x9000u + scaled_count++);
}

static u16 LoadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1ul]);
}

static void CheckFixedProfiles(void)
{
    static const u8 encoded_baselines[5] = { 0x43u, 0x04u, 0x45u, 0x06u, 0x47u };
    unsigned profile_index;

    memset(memory, 0xa5, sizeof(memory));
    BuildFixedAndGeneratedControllerRegisterProfiles(0x4321u);

    assert(initialize_count == 1u && generated_count == 1u);
    assert(normalized_count == 5u && scaled_count == 5u);
    assert(event_count == 12u && events[0] == 1u && events[1] == 2u);
    for(profile_index = 0u; profile_index < 5u; ++profile_index)
    {
        fw_addr_t record = 0x00e75cul + profile_index * 34u;
        unsigned index;

        assert(events[2u + profile_index * 2u] == (u8)(3u + profile_index * 2u));
        assert(events[3u + profile_index * 2u] == (u8)(4u + profile_index * 2u));
        assert(normalized_inputs[profile_index] == expected_values[profile_index]);
        assert(scaled_inputs[profile_index] == expected_values[profile_index]);
        assert(memory[record] == encoded_baselines[profile_index]);
        for(index = 0u; index < 18u; ++index)
            assert(memory[record + 1u + index] == (u8)(0x10u + profile_index * 0x20u + index));
        for(index = 0u; index < 9u; ++index)
            assert(memory[record + 19u + index] == (u8)(0x80u + profile_index * 0x10u + index));
        assert(memory[record + 28u] == (u8)(0x8du + profile_index * 0x10u));
        assert(memory[record + 29u] == (u8)(0x89u + profile_index * 0x10u));
        assert(memory[record + 30u] == (u8)(0x8au + profile_index * 0x10u));
        assert(memory[record + 31u] == (u8)(0x8bu + profile_index * 0x10u));
        assert(memory[record + 32u] == (u8)(0x8cu + profile_index * 0x10u));
        assert(memory[record + 33u] == 0xffu);
        assert(LoadWord(0x00e752ul + profile_index * 2u) == (u16)(0x9000u + profile_index));
    }
}

int main(void)
{
    CheckFixedProfiles();
    puts("controller_fixed_profile_generation_tests: ok");
    return 0;
}
