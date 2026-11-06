#include <assert.h>
#include <string.h>
#include "platform/controller_parameter_fc_calibration.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 trial_values[32];
static unsigned trial_index;
static unsigned trial_sample_index;
static unsigned measurement_calls;
static unsigned wait7000_calls;
static unsigned wait400_calls;
static unsigned wait200_calls;
static unsigned setup_calls;
static unsigned restore_calls;
static u8 fc_writes[64];
static unsigned fc_write_count;
static u8 timeout_on_400;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void WriteControllerParametersD4AndE2ForCode(u8 code)
{
    assert(code == 0x34u);
    ++setup_calls;
}
void WriteControllerParameterFCValue(u8 value)
{
    fc_writes[fc_write_count++] = value;
}
void WaitTimer1ScaledDuration(u16 duration, u8 yield)
{
    if(duration == 7000u)
    {
        assert(yield == 1u);
        ++wait7000_calls;
    }
    else if(duration == 400u)
    {
        assert(yield == 1u);
        ++wait400_calls;
        if(timeout_on_400 != 0u)
            memory[0x00120aul] = 0xffu;
    }
    else
    {
        assert(duration == 200u && yield == 0u);
        ++wait200_calls;
    }
}
u8 MeasureAveragedSfrD8MinusD9WithOffsets(void)
{
    u8 value = trial_values[trial_index];

    ++measurement_calls;
    if(++trial_sample_index == 8u)
    {
        trial_sample_index = 0u;
        ++trial_index;
    }
    return value;
}
void WriteControllerParametersD412AndE2Value58(void)
{
    ++restore_calls;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(trial_values, 0, sizeof(trial_values));
    memset(fc_writes, 0, sizeof(fc_writes));
    trial_index = trial_sample_index = measurement_calls = 0u;
    wait7000_calls = wait400_calls = wait200_calls = 0u;
    setup_calls = restore_calls = fc_write_count = 0u;
    timeout_on_400 = 0u;
}

static void TestModeZeroInclusiveAcceptance(void)
{
    Reset();
    trial_values[0] = 77u;
    assert(CalibrateControllerParameterFC() == 1u);
    assert(memory[0x00fc0ful] == 0xa0u && memory[0x00fac0ul] == 0xa0u);
    assert(fc_write_count == 1u && fc_writes[0] == 0xa0u);
    assert(measurement_calls == 8u && wait200_calls == 8u);
    assert(wait7000_calls == 1u && restore_calls == 1u);

    Reset();
    trial_values[0] = 85u;
    assert(CalibrateControllerParameterFC() == 1u);
}

static void TestHighMeasurementStepsDownAndConverges(void)
{
    Reset();
    trial_values[0] = 86u;
    trial_values[1] = 80u;
    assert(CalibrateControllerParameterFC() == 1u);
    assert(fc_write_count == 2u && fc_writes[0] == 0xa0u && fc_writes[1] == 0x9cu);
    assert(memory[0x00fac0ul] == 0x9cu);
    assert(wait400_calls == 1u && measurement_calls == 16u);
}

static void TestStateOnePublishesPrimaryAndSecondary(void)
{
    Reset();
    memory[0x001008ul] = 1u;
    trial_values[0] = 80u;
    assert(CalibrateControllerParameterFC() == 1u);
    assert(memory[0x00fac0ul] == 0xb8u);
    assert(memory[0x00fac1ul] == 0x78u);
}

static void TestStateOneRejectsCandidateB0(void)
{
    unsigned index;

    Reset();
    memory[0x001008ul] = 1u;
    for(index = 0u; index < 14u; ++index)
        trial_values[index] = 76u;
    trial_values[14] = 80u;
    assert(CalibrateControllerParameterFC() == 0u);
    assert(memory[0x00fc0ful] == 0xb0u);
    assert(restore_calls == 0u);
}

static void TestSignedTimeoutReturnsWithoutRestore(void)
{
    Reset();
    trial_values[0] = 86u;
    trial_values[1] = 86u;
    timeout_on_400 = 1u;
    assert(CalibrateControllerParameterFC() == 0u);
    assert(memory[0x00120aul] == 0xffu);
    assert(memory[0x00fc0ful] == 0x98u);
    assert(restore_calls == 0u);
}

static void TestOtherStateRestoresWithoutPublishingCalibration(void)
{
    Reset();
    memory[0x001008ul] = 2u;
    trial_values[0] = 80u;
    assert(CalibrateControllerParameterFC() == 1u);
    assert(memory[0x00fc0ful] == 0x78u);
    assert(memory[0x00fac0ul] == 0u && memory[0x00fac1ul] == 0u);
    assert(restore_calls == 1u);
}

int main(void)
{
    TestModeZeroInclusiveAcceptance();
    TestHighMeasurementStepsDownAndConverges();
    TestStateOnePublishesPrimaryAndSecondary();
    TestStateOneRejectsCandidateB0();
    TestSignedTimeoutReturnsWithoutRestore();
    TestOtherStateRestoresWithoutPublishingCalibration();
    return 0;
}
