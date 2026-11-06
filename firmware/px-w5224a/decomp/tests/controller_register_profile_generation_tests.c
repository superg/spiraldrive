#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_register_profile_generation.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static float coordinates[100];
static float scaled_inputs[100];
static float normalized_inputs[50];
static unsigned initialize_count;
static unsigned coordinate_count;
static unsigned scaled_count;
static unsigned normalized_count;

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

void InitializeGeneratedControllerProfileStorage(void)
{
    ++initialize_count;
}

float ComputeRadialModelProfileCoordinate(u8 profile_index, u16 radial_model_endpoint)
{
    assert(radial_model_endpoint == 0x4321u);
    assert(profile_index == coordinate_count);
    return coordinates[coordinate_count++];
}

s8 GenerateAndNormalizeControllerProfileSamples(float coordinate)
{
    unsigned index;

    assert(normalized_count < 50u);
    normalized_inputs[normalized_count] = coordinate;
    for(index = 0u; index < 18u; ++index)
    {
        memory[0x00f9b7ul + index] = (u8)(0x10u + normalized_count + index);
        memory[0x00f9cbul + index] = (u8)(0x80u + normalized_count + index);
    }
    return normalized_count++ == 0u ? -3 : 4;
}

u16 ComputeControllerProfileScaledWord(float coordinate)
{
    scaled_inputs[scaled_count] = coordinate;
    return (u16)(0x8000u + scaled_count++);
}

static u16 LoadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1ul]);
}

static void Reset(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(coordinates, 0, sizeof(coordinates));
    memset(scaled_inputs, 0, sizeof(scaled_inputs));
    memset(normalized_inputs, 0, sizeof(normalized_inputs));
    initialize_count = 0u;
    coordinate_count = 0u;
    scaled_count = 0u;
    normalized_count = 0u;
}

static void CheckDistinctProfileRecords(void)
{
    unsigned index;
    fw_addr_t first = 0x00e806ul;
    fw_addr_t second = first + 34u;

    Reset();
    for(index = 0u; index < 100u; ++index)
        coordinates[index] = index == 0u ? 1.9f : index == 1u ? 1.1f : 2.9f;
    GenerateControllerRegisterProfiles(0x4321u);

    assert(initialize_count == 1u);
    assert(coordinate_count == 100u && scaled_count == 100u);
    assert(normalized_count == 2u);
    assert(memory[0x00fa02ul] == 1u);
    assert(memory[first] == 0x43u && memory[second] == 4u);
    assert(memory[first + 33u] == 0u && memory[second + 33u] == 2u);
    for(index = 0u; index < 18u; ++index)
    {
        assert(memory[first + 1u + index] == (u8)(0x10u + index));
        assert(memory[second + 1u + index] == (u8)(0x11u + index));
    }
    for(index = 0u; index < 9u; ++index)
        assert(memory[first + 19u + index] == (u8)(0x80u + index));
    assert(memory[first + 28u] == 0x8du);
    assert(memory[first + 29u] == 0x89u);
    assert(memory[first + 30u] == 0x8au);
    assert(memory[first + 31u] == 0x8bu);
    assert(memory[first + 32u] == 0x8cu);
    for(index = 0u; index < 100u; ++index)
    {
        assert(scaled_inputs[index] == coordinates[index]);
        assert(LoadWord(0x00eeaaul + index * 2u) == (u16)(0x8000u + index));
    }
    assert(normalized_inputs[0] == 1.0f && normalized_inputs[1] == 2.0f);
}

static void CheckFiftyRecordLimit(void)
{
    unsigned index;

    Reset();
    for(index = 0u; index < 100u; ++index)
        coordinates[index] = (float)index;
    GenerateControllerRegisterProfiles(0x4321u);
    assert(normalized_count == 50u);
    assert(memory[0x00e806ul + 49u * 34u + 33u] == 49u);
    assert(scaled_count == 100u);
}

int main(void)
{
    CheckDistinctProfileRecords();
    CheckFiftyRecordLimit();
    puts("controller_register_profile_generation_tests: ok");
    return 0;
}
