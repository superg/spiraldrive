#include <assert.h>
#include <string.h>
#include "media/blank_format_track_state.h"
#include "platform/firmware_memory.h"

enum Event
{
    EVENT_CLEAR_WORKING = 1,
    EVENT_RESET_TABLE,
    EVENT_LOAD,
    EVENT_CLEAR_RECORD,
    EVENT_RESET_RECORDS,
    EVENT_COMMIT
};

static u8 memory[1u << 24];
static unsigned events[32];
static unsigned event_count;
static u8 loaded[8];
static unsigned load_count;
static u8 cleared_record;
static u8 reset_first;
static u8 last_track;
static u8 tail_uninitialized;
static s32 converted_frame;
static fw_addr_t converted_address;
static unsigned find_entry_calls;
static u8 find_entry_args[4];
static unsigned find_last_calls;
static u8 find_last_arg;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void Event(unsigned event)
{
    events[event_count++] = event;
}

void ObserveBlankTrackClearWorking(void)
{
    Event(EVENT_CLEAR_WORKING);
}
void ObserveBlankTrackResetTable(void)
{
    Event(EVENT_RESET_TABLE);
}
void ObserveBlankTrackLoad(u8 track)
{
    Event(EVENT_LOAD);
    loaded[load_count++] = track;
}
s32 ObserveBlankTrackConvertMsf(fw_addr_t address)
{
    converted_address = address;
    return converted_frame;
}
u8 ObserveBlankTrackLastTrack(void)
{
    return last_track;
}
void ObserveBlankTrackClearRecord(u8 index)
{
    Event(EVENT_CLEAR_RECORD);
    cleared_record = index;
}
u8 ObserveBlankTrackFindEntry(u8 track_number)
{
    find_entry_args[find_entry_calls++] = track_number;
    return (u8)(track_number + 10u);
}
u8 ObserveBlankTrackFindLastForSession(u8 session)
{
    ++find_last_calls;
    find_last_arg = session;
    return (u8)(session + 20u);
}
u8 ObserveBlankTrackTailUninitialized(void)
{
    return tail_uninitialized;
}
void ObserveBlankTrackResetRecords(u8 first_index)
{
    Event(EVENT_RESET_RECORDS);
    reset_first = first_index;
}
void ObserveBlankTrackCommit(void)
{
    Event(EVENT_COMMIT);
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadBe32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(loaded, 0, sizeof(loaded));
    memset(find_entry_args, 0, sizeof(find_entry_args));
    event_count = 0u;
    load_count = 0u;
    cleared_record = 0u;
    reset_first = 0u;
    last_track = 9u;
    tail_uninitialized = 0u;
    converted_frame = 5000;
    converted_address = 0u;
    find_entry_calls = 0u;
    find_last_calls = 0u;
    find_last_arg = 0u;
    WriteBe32(0x00114fu, 1234ul);
}

static void AssertFinalLoad(u8 high)
{
    assert(events[0] == EVENT_CLEAR_WORKING);
    assert(events[event_count - 2u] == EVENT_COMMIT);
    assert(events[event_count - 1u] == EVENT_LOAD);
    assert(loaded[load_count - 1u] == high);
}

static void TestEmptyInitialization(void)
{
    u8 operation;
    for(operation = 0u; operation <= 1u; ++operation)
    {
        ResetState();
        memory[0x00e600u] = operation;
        memset(&memory[0x00dd34u], 0xff, 0x4cu);
        UpdateTrackStateAfterBlankOrFormatOperation();
        assert(memory[0x00dd34u] == 0u && memory[0x00dd35u] == 1u);
        assert(memory[0x00dd36u] == 0x41u);
        assert(ReadBe32(0x00dd3du) == 0u && ReadBe32(0x00dd41u) == 0u);
        assert(ReadBe32(0x00dd69u) == 1234ul);
        assert(memory[0x00dd56u] == 0u && memory[0x00dd5fu] == 0u);
        assert(memory[0x00dd60u] == 1u && memory[0x00dd64u] == 1u);
        assert(memory[0x00dd79u] == 0u);
        assert(memory[0x00dd7eu] == 1u && memory[0x00dd7fu] == 1u);
        assert(events[1] == EVENT_RESET_TABLE);
        AssertFinalLoad(1u);
    }
}

static void TestType2Routes(void)
{
    ResetState();
    memory[0x00e600u] = 2u;
    memory[0x00e601u] = 5u;
    memory[0x00dd7fu] = 5u;
    memory[0x00dd36u] = 0x50u;
    memory[0x00dd34u] = 4u;
    WriteBe32(0x00dd3du, 1000ul);
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd34u] == 3u && memory[0x00dd7fu] == 4u);
    assert(ReadBe32(0x00dd45u) == 995ul);
    assert(loaded[0] == 5u);
    AssertFinalLoad(4u);

    ResetState();
    memory[0x00e600u] = 2u;
    memory[0x00e601u] = 5u;
    memory[0x00dd7fu] = 7u;
    memory[0x00dd36u] = 0x50u;
    converted_frame = 8000;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd34u] == 3u);
    assert(converted_address == 0x00dde0ul);
    assert(ReadBe32(0x00dd45u) == 7995ul);

    ResetState();
    memory[0x00e600u] = 2u;
    memory[0x00e601u] = 5u;
    memory[0x00dd7fu] = 7u;
    memory[0x00dd36u] = 0x40u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd34u] == 2u && converted_address == 0u);
}

static void TestTypes3And4(void)
{
    ResetState();
    memory[0x00e600u] = 3u;
    memory[0x00dd7fu] = 4u;
    memory[0x00dd87u + 4u * 19u] = 2u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd7fu] == 3u);
    assert(cleared_record == 9u);
    AssertFinalLoad(3u);

    ResetState();
    memory[0x00e600u] = 4u;
    memory[0x00e601u] = 6u;
    memory[0x00dd34u] = 2u;
    memory[0x00dd7fu] = 8u;
    WriteBe32(0x00e602u, 2000ul);
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(loaded[0] == 6u && memory[0x00dd34u] == 3u);
    assert(ReadBe32(0x00dd45u) == 2145ul);
    AssertFinalLoad(8u);
}

static void TestType5(void)
{
    ResetState();
    memory[0x00e600u] = 5u;
    memory[0x00dd64u] = 4u;
    memory[0x00dd79u] = 1u;
    WriteBe32(0x00e612u, 100ul);
    WriteBe32(0x00e616u, 200ul);
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd64u] == 3u && memory[0x00dd79u] == 0u);
    assert(ReadBe32(0x00dd69u) == 100ul && ReadBe32(0x00dd6du) == 200ul);
    assert(memory[0x00dd7eu] == 13u && memory[0x00dd7fu] == 24u);
    assert(find_entry_args[0] == 3u && find_last_arg == 3u);
    AssertFinalLoad(24u);
}

static void TestType6Routes(void)
{
    ResetState();
    memory[0x00e600u] = 6u;
    memory[0x00dd79u] = 1u;
    memory[0x00dd64u] = 3u;
    WriteBe32(0x00e612u, 100u);
    WriteBe32(0x00e616u, 200u);
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd79u] == 0u && memory[0x00dd34u] == 0u);
    assert(memory[0x00dd7eu] == 13u && memory[0x00dd7fu] == 13u);
    assert(find_entry_calls == 2u && reset_first == 13u);
    AssertFinalLoad(13u);

    ResetState();
    memory[0x00e600u] = 6u;
    memory[0x00dd64u] = 4u;
    tail_uninitialized = 1u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(memory[0x00dd64u] == 3u);
    assert(memory[0x00dd7eu] == 13u && memory[0x00dd7fu] == 13u);

    ResetState();
    memory[0x00e600u] = 6u;
    memory[0x00dd64u] = 2u;
    tail_uninitialized = 1u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(events[1] == EVENT_RESET_TABLE);
    assert(memory[0x00dd64u] == 1u && memory[0x00dd7fu] == 1u);

    ResetState();
    memory[0x00e600u] = 6u;
    memory[0x00dd7eu] = 7u;
    tail_uninitialized = 0u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(reset_first == 7u && memory[0x00dd7fu] == 7u);
    AssertFinalLoad(7u);
}

static void TestUnknownOperationStillFinalizes(void)
{
    ResetState();
    memory[0x00e600u] = 9u;
    memory[0x00dd7fu] = 12u;
    UpdateTrackStateAfterBlankOrFormatOperation();
    assert(event_count == 3u);
    AssertFinalLoad(12u);
}

int main(void)
{
    TestEmptyInitialization();
    TestType2Routes();
    TestTypes3And4();
    TestType5();
    TestType6Routes();
    TestUnknownOperationStillFinalizes();
    return 0;
}
