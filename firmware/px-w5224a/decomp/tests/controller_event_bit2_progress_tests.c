#include <assert.h>
#include <string.h>
#include "platform/controller_event_bit2_progress.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 bit37;
static u8 bit33;
static u8 bit28;
static u8 bit2b;
static u8 bit43;
static unsigned advance_calls;
static unsigned service_calls;
static unsigned initialize_calls;
static unsigned lookup_calls;
static unsigned within_calls;
static unsigned finalize_calls;
static u8 within_argument;
static u8 within_result;
static u16 lookup_result;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveEventBit2ReadBit37(void)
{
    return bit37;
}

u8 ObserveEventBit2ReadBit33(void)
{
    return bit33;
}

u8 ObserveEventBit2ReadBit28(void)
{
    return bit28;
}

void ObserveEventBit2SetBit28(void)
{
    bit28 = 1u;
}

void ObserveEventBit2SetBit33(void)
{
    bit33 = 1u;
}

void ObserveEventBit2ClearBit2B(void)
{
    bit2b = 0u;
}

void ObserveEventBit2ClearBit43(void)
{
    bit43 = 0u;
}

void ObserveEventBit2AdvanceSetupPhase(void)
{
    advance_calls++;
}

void ObserveEventBit2ServiceSetupState(void)
{
    service_calls++;
}

void ObserveEventBit2InitializeMeasurementPhase(void)
{
    initialize_calls++;
}

u16 ObserveEventBit2LookupAdjustment(void)
{
    lookup_calls++;
    return lookup_result;
}

u8 ObserveEventBit2WithinOperatingWindow(u8 use_wide_window)
{
    within_calls++;
    within_argument = use_wide_window;
    return within_result;
}

void ObserveEventBit2FinalizeOperatingUpdate(void)
{
    finalize_calls++;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    bit37 = 1u;
    bit33 = 0u;
    bit28 = 0u;
    bit2b = 1u;
    bit43 = 1u;
    advance_calls = 0u;
    service_calls = 0u;
    initialize_calls = 0u;
    lookup_calls = 0u;
    within_calls = 0u;
    finalize_calls = 0u;
    within_argument = 0u;
    within_result = 1u;
    lookup_result = 0u;
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

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void TestSetupRoutes(void)
{
    ResetState();
    memory[0x001308u] = 1u;
    memory[0x00130au] = 8u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(advance_calls == 1u && service_calls == 0u);

    ResetState();
    bit37 = 0u;
    memory[0x000e41u] = 1u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(initialize_calls == 1u && service_calls == 0u);

    ResetState();
    bit37 = 0u;
    memory[0x018032u] = 0x20u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(service_calls == 1u);
    assert(memory[0x00150au] == 0x20u);
    assert(memory[0x00154du] == 1u);
    assert(memory[0x000efbu] == 2u);
    assert(memory[0x018004u] == 2u);
    assert(memory[0x018035u] == 0u);
    assert(memory[0x00fba1u] == 15u);
}

static void AssertTransition(u8 from, u8 to)
{
    ResetState();
    memory[0x00fbd6u] = from;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == to);
    assert(service_calls == 1u);
}

static void TestSimpleSubstateTransitions(void)
{
    AssertTransition(1u, 2u);

    ResetState();
    bit33 = 1u;
    memory[0x00fbd6u] = 1u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == 4u);

    AssertTransition(2u, 3u);
    AssertTransition(4u, 5u);
    AssertTransition(5u, 6u);
    AssertTransition(6u, 7u);
    AssertTransition(11u, 12u);
}

static void TestConfiguredSubstates(void)
{
    ResetState();
    memory[0x00fbd6u] = 3u;
    memory[0x000fcau] = 0xffu;
    memory[0x001157u] = 2u;
    memory[0x001008u] = 2u;
    memory[0x000c46u] = 1u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(bit28 == 1u && memory[0x00fbd6u] == 0u);
    assert(memory[0x0180c0u] == 0u);
    assert(memory[0x0180cfu] == 4u);
    assert(memory[0x0180c3u] == 0x60u);
    assert(memory[0x000fcau] == 0xe7u && memory[0x018002u] == 0xe7u);

    ResetState();
    memory[0x00fbd6u] = 7u;
    memory[0x000fcau] = 0xffu;
    memory[0x001157u] = 2u;
    memory[0x001158u] = 5u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(bit28 == 1u && memory[0x00fbd6u] == 8u);
    assert(memory[0x0180c0u] == 0x20u);
    assert(memory[0x0180cfu] == 0xc4u);
    assert(memory[0x0180c3u] == 0xe0u);
    assert(memory[0x0180c6u] == 0x20u);

    ResetState();
    memory[0x00fbd6u] = 8u;
    memory[0x000fcau] = 0xffu;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == 0u);
    assert(memory[0x0180c0u] == 0u);
    assert(memory[0x0180cfu] == 0xc0u);
    assert(memory[0x0180c3u] == 0xe0u);
    assert(memory[0x0180c6u] == 0x23u);

    ResetState();
    memory[0x00fbd6u] = 10u;
    memory[0x001157u] = 2u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == 0u);
    assert(memory[0x0180c0u] == 0u);
    assert(memory[0x0180cfu] == 0xc4u);
    assert(memory[0x0180c3u] == 0x60u);

    ResetState();
    memory[0x00fbd6u] = 12u;
    memory[0x001157u] = 2u;
    memory[0x001158u] = 5u;
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == 13u);
    assert(memory[0x0180c0u] == 0u);
    assert(memory[0x0180cfu] == 4u);
    assert(memory[0x0180c3u] == 0xa0u);
    assert(memory[0x0180c6u] == 0x20u);
}

static void TestSubstate13QueueCompletion(void)
{
    ResetState();
    memory[0x00fbd6u] = 13u;
    WriteU16(0x001292u, 10u);
    WriteU16(0x001294u, 2u);
    WriteU16(0x00129au, 7u);
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(memory[0x00fbd6u] == 0u && bit33 == 1u);
    assert(ReadU16(0x001292u) == 15u);
    assert(ReadU16(0x001294u) == 5u);
    assert(finalize_calls == 1u);
}

static void TestAdjustmentClamp(void)
{
    ResetState();
    memory[0x00fb9bu] = 2u;
    bit28 = 1u;
    memory[0x001008u] = 2u;
    lookup_result = 100u;
    WriteU16(0x00ef81u, 50u);
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(lookup_calls == 1u);
    assert(ReadU16(0x00fbc3u) == 0x7eu);

    ResetState();
    memory[0x00fb9bu] = 2u;
    bit28 = 1u;
    memory[0x001008u] = 2u;
    lookup_result = 10u;
    WriteU16(0x00ef81u, 20u);
    HandleControllerEventBit2ReadMeasurementProgress();
    assert(ReadU16(0x00fbc3u) == 30u);
}

static void TestOutOfWindowShutdown(void)
{
    ResetState();
    bit37 = 0u;
    bit33 = 1u;
    within_result = 0u;
    WriteU32(0x001235u, 100ul);
    memory[0x000fadu] = 0xffu;
    memory[0x000efeu] = 0xffu;
    memory[0x000effu] = 0xffu;
    memory[0x000efcu] = 0xa5u;
    memory[0x018005u] = 0xa5u;

    HandleControllerEventBit2ReadMeasurementProgress();

    assert(service_calls == 1u);
    assert(within_calls == 1u && within_argument == 1u);
    assert(bit2b == 0u && bit43 == 0u);
    assert(memory[0x000efcu] == 0u && memory[0x018005u] == 0u);
    assert(ReadU32(0x0014deu) == 98ul);
    assert(memory[0x000fadu] == 0xefu);
    assert(memory[0x000efeu] == 0x1fu);
    assert(memory[0x000effu] == 0xefu);
    assert(memory[0x00fb9fu] == 2u);
    assert(memory[0x018003u] == 0xefu);
    assert(memory[0x018018u] == 0x1fu);
    assert(memory[0x018019u] == 0xefu);
    assert(finalize_calls == 1u);
}

int main(void)
{
    TestSetupRoutes();
    TestSimpleSubstateTransitions();
    TestConfiguredSubstates();
    TestSubstate13QueueCompletion();
    TestAdjustmentClamp();
    TestOutOfWindowShutdown();
    return 0;
}
