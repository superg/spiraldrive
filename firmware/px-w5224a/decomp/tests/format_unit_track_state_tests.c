#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_unit_track_state.h"

enum event_kind
{
    EVENT_CLEAR,
    EVENT_RESET_TABLE,
    EVENT_UPDATE_DATA_BLOCK,
    EVENT_TAIL,
    EVENT_LOAD,
    EVENT_COMMIT,
    EVENT_SET_BIT
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u32 second;
};

static u8 memory[1u << 24];
static struct event_record events[32];
static size_t event_count;
static u8 tail_result;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void Record(enum event_kind kind, u32 first, u32 second)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].kind = kind;
    events[event_count].first = first;
    events[event_count].second = second;
    event_count++;
}

void ObserveFormatTrackClear(void)
{
    Record(EVENT_CLEAR, 0u, 0u);
}

void ObserveFormatTrackResetTable(void)
{
    Record(EVENT_RESET_TABLE, 0u, 0u);
}

void ObserveFormatTrackUpdateDataBlock(u8 old_class, u8 old_mask)
{
    u8 type = memory[0x000c48u];
    u8 data_class = old_class;
    u8 mask = old_mask;
    Record(EVENT_UPDATE_DATA_BLOCK, old_class, old_mask);
    if(type <= 3u || type == 8u)
        data_class = 1u;
    else if(type >= 9u && type <= 13u)
        data_class = 2u;
    if(type == 8u)
        mask = 0x40u;
    else if(type == 9u)
        mask = 0x20u;
    else if(type == 10u || type == 11u || type == 13u)
        mask = 0x10u;
    else if(type == 12u)
        mask = 8u;
    memory[0x00dd37u] = data_class;
    memory[0x00dd38u] = mask;
}

u8 ObserveFormatTrackTailUninitialized(void)
{
    Record(EVENT_TAIL, tail_result, 0u);
    return tail_result;
}

void ObserveFormatTrackLoad(u8 track_number)
{
    Record(EVENT_LOAD, track_number, 0u);
}

void ObserveFormatTrackCommit(void)
{
    Record(EVENT_COMMIT, 0u, 0u);
}

void ObserveFormatTrackSetBit000A(void)
{
    Record(EVENT_SET_BIT, 0u, 0u);
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

static size_t Count(enum event_kind kind)
{
    size_t index;
    size_t count = 0u;
    for(index = 0u; index < event_count; ++index)
        if(events[index].kind == kind)
            count++;
    return count;
}

static const struct event_record *Find(enum event_kind kind, size_t occurrence)
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

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    tail_result = 1u;
    memory[0x000c47u] = 5u;
    memory[0x000c48u] = 8u;
    WriteU32(0x000c4bu, 16u);
    WriteU32(0x00e612u, 100u);
    WriteU32(0x00e616u, 200u);
    WriteU32(0x00e61au, 300u);
    WriteU32(0x00e61eu, 400u);
}

static void TestNewDiscWithoutExtension(void)
{
    ResetState();
    memory[0x00dd37u] = 7u;
    memory[0x00dd38u] = 9u;

    UpdateFormatUnitTrackStateAfterSuccessfulPreparation();

    assert(Count(EVENT_CLEAR) == 1u);
    assert(Count(EVENT_RESET_TABLE) == 1u);
    assert(Find(EVENT_UPDATE_DATA_BLOCK, 0u)->first == 0xebu);
    assert(Find(EVENT_UPDATE_DATA_BLOCK, 0u)->second == 100u);
    assert(memory[0x00dd34u] == 4u);
    assert(memory[0x00dd35u] == 1u);
    assert(memory[0x00dd36u] == 0x51u);
    assert(memory[0x00dd37u] == 1u);
    assert(memory[0x00dd38u] == 0x40u);
    assert(ReadU32(0x00dd3du) == 0x96u);
    assert(ReadU32(0x00dd41u) == 300u);
    assert(ReadU32(0x00dd57u) == 16u);
    assert(memory[0x00dd60u] == 1u);
    assert(memory[0x00dd5fu] == 1u);
    assert(memory[0x00dd64u] == 1u);
    assert(ReadU32(0x00dd69u) == 100u);
    assert(ReadU32(0x00dd6du) == 201u);
    assert(ReadU32(0x00dd71u) == 300u);
    assert(ReadU32(0x00dd75u) == 400u);
    assert(memory[0x00dd79u] == 1u);
    assert(Count(EVENT_SET_BIT) == 1u);
    assert(Count(EVENT_COMMIT) == 1u);
}

static void TestNewDiscExtension(void)
{
    ResetState();
    memory[0x000c45u] = 0xc0u;

    UpdateFormatUnitTrackStateAfterSuccessfulPreparation();

    assert(Count(EVENT_CLEAR) == 2u);
    assert(Count(EVENT_COMMIT) == 2u);
    assert(memory[0x00dd7eu] == 2u);
    assert(memory[0x00dd7fu] == 2u);
    assert(ReadU32(0x00dd7au) == 4900u);
    assert(memory[0x00dd34u] == 0u);
    assert(memory[0x00dd35u] == 2u);
    assert(ReadU32(0x00dd69u) == 400u);
    assert(ReadU32(0x00dd6du) == 4900u);
}

static void TestExistingSessionTail(void)
{
    ResetState();
    memory[0x00e5fau] = 1u;
    memory[0x0012c0u] = 5u;
    memory[0x0012c9u] = 2u;

    UpdateFormatUnitTrackStateAfterSuccessfulPreparation();

    assert(Count(EVENT_TAIL) == 1u);
    assert(Find(EVENT_LOAD, 0u)->first == 5u);
    assert(memory[0x00dd35u] == 5u);
    assert(ReadU32(0x00dd3du) == 351u);
    assert(memory[0x00dd60u] == 2u);
    assert(memory[0x00dd64u] == 2u);
    assert(memory[0x00dd79u] == 1u);
    assert(Count(EVENT_COMMIT) == 1u);
}

static void TestExistingSessionInactiveTailSkipsPopulation(void)
{
    ResetState();
    memory[0x00e5fau] = 1u;
    memory[0x00dd79u] = 1u;

    UpdateFormatUnitTrackStateAfterSuccessfulPreparation();

    assert(Count(EVENT_TAIL) == 0u);
    assert(Count(EVENT_LOAD) == 0u);
    assert(Count(EVENT_UPDATE_DATA_BLOCK) == 0u);
    assert(Count(EVENT_CLEAR) == 1u);
    assert(Count(EVENT_COMMIT) == 1u);
    assert(Count(EVENT_SET_BIT) == 1u);
}

static void TestAppendExtension(void)
{
    ResetState();
    memory[0x00e5fbu] = 1u;
    memory[0x000c45u] = 0xc0u;
    memory[0x0012c0u] = 3u;

    UpdateFormatUnitTrackStateAfterSuccessfulPreparation();

    assert(Count(EVENT_TAIL) == 1u);
    assert(Find(EVENT_LOAD, 0u)->first == 3u);
    assert(ReadU32(0x00dd45u) == 400u);
    assert(ReadU32(0x00dd41u) == 300u);
    assert(memory[0x00dd7eu] == 4u);
    assert(memory[0x00dd7fu] == 4u);
    assert(memory[0x00dd79u] == 0u);
    assert(memory[0x00dd35u] == 4u);
    assert(Count(EVENT_CLEAR) == 2u);
    assert(Count(EVENT_COMMIT) == 2u);
}

static void TestFailedNewDisc(void)
{
    ResetState();

    UpdateFormatUnitTrackStateAfterFailedPreparation();

    assert(Count(EVENT_RESET_TABLE) == 1u);
    assert(Count(EVENT_UPDATE_DATA_BLOCK) == 0u);
    assert(memory[0x00dd34u] == 5u);
    assert(memory[0x00dd35u] == 1u);
    assert(ReadU32(0x00dd51u) == 0x96u);
    assert(ReadU32(0x00dd3du) == 0x96u);
    assert(memory[0x00dd36u] == 0x51u);
    assert(Count(EVENT_COMMIT) == 1u);
}

static void TestFailedExistingSessionTail(void)
{
    ResetState();
    memory[0x00e5fau] = 1u;
    memory[0x00dd7eu] = 4u;
    memory[0x0012c0u] = 5u;
    memory[0x0012c9u] = 2u;

    UpdateFormatUnitTrackStateAfterFailedPreparation();

    assert(Find(EVENT_LOAD, 0u)->first == 4u);
    assert(Count(EVENT_UPDATE_DATA_BLOCK) == 1u);
    assert(memory[0x00dd34u] == 5u);
    assert(memory[0x00dd35u] == 4u);
    assert(ReadU32(0x00dd3du) == 351u);
    assert(ReadU32(0x00dd51u) == 351u);
    assert(memory[0x00dd60u] == 2u);
    assert(memory[0x00dd64u] == 2u);
}

static void TestFailedExistingSessionSkipStillCapturesStart(void)
{
    ResetState();
    memory[0x00e5fau] = 1u;
    memory[0x00dd79u] = 1u;
    WriteU32(0x00dd3du, 777u);

    UpdateFormatUnitTrackStateAfterFailedPreparation();

    assert(Count(EVENT_TAIL) == 0u);
    assert(Count(EVENT_LOAD) == 0u);
    assert(ReadU32(0x00dd51u) == 777u);
    assert(Count(EVENT_COMMIT) == 1u);
}

static void TestFailedAppendExtension(void)
{
    ResetState();
    memory[0x00e5fbu] = 1u;
    memory[0x000c45u] = 0xc0u;
    memory[0x0012c0u] = 3u;
    WriteU32(0x00dd3du, 250u);

    UpdateFormatUnitTrackStateAfterFailedPreparation();

    assert(Find(EVENT_LOAD, 0u)->first == 3u);
    assert(memory[0x00dd34u] == 0u);
    assert(memory[0x00dd35u] == 4u);
    assert(ReadU32(0x00dd51u) == 250u);
    assert(ReadU32(0x00dd45u) == 400u);
    assert(memory[0x00dd79u] == 0u);
    assert(Count(EVENT_CLEAR) == 2u);
    assert(Count(EVENT_COMMIT) == 2u);
}

int main(void)
{
    TestNewDiscWithoutExtension();
    TestNewDiscExtension();
    TestExistingSessionTail();
    TestExistingSessionInactiveTailSkipsPopulation();
    TestAppendExtension();
    TestFailedNewDisc();
    TestFailedExistingSessionTail();
    TestFailedExistingSessionSkipStillCapturesStart();
    TestFailedAppendExtension();
    return 0;
}
