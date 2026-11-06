#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_command_range.h"

static u8 memory[1u << 24];
static u8 validate_parameters_result;
static u8 validate_media_result;
static u8 validate_track_result;
static u8 find_track_result;
static u8 load_snapshot_result;
static u8 initialize_high_track_result;
static u8 validate_type_result;
static u8 validate_track_state_result;
static u8 validate_pma_result;
static u8 ensure_opc_result;
static u8 build_raw_q_result;
static u8 acquire_zero_result;
static u8 reconcile_result;
static u8 validate_extent_result;
static u8 initialize_pipeline_result;
static u8 lowest_speed_result;
static u8 reconcile_sets_continuation;
static u32 mapped_address;
static s32 packet_progress;
static unsigned normalize_calls;
static unsigned sequence_error_calls;
static unsigned controller_calls;
static unsigned clear_track_calls;
static unsigned unsupported_calls;
static unsigned reset_calls;
static unsigned coroutine_calls;
static unsigned yield_calls;
static unsigned sense_calls;
static unsigned no_sense_calls;
static unsigned pipeline_calls;
static unsigned advance_type1_calls;
static unsigned advance_after_calls;
static unsigned failure_calls;
static u8 last_sense_key;
static u16 last_asc_ascq;
static u8 last_extent_flag;
static u32 last_extent_count;
static u8 last_pipeline_flag;
static s32 last_advance_start;
static u32 last_advance_count;
static u8 last_advance_flag;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
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
    return (u32)memory[address] << 24 | (u32)memory[address + 1u] << 16 | (u32)memory[address + 2u] << 8 | memory[address + 3u];
}

void ObserveWriteRangeNormalizeSpeed(void)
{
    normalize_calls++;
}
u8 ObserveWriteRangeValidateParameters(void)
{
    return validate_parameters_result;
}
u8 ObserveWriteRangeValidateMedia(void)
{
    return validate_media_result;
}
void ObserveWriteRangeSequenceError(void)
{
    sequence_error_calls++;
}
void ObserveWriteRangeCopyBytes16(u16 destination, u16 source, u8 count)
{
    assert(count == 8u);
    (void)destination;
    (void)source;
}
void ObserveWriteRangeSetControllerEnable(u8 enabled)
{
    assert(enabled <= 1u);
    controller_calls++;
    memory[0x000c87u] = (u8)((memory[0x000c87u] & 0xfeu) | enabled);
}
void ObserveWriteRangeClearWorkingTrack(void)
{
    clear_track_calls++;
}
u8 ObserveWriteRangeValidateTrackTable(s32 frame_position)
{
    (void)frame_position;
    return validate_track_result;
}
u8 ObserveWriteRangeFindWritableTrack(s32 frame_position)
{
    (void)frame_position;
    return find_track_result;
}
u8 ObserveWriteRangeLoadSnapshot(u8 track_number)
{
    (void)track_number;
    return load_snapshot_result;
}
u8 ObserveWriteRangeInitializeHighTrack(void)
{
    return initialize_high_track_result;
}
void ObserveWriteRangeUnsupported(void)
{
    unsupported_calls++;
}
void ObserveWriteRangeSelectAlternateCursor(void) {}
void ObserveWriteRangeSelectCursor(void) {}
u8 ObserveWriteRangeValidateWriteType(void)
{
    return validate_type_result;
}
u8 ObserveWriteRangeValidateTrackState(void)
{
    return validate_track_state_result;
}
u8 ObserveWriteRangeValidatePma(void)
{
    return validate_pma_result;
}
u32 ObserveWriteRangeMapAddress(u32 logical_address)
{
    (void)logical_address;
    return mapped_address;
}
void ObserveWriteRangeResetRawRead(void)
{
    reset_calls++;
}
void ObserveWriteRangeInitializeCoroutine2(u32 entry_point)
{
    assert(entry_point == 0x00f84a00ul);
    coroutine_calls++;
}
void ObserveWriteRangeYield(void)
{
    yield_calls++;
}
void ObserveWriteRangeSetSense(u8 sense_key, u16 asc_ascq)
{
    sense_calls++;
    last_sense_key = sense_key;
    last_asc_ascq = asc_ascq;
}
u8 ObserveWriteRangeEnsureOpc(void)
{
    return ensure_opc_result;
}
u8 ObserveWriteRangeBuildRawQ(void)
{
    return build_raw_q_result;
}
u8 ObserveWriteRangeAcquireZeroRange(u32 caller_target)
{
    (void)caller_target;
    return acquire_zero_result;
}
void ObserveWriteRangeNoSense(void)
{
    no_sense_calls++;
}
u8 ObserveWriteRangeReconcileStart(s32 start_lba)
{
    (void)start_lba;
    if(reconcile_sets_continuation != 0u)
        memory[0x000c84u] |= 0x10u;
    return reconcile_result;
}
s32 ObserveWriteRangeSelectPacketProgress(void)
{
    return packet_progress;
}
u8 ObserveWriteRangeValidateExtent(u8 continuation, u32 count)
{
    last_extent_flag = continuation;
    last_extent_count = count;
    return validate_extent_result;
}
u8 ObserveWriteRangeInitializePipeline(u8 continuation)
{
    pipeline_calls++;
    last_pipeline_flag = continuation;
    return initialize_pipeline_result;
}
u8 ObserveWriteRangeSelectLowestSpeed(void)
{
    return lowest_speed_result;
}
void ObserveWriteRangeAdvanceType1(u32 count, u8 continuation)
{
    advance_type1_calls++;
    last_advance_count = count;
    last_advance_flag = continuation;
}
void ObserveWriteRangeAdvanceAfterTransfer(s32 start_lba, u32 count, u8 continuation)
{
    advance_after_calls++;
    last_advance_start = start_lba;
    last_advance_count = count;
    last_advance_flag = continuation;
}
void ObserveWriteRangeUpdateFailure(void)
{
    failure_calls++;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    validate_parameters_result = validate_media_result = 1u;
    validate_track_result = find_track_result = 1u;
    load_snapshot_result = initialize_high_track_result = 1u;
    validate_type_result = validate_track_state_result = validate_pma_result = 1u;
    ensure_opc_result = build_raw_q_result = acquire_zero_result = 1u;
    reconcile_result = validate_extent_result = initialize_pipeline_result = 1u;
    lowest_speed_result = 12u;
    reconcile_sets_continuation = 0u;
    mapped_address = 0x12345678ul;
    packet_progress = 0x10203040l;
    normalize_calls = sequence_error_calls = controller_calls = 0u;
    clear_track_calls = unsupported_calls = reset_calls = coroutine_calls = 0u;
    yield_calls = sense_calls = no_sense_calls = pipeline_calls = 0u;
    advance_type1_calls = advance_after_calls = failure_calls = 0u;
    last_sense_key = last_extent_flag = last_pipeline_flag = 0u;
    last_asc_ascq = 0u;
    last_extent_count = last_advance_count = 0ul;
    last_advance_start = 0l;
    last_advance_flag = 0u;
    WriteBe32(0x001153u, 0x00080000ul);
}

static void TestEarlyValidationStops(void)
{
    ResetState();
    validate_parameters_result = 0u;
    ExecuteWriteCommandRange(100l, 5ul);
    assert(normalize_calls == 1u && reset_calls == 0u);
    assert(pipeline_calls == 0u && sense_calls == 0u);
}

static void TestPacketOptionSequenceError(void)
{
    ResetState();
    memory[0x000c44u] = 0u;
    memory[0x000c46u] = 1u;
    ExecuteWriteCommandRange(100l, 5ul);
    assert(sequence_error_calls == 1u && reset_calls == 0u);
}

static void TestNewZeroLengthCompletion(void)
{
    ResetState();
    memory[0x000c44u] = 2u;
    memory[0x000c46u] = 1u;
    memory[0x000c87u] = 1u;
    ExecuteWriteCommandRange(100l, 0ul);
    assert(reset_calls == 1u && coroutine_calls == 1u && yield_calls == 1u);
    assert(no_sense_calls == 1u && pipeline_calls == 0u);
    assert(controller_calls == 1u && (memory[0x000c87u] & 1u) == 0u);
    assert(ReadBe32(0x000b02u) == mapped_address);
    assert((memory[0x000c84u] & 8u) == 0u && (memory[0x000c84u] & 0x10u) != 0u);
}

static void TestContinuationZeroLength(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    ExecuteWriteCommandRange(77l, 0ul);
    assert(no_sense_calls == 1u && normalize_calls == 0u);
    assert(pipeline_calls == 0u);
}

static void TestExtentFailure(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x00128cu] = 1u;
    validate_extent_result = 0u;
    ExecuteWriteCommandRange(44l, 7ul);
    assert(last_extent_count == 7ul && last_extent_flag == 0u);
    assert(sense_calls == 1u && last_sense_key == 5u && last_asc_ascq == 0x2400u);
    assert(memory[0x000c3au] == 0x24u && memory[0x000c3bu] == 0u);
    assert((memory[0x000c84u] & 0x80u) != 0u);
}

static void TestPipelineSuccessAndFailure(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 0u;
    memory[0x000e51u] = 9u;
    memory[0x000c7du] = 16u;
    reconcile_sets_continuation = 1u;
    ExecuteWriteCommandRange(55l, 6ul);
    assert(pipeline_calls == 1u && last_pipeline_flag == 1u);
    assert(advance_after_calls == 1u && last_advance_start == 55l && last_advance_count == 6ul && last_advance_flag == 1u);
    assert(memory[0x000c7cu] == 9u && memory[0x000c7au] == 12u && memory[0x000c7bu] == 16u);

    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c85u] = 4u;
    initialize_pipeline_result = 0u;
    ExecuteWriteCommandRange(22l, 3ul);
    assert(failure_calls == 1u && advance_after_calls == 0u);
    assert((memory[0x000c84u] & 8u) == 0u && (memory[0x000c85u] & 4u) == 0u);
}

static void TestNewType3RangeAndInitializationError(void)
{
    ResetState();
    memory[0x000c44u] = 3u;
    WriteBe32(0x00dd69u, 1150ul);
    WriteBe32(0x00dd65u, 1150ul);
    ExecuteWriteCommandRange(1000l, 4ul);
    assert(controller_calls == 1u && (memory[0x000c87u] & 1u) != 0u);
    assert(pipeline_calls == 1u && last_pipeline_flag == 1u);
    assert(ReadBe32(0x00dd65u) == 1154ul);

    ResetState();
    memory[0x000c44u] = 2u;
    memory[0x000c46u] = 1u;
    memory[0x000faeu] = 1u;
    memory[0xf84455u] = 3u;
    memory[0xf8446au] = 0x73u;
    memory[0xf8446bu] = 0x04u;
    ExecuteWriteCommandRange(200l, 5ul);
    assert(sense_calls == 1u && last_sense_key == 3u && last_asc_ascq == 0x7304u);
    assert(pipeline_calls == 0u && (memory[0x000c84u] & 8u) == 0u);
}

static void TestType1CursorAdvance(void)
{
    ResetState();
    memory[0x000c84u] = 8u;
    memory[0x000c44u] = 1u;
    reconcile_sets_continuation = 1u;
    ExecuteWriteCommandRange(33l, 9ul);
    assert(advance_type1_calls == 1u && last_advance_count == 9ul && last_advance_flag == 1u);
    assert(advance_after_calls == 0u);
}

int main(void)
{
    TestEarlyValidationStops();
    TestPacketOptionSequenceError();
    TestNewZeroLengthCompletion();
    TestContinuationZeroLength();
    TestExtentFailure();
    TestPipelineSuccessAndFailure();
    TestNewType3RangeAndInitializationError();
    TestType1CursorAdvance();
    return 0;
}
