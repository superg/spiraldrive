#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_operating_state_initialization.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_RESET_STATE,
    EVENT_RESET_HARDWARE,
    EVENT_PARAMETER_STATE2,
    EVENT_INDEXED,
    EVENT_OUTCOME,
    EVENT_P3,
    EVENT_SELECT_FB,
    EVENT_WAIT_CONFIG,
    EVENT_ESTABLISH,
    EVENT_APPLY_CHANNELS,
    EVENT_VALIDATE,
    EVENT_CURRENT_CALIBRATION,
    EVENT_COMMAND03,
    EVENT_LOAD_PROFILE,
    EVENT_REGISTER01,
    EVENT_P1,
    EVENT_PULSE,
    EVENT_READ_WORD,
    EVENT_SUBMIT_WORD,
    EVENT_RETRY_PORT,
    EVENT_SET_CHANNEL,
    EVENT_CONFIGURE_RANGE,
    EVENT_CONFIGURE_CODE4
};

struct event_record
{
    enum event_kind kind;
    u16 first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[64];
static size_t event_count;
static u8 indexed_result;
static u8 establish_result;
static u8 validate_result;
static u8 current_result;
static u8 retry_result;
static u16 read_word_result;
static u8 expire_countdown;
static size_t countdown_reads;

u8 FirmwareRead8(fw_addr_t address)
{
    u8 value = memory[address & 0xfffffful];
    if(address == 0x001220ul && expire_countdown != 0u)
    {
        countdown_reads++;
        if(countdown_reads == 1u)
            memory[0x001220u] = 0xffu;
    }
    return value;
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void RecordEvent(enum event_kind kind, u16 first, u16 second)
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
SIMPLE_OBSERVER(ObserveOperatingStateResetState, EVENT_RESET_STATE)
SIMPLE_OBSERVER(ObserveOperatingStateResetHardware, EVENT_RESET_HARDWARE)
SIMPLE_OBSERVER(ObserveOperatingStateConfigureParameterState2, EVENT_PARAMETER_STATE2)
SIMPLE_OBSERVER(ObserveOperatingStateSetP3LowHigh, EVENT_P3)
SIMPLE_OBSERVER(ObserveOperatingStateSelectParameterFb, EVENT_SELECT_FB)
SIMPLE_OBSERVER(ObserveOperatingStateWaitConfigurationBit7, EVENT_WAIT_CONFIG)
SIMPLE_OBSERVER(ObserveOperatingStateApplyChannels, EVENT_APPLY_CHANNELS)
SIMPLE_OBSERVER(ObserveOperatingStateRunCommand03, EVENT_COMMAND03)
SIMPLE_OBSERVER(ObserveOperatingStateSetP1Pins, EVENT_P1)
SIMPLE_OBSERVER(ObserveOperatingStateConfigureRange, EVENT_CONFIGURE_RANGE)
SIMPLE_OBSERVER(ObserveOperatingStateConfigureCode4, EVENT_CONFIGURE_CODE4)
#undef SIMPLE_OBSERVER

u8 ObserveOperatingStateIsIndexedState(u8 state_code)
{
    RecordEvent(EVENT_INDEXED, state_code, 0u);
    return indexed_result;
}

void ObserveOperatingStateUpdateOutcome(u8 stage)
{
    RecordEvent(EVENT_OUTCOME, stage, 0u);
}

u8 ObserveOperatingStateEstablishState6(void)
{
    RecordEvent(EVENT_ESTABLISH, 0u, 0u);
    return establish_result;
}

u8 ObserveOperatingStateValidateState5(void)
{
    RecordEvent(EVENT_VALIDATE, 0u, 0u);
    return validate_result;
}

u8 ObserveOperatingStateRunCurrentCalibration(void)
{
    RecordEvent(EVENT_CURRENT_CALIBRATION, 0u, 0u);
    return current_result;
}

void ObserveOperatingStateLoadProfile(u8 profile_index)
{
    RecordEvent(EVENT_LOAD_PROFILE, profile_index, 0u);
}

void ObserveOperatingStateWriteRegister01(u8 value)
{
    RecordEvent(EVENT_REGISTER01, value, 0u);
}

void ObserveOperatingStatePulseWord(u16 duration)
{
    RecordEvent(EVENT_PULSE, duration, 0u);
}

u16 ObserveOperatingStateReadWord(u16 selector)
{
    RecordEvent(EVENT_READ_WORD, selector, 0u);
    return read_word_result;
}

void ObserveOperatingStateSubmitWord(u16 value, u16 selector)
{
    RecordEvent(EVENT_SUBMIT_WORD, value, selector);
}

u8 ObserveOperatingStateRetryPortCalibration(u8 mode)
{
    RecordEvent(EVENT_RETRY_PORT, mode, 0u);
    return retry_result;
}

void ObserveOperatingStateSetChannel(u8 channel, u8 value)
{
    RecordEvent(EVENT_SET_CHANNEL, channel, value);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    indexed_result = 0u;
    establish_result = 1u;
    validate_result = 1u;
    current_result = 1u;
    retry_result = 1u;
    read_word_result = 0x4567u;
    expire_countdown = 1u;
    countdown_reads = 0u;
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

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void TestActivePath(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    memory[0x00fb98u] = 2u;
    memory[0x001308u] = 2u;
    memory[0x00faacu] = 0x80u;
    indexed_result = 1u;

    assert(InitializeControllerOperatingStateAndRunCalibration() == 1u);
    assert(CountEvents(EVENT_PARAMETER_STATE2) == 1u);
    assert(CountEvents(EVENT_OUTCOME) == 1u);
    assert(CountEvents(EVENT_P3) == 1u);
    assert(CountEvents(EVENT_WAIT_CONFIG) == 1u);
    assert(CountEvents(EVENT_ESTABLISH) == 1u);
    assert(CountEvents(EVENT_APPLY_CHANNELS) == 1u);
    assert(CountEvents(EVENT_COMMAND03) == 1u);
    assert(memory[0x00fab4u] == 0x7fu);
    assert(memory[0x001006u] == 0u);
    assert(memory[0x001225u] == 2u);
    assert(memory[0x001300u] == 1u);
    assert(ReadU32(0x000ef5u) == 0x96u);
    assert(memory[0x001304u] == 8u);
}

static void TestActiveEstablishmentAndCalibrationFailures(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    establish_result = 0u;
    assert(InitializeControllerOperatingStateAndRunCalibration() == 0u);
    assert(CountEvents(EVENT_VALIDATE) == 0u);

    ResetState();
    memory[0x001304u] = 1u;
    validate_result = 2u;
    assert(InitializeControllerOperatingStateAndRunCalibration() == 0u);
    assert(CountEvents(EVENT_CURRENT_CALIBRATION) == 0u);

    ResetState();
    memory[0x001304u] = 1u;
    current_result = 2u;
    assert(InitializeControllerOperatingStateAndRunCalibration() == 0u);
    assert(CountEvents(EVENT_CURRENT_CALIBRATION) == 1u);
}

static void TestAlternateRetryFailureRestoresWord(void)
{
    ResetState();
    retry_result = 0u;
    read_word_result = 0x1234u;

    assert(InitializeControllerOperatingStateAndRunCalibration() == 0u);
    assert(CountEvents(EVENT_PULSE) == 1u);
    assert(FindEvent(EVENT_PULSE, 0u)->first == 50u);
    assert(CountEvents(EVENT_SUBMIT_WORD) == 2u);
    assert(FindEvent(EVENT_SUBMIT_WORD, 0u)->first == 0u && FindEvent(EVENT_SUBMIT_WORD, 0u)->second == 0x0204u);
    assert(FindEvent(EVENT_SUBMIT_WORD, 1u)->first == 0x1234u && FindEvent(EVENT_SUBMIT_WORD, 1u)->second == 0x0204u);
    assert(CountEvents(EVENT_VALIDATE) == 0u);
}

static void TestAlternateSuccessReturnsAfterCountdown(void)
{
    ResetState();
    memory[0x001159u] = 1u;
    memory[0x001304u] = 0u;
    read_word_result = 0x9abcu;

    assert(InitializeControllerOperatingStateAndRunCalibration() == 1u);
    assert(countdown_reads == 2u);
    assert(ReadU32(0x0011fcu) == 250u);
    assert(memory[0x00f9a8u] == 0u);
    assert(CountEvents(EVENT_SET_CHANNEL) == 2u);
    assert(CountEvents(EVENT_CONFIGURE_RANGE) == 1u);
    assert(CountEvents(EVENT_CONFIGURE_CODE4) == 1u);
    assert(CountEvents(EVENT_SUBMIT_WORD) == 2u);
    assert(CountEvents(EVENT_VALIDATE) == 1u);
    assert(CountEvents(EVENT_CURRENT_CALIBRATION) == 1u);
    assert(CountEvents(EVENT_COMMAND03) == 0u);
    assert(memory[0x001304u] == 8u);
}

static void TestIndexedResultMustEqualOne(void)
{
    ResetState();
    memory[0x001304u] = 1u;
    indexed_result = 2u;

    assert(InitializeControllerOperatingStateAndRunCalibration() == 1u);
    assert(CountEvents(EVENT_OUTCOME) == 0u);
    assert(CountEvents(EVENT_P3) == 0u);
}

int main(void)
{
    TestActivePath();
    TestActiveEstablishmentAndCalibrationFailures();
    TestAlternateRetryFailureRestoresWord();
    TestAlternateSuccessReturnsAfterCountdown();
    TestIndexedResultMustEqualOne();
    return 0;
}
