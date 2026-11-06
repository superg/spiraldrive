#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/set_cd_speed.h"

enum Event
{
    EVENT_CLEAR_BIT_000E = 1,
    EVENT_RESET_QUEUE,
    EVENT_INITIALIZE_COROUTINE2,
    EVENT_DISPATCH_REQUEST5,
    EVENT_YIELD
};

static u8 memory[1u << 24];
static u8 validate_result;
static fw_addr_t validate_mask;
static unsigned validate_calls;
static unsigned unsupported_calls;
static u8 bit_0008;
static unsigned reset_progress_calls;
static unsigned no_sense_calls;
static unsigned events[8];
static unsigned event_count;
static u32 coroutine_entry;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveSetCdSpeedValidate(fw_addr_t mask_descriptor)
{
    ++validate_calls;
    validate_mask = mask_descriptor;
    return validate_result;
}

void ObserveSetCdSpeedUnsupported(void)
{
    ++unsupported_calls;
}

u8 ObserveSetCdSpeedBit0008(void)
{
    return bit_0008;
}

void ObserveSetCdSpeedClearBit000E(void)
{
    events[event_count++] = EVENT_CLEAR_BIT_000E;
}

void ObserveSetCdSpeedResetProgress(void)
{
    ++reset_progress_calls;
}

void ObserveSetCdSpeedResetQueue(void)
{
    events[event_count++] = EVENT_RESET_QUEUE;
}

void ObserveSetCdSpeedInitializeCoroutine2(u32 entry_point)
{
    events[event_count++] = EVENT_INITIALIZE_COROUTINE2;
    coroutine_entry = entry_point;
}

void ObserveSetCdSpeedDispatchRequest5(void)
{
    events[event_count++] = EVENT_DISPATCH_REQUEST5;
}

void ObserveSetCdSpeedYield(void)
{
    events[event_count++] = EVENT_YIELD;
}

void ObserveSetCdSpeedNoSense(void)
{
    ++no_sense_calls;
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadBe16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    validate_result = 1u;
    validate_mask = 0u;
    validate_calls = 0u;
    unsupported_calls = 0u;
    bit_0008 = 0u;
    reset_progress_calls = 0u;
    no_sense_calls = 0u;
    event_count = 0u;
    coroutine_entry = 0u;
    memory[0x00fc1fu] = 0xffu;
}

static void SetRequestedSpeeds(u16 read_kbps, u16 write_kbps)
{
    WriteBe16(0x000b0cu, read_kbps);
    WriteBe16(0x000b0eu, write_kbps);
}

static void TestValidationAndDisabledRoute(void)
{
    ResetState();
    validate_result = 0u;
    HandleSetCdSpeedCommand();
    assert(validate_calls == 1u && validate_mask == 0x00ff8e32ul);
    assert(unsupported_calls == 1u && no_sense_calls == 0u);

    ResetState();
    SetRequestedSpeeds(0xffffu, 0xffffu);
    memory[0x000c88u] = 0x40u;
    memory[0x000c37u] = 7u;
    HandleSetCdSpeedCommand();
    assert(unsupported_calls == 0u && no_sense_calls == 1u);
    assert(memory[0x000c37u] == 7u && memory[0x000c7du] == 0u);
    assert(reset_progress_calls == 0u && event_count == 0u);
}

static void TestReadSpeedThresholdsAndCap(void)
{
    struct Case
    {
        u16 kbps;
        u8 flags_c86;
        u8 flags_c88;
        u8 flags_faed;
        u8 expected;
    } cases[] = {
        { 0x02bcu, 0x40u, 0u, 0u, 1u  },
        { 0x02bcu, 0u,    0u, 0u, 4u  },
        { 0x057du, 0u,    0u, 0u, 4u  },
        { 0x057eu, 0u,    0u, 0u, 8u  },
        { 0x06dfu, 0x40u, 0u, 0u, 10u },
        { 0x06dfu, 0u,    0u, 0u, 8u  },
        { 0x1085u, 0u,    0u, 0u, 24u },
        { 0x1608u, 0u,    0u, 0u, 32u },
        { 0x1b8bu, 0u,    0u, 0u, 40u },
        { 0x210eu, 0u,    0u, 0u, 40u },
        { 0x210eu, 0u,    4u, 0u, 52u },
        { 0x210eu, 0u,    0u, 4u, 52u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetState();
        SetRequestedSpeeds(cases[index].kbps, 0u);
        memory[0x000c86u] = cases[index].flags_c86;
        memory[0x000c88u] = cases[index].flags_c88;
        memory[0x00faedu] = cases[index].flags_faed;
        HandleSetCdSpeedCommand();
        assert(memory[0x00fc20u] == cases[index].expected);
        assert(memory[0x000c37u] == cases[index].expected);
        assert(no_sense_calls == 1u);
    }

    ResetState();
    SetRequestedSpeeds(0xffffu, 0u);
    memory[0x000c88u] = 4u;
    memory[0x00fc1fu] = 24u;
    HandleSetCdSpeedCommand();
    assert(memory[0x00fc20u] == 52u);
    assert(memory[0x000c37u] == 24u);
}

static void TestWriteSpeedThresholds(void)
{
    struct Case
    {
        u16 kbps;
        u8 command_mode;
        u8 expected;
    } cases[] = {
        { 0x057du, 0u, 4u  },
        { 0x057eu, 0u, 8u  },
        { 0x0b01u, 0u, 16u },
        { 0x0dc3u, 0u, 20u },
        { 0x1085u, 0u, 24u },
        { 0x1608u, 0u, 32u },
        { 0x1b8bu, 0u, 40u },
        { 0x210eu, 0u, 40u },
        { 0x210eu, 6u, 52u },
        { 0x23d0u, 0u, 52u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetState();
        SetRequestedSpeeds(0u, cases[index].kbps);
        memory[0x000c7fu] = cases[index].command_mode;
        HandleSetCdSpeedCommand();
        assert(memory[0x000c7eu] == cases[index].expected);
        assert(memory[0x000c7du] == cases[index].expected);
        assert(memory[0x000c38u] == cases[index].expected);
    }

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x00fb88u] = 2u;
    SetRequestedSpeeds(0u, 0u);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c7du] == 24u);

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x00fb87u] = 0x40u;
    SetRequestedSpeeds(0u, 0x06deu);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c7du] == 4u);

    ResetState();
    memory[0x001008u] = 2u;
    memory[0x00fb87u] = 0x40u;
    SetRequestedSpeeds(0u, 0x06dfu);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c7du] == 10u);

    ResetState();
    memory[0x001008u] = 2u;
    SetRequestedSpeeds(0u, 0xffffu);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c7du] == 4u);
}

static void TestSavedWriteSpeedReconciliation(void)
{
    ResetState();
    SetRequestedSpeeds(0u, 0x1085u);
    memory[0x000c38u] = 32u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000c38u] == 24u);

    ResetState();
    SetRequestedSpeeds(0u, 0x1085u);
    memory[0x000c38u] = 32u;
    WriteBe16(0x00fad7u, 4u);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c38u] == 24u && ReadBe16(0x00fad7u) == 0u);

    ResetState();
    SetRequestedSpeeds(0u, 0x1608u);
    memory[0x000c38u] = 24u;
    WriteBe16(0x00fad7u, 4u);
    HandleSetCdSpeedCommand();
    assert(memory[0x000c38u] == 24u && memory[0x00fad3u] == 32u);
    assert(ReadBe16(0x00fad7u) == 4u);
}

static void TestProgressResetAndActiveRestart(void)
{
    ResetState();
    bit_0008 = 1u;
    HandleSetCdSpeedCommand();
    assert(reset_progress_calls == 1u);

    ResetState();
    memory[0x000f46u] = 1u;
    HandleSetCdSpeedCommand();
    assert(reset_progress_calls == 1u);

    ResetState();
    SetRequestedSpeeds(0x1085u, 0x1085u);
    memory[0x000c85u] = 2u;
    memory[0x000e46u] = 8u;
    memory[0x001037u] = 4u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000e46u] == 24u && memory[0x000e49u] == 24u);
    assert(memory[0x000e47u] == 24u);
    assert(event_count == 5u);
    assert(events[0] == EVENT_CLEAR_BIT_000E);
    assert(events[1] == EVENT_RESET_QUEUE);
    assert(events[2] == EVENT_INITIALIZE_COROUTINE2);
    assert(events[3] == EVENT_DISPATCH_REQUEST5);
    assert(events[4] == EVENT_YIELD);
    assert(coroutine_entry == 0x00f84a00ul);

    ResetState();
    SetRequestedSpeeds(0x1085u, 0u);
    memory[0x000c85u] = 2u;
    memory[0x000e46u] = 8u;
    memory[0x001037u] = 5u;
    memory[0x000f45u] = 32u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000e46u] == 24u);

    ResetState();
    SetRequestedSpeeds(0x1608u, 0u);
    memory[0x000c85u] = 2u;
    memory[0x000e46u] = 8u;
    memory[0x001037u] = 5u;
    memory[0x000f45u] = 24u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000e46u] == 24u && memory[0x000e49u] == 24u);

    ResetState();
    SetRequestedSpeeds(0x057eu, 0x1085u);
    memory[0x000c85u] = 2u;
    memory[0x000e46u] = 8u;
    HandleSetCdSpeedCommand();
    assert(event_count == 0u && memory[0x000e47u] == 24u);
}

static void TestStateFourAndCountdown(void)
{
    ResetState();
    SetRequestedSpeeds(0x1085u, 0x1608u);
    memory[0x000faeu] = 4u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000e46u] == 24u && memory[0x000e49u] == 24u);
    assert(memory[0x000e47u] == 32u && event_count == 0u);

    ResetState();
    memory[0x000c9eu] = 3u;
    memory[0x000c89u] = 0x80u;
    memory[0x000c8au] = 0xa0u;
    HandleSetCdSpeedCommand();
    assert(memory[0x000c8au] == 0xa1u && memory[0x00122cu] == 10u);

    ResetState();
    memory[0x000c9eu] = 3u;
    memory[0x000c89u] = 0x80u;
    memory[0x000c8bu] = 8u;
    HandleSetCdSpeedCommand();
    assert(memory[0x00122cu] == 60u);
}

int main(void)
{
    TestValidationAndDisabledRoute();
    TestReadSpeedThresholdsAndCap();
    TestWriteSpeedThresholds();
    TestSavedWriteSpeedReconciliation();
    TestProgressResetAndActiveRestart();
    TestStateFourAndCountdown();
    return 0;
}
