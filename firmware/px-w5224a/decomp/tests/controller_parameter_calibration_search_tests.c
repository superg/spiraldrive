#include <assert.h>
#include <string.h>
#include "platform/controller_parameter_calibration_search.h"
#include "platform/firmware_memory.h"

typedef struct
{
    u8 code;
    u8 value;
} CalibrationWrite;

static u8 memory[0x1000000ul];
static u16 measurements[16];
static unsigned measurement_index;
static CalibrationWrite calibration_writes[32];
static unsigned calibration_write_count;
static u16 parameter_words[32];
static unsigned parameter_word_count;
static u8 e2_values[8];
static unsigned e2_value_count;
static u8 interrupt_events[32];
static unsigned interrupt_event_count;
static unsigned initial_delay_count;
static unsigned loop_delay_count;
static unsigned strobe_count;
static fw_addr_t last_strobe_port;
static u8 last_strobe_value;
static u8 computed_ffe0_value;
static u8 force_timeout_on_loop_delay;
static unsigned select_f6_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

void WaitTimer1ScaledDuration(u16 duration, u8 yield)
{
    assert(yield == 1u);
    if(duration == 2000u)
        ++initial_delay_count;
    else
    {
        assert(duration == 400u);
        ++loop_delay_count;
        if(force_timeout_on_loop_delay != 0u)
            memory[0x001220ul] = 0xffu;
    }
}

void ObserveControllerParameterCalibrationInterrupts(u8 enabled)
{
    interrupt_events[interrupt_event_count++] = enabled;
}

u16 MeasureAverageSfrDbSamples(u8 count)
{
    assert(count == 8u);
    return measurements[measurement_index++];
}

void StrobeControllerPortWriteByteFromC(fw_addr_t port, u8 value)
{
    ++strobe_count;
    last_strobe_port = port;
    last_strobe_value = value;
}

void WriteControllerRegister01Clamped(u8 value)
{
    assert(value == 0x0cu);
}
void WriteCalibratedControllerParameterForCode(u8 code, u8 value)
{
    calibration_writes[calibration_write_count].code = code;
    calibration_writes[calibration_write_count].value = value;
    ++calibration_write_count;
}
void WriteControllerParameterE2ValueAndRefreshEE(u8 value)
{
    e2_values[e2_value_count++] = value;
}
void WriteControllerParameterWord(u16 value)
{
    parameter_words[parameter_word_count++] = value;
}
void WriteControllerParameterF6ValueWithBits0And1Set(u8 value)
{
    assert(value == 0xf8u);
}
void SelectAndWriteControllerParameterF6ForOperatingCode(u8 code)
{
    assert(code == 1u);
    ++select_f6_count;
}
u8 ComputeControllerParametersFFE0Value(void)
{
    return computed_ffe0_value;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(measurements, 0, sizeof(measurements));
    memset(calibration_writes, 0, sizeof(calibration_writes));
    memset(parameter_words, 0, sizeof(parameter_words));
    memset(e2_values, 0, sizeof(e2_values));
    memset(interrupt_events, 0, sizeof(interrupt_events));
    measurement_index = calibration_write_count = parameter_word_count = 0u;
    e2_value_count = interrupt_event_count = 0u;
    initial_delay_count = loop_delay_count = strobe_count = 0u;
    last_strobe_port = 0ul;
    last_strobe_value = computed_ffe0_value = 0u;
    force_timeout_on_loop_delay = 0u;
    select_f6_count = 0u;
    WriteU16(0x00fc05ul, 100u);
}

static void AssertOneMeasurementBracket(void)
{
    assert(initial_delay_count == 1u);
    assert(interrupt_event_count >= 2u);
    assert(interrupt_events[0] == 0u && interrupt_events[1] == 1u);
}

static void TestCode04ImmediateConvergence(void)
{
    Reset();
    measurements[0] = 102u;
    assert(CalibrateControllerParameterForCode(0x04u) == 1u);
    assert(memory[0x00fc0aul] == 0u);
    assert(calibration_write_count == 1u);
    assert(e2_value_count == 2u && e2_values[0] == 0x04u && e2_values[1] == 0x58u);
    AssertOneMeasurementBracket();
}

static void TestCode00InitializesAndRestoresControllerPorts(void)
{
    Reset();
    memory[0x00131cul] = 0x55u;
    memory[0x000efbul] = 0x12u;
    memory[0x000efcul] = 0x34u;
    measurements[0] = 100u;
    assert(CalibrateControllerParameterForCode(0x00u) == 1u);
    assert(strobe_count == 2u && last_strobe_port == 0x018080ul && last_strobe_value == 0x55u);
    assert(memory[0x00fc09ul] == 0x20u);
    assert(parameter_words[0] == 0xf220u);
    assert(memory[0x018001ul] == 4u && memory[0x018004ul] == 0x12u && memory[0x018005ul] == 0x34u && memory[0x018032ul] == 0x80u);
}

static void TestCode24SearchCommitsStateTwoValue(void)
{
    Reset();
    memory[0x001008ul] = 2u;
    measurements[0] = 120u;
    measurements[1] = 101u;
    measurements[2] = 101u;
    assert(CalibrateControllerParameterForCode(0x24u) == 1u);
    assert(calibration_write_count == 3u);
    assert(calibration_writes[0].value == 0u && calibration_writes[1].value == 2u && calibration_writes[2].value == 4u);
    assert(memory[0x00fc0eul] == 4u && memory[0x00fc0dul] == 0u);
    assert(parameter_words[0] == 0xfb81u);
    assert(loop_delay_count == 2u);
}

static void TestCode2CConfiguresAndCommitsF6(void)
{
    Reset();
    measurements[0] = 100u;
    assert(CalibrateControllerParameterForCode(0x2cu) == 1u);
    assert(select_f6_count == 1u && memory[0x00fc0bul] == 0u);
}

static void TestCode3CCommitsPairedParameters(void)
{
    Reset();
    measurements[0] = 100u;
    computed_ffe0_value = 0xabu;
    assert(CalibrateControllerParameterForCode(0x3cu) == 1u);
    assert(parameter_word_count == 4u);
    assert(parameter_words[0] == 0xffe0u && parameter_words[1] == 0xe0e0u);
    assert(parameter_words[2] == 0xffabu && parameter_words[3] == 0xe0abu);
    assert(memory[0x00fc10ul] == 0u);
}

static void TestSignedTimeoutReturnsFailure(void)
{
    Reset();
    measurements[0] = 120u;
    measurements[1] = 120u;
    force_timeout_on_loop_delay = 1u;
    assert(CalibrateControllerParameterForCode(0x1cu) == 0u);
    assert(memory[0x001220ul] == 0xffu);
    assert(memory[0x00fc0cul] == 4u);
    assert(loop_delay_count == 1u);
}

int main(void)
{
    TestCode04ImmediateConvergence();
    TestCode00InitializesAndRestoresControllerPorts();
    TestCode24SearchCommitsStateTwoValue();
    TestCode2CConfiguresAndCommitsF6();
    TestCode3CCommitsPairedParameters();
    TestSignedTimeoutReturnsFailure();
    return 0;
}
