#include <assert.h>
#include <string.h>
#include "platform/controller_measurement_transition.h"
#include "platform/firmware_memory.h"

enum
{
    EVENT_P1 = 1,
    EVENT_PORT,
    EVENT_DIVIDE,
    EVENT_REFERENCE,
    EVENT_SUBMIT,
    EVENT_PROFILE
};

static u8 memory[1u << 24];
static u8 events[64];
static unsigned event_count, p1_count, port_count, submit_count;
static unsigned divide_count, reference_count, profile_count;
static fw_addr_t port_addresses[4];
static u8 port_values[4];
static u16 submit_values[4], submit_selectors[4];
static u32 divide_dividend, divide_divisor, divide_result;
static u32 reference_position, reference_result;
static u16 reference_scale;
static u8 reference_code;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void ObserveSetControllerTransitionP1Bits(void)
{
    events[event_count++] = EVENT_P1;
    ++p1_count;
}

void WaitControllerPortReadyAndWriteByte(fw_addr_t address, u8 value)
{
    events[event_count++] = EVENT_PORT;
    port_addresses[port_count] = address;
    port_values[port_count++] = value;
}

u32 DivideU32QuotientNative(u32 dividend, u32 divisor)
{
    events[event_count++] = EVENT_DIVIDE;
    ++divide_count;
    divide_dividend = dividend;
    divide_divisor = divisor;
    return divide_result;
}

u32 ComputeControllerMeasurementReferenceForOperatingCode(u32 position, u16 scale, u8 code)
{
    events[event_count++] = EVENT_REFERENCE;
    ++reference_count;
    reference_position = position;
    reference_scale = scale;
    reference_code = code;
    return reference_result;
}

void SubmitControllerWordPair(u16 value, u16 selector)
{
    events[event_count++] = EVENT_SUBMIT;
    submit_values[submit_count] = value;
    submit_selectors[submit_count++] = selector;
}

void UpdateControllerProfileFromReadAcquisitionTarget(void)
{
    events[event_count++] = EVENT_PROFILE;
    ++profile_count;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(port_addresses, 0, sizeof(port_addresses));
    memset(port_values, 0, sizeof(port_values));
    memset(submit_values, 0, sizeof(submit_values));
    memset(submit_selectors, 0, sizeof(submit_selectors));
    event_count = p1_count = port_count = submit_count = 0u;
    divide_count = reference_count = profile_count = 0u;
    divide_dividend = divide_divisor = 0ul;
    divide_result = 100ul;
    reference_position = reference_result = 0ul;
    reference_result = 0x1234ul;
    reference_scale = 0u;
    reference_code = 0u;
    memory[0x000e43ul] = 8u;
    memory[0x000e4dul] = 8u;
    memory[0x000e4ful] = 4u;
    memory[0x00f98ful] = 0x5du;
    WriteU16(0x00fc25ul, 4u);
    WriteU32(0x000cf5ul, 6500ul);
    WriteU32(0x00125eul, 12345ul);
}

static void AssertCommonReference(u32 position, u16 reference)
{
    assert(divide_count == 1u);
    assert(divide_dividend == 6500ul && divide_divisor == 65ul);
    assert(reference_count == 1u);
    assert(reference_position == position);
    assert(reference_scale == 100u && reference_code == memory[0x000e4ful]);
    assert(ReadU16(0x00fc27ul) == reference);
    assert(submit_count == 2u);
    assert(submit_values[0] == reference && submit_selectors[0] == 0x035fu);
    assert(submit_selectors[1] == 0x0369u);
    assert(port_addresses[port_count - 1u] == 0x0180a6ul);
    assert(port_values[port_count - 1u] == 0xfau);
}

static void TestOrdinaryReferenceAndTransitionWord(void)
{
    ResetState();
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    AssertCommonReference(12345ul, 0x1234u);
    assert(p1_count == 1u && port_count == 1u);
    assert(submit_values[1] == 0x1fffu);
    assert(ReadU16(0x00fc25ul) == 4u);
    assert(profile_count == 0u);
}

static void TestState4SpecialPositionAndScaledReference(void)
{
    ResetState();
    memory[0x001300ul] = 4u;
    memory[0x000e43ul] = 0x10u;
    memory[0x001006ul] = 1u;
    memory[0x000ed7ul] = 1u;
    WriteU32(0x000e3bul, 999ul);
    WriteU16(0x00fc25ul, 2u);
    reference_result = 1000ul;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    AssertCommonReference(150ul, 1100u);
    assert(p1_count == 2u);
    assert(submit_values[1] == 0x3fffu);

    ResetState();
    memory[0x001300ul] = 4u;
    memory[0x000ed7ul] = 1u;
    memory[0x000e3ful] = 1u;
    WriteU32(0x000e3bul, 0x10000000ul);
    WriteU32(0x000ee9ul, 777ul);
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    AssertCommonReference(777ul, 0x1234u);
}

static void TestState2TransitionOverrideAndOrder(void)
{
    static const u8 expected_events[] = { EVENT_P1, EVENT_PORT, EVENT_P1, EVENT_P1, EVENT_DIVIDE, EVENT_REFERENCE, EVENT_SUBMIT, EVENT_SUBMIT, EVENT_PORT };

    ResetState();
    memory[0x001300ul] = 2u;
    memory[0x000e43ul] = 1u;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    AssertCommonReference(12345ul, 0x1234u);
    assert(p1_count == 3u && port_count == 2u);
    assert(port_addresses[0] == 0x0180a8ul && port_values[0] == 0u);
    assert(submit_values[1] == 0x7fffu);
    assert(event_count == sizeof(expected_events));
    assert(memcmp(events, expected_events, sizeof(expected_events)) == 0);
}

static void TestAppliedCodeStrobesAndFlagClear(void)
{
    ResetState();
    memory[0x000e4dul] = 9u;
    memory[0x001306ul] = 0x20u;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    assert(p1_count == 3u);
    assert(memory[0x001306ul] == 0x20u);

    ResetState();
    memory[0x000e4dul] = 9u;
    memory[0x001306ul] = 0x30u;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    assert(p1_count == 2u);
    assert(memory[0x001306ul] == 0x20u);
}

static void TestForcedTransitionWordsAndProfileRefresh(void)
{
    ResetState();
    memory[0x000e4ful] = 0x18u;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    assert(submit_values[1] == 0x7fffu);

    ResetState();
    memory[0x001006ul] = 1u;
    memory[0x000e4ful] = 0xffu;
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    assert(submit_values[1] == 0x7fffu);

    ResetState();
    memory[0x000e43ul] = 0x18u;
    WriteU16(0x00fc25ul, 0xffffu);
    ApplyControllerMeasurementReferenceAndTransitionProfile();
    assert(ReadU16(0x00fc25ul) == 1u);
    assert(submit_values[1] == 0x7fffu);
    assert(profile_count == 1u);
}

int main(void)
{
    TestOrdinaryReferenceAndTransitionWord();
    TestState4SpecialPositionAndScaledReference();
    TestState2TransitionOverrideAndOrder();
    TestAppliedCodeStrobesAndFlagClear();
    TestForcedTransitionWordsAndProfileRefresh();
    return 0;
}
