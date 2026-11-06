#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_operating_state_reset.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_CONFIGURE_PORTS,
    EVENT_STROBE,
    EVENT_CONFIGURE_PARAMETERS,
    EVENT_APPLY_D1,
    EVENT_INITIALIZE_BLOCK,
    EVENT_RESET_PARAMETERS,
    EVENT_WRITE_PORT,
    EVENT_SET_FLAG,
    EVENT_RESET_CHANNELS,
    EVENT_READ_PORT,
    EVENT_LOAD_PROFILE,
    EVENT_INITIALIZE_D4_D5,
    EVENT_READ_P1,
    EVENT_WRITE_P1,
    EVENT_TAIL,
    EVENT_RESET_PORTS,
    EVENT_INITIALIZE_C2_D0_D3,
    EVENT_INITIALIZE_D8_D9,
    EVENT_CLEAR_DC_DD,
    EVENT_SUBMIT_BYTE_WORD,
    EVENT_SUBMIT_WORD
};

struct event_record
{
    enum event_kind kind;
    fw_addr_t first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static u8 p1_value;
static u8 read_port_values[8];
static size_t read_port_count;
static size_t read_port_index;
static u8 clear_active_flag_during_channel_reset;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, fw_addr_t first, u16 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

#define SIMPLE_OBSERVER(name, kind)                                                                                                                                                                    \
    void name(void)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        RecordEvent((kind), 0u, 0u);                                                                                                                                                                   \
    }
SIMPLE_OBSERVER(ObserveOperatingResetConfigurePorts, EVENT_CONFIGURE_PORTS)
SIMPLE_OBSERVER(ObserveOperatingResetApplyParameterD1, EVENT_APPLY_D1)
SIMPLE_OBSERVER(ObserveOperatingResetInitializeRegisterBlock, EVENT_INITIALIZE_BLOCK)
SIMPLE_OBSERVER(ObserveOperatingResetResetParameterState, EVENT_RESET_PARAMETERS)
SIMPLE_OBSERVER(ObserveOperatingResetInitializeD4D5, EVENT_INITIALIZE_D4_D5)
SIMPLE_OBSERVER(ObserveOperatingResetTail, EVENT_TAIL)
SIMPLE_OBSERVER(ObserveOperatingResetResetPorts, EVENT_RESET_PORTS)
SIMPLE_OBSERVER(ObserveOperatingResetInitializeC2D0D3, EVENT_INITIALIZE_C2_D0_D3)
SIMPLE_OBSERVER(ObserveOperatingResetInitializeD8D9, EVENT_INITIALIZE_D8_D9)
SIMPLE_OBSERVER(ObserveOperatingResetClearDCDD, EVENT_CLEAR_DC_DD)
#undef SIMPLE_OBSERVER

void ObserveOperatingResetStrobePort(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_STROBE, address, value);
}

void ObserveOperatingResetConfigureParameters(u8 set_p0_bit3)
{
    RecordEvent(EVENT_CONFIGURE_PARAMETERS, set_p0_bit3, 0u);
}

void ObserveOperatingResetWritePort(fw_addr_t address, u8 value)
{
    RecordEvent(EVENT_WRITE_PORT, address, value);
}

void ObserveOperatingResetSetControllerFlag(u8 enabled)
{
    RecordEvent(EVENT_SET_FLAG, enabled, 0u);
}

void ObserveOperatingResetResetChannels(void)
{
    RecordEvent(EVENT_RESET_CHANNELS, 0u, 0u);
    if(clear_active_flag_during_channel_reset != 0u)
        memory[0x001304u] &= 0xfeu;
}

u8 ObserveOperatingResetReadPort(fw_addr_t address)
{
    u8 value;

    assert(read_port_index < read_port_count);
    value = read_port_values[read_port_index++];
    RecordEvent(EVENT_READ_PORT, address, value);
    return value;
}

void ObserveOperatingResetLoadProfile(u8 profile_index)
{
    RecordEvent(EVENT_LOAD_PROFILE, profile_index, 0u);
}

u8 ObserveOperatingResetReadP1Bit(u8 bit_index)
{
    u8 value = (u8)((p1_value >> bit_index) & 1u);
    RecordEvent(EVENT_READ_P1, bit_index, value);
    return value;
}

void ObserveOperatingResetWriteP1Bit(u8 bit_index, u8 value)
{
    u8 mask = (u8)(1u << bit_index);

    if(value != 0u)
        p1_value |= mask;
    else
        p1_value &= (u8)~mask;
    RecordEvent(EVENT_WRITE_P1, bit_index, (u8)(value != 0u));
}

void ObserveOperatingResetSubmitByteWord(u8 value, u16 selector)
{
    RecordEvent(EVENT_SUBMIT_BYTE_WORD, value, selector);
}

void ObserveOperatingResetSubmitWordPair(u16 value, u16 selector)
{
    RecordEvent(EVENT_SUBMIT_WORD, value, selector);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(read_port_values, 0, sizeof(read_port_values));
    event_count = 0u;
    p1_value = 0u;
    read_port_count = 1u;
    read_port_index = 0u;
    clear_active_flag_during_channel_reset = 0u;
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

static size_t FindEventIndex(enum event_kind kind, size_t occurrence)
{
    size_t index;

    for(index = 0u; index < event_count; ++index)
    {
        if(events[index].kind == kind)
        {
            if(occurrence == 0u)
                return index;
            occurrence--;
        }
    }
    assert(0);
    return 0u;
}

static void AssertCommonResults(void)
{
    const struct event_record *event;

    assert(memory[0x00125du] == 0xffu);
    assert(memory[0x00f994u] == 0u);
    assert(memory[0x00f995u] == memory[0x00f9a8u]);
    assert(memory[0x00fc2bu] == 0u && memory[0x00fc2cu] == 0u);
    assert(memory[0x00fc2du] == 2u && memory[0x00fc2eu] == 2u && memory[0x00fc2fu] == 3u);
    assert(read_port_index == read_port_count);
    assert(CountEvents(EVENT_SUBMIT_BYTE_WORD) == 3u);
    event = FindEvent(EVENT_SUBMIT_BYTE_WORD, 0u);
    assert(event->first == 0u && event->second == 0x0256u);
    event = FindEvent(EVENT_SUBMIT_BYTE_WORD, 1u);
    assert(event->first == memory[0x00f9a8u] && event->second == 0x0255u);
    event = FindEvent(EVENT_SUBMIT_BYTE_WORD, 2u);
    assert(event->first == 0x80u && event->second == 0x0257u);
    event = FindEvent(EVENT_SUBMIT_WORD, 0u);
    assert(event->first == 0u && event->second == 0x024fu);
    assert(CountEvents(EVENT_WRITE_P1) == 8u);
    assert(FindEvent(EVENT_WRITE_P1, 4u)->first == 1u && FindEvent(EVENT_WRITE_P1, 4u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 5u)->first == 0u && FindEvent(EVENT_WRITE_P1, 5u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 6u)->first == 1u && FindEvent(EVENT_WRITE_P1, 6u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 7u)->first == 0u && FindEvent(EVENT_WRITE_P1, 7u)->second == 1u);
    assert(p1_value == 3u);
    assert(FindEventIndex(EVENT_INITIALIZE_D4_D5, 0u) < FindEventIndex(EVENT_TAIL, 0u));
    assert(FindEventIndex(EVENT_TAIL, 0u) < FindEventIndex(EVENT_RESET_PORTS, 0u));
    assert(FindEventIndex(EVENT_RESET_PORTS, 0u) < FindEventIndex(EVENT_INITIALIZE_C2_D0_D3, 0u));
    assert(FindEventIndex(EVENT_INITIALIZE_C2_D0_D3, 0u) < FindEventIndex(EVENT_INITIALIZE_D8_D9, 0u));
    assert(FindEventIndex(EVENT_INITIALIZE_D8_D9, 0u) < FindEventIndex(EVENT_CLEAR_DC_DD, 0u));
}

static void TestActiveResetAndPolling(void)
{
    const struct event_record *event;

    ResetState();
    memory[0x001304u] = 0x09u;
    memory[0x001301u] = 0xffu;
    memory[0x00fac0u] = 0x25u;
    memory[0x00f9a8u] = 0x5au;
    p1_value = 3u;
    read_port_values[0] = 0x81u;
    read_port_values[1] = 0x80u;
    read_port_count = 2u;

    ResetControllerOperatingState();

    assert(memory[0x001008u] == 0u && memory[0x00fbecu] == 1u);
    assert(memory[0x00fc0fu] == 0xfdu);
    assert(memory[0x001304u] == 1u);
    assert(memory[0x001301u] == 0xfau);
    assert(CountEvents(EVENT_STROBE) == 2u);
    event = FindEvent(EVENT_STROBE, 0u);
    assert(event->first == 0x01803bul && event->second == 0x88u);
    event = FindEvent(EVENT_STROBE, 1u);
    assert(event->first == 0x018079ul && event->second == 0x10u);
    assert(CountEvents(EVENT_CONFIGURE_PARAMETERS) == 1u);
    assert(CountEvents(EVENT_APPLY_D1) == 1u);
    assert(CountEvents(EVENT_READ_PORT) == 2u);
    assert(CountEvents(EVENT_LOAD_PROFILE) == 1u);
    assert(FindEvent(EVENT_READ_P1, 0u)->first == 1u && FindEvent(EVENT_READ_P1, 0u)->second == 1u);
    assert(FindEvent(EVENT_READ_P1, 1u)->first == 0u && FindEvent(EVENT_READ_P1, 1u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 2u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 3u)->second == 1u);
    AssertCommonResults();
}

static void TestInactiveResetPreservesThenForcesP1(void)
{
    ResetState();
    memory[0x001304u] = 0x88u;
    memory[0x001301u] = 0x07u;
    memory[0x00fc0fu] = 0x66u;
    memory[0x00f9a8u] = 0xa5u;
    p1_value = 2u;

    ResetControllerOperatingState();

    assert(memory[0x00fc0fu] == 0x66u);
    assert(memory[0x001304u] == 0x80u);
    assert(memory[0x001301u] == 2u);
    assert(CountEvents(EVENT_STROBE) == 0u);
    assert(CountEvents(EVENT_CONFIGURE_PARAMETERS) == 0u);
    assert(CountEvents(EVENT_APPLY_D1) == 0u);
    assert(CountEvents(EVENT_LOAD_PROFILE) == 0u);
    assert(FindEvent(EVENT_WRITE_P1, 2u)->first == 1u && FindEvent(EVENT_WRITE_P1, 2u)->second == 1u);
    assert(FindEvent(EVENT_WRITE_P1, 3u)->first == 0u && FindEvent(EVENT_WRITE_P1, 3u)->second == 0u);
    AssertCommonResults();
}

static void TestFreshFlagAndErasedCalibrationDefault(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    memory[0x00fac0u] = 0xffu;
    clear_active_flag_during_channel_reset = 1u;

    ResetControllerOperatingState();

    assert(memory[0x00fc0fu] == 0xa0u);
    assert(memory[0x001304u] == 0u);
    assert(CountEvents(EVENT_STROBE) == 2u);
    assert(CountEvents(EVENT_LOAD_PROFILE) == 0u);
    AssertCommonResults();
}

int main(void)
{
    TestActiveResetAndPolling();
    TestInactiveResetPreservesThenForcesP1();
    TestFreshFlagAndErasedCalibrationDefault();
    return 0;
}
