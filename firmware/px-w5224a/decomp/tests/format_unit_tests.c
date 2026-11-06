#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/format_unit.h"

enum event_kind
{
    EVENT_VALIDATE,
    EVENT_UNSUPPORTED,
    EVENT_SENSE,
    EVENT_STATUS_SENSE,
    EVENT_SEQUENCE_ERROR,
    EVENT_DEFAULTS,
    EVENT_VALIDATE_CODE,
    EVENT_VALIDATE_FLAGS,
    EVENT_NORMALIZE,
    EVENT_RECEIVE,
    EVENT_PARSE_CODE11,
    EVENT_TRACK_FLAGS,
    EVENT_RESET_RAW_READ,
    EVENT_INIT_COROUTINE,
    EVENT_YIELD,
    EVENT_ENSURE_OPC,
    EVENT_RAW_READ_PLAN,
    EVENT_UPDATE_TRACK,
    EVENT_RESET_TRACKS,
    EVENT_LOAD_TRACK,
    EVENT_GET_LAST_TRACK,
    EVENT_TAIL,
    EVENT_UPDATE_TABLE,
    EVENT_COMPLETE,
    EVENT_START_CODE11,
    EVENT_SCAN_CODE11,
    EVENT_EXECUTE_MEDIA,
    EVENT_EXACT_MULTIPLE
};

struct event_record
{
    enum event_kind kind;
    u32 first;
    u32 second;
};

static u8 memory[1u << 24];
static struct event_record events[128];
static size_t event_count;
static u8 validate_result;
static u8 validate_code_result;
static u8 validate_flags_result;
static u8 receive_result;
static u8 parse_result;
static u8 ensure_opc_result;
static u8 last_track_result;
static u8 tail_result;
static u8 exact_multiple_result;

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

u8 ObserveFormatValidateCdb(fw_addr_t mask)
{
    Record(EVENT_VALIDATE, mask, validate_result);
    return validate_result;
}
void ObserveFormatUnsupported(void)
{
    Record(EVENT_UNSUPPORTED, 0u, 0u);
}
void ObserveFormatSetSense(u8 key, u16 code)
{
    Record(EVENT_SENSE, key, code);
}
void ObserveFormatStatusSense(u8 status)
{
    Record(EVENT_STATUS_SENSE, status, 0u);
}
void ObserveFormatSequenceError(void)
{
    Record(EVENT_SEQUENCE_ERROR, 0u, 0u);
}
void ObserveFormatInitializeDefaults(void)
{
    Record(EVENT_DEFAULTS, 0u, 0u);
}
u8 ObserveFormatValidateControllerCode(void)
{
    Record(EVENT_VALIDATE_CODE, 0u, validate_code_result);
    return validate_code_result;
}
u8 ObserveFormatValidateControllerFlags(void)
{
    Record(EVENT_VALIDATE_FLAGS, 0u, validate_flags_result);
    return validate_flags_result;
}
void ObserveFormatNormalizeSpeed(void)
{
    Record(EVENT_NORMALIZE, 0u, 0u);
}
u8 ObserveFormatReceiveParameters(fw_addr_t destination, u32 count)
{
    Record(EVENT_RECEIVE, destination, count);
    return receive_result;
}
u8 ObserveFormatParseCode11(void)
{
    Record(EVENT_PARSE_CODE11, parse_result, 0u);
    return parse_result;
}
void ObserveFormatInitializeTrackFlags(void)
{
    Record(EVENT_TRACK_FLAGS, 0u, 0u);
}
void ObserveFormatResetRawRead(void)
{
    Record(EVENT_RESET_RAW_READ, 0u, 0u);
}
void ObserveFormatInitializeCoroutine2(u32 entry_point)
{
    Record(EVENT_INIT_COROUTINE, entry_point, 0u);
}
void ObserveFormatYield(void)
{
    Record(EVENT_YIELD, 0u, 0u);
}
u8 ObserveFormatEnsureOpc(void)
{
    Record(EVENT_ENSURE_OPC, ensure_opc_result, 0u);
    return ensure_opc_result;
}
void ObserveFormatExecuteRawReadPlan(void)
{
    Record(EVENT_RAW_READ_PLAN, 0u, 0u);
}
void ObserveFormatUpdateTrackState(void)
{
    Record(EVENT_UPDATE_TRACK, 0u, 0u);
}
void ObserveFormatResetTrackRecords(u8 first_index)
{
    Record(EVENT_RESET_TRACKS, first_index, 0u);
}
void ObserveFormatLoadTrack(u8 track_number)
{
    Record(EVENT_LOAD_TRACK, track_number, 0u);
}
u8 ObserveFormatGetLastTrack(void)
{
    Record(EVENT_GET_LAST_TRACK, last_track_result, 0u);
    return last_track_result;
}
u8 ObserveFormatTailUninitialized(void)
{
    Record(EVENT_TAIL, tail_result, 0u);
    return tail_result;
}
void ObserveFormatUpdateTrackTable(void)
{
    Record(EVENT_UPDATE_TABLE, 0u, 0u);
}
void ObserveFormatCompleteNoSense(void)
{
    Record(EVENT_COMPLETE, 0u, 0u);
}
void ObserveFormatStartCode11(u32 variant)
{
    Record(EVENT_START_CODE11, variant, 0u);
}
void ObserveFormatRunCode11Scan(void)
{
    Record(EVENT_SCAN_CODE11, 0u, 0u);
}
void ObserveFormatExecuteMedia(void)
{
    Record(EVENT_EXECUTE_MEDIA, 0u, 0u);
}
u8 ObserveFormatExactPacketMultiple(u32 block_count)
{
    Record(EVENT_EXACT_MULTIPLE, block_count, exact_multiple_result);
    return exact_multiple_result;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
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
    validate_result = 1u;
    validate_code_result = 1u;
    validate_flags_result = 1u;
    receive_result = 0u;
    parse_result = 0u;
    ensure_opc_result = 1u;
    last_track_result = 3u;
    tail_result = 1u;
    exact_multiple_result = 1u;
    memory[0x000faeu] = 4u;
    memory[0x000c46u] = 2u;
    memory[0x000c47u] = 5u;
    memory[0x000c48u] = 8u;
    memory[0x001008u] = 2u;
    WriteU32(0x000c4bu, 4u);
    WriteU32(0x001153u, 0x7ffffffful);
}

static void AssertSense(u8 key, u16 code)
{
    assert(Count(EVENT_SENSE) == 1u);
    assert(Find(EVENT_SENSE, 0u)->first == key);
    assert(Find(EVENT_SENSE, 0u)->second == code);
}

static void PrepareCode17(u8 flags, u32 block_count)
{
    memory[0x000b0bu] = 0x17u;
    memory[0x000b2du] = 8u;
    memory[0x000b32u] = flags;
    WriteU32(0x000b36u, block_count);
}

static void TestReadinessAndValidationGates(void)
{
    ResetState();
    memory[0x000faeu] = 7u;
    HandleFormatUnitCommand();
    assert(Count(EVENT_STATUS_SENSE) == 1u);
    assert(Find(EVENT_STATUS_SENSE, 0u)->first == 7u);

    ResetState();
    memory[0x000faeu] = 0u;
    memory[0x000c85u] = 0u;
    HandleFormatUnitCommand();
    AssertSense(2u, 0x0401u);

    ResetState();
    validate_result = 0u;
    HandleFormatUnitCommand();
    assert(Find(EVENT_VALIDATE, 0u)->first == 0xff8e6ful);
    assert(Count(EVENT_UNSUPPORTED) == 1u);

    ResetState();
    memory[0x000b0bu] = 0x12u;
    HandleFormatUnitCommand();
    assert(Count(EVENT_UNSUPPORTED) == 1u);
}

static void TestCode11ImmediateScan(void)
{
    ResetState();
    memory[0x000b0bu] = 0x11u;
    memory[0x000b2bu] = 2u;
    memory[0x000c89u] = 0x88u;
    memory[0x000c9eu] = 3u;
    parse_result = 2u;

    HandleFormatUnitCommand();

    assert(Count(EVENT_DEFAULTS) == 1u);
    assert(Find(EVENT_RECEIVE, 0u)->first == 0x000b2aul);
    assert(Find(EVENT_RECEIVE, 0u)->second == 12u);
    assert(Count(EVENT_PARSE_CODE11) == 1u);
    assert(Find(EVENT_START_CODE11, 0u)->first == 2u);
    assert(Count(EVENT_SCAN_CODE11) == 1u);
    assert(Count(EVENT_COMPLETE) == 1u);
    assert((memory[0x000c85u] & 1u) != 0u);
    assert((memory[0x000c84u] & 8u) != 0u);
}

static void TestCode17NewFirstSession(void)
{
    ResetState();
    PrepareCode17(0u, 8u);
    WriteU32(0x00114fu, 100u);

    HandleFormatUnitCommand();

    assert(Find(EVENT_RECEIVE, 0u)->second == 16u);
    assert(Count(EVENT_TRACK_FLAGS) == 1u);
    assert(Find(EVENT_RESET_TRACKS, 0u)->first == 1u);
    assert(Find(EVENT_LOAD_TRACK, 0u)->first == 1u);
    assert(ReadU32(0x00e612u) == 100u);
    assert(ReadU32(0x00e616u) == 0xfffffffful);
    assert(ReadU32(0x00e61au) == 167u);
    assert(ReadU32(0x00e61eu) == 6917u);
    assert(Find(EVENT_EXACT_MULTIPLE, 0u)->first == 8u);
    assert(Count(EVENT_EXECUTE_MEDIA) == 1u);
    assert((memory[0x000c87u] & 0x20u) == 0u);
    assert((memory[0x000c84u] & 8u) == 0u);
    assert(ReadU32(0x000ce1u) == 0u);
    assert(ReadU32(0x000ce5u) == 0u);
}

static void TestCode17OpcFailureUnwinds(void)
{
    ResetState();
    PrepareCode17(0u, 8u);
    memory[0x0012c9u] = 1u;
    ensure_opc_result = 0u;

    HandleFormatUnitCommand();

    assert(Count(EVENT_RESET_RAW_READ) == 1u);
    assert(Find(EVENT_INIT_COROUTINE, 0u)->first == 0xf84a00ul);
    assert(Count(EVENT_YIELD) == 1u);
    assert(Count(EVENT_ENSURE_OPC) == 1u);
    assert(Count(EVENT_RAW_READ_PLAN) == 0u);
    assert(Count(EVENT_EXECUTE_MEDIA) == 0u);
    assert((memory[0x000c84u] & 8u) == 0u);
}

static void TestCode17AppendValidation(void)
{
    ResetState();
    PrepareCode17(0x40u, 8u);
    memory[0x0012c9u] = 1u;
    memory[0x00dd36u] = 0x50u;
    memory[0x00dd37u] = 1u;
    memory[0x00dd56u] = 1u;
    memory[0x00dd79u] = 1u;
    WriteU32(0x00dd57u, 4u);
    WriteU32(0x00114fu, 200u);
    WriteU32(0x00dd41u, 0u);

    HandleFormatUnitCommand();

    assert(Count(EVENT_GET_LAST_TRACK) == 2u);
    assert(Count(EVENT_LOAD_TRACK) == 2u);
    assert(Count(EVENT_TAIL) == 0u);
    assert(ReadU32(0x00e612u) == 200u);
    assert(ReadU32(0x00e61au) == 167u);
    assert(Count(EVENT_EXECUTE_MEDIA) == 1u);
}

static void TestCode17ParameterMaskFailure(void)
{
    ResetState();
    PrepareCode17(0u, 8u);
    memory[0x000b2au] = 0x80u;
    memory[0xf78e83u] = 0x80u;

    HandleFormatUnitCommand();

    AssertSense(5u, 0x2600u);
    assert(Count(EVENT_TRACK_FLAGS) == 0u);
    assert(Count(EVENT_EXECUTE_MEDIA) == 0u);
}

int main(void)
{
    TestReadinessAndValidationGates();
    TestCode11ImmediateScan();
    TestCode17NewFirstSession();
    TestCode17OpcFailureUnwinds();
    TestCode17AppendValidation();
    TestCode17ParameterMaskFailure();
    return 0;
}
