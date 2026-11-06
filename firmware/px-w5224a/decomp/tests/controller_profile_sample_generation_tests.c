#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_profile_sample_generation.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static s8 interpolated[36];
static s8 auxiliary[5];
static u8 interpolation_indices[36];
static u8 auxiliary_indices[5];
static float observed_values[41];
static unsigned interpolation_count;
static unsigned auxiliary_count;

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

s8 ObserveInterpolateControllerProfileSample(float profile_value, u8 sample_index)
{
    assert(interpolation_count < 36u);
    interpolation_indices[interpolation_count] = sample_index;
    observed_values[interpolation_count] = profile_value;
    ++interpolation_count;
    return interpolated[sample_index];
}

s8 ObserveLookupControllerProfileAuxiliaryValue(float profile_value, u8 auxiliary_index)
{
    assert(auxiliary_count < 5u);
    auxiliary_indices[auxiliary_count] = auxiliary_index;
    observed_values[36u + auxiliary_count] = profile_value;
    ++auxiliary_count;
    return auxiliary[auxiliary_index];
}

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    memset(interpolated, 0, sizeof(interpolated));
    memset(auxiliary, 0, sizeof(auxiliary));
    memset(interpolation_indices, 0, sizeof(interpolation_indices));
    memset(auxiliary_indices, 0, sizeof(auxiliary_indices));
    memset(observed_values, 0, sizeof(observed_values));
    interpolation_count = auxiliary_count = 0u;
    for(index = 0u; index != 5u; ++index)
        auxiliary[index] = (s8)(0x31u + index);
}

static s8 BuildExpected(u8 expected[36])
{
    s8 baseline = 0x7f;
    s8 difference;
    s8 value;
    u8 index;

    for(index = 0u; index != 18u; ++index)
    {
        expected[index] = (u8)interpolated[index];
        expected[20u + index] = (u8)interpolated[18u + index];
    }
    for(index = 0u; index != 9u; ++index)
    {
        value = (s8)expected[index];
        if(value < baseline)
            baseline = value;
    }
    for(index = 0u; index != 9u; ++index)
    {
        difference = (s8)(u8)((u8)baseline - expected[29u + index]);
        if(difference > 15)
            baseline = -15;
    }
    if(baseline < -15)
        baseline = -15;
    for(index = 0u; index != 9u; ++index)
    {
        value = (s8)(u8)(expected[index] - (u8)baseline);
        if(value < 0)
            value = 0;
        expected[index] = (u8)value;
    }
    for(index = 0u; index != 9u; ++index)
    {
        difference = (s8)(u8)(expected[29u + index] - (u8)baseline);
        if(difference < -15)
            difference = -15;
        expected[20u + index] = (u8)((expected[20u + index] << 4) | ((u8)(0u - (u8)difference) & 0x0fu));
    }
    memset(expected + 29u, 0, 9u);
    for(index = 0u; index != 5u; ++index)
        expected[29u + index] = (u8)auxiliary[index];
    return baseline;
}

static void RunAndCheck(float profile_value)
{
    u8 expected[38];
    s8 expected_baseline;
    s8 actual_baseline;
    unsigned index;

    memset(expected, 0xa5, sizeof(expected));
    expected_baseline = BuildExpected(expected);
    actual_baseline = GenerateAndNormalizeControllerProfileSamples(profile_value);

    assert(actual_baseline == expected_baseline);
    assert(memcmp(&memory[0x00f9b7ul], expected, sizeof(expected)) == 0);
    assert(interpolation_count == 36u && auxiliary_count == 5u);
    for(index = 0u; index != 18u; ++index)
    {
        assert(interpolation_indices[index * 2u] == index);
        assert(interpolation_indices[index * 2u + 1u] == index + 18u);
    }
    for(index = 0u; index != 5u; ++index)
        assert(auxiliary_indices[index] == index);
    for(index = 0u; index != 41u; ++index)
        assert(observed_values[index] == profile_value);
}

static void TestOrdinaryNormalization(void)
{
    unsigned index;

    ResetFixture();
    for(index = 0u; index != 36u; ++index)
        interpolated[index] = (s8)(index - 4u);
    RunAndCheck(23.5f);
}

static void TestNegativeClampAndPackedLimits(void)
{
    unsigned index;

    ResetFixture();
    for(index = 0u; index != 18u; ++index)
        interpolated[index] = (s8)(index == 3u ? -24 : index + 2u);
    for(index = 18u; index != 36u; ++index)
        interpolated[index] = (s8)(index < 27u ? index - 20u : -40);
    RunAndCheck(-7.25f);
}

int main(void)
{
    TestOrdinaryNormalization();
    TestNegativeClampAndPackedLimits();
    puts("controller_profile_sample_generation_tests: ok");
    return 0;
}
