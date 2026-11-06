#include <assert.h>
#include <string.h>
#include "media/raw_read_speed_update.h"
#include "platform/firmware_memory.h"

enum
{
    MEMORY_SIZE = 1u << 24,
    TRACE_CAPACITY = 64
};

static u8 memory[MEMORY_SIZE];
static u8 bits[0x22];
static fw_addr_t write_addresses[TRACE_CAPACITY];
static u8 write_values[TRACE_CAPACITY];
static unsigned write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    if(write_count < TRACE_CAPACITY)
    {
        write_addresses[write_count] = address;
        write_values[write_count] = value;
    }
    ++write_count;
    memory[address & 0xfffffful] = value;
}

u8 ObserveRawReadSpeedBit(u8 bit_address)
{
    return bits[bit_address];
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

typedef struct RawReadSpeedState
{
    u8 selected;
    u8 current;
    u8 write_speed;
    u8 operating;
    u8 cached;
    u8 published;
    u8 transition;
    u8 queue_flag;
    u8 retry_count;
    u8 media_kind;
    u8 raw_format;
    u8 media_mode;
    u8 limit_flags;
    u8 media_class;
    u8 command;
    u8 write_active;
    u8 command_mode;
    u8 branch_flag;
    u8 recovery;
    u8 media_ready;
    u8 direct_000c;
    u8 direct_000f;
    u8 direct_001b;
    u8 direct_001c;
    u8 direct_001d;
    u8 direct_0021;
    s32 target;
    u16 high_water;
} RawReadSpeedState;

static void ApplyReference(RawReadSpeedState *state)
{
    if(state->transition < 5u)
    {
        if(state->queue_flag == 0u && state->operating < state->current)
            ++state->transition;
        state->current = state->selected;
        if(state->media_mode != 1u && (s8)state->current > 24)
            state->current = 24u;
        if(state->direct_000c != 0u && state->write_active == 0u && (s8)state->current > 32)
            state->current = 32u;
        if(state->write_active != 0u)
        {
            if((s8)state->current > 40)
                state->current = 40u;
            if((state->command_mode & 1u) != 0u && (s8)state->current > 32)
                state->current = 32u;
            if(state->media_class == 2u && state->branch_flag == 0u)
                state->current = 24u;
            if(state->recovery == 1u)
            {
                if(state->write_speed < 24u)
                    state->current = state->write_speed < 10u ? 4u : 8u;
                else
                    state->current = 32u;
            }
            if(state->media_class != 0u && state->command == 0xb9u)
                state->current = state->current < 4u ? 4u : 8u;
        }
        if((state->limit_flags & 1u) != 0u && (s8)state->current > 8)
        {
            state->write_speed = 8u;
            state->current = 8u;
        }
        if(state->media_kind == 2u)
        {
            if((s8)state->raw_format > 0 && (s8)state->current > 8)
                state->current = 8u;
        }
        else if((s8)state->raw_format > 0 && (s8)state->current > 32)
        {
            state->current = 32u;
        }
        if(state->direct_001c != 0u || state->direct_001d != 0u)
            state->current = 4u;
        if(((state->media_ready >> 1) & 1u) == 0u)
            state->current = 4u;
        if(state->queue_flag != 0u)
        {
            if(state->target < 150 || state->target >= (s32)(u32)state->high_water)
                state->queue_flag = 0u;
            else
                state->current = 8u;
        }
        if(state->retry_count >= 10u)
        {
            if(state->current < 8u)
                state->retry_count = 0u;
            else
                state->current = 8u;
        }
        state->published = state->current;
        state->operating = state->current;
        state->cached = state->current;
    }
    else
    {
        state->retry_count = 0u;
        if((s8)state->cached > (s8)state->selected)
        {
            state->current = state->selected;
            state->operating = state->selected;
        }
        else if((s8)state->cached > 0)
        {
            state->current = state->cached;
            state->operating = state->cached;
        }
    }
    if(state->direct_000f != 0u && state->selected >= 4u)
        state->current = 4u;
    if(state->direct_001b != 0u)
    {
        state->cached = 8u;
        state->current = 8u;
        state->operating = 8u;
    }
    if(state->direct_0021 != 0u)
    {
        state->cached = 8u;
        state->current = 8u;
        state->operating = 8u;
    }
}

static void LoadState(const RawReadSpeedState *state)
{
    memset(memory, 0, sizeof(memory));
    memset(bits, 0, sizeof(bits));
    write_count = 0u;
    memory[0x000c37u] = state->selected;
    memory[0x000e46u] = state->current;
    memory[0x000e47u] = state->write_speed;
    memory[0x000e49u] = state->operating;
    memory[0x000f45u] = state->cached;
    memory[0x000f3au] = state->published;
    memory[0x001037u] = state->transition;
    memory[0x001591u] = state->queue_flag;
    memory[0x0015e2u] = state->retry_count;
    memory[0x001008u] = state->media_kind;
    memory[0x000f06u] = state->raw_format;
    memory[0x00fbecu] = state->media_mode;
    memory[0x000c9eu] = state->media_class;
    memory[0x000b0au] = state->command;
    memory[0x0015d7u] = state->write_active;
    memory[0x000c89u] = state->command_mode;
    memory[0x2ba5afu] = state->branch_flag;
    memory[0x000ca1u] = state->recovery;
    memory[0x00faefu] = state->limit_flags;
    memory[0x000c85u] = state->media_ready;
    bits[0x0c] = state->direct_000c;
    bits[0x0f] = state->direct_000f;
    bits[0x1b] = state->direct_001b;
    bits[0x1c] = state->direct_001c;
    bits[0x1d] = state->direct_001d;
    bits[0x21] = state->direct_0021;
    WriteBe32(0x001598u, (u32)state->target);
    WriteBe16(0x00129cu, state->high_water);
}

static void AssertState(const RawReadSpeedState *state)
{
    assert(memory[0x000e46u] == state->current);
    assert(memory[0x000e47u] == state->write_speed);
    assert(memory[0x000e49u] == state->operating);
    assert(memory[0x000f45u] == state->cached);
    assert(memory[0x000f3au] == state->published);
    assert(memory[0x001037u] == state->transition);
    assert(memory[0x001591u] == state->queue_flag);
    assert(memory[0x0015e2u] == state->retry_count);
}

static RawReadSpeedState DefaultState(void)
{
    RawReadSpeedState state;

    memset(&state, 0, sizeof(state));
    state.selected = 24u;
    state.current = 16u;
    state.write_speed = 16u;
    state.operating = 8u;
    state.media_mode = 1u;
    state.media_ready = 2u;
    state.high_water = 300u;
    state.target = 200;
    return state;
}

static void TestPrimaryPublicationOrder(void)
{
    RawReadSpeedState state = DefaultState();

    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(write_count == 5u);
    assert(write_addresses[0] == 0x001037ul && write_values[0] == 1u);
    assert(write_addresses[1] == 0x000e46ul && write_values[1] == 24u);
    assert(write_addresses[2] == 0x000f3aul && write_values[2] == 24u);
    assert(write_addresses[3] == 0x000e49ul && write_values[3] == 24u);
    assert(write_addresses[4] == 0x000f45ul && write_values[4] == 24u);
}

static void TestQueueAndRetryRoutes(void)
{
    RawReadSpeedState state = DefaultState();

    state.queue_flag = 1u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x001591u] == 1u && memory[0x000e46u] == 8u);

    state.target = 149;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x001591u] == 0u && memory[0x000e46u] == 24u);

    state = DefaultState();
    state.selected = 4u;
    state.retry_count = 10u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x0015e2u] == 0u && memory[0x000e46u] == 4u);

    state.selected = 40u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x0015e2u] == 10u && memory[0x000e46u] == 8u);
}

static void TestTransitionAndFinalOverrides(void)
{
    RawReadSpeedState state = DefaultState();

    state.transition = 5u;
    state.cached = 24u;
    state.selected = 16u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x0015e2u] == 0u);
    assert(memory[0x000e46u] == 16u && memory[0x000e49u] == 16u);
    assert(memory[0x000f45u] == 24u);

    state = DefaultState();
    state.direct_000f = 1u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(memory[0x000e46u] == 4u);
    assert(memory[0x000f3au] == 24u && memory[0x000e49u] == 24u && memory[0x000f45u] == 24u);

    state.direct_001b = 1u;
    state.direct_0021 = 1u;
    LoadState(&state);
    UpdateRawReadSpeedForCurrentPipelineState();
    assert(write_count >= 6u);
    assert(write_addresses[write_count - 6u] == 0x000f45ul);
    assert(write_addresses[write_count - 5u] == 0x000e46ul);
    assert(write_addresses[write_count - 4u] == 0x000e49ul);
    assert(write_addresses[write_count - 3u] == 0x000f45ul);
    assert(write_addresses[write_count - 2u] == 0x000e46ul);
    assert(write_addresses[write_count - 1u] == 0x000e49ul);
    assert(memory[0x000f45u] == 8u && memory[0x000e46u] == 8u && memory[0x000e49u] == 8u);
}

static u32 NextRandom(u32 *seed)
{
    *seed = *seed * 1664525ul + 1013904223ul;
    return *seed;
}

static void TestReferenceMatrix(void)
{
    u32 seed = 0x5224a104ul;
    unsigned iteration;

    for(iteration = 0u; iteration < 10000u; ++iteration)
    {
        RawReadSpeedState actual;
        RawReadSpeedState expected;
        u8 *bytes = (u8 *)&actual;
        unsigned index;

        for(index = 0u; index < sizeof(actual); ++index)
            bytes[index] = (u8)(NextRandom(&seed) >> 24);
        actual.direct_000c &= 1u;
        actual.direct_000f &= 1u;
        actual.direct_001b &= 1u;
        actual.direct_001c &= 1u;
        actual.direct_001d &= 1u;
        actual.direct_0021 &= 1u;
        expected = actual;
        ApplyReference(&expected);
        LoadState(&actual);
        UpdateRawReadSpeedForCurrentPipelineState();
        AssertState(&expected);
    }
}

int main(void)
{
    TestPrimaryPublicationOrder();
    TestQueueAndRetryRoutes();
    TestTransitionAndFinalOverrides();
    TestReferenceMatrix();
    return 0;
}
