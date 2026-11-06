#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/controller_state6_sample_calibration.h"
#include "platform/firmware_memory.h"

enum event_kind
{
    EVENT_WRITE_D4_E2,
    EVENT_YIELD,
    EVENT_WRITE_FC,
    EVENT_WRITE_FB,
    EVENT_SUBMIT_WORD,
    EVENT_SELECT_FC,
    EVENT_RESTORE_D412,
    EVENT_INDEXED,
    EVENT_SELECT_FB,
    EVENT_SET_CHANNEL,
    EVENT_CLEAR_D1,
    EVENT_WRITE_D2_D3,
    EVENT_WAIT_E4,
    EVENT_APPLY_D1,
    EVENT_WRITE_D1_D2_D3,
    EVENT_CONFIGURE_RANGE,
    EVENT_REFRESH_FB,
    EVENT_TEST_PULSE
};

struct event_record
{
    enum event_kind kind;
    u16 first;
    u16 second;
};

static u8 memory[1u << 24];
static struct event_record events[96];
static size_t event_count;
static size_t yield_count;
static u8 producer_mode;
static u8 indexed_results[4];
static size_t indexed_count;
static size_t indexed_index;
static u8 select_fb_result;
static u8 pulse_result;
static u8 wait_1215_values[4];
static size_t wait_1215_count;
static size_t wait_1215_index;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x001215ul)
    {
        assert(wait_1215_index < wait_1215_count);
        return wait_1215_values[wait_1215_index++];
    }
    return memory[address & 0xfffffful];
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

void ObserveState6SamplesWriteD4E2(u8 code)
{
    RecordEvent(EVENT_WRITE_D4_E2, code, 0u);
}

void ObserveState6SamplesYield(void)
{
    u8 index;

    RecordEvent(EVENT_YIELD, 0u, 0u);
    yield_count++;
    if(yield_count == 1u)
    {
        memory[0x0011fcu] = 0u;
        memory[0x0011fdu] = 0u;
        memory[0x0011feu] = 0u;
        memory[0x0011ffu] = 0u;
    }
    else if(yield_count == 2u && producer_mode != 0u)
    {
        for(index = 0u; index < 15u; ++index)
        {
            u8 value = producer_mode == 1u ? (u8)(index + 1u) : 1u;
            memory[0x00f9b7u + index] = value;
            memory[0x00f9cbu + index] = value;
        }
        memory[0x001301u] &= 0xfbu;
    }
}

void ObserveState6SamplesWriteFC(u8 value)
{
    RecordEvent(EVENT_WRITE_FC, value, 0u);
}

void ObserveState6SamplesWriteFB(u8 value)
{
    RecordEvent(EVENT_WRITE_FB, value, 0u);
}

void ObserveState6SamplesSubmitWord(u16 value, u16 selector)
{
    RecordEvent(EVENT_SUBMIT_WORD, value, selector);
}

void ObserveState6SamplesSelectFC(u8 operating_code)
{
    RecordEvent(EVENT_SELECT_FC, operating_code, 0u);
}

void ObserveState6SamplesRestoreD412(void)
{
    RecordEvent(EVENT_RESTORE_D412, 0u, 0u);
}

u8 ObserveState6SamplesIsIndexed(u8 state)
{
    assert(indexed_index < indexed_count);
    RecordEvent(EVENT_INDEXED, state, indexed_results[indexed_index]);
    return indexed_results[indexed_index++];
}

u8 ObserveState6SamplesSelectFB(void)
{
    RecordEvent(EVENT_SELECT_FB, 0u, 0u);
    return select_fb_result;
}

void ObserveState6SamplesSetChannel(u8 channel, u8 value)
{
    RecordEvent(EVENT_SET_CHANNEL, channel, value);
}

#define SIMPLE_OBSERVER(name, kind)                                                                                                                                                                    \
    void name(void)                                                                                                                                                                                    \
    {                                                                                                                                                                                                  \
        RecordEvent((kind), 0u, 0u);                                                                                                                                                                   \
    }
SIMPLE_OBSERVER(ObserveState6SamplesClearD1, EVENT_CLEAR_D1)
SIMPLE_OBSERVER(ObserveState6SamplesWriteD2D3, EVENT_WRITE_D2_D3)
SIMPLE_OBSERVER(ObserveState6SamplesWaitPortE4, EVENT_WAIT_E4)
SIMPLE_OBSERVER(ObserveState6SamplesApplyD1, EVENT_APPLY_D1)
SIMPLE_OBSERVER(ObserveState6SamplesWriteD1D2D3, EVENT_WRITE_D1_D2_D3)
SIMPLE_OBSERVER(ObserveState6SamplesConfigureRange, EVENT_CONFIGURE_RANGE)
SIMPLE_OBSERVER(ObserveState6SamplesRefreshFB, EVENT_REFRESH_FB)
#undef SIMPLE_OBSERVER

u8 ObserveState6SamplesTestPulse(u8 count)
{
    RecordEvent(EVENT_TEST_PULSE, count, pulse_result);
    return pulse_result;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(indexed_results, 0, sizeof(indexed_results));
    memset(wait_1215_values, 0xff, sizeof(wait_1215_values));
    event_count = 0u;
    yield_count = 0u;
    producer_mode = 0u;
    indexed_count = 0u;
    indexed_index = 0u;
    select_fb_result = 0xa0u;
    pulse_result = 0u;
    wait_1215_count = 0u;
    wait_1215_index = 0u;
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

static void PrepareSuccessfulProducer(u8 mode)
{
    producer_mode = mode;
    memory[0x0011efu] = 2u;
}

static void AssertTailWords(void)
{
    const u16 values[] = { 0u, 0x0420u, 0x0420u, 0x0160u, 0x0160u };
    const u16 selectors[] = { 0x0251u, 0x0133u, 0x0134u, 0x0133u, 0x0134u };
    size_t index;

    assert(CountEvents(EVENT_SUBMIT_WORD) == 5u);
    for(index = 0u; index < 5u; ++index)
    {
        assert(FindEvent(EVENT_SUBMIT_WORD, index)->first == values[index]);
        assert(FindEvent(EVENT_SUBMIT_WORD, index)->second == selectors[index]);
    }
}

static void TestProducerFailure(void)
{
    ResetState();
    memory[0x001303u] = 0u;

    assert(CalibrateControllerParameterFBFromSampleSeriesForState6() == 0u);
    assert(memory[0x000e41u] == 0x15u);
    assert(memory[0x00fc16u] == 0x80u);
    assert(FindEvent(EVENT_WRITE_FB, 0u)->first == 0xc0u);
    assert(FindEvent(EVENT_WRITE_FB, 1u)->first == 0x80u);
    assert(CountEvents(EVENT_INDEXED) == 0u);
    assert(yield_count == 1u);
    assert(wait_1215_index == 0u);
}

static void TestIncreasingSamplesIndexed(void)
{
    ResetState();
    PrepareSuccessfulProducer(1u);
    indexed_results[0] = 1u;
    indexed_results[1] = 1u;
    indexed_count = 2u;
    wait_1215_count = 1u;
    pulse_result = 1u;

    assert(CalibrateControllerParameterFBFromSampleSeriesForState6() == 1u);
    assert(memory[0x00fc16u] == 0xc0u);
    assert(FindEvent(EVENT_WRITE_FB, 1u)->first == 0xc0u);
    assert(CountEvents(EVENT_SELECT_FB) == 0u);
    assert(CountEvents(EVENT_CONFIGURE_RANGE) == 0u);
    assert(CountEvents(EVENT_REFRESH_FB) == 0u);
    assert((memory[0x001301u] & 8u) != 0u);
    assert(yield_count == 2u);
    assert(indexed_index == indexed_count);
    assert(wait_1215_index == wait_1215_count);
    AssertTailWords();
}

static void TestConstantSamplesNonIndexed(void)
{
    ResetState();
    PrepareSuccessfulProducer(2u);
    indexed_results[0] = 0u;
    indexed_results[1] = 0u;
    indexed_count = 2u;
    wait_1215_count = 2u;
    memory[0x001008u] = 2u;
    memory[0x00fc0fu] = 0x10u;
    memory[0x001301u] = 8u;

    assert(CalibrateControllerParameterFBFromSampleSeriesForState6() == 1u);
    assert(memory[0x00fc16u] == 0x88u);
    assert(CountEvents(EVENT_SELECT_FB) == 1u);
    assert(CountEvents(EVENT_CONFIGURE_RANGE) == 1u);
    assert(CountEvents(EVENT_REFRESH_FB) == 1u);
    assert(FindEvent(EVENT_WRITE_FC, 0u)->first == 0x68u);
    assert(FindEvent(EVENT_WRITE_FC, 1u)->first == 0x30u);
    assert(FindEvent(EVENT_SET_CHANNEL, 0u)->first == 1u && FindEvent(EVENT_SET_CHANNEL, 0u)->second == 0x94u);
    assert(FindEvent(EVENT_SET_CHANNEL, 1u)->first == 2u && FindEvent(EVENT_SET_CHANNEL, 1u)->second == 0xc1u);
    assert((memory[0x001301u] & 8u) == 0u);
    assert(indexed_index == indexed_count);
    assert(wait_1215_index == wait_1215_count);
    AssertTailWords();
}

static void TestProducerFailureOverrideNonIndexed(void)
{
    ResetState();
    memory[0x001303u] = 0x40u;
    indexed_results[0] = 0u;
    indexed_results[1] = 0u;
    indexed_count = 2u;
    wait_1215_count = 2u;
    select_fb_result = 0xa8u;

    assert(CalibrateControllerParameterFBFromSampleSeriesForState6() == 1u);
    assert(memory[0x000e41u] == 0u);
    assert(memory[0x00fc16u] == 0xa8u);
    assert(CountEvents(EVENT_SELECT_FB) == 1u);
    assert(FindEvent(EVENT_WRITE_FB, 0u)->first == 0xc0u);
    assert(CountEvents(EVENT_WRITE_FB) == 1u);
    assert(CountEvents(EVENT_CONFIGURE_RANGE) == 1u);
    assert(CountEvents(EVENT_REFRESH_FB) == 1u);
    assert(indexed_index == indexed_count);
    assert(wait_1215_index == wait_1215_count);
    AssertTailWords();
}

int main(void)
{
    TestProducerFailure();
    TestIncreasingSamplesIndexed();
    TestConstantSamplesNonIndexed();
    TestProducerFailureOverrideNonIndexed();
    return 0;
}
