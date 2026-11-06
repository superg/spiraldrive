#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/blank.h"

enum Event
{
    EVENT_NORMALIZE = 1,
    EVENT_INIT_COROUTINE1,
    EVENT_RESET_RAW_READ,
    EVENT_INIT_COROUTINE2,
    EVENT_YIELD,
    EVENT_ENSURE_OPC,
    EVENT_EXECUTE_PLAN,
    EVENT_UPDATE_TRACK,
    EVENT_RESET_TYPE2,
    EVENT_NO_SENSE,
    EVENT_COMPLETE_NO_SENSE
};

static u8 memory[1u << 24];
static u8 validate_code;
static u8 validate_flags;
static u8 validate_cdb;
static fw_addr_t validate_mask;
static u8 tail_uninitialized;
static u8 last_track;
static u8 found_track;
static u8 ensure_opc;
static u8 execute_plan;
static unsigned unsupported_calls;
static unsigned sequence_error_calls;
static unsigned status_sense_calls;
static u8 status_sense_value;
static unsigned sense_calls;
static u8 sense_key;
static u16 sense_code;
static unsigned load_count;
static u8 loaded_tracks[8];
static s32 find_frame;
static u32 coroutine1_entry;
static u32 coroutine2_entry;
static unsigned events[32];
static unsigned event_count;

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

u8 ObserveBlankValidateCode(void)
{
    return validate_code;
}
u8 ObserveBlankValidateFlags(void)
{
    return validate_flags;
}
u8 ObserveBlankValidateCdb(fw_addr_t mask)
{
    validate_mask = mask;
    return validate_cdb;
}
void ObserveBlankUnsupported(void)
{
    ++unsupported_calls;
}
void ObserveBlankSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}
void ObserveBlankStatusSense(u8 status)
{
    ++status_sense_calls;
    status_sense_value = status;
}
void ObserveBlankSequenceError(void)
{
    ++sequence_error_calls;
}
void ObserveBlankNormalizeSpeed(void)
{
    Event(EVENT_NORMALIZE);
}
void ObserveBlankNoSense(void)
{
    Event(EVENT_NO_SENSE);
}
void ObserveBlankCompleteNoSense(void)
{
    Event(EVENT_COMPLETE_NO_SENSE);
}
u8 ObserveBlankLastTrack(void)
{
    return last_track;
}
u8 ObserveBlankTailUninitialized(void)
{
    return tail_uninitialized;
}
void ObserveBlankLoadTrack(u8 track)
{
    loaded_tracks[load_count++] = track;
}
u8 ObserveBlankFindTrack(s32 frame_position)
{
    find_frame = frame_position;
    return found_track;
}
void ObserveBlankInitializeCoroutine1(u32 entry)
{
    coroutine1_entry = entry;
    Event(EVENT_INIT_COROUTINE1);
}
void ObserveBlankResetRawRead(void)
{
    Event(EVENT_RESET_RAW_READ);
}
void ObserveBlankInitializeCoroutine2(u32 entry)
{
    coroutine2_entry = entry;
    Event(EVENT_INIT_COROUTINE2);
}
void ObserveBlankYield(void)
{
    Event(EVENT_YIELD);
}
u8 ObserveBlankEnsureOpc(void)
{
    Event(EVENT_ENSURE_OPC);
    return ensure_opc;
}
u8 ObserveBlankExecutePlan(void)
{
    Event(EVENT_EXECUTE_PLAN);
    return execute_plan;
}
void ObserveBlankUpdateTrackState(void)
{
    Event(EVENT_UPDATE_TRACK);
}
void ObserveBlankResetType2Records(void)
{
    Event(EVENT_RESET_TYPE2);
}
s32 ObserveBlankConvertMsf(u8 minute, u8 second, u8 frame)
{
    return (s32)(((u32)minute * 60ul + second) * 75ul + frame);
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

static u32 ReadBe32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    memset(loaded_tracks, 0, sizeof(loaded_tracks));
    validate_code = 1u;
    validate_flags = 1u;
    validate_cdb = 1u;
    validate_mask = 0u;
    tail_uninitialized = 0u;
    last_track = 7u;
    found_track = 3u;
    ensure_opc = 1u;
    execute_plan = 1u;
    unsupported_calls = 0u;
    sequence_error_calls = 0u;
    status_sense_calls = 0u;
    status_sense_value = 0u;
    sense_calls = 0u;
    sense_key = 0u;
    sense_code = 0u;
    load_count = 0u;
    find_frame = 0;
    coroutine1_entry = 0u;
    coroutine2_entry = 0u;
    event_count = 0u;
    memory[0x001008u] = 2u;
    memory[0x000c85u] = 2u;
    WriteBe32(0x00114fu, 10000ul);
    WriteBe32(0x001153u, 20000ul);
}

static void AssertSense(u8 key, u16 code)
{
    assert(sense_calls == 1u && sense_key == key && sense_code == code);
}

static void TestAdmissionFailures(void)
{
    ResetState();
    memory[0x001008u] = 1u;
    HandleBlankCommand();
    AssertSense(2u, 0x3006u);

    ResetState();
    validate_code = 0u;
    HandleBlankCommand();
    assert(event_count == 0u && unsupported_calls == 0u);

    ResetState();
    validate_flags = 0u;
    HandleBlankCommand();
    assert(event_count == 0u);

    ResetState();
    memory[0x000c46u] = 1u;
    HandleBlankCommand();
    assert(sequence_error_calls == 1u && event_count == 0u);

    ResetState();
    validate_cdb = 0u;
    HandleBlankCommand();
    assert(validate_mask == 0x00ff8e76ul && unsupported_calls == 1u);

    ResetState();
    memory[0x000b0bu] = 7u;
    HandleBlankCommand();
    assert(unsupported_calls == 1u && events[0] == EVENT_NORMALIZE);

    ResetState();
    memory[0x000faeu] = 3u;
    HandleBlankCommand();
    assert(status_sense_calls == 1u && status_sense_value == 3u);
}

static void TestTypeZeroSuccessAndImmediate(void)
{
    ResetState();
    memory[0x000b0bu] = 0x10u;
    memory[0x000c84u] = 0x20u;
    memory[0x000c86u] = 0xffu;
    memory[0x000c9eu] = 1u;
    HandleBlankCommand();
    assert(ReadBe32(0x00e606u) == 22250ul);
    assert(ReadBe32(0x00e60au) == 13250ul);
    assert(ReadBe32(0x001455u) == 0ul);
    assert((memory[0x000c86u] & 2u) == 0u);
    assert((memory[0x000c84u] & 8u) == 0u);
    assert((memory[0x000c85u] & 1u) == 0u);
    assert(memory[0x00e622u] == 0u);
    assert(coroutine2_entry == 0x00f84a00ul);
    assert(events[0] == EVENT_NORMALIZE);
    assert(events[1] == EVENT_COMPLETE_NO_SENSE);
    assert(events[2] == EVENT_RESET_RAW_READ);
    assert(events[3] == EVENT_INIT_COROUTINE2);
    assert(events[4] == EVENT_YIELD);
    assert(events[5] == EVENT_ENSURE_OPC);
    assert(events[6] == EVENT_EXECUTE_PLAN);
    assert(events[7] == EVENT_UPDATE_TRACK);
    assert(events[8] == EVENT_RESET_TYPE2);
    assert(events[9] == EVENT_NO_SENSE);
}

static void TestStateFourRestart(void)
{
    ResetState();
    memory[0x000faeu] = 4u;
    memory[0x001009u] = 9u;
    HandleBlankCommand();
    assert(memory[0x001009u] == 0u && memory[0x000fafu] == 3u);
    assert(memory[0x000faeu] == 0u && memory[0x00e600u] == 0u);
    assert(ReadBe32(0x000e3bu) == 150ul && memory[0x000e3fu] == 1u);
    assert(coroutine1_entry == 0x00f85d74ul);
    assert(events[1] == EVENT_INIT_COROUTINE1 && events[2] == EVENT_YIELD);
}

static void TestTrackBlankTypes(void)
{
    ResetState();
    memory[0x000b0bu] = 2u;
    memory[0x000b0fu] = 5u;
    memory[0x00dd7eu] = 4u;
    memory[0x00dd36u] = 0x50u;
    memory[0x00dd34u] = 1u;
    WriteBe32(0x00dd4du, 12345ul);
    HandleBlankCommand();
    assert(memory[0x00e601u] == 5u && load_count == 1u);
    assert(loaded_tracks[0] == 5u && ReadBe32(0x00dd41u) == 12345ul);

    ResetState();
    memory[0x000b0bu] = 2u;
    memory[0x000b0fu] = 8u;
    HandleBlankCommand();
    assert(unsupported_calls == 1u);

    ResetState();
    memory[0x000b0bu] = 3u;
    memory[0x00dd79u] = 1u;
    HandleBlankCommand();
    AssertSense(5u, 0x2700u);
}

static void TestTypeFourPacketMapping(void)
{
    ResetState();
    memory[0x000b0bu] = 4u;
    WriteBe32(0x000b0cu, 1200ul);
    memory[0x00dd36u] = 0x50u;
    memory[0x00dd56u] = 1u;
    memory[0x00dd34u] = 1u;
    WriteBe32(0x00dd3du, 1150ul);
    WriteBe32(0x00dd57u, 100ul);
    WriteBe32(0x00dd4du, 1400ul);
    HandleBlankCommand();
    assert(find_frame == 1350 && memory[0x00e601u] == 3u);
    assert(ReadBe32(0x00e602u) == 1214ul);
    assert(ReadBe32(0x00dd41u) == 1400ul);

    ResetState();
    memory[0x000b0bu] = 4u;
    WriteBe32(0x000b0cu, 1000ul);
    memory[0x00dd36u] = 0x40u;
    HandleBlankCommand();
    AssertSense(5u, 0x6400u);
}

static void SetDescriptor(u16 index, u8 session, u8 point, u8 minute, u8 second, u8 frame, u8 msf_offset)
{
    fw_addr_t descriptor = 0x002b8000ul + (fw_addr_t)index * 11ul;
    memory[descriptor] = session;
    memory[descriptor + 3u] = point;
    memory[descriptor + msf_offset] = minute;
    memory[descriptor + msf_offset + 1u] = second;
    memory[descriptor + msf_offset + 2u] = frame;
}

static void TestSessionBlankTypes(void)
{
    ResetState();
    memory[0x000b0bu] = 5u;
    memory[0x00dd79u] = 1u;
    memory[0x0012c9u] = 2u;
    WriteBe16(0x0012cau, 22u);
    SetDescriptor(1u, 2u, 0xa2u, 1u, 0u, 0u, 8u);
    SetDescriptor(2u, 1u, 0xb0u, 0u, 30u, 0u, 4u);
    HandleBlankCommand();
    assert(memory[0x00dd63u] == 2u);
    assert(ReadBe32(0x00e61au) == 4500ul);
    assert(ReadBe32(0x00e616u) == 2250ul);
    assert(ReadBe32(0x00e612u) == (u32)(2250ul - 4500ul));
    assert(ReadBe32(0x00dd75u) == 6750ul);
    assert(ReadBe32(0x00e61eu) == 6750ul);

    ResetState();
    memory[0x000b0bu] = 6u;
    memory[0x00dd79u] = 0u;
    tail_uninitialized = 0u;
    memory[0x0012c9u] = 0u;
    memory[0x00dd36u] = 0x50u;
    memory[0x00dd34u] = 1u;
    WriteBe32(0x00dd4du, 9000ul);
    HandleBlankCommand();
    assert(memory[0x00dd63u] == 1u);
    assert(ReadBe32(0x00e61au) == 9000ul);
    assert(ReadBe32(0x00e612u) == 10000ul);
    assert(ReadBe32(0x00dd75u) == 15750ul);
    assert(ReadBe32(0x00e61eu) == 15750ul);

    ResetState();
    memory[0x000b0bu] = 6u;
    tail_uninitialized = 1u;
    memory[0x0012c9u] = 0u;
    HandleBlankCommand();
    assert(events[1] == EVENT_NO_SENSE && event_count == 2u);
}

static void TestExecutionFailures(void)
{
    ResetState();
    ensure_opc = 0u;
    HandleBlankCommand();
    assert((memory[0x000c84u] & 8u) == 0u);
    assert(event_count == 5u && events[4] == EVENT_ENSURE_OPC);

    ResetState();
    memory[0x000b0bu] = 0x10u;
    execute_plan = 0u;
    HandleBlankCommand();
    assert((memory[0x000c85u] & 8u) != 0u);
    assert((memory[0x000c85u] & 1u) == 0u);
    assert(memory[0x00e622u] == 0u);
    assert(events[event_count - 1u] == EVENT_UPDATE_TRACK);
}

int main(void)
{
    TestAdmissionFailures();
    TestTypeZeroSuccessAndImmediate();
    TestStateFourRestart();
    TestTrackBlankTypes();
    TestTypeFourPacketMapping();
    TestSessionBlankTypes();
    TestExecutionFailures();
    return 0;
}
