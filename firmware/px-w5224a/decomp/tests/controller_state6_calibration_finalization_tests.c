#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_state6_calibration_finalization.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_PULSE,
    EVENT_LOAD_PROFILE,
    EVENT_CONFIGURE_PORT8006,
    EVENT_APPLY_CHANNELS,
    EVENT_CONFIGURE_RANGE,
    EVENT_YIELD,
    EVENT_STROBE,
    EVENT_CONFIGURE_P0,
    EVENT_WAIT_TOLERANCE,
    EVENT_INDEXED,
    EVENT_UPDATE_OUTCOME,
    EVENT_READ_P0_BIT7
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u32 second;
};

struct byte_script
{
    u8 values[16];
    size_t count;
    size_t index;
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static struct byte_script countdown_1215;
static struct byte_script countdown_1209;
static struct byte_script port_802a;
static struct byte_script pulse_results;
static struct byte_script indexed_results;
static struct byte_script p0_results;

static u8 ReadScript(struct byte_script *script)
{
    assert(script->index < script->count);
    return script->values[script->index++];
}

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x001215ul && countdown_1215.count != 0u)
        return ReadScript(&countdown_1215);
    if(address == 0x001209ul && countdown_1209.count != 0u)
        return ReadScript(&countdown_1209);
    if(address == 0x01802aul && port_802a.count != 0u)
        return ReadScript(&port_802a);
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u32 first, u32 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

u8 ObserveState6FinalizationPulse(u8 sample_count)
{
    u8 result = ReadScript(&pulse_results);
    RecordEvent(EVENT_PULSE, sample_count, result);
    return result;
}

void ObserveState6FinalizationLoadProfile(u8 profile_index)
{
    RecordEvent(EVENT_LOAD_PROFILE, profile_index, 0u);
}

void ObserveState6FinalizationConfigurePort8006(u8 enabled)
{
    RecordEvent(EVENT_CONFIGURE_PORT8006, enabled, 0u);
}

void ObserveState6FinalizationApplyChannels(void)
{
    RecordEvent(EVENT_APPLY_CHANNELS, 0u, 0u);
}

void ObserveState6FinalizationConfigureRange(void)
{
    RecordEvent(EVENT_CONFIGURE_RANGE, 0u, 0u);
}

void ObserveState6FinalizationYield(void)
{
    RecordEvent(EVENT_YIELD, 0u, 0u);
}

void ObserveState6FinalizationStrobe(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_STROBE, address, value);
}

void ObserveState6FinalizationConfigureP0(u8 enabled)
{
    RecordEvent(EVENT_CONFIGURE_P0, enabled, 0u);
}

void ObserveState6FinalizationWaitForTolerance(u8 wait_for_initial_advance, u8 lower_bound_only)
{
    RecordEvent(EVENT_WAIT_TOLERANCE, wait_for_initial_advance, lower_bound_only);
}

u8 ObserveState6FinalizationIsIndexed(u8 state)
{
    u8 result = ReadScript(&indexed_results);
    RecordEvent(EVENT_INDEXED, state, result);
    return result;
}

void ObserveState6FinalizationUpdateOutcome(u8 stage)
{
    RecordEvent(EVENT_UPDATE_OUTCOME, stage, 0u);
}

u8 ObserveState6FinalizationReadP0Bit7(void)
{
    u8 result = ReadScript(&p0_results);
    RecordEvent(EVENT_READ_P0_BIT7, result, 0u);
    return result;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(&countdown_1215, 0, sizeof(countdown_1215));
    memset(&countdown_1209, 0, sizeof(countdown_1209));
    memset(&port_802a, 0, sizeof(port_802a));
    memset(&pulse_results, 0, sizeof(pulse_results));
    memset(&indexed_results, 0, sizeof(indexed_results));
    memset(&p0_results, 0, sizeof(p0_results));
    event_count = 0u;
}

static void SetScript(struct byte_script *script, const u8 *values, size_t count)
{
    assert(count <= sizeof(script->values));
    memcpy(script->values, values, count);
    script->count = count;
    script->index = 0u;
}

static size_t CountEvents(enum event_kind kind)
{
    size_t index;
    size_t count = 0u;
    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            count++;
    return count;
}

static const struct event_record *FindEvent(enum event_kind kind, size_t occurrence)
{
    size_t index;
    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].kind == kind)
        {
            if(occurrence == 0u)
                return &events[index];
            occurrence--;
        }
    }
    assert(0);
    return NULL;
}

static void AssertScriptsConsumed(void)
{
    assert(countdown_1215.index == countdown_1215.count);
    assert(countdown_1209.index == countdown_1209.count);
    assert(port_802a.index == port_802a.count);
    assert(pulse_results.index == pulse_results.count);
    assert(indexed_results.index == indexed_results.count);
    assert(p0_results.index == p0_results.count);
}

static void TestInitialFlagFailure(void)
{
    static const u8 timer_values[] = { 5u };

    ResetState();
    SetScript(&countdown_1215, timer_values, 1u);
    memory[0x001304u] = 0x10u;

    assert(FinalizeControllerState6OperatingCalibration() == 0u);
    assert(memory[0x001304u] == 0x14u);
    assert(memory[0x001215u] == 5u);
    assert(event_count == 0u);
    AssertScriptsConsumed();
}

static void TestPortReadyNonModeSuccess(void)
{
    static const u8 timer_values[] = { 0u, 0u, 0u };
    static const u8 outer_values[] = { 15u, 15u, 5u };
    static const u8 port_values[] = { 0x40u, 0x40u };
    static const u8 pulse_values[] = { 1u };
    static const u8 indexed_values[] = { 1u };
    static const u8 p0_values[] = { 1u };

    ResetState();
    SetScript(&countdown_1215, timer_values, 3u);
    SetScript(&countdown_1209, outer_values, 3u);
    SetScript(&port_802a, port_values, 2u);
    SetScript(&pulse_results, pulse_values, 1u);
    SetScript(&indexed_results, indexed_values, 1u);
    SetScript(&p0_results, p0_values, 1u);
    memory[0x0011efu] = 2u;
    memory[0x001008u] = 1u;
    memory[0x00fab4u] = 0xffu;
    memory[0x00fc16u] = 0x88u;

    assert(FinalizeControllerState6OperatingCalibration() == 1u);
    assert(memory[0x001008u] == 1u);
    assert(memory[0x001006u] == 1u);
    assert(memory[0x00fab4u] == 0xb6u);
    assert(memory[0x00fab6u] == 0x88u);
    assert(memory[0x001304u] == 0u);
    assert(CountEvents(EVENT_LOAD_PROFILE) == 2u);
    assert(CountEvents(EVENT_CONFIGURE_PORT8006) == 1u);
    assert(FindEvent(EVENT_CONFIGURE_PORT8006, 0u)->first == 1u);
    assert(CountEvents(EVENT_UPDATE_OUTCOME) == 1u);
    assert(FindEvent(EVENT_UPDATE_OUTCOME, 0u)->first == 1u);
    assert(CountEvents(EVENT_YIELD) == 0u);
    AssertScriptsConsumed();
}

static void TestMode2MeasurementTimeoutFailure(void)
{
    static const u8 timer_values[] = { 0u, 0u, 0u, 0xffu };
    static const u8 outer_values[] = { 15u, 0u, 0u };
    static const u8 port_values[] = { 0u, 0u };
    static const u8 pulse_values[] = { 2u };

    ResetState();
    SetScript(&countdown_1215, timer_values, 4u);
    SetScript(&countdown_1209, outer_values, 3u);
    SetScript(&port_802a, port_values, 2u);
    SetScript(&pulse_results, pulse_values, 1u);
    memory[0x0011efu] = 2u;
    memory[0x001008u] = 2u;
    memory[0x001304u] = 0x10u;
    memory[0x00fb98u] = 7u;

    assert(FinalizeControllerState6OperatingCalibration() == 0u);
    assert(memory[0x00fb98u] == 0u);
    assert(memory[0x001304u] == 0x14u);
    assert(CountEvents(EVENT_YIELD) == 1u);
    assert(CountEvents(EVENT_STROBE) == 2u);
    assert(FindEvent(EVENT_STROBE, 0u)->first == 0x01803bul);
    assert(FindEvent(EVENT_STROBE, 0u)->second == 0x88u);
    assert(FindEvent(EVENT_STROBE, 1u)->first == 0x018079ul);
    assert(FindEvent(EVENT_STROBE, 1u)->second == 0x10u);
    assert(CountEvents(EVENT_CONFIGURE_P0) == 1u);
    assert(FindEvent(EVENT_CONFIGURE_P0, 0u)->first == 0u);
    assert(CountEvents(EVENT_WAIT_TOLERANCE) == 1u);
    assert(FindEvent(EVENT_WAIT_TOLERANCE, 0u)->first == 0u);
    assert(FindEvent(EVENT_WAIT_TOLERANCE, 0u)->second == 0u);
    assert(CountEvents(EVENT_INDEXED) == 0u);
    AssertScriptsConsumed();
}

static void TestThreePulseFailuresStillFinalize(void)
{
    static const u8 timer_values[] = { 0u, 0u, 0xffu, 0xffu, 0xffu };
    static const u8 outer_values[] = { 15u, 15u, 15u, 15u, 5u };
    static const u8 port_values[] = { 0u, 0u, 0u };
    static const u8 pulse_values[] = { 0u, 0u, 0u, 0u };
    static const u8 indexed_values[] = { 0u };
    static const u8 p0_values[] = { 1u };

    ResetState();
    SetScript(&countdown_1215, timer_values, 5u);
    SetScript(&countdown_1209, outer_values, 5u);
    SetScript(&port_802a, port_values, 3u);
    SetScript(&pulse_results, pulse_values, 4u);
    SetScript(&indexed_results, indexed_values, 1u);
    SetScript(&p0_results, p0_values, 1u);
    memory[0x0011efu] = 2u;

    assert(FinalizeControllerState6OperatingCalibration() == 1u);
    assert(memory[0x001008u] == 1u);
    assert(memory[0x001006u] == 1u);
    assert(CountEvents(EVENT_PULSE) == 4u);
    assert(CountEvents(EVENT_CONFIGURE_PORT8006) == 2u);
    assert(CountEvents(EVENT_UPDATE_OUTCOME) == 0u);
    AssertScriptsConsumed();
}

static void TestFinalP0Expiry(void)
{
    static const u8 timer_values[] = { 0u, 0u, 0xffu };
    static const u8 outer_values[] = { 15u, 0u, 0u, 0u };
    static const u8 port_values[] = { 0u };
    static const u8 pulse_values[] = { 1u, 1u };
    static const u8 indexed_values[] = { 1u };

    ResetState();
    SetScript(&countdown_1215, timer_values, 3u);
    SetScript(&countdown_1209, outer_values, 4u);
    SetScript(&port_802a, port_values, 1u);
    SetScript(&pulse_results, pulse_values, 2u);
    SetScript(&indexed_results, indexed_values, 1u);
    memory[0x0011efu] = 2u;
    memory[0x001304u] = 0x20u;
    memory[0x00fab4u] = 0xffu;
    memory[0x00fc16u] = 0x90u;

    assert(FinalizeControllerState6OperatingCalibration() == 0u);
    assert(memory[0x001008u] == 0u);
    assert(memory[0x001006u] == 0u);
    assert(memory[0x00fab4u] == 0xbfu);
    assert(memory[0x00fab6u] == 0x90u);
    assert(memory[0x001304u] == 0x24u);
    assert(CountEvents(EVENT_CONFIGURE_PORT8006) == 1u);
    assert(FindEvent(EVENT_CONFIGURE_PORT8006, 0u)->first == 0u);
    assert(CountEvents(EVENT_UPDATE_OUTCOME) == 1u);
    assert(CountEvents(EVENT_READ_P0_BIT7) == 0u);
    AssertScriptsConsumed();
}

int main(void)
{
    TestInitialFlagFailure();
    TestPortReadyNonModeSuccess();
    TestMode2MeasurementTimeoutFailure();
    TestThreePulseFailuresStillFinalize();
    TestFinalP0Expiry();
    return 0;
}
