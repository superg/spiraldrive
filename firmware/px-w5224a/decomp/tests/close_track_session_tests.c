#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/close_track_session.h"

static u8 memory[1u << 24];
static u8 inherited_b;
static u8 validate_cdb_result;
static u8 finalize_active_result;
static u8 prepare_result;
static u8 validate_media_result;
static u8 validate_session_result;
static u8 load_result;
static u8 validate_write_result;
static u8 validate_pma_result;
static u8 loaded_working_type;
static u8 mutate_parameter_on_session_complete;
static fw_addr_t cdb_mask;
static u8 normalize_count;
static u8 finalize_active_count;
static u8 finalize_existing_count;
static u8 complete_no_sense_count;
static u8 reset_count;
static u8 init_coroutine_count;
static u32 coroutine_entry;
static u8 yield_count;
static u8 prepare_count;
static u8 update_failure_count;
static u8 unsupported_count;
static u8 validate_media_count;
static u8 validate_session_count;
static u8 update_tail_count;
static u8 clear_working_count;
static u8 load_count;
static u8 loaded_track;
static u8 loaded_b;
static u8 no_sense_count;
static u8 sequence_error_count;
static u8 validate_write_count;
static u8 validate_pma_count;
static u8 complete_session_count;
static u8 execute_operation_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 ObserveCloseTrackCaptureInheritedB(void)
{
    return inherited_b;
}

u8 ObserveCloseTrackValidateCdb(fw_addr_t mask)
{
    cdb_mask = mask;
    return validate_cdb_result;
}

void ObserveCloseTrackNormalizeSpeed(void)
{
    normalize_count++;
}

u8 ObserveCloseTrackFinalizeActiveWrite(void)
{
    finalize_active_count++;
    return finalize_active_result;
}

void ObserveCloseTrackFinalizeExistingState(void)
{
    finalize_existing_count++;
}

void ObserveCloseTrackCompleteNoSense(void)
{
    complete_no_sense_count++;
}

void ObserveCloseTrackResetRawRead(void)
{
    reset_count++;
}

void ObserveCloseTrackInitializeCoroutine2(u32 entry_point)
{
    init_coroutine_count++;
    coroutine_entry = entry_point;
}

void ObserveCloseTrackYield(void)
{
    yield_count++;
}

u8 ObserveCloseTrackPrepareControllerState(void)
{
    prepare_count++;
    return prepare_result;
}

void ObserveCloseTrackUpdateFailure(void)
{
    update_failure_count++;
}

void ObserveCloseTrackUnsupported(void)
{
    unsupported_count++;
}

u8 ObserveCloseTrackValidateMedia(void)
{
    validate_media_count++;
    return validate_media_result;
}

u8 ObserveCloseTrackValidateSessionState(void)
{
    validate_session_count++;
    return validate_session_result;
}

void ObserveCloseTrackUpdateTailBounds(void)
{
    update_tail_count++;
}

void ObserveCloseTrackClearWorking(void)
{
    clear_working_count++;
}

u8 ObserveCloseTrackLoadTrack(u8 track_number, u8 captured_b)
{
    load_count++;
    loaded_track = track_number;
    loaded_b = captured_b;
    memory[0x00dd34u] = loaded_working_type;
    return load_result;
}

void ObserveCloseTrackNoSense(void)
{
    no_sense_count++;
}

void ObserveCloseTrackSequenceError(void)
{
    sequence_error_count++;
}

u8 ObserveCloseTrackValidateWriteState(void)
{
    validate_write_count++;
    return validate_write_result;
}

u8 ObserveCloseTrackValidatePma(void)
{
    validate_pma_count++;
    return validate_pma_result;
}

void ObserveCloseTrackCompleteSession(void)
{
    complete_session_count++;
    if(mutate_parameter_on_session_complete != 0u)
        memory[0x000c45u] = 0u;
}

void ObserveCloseTrackExecuteOperation(void)
{
    execute_operation_count++;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    inherited_b = 0x5au;
    validate_cdb_result = 1u;
    finalize_active_result = 1u;
    prepare_result = 1u;
    validate_media_result = 1u;
    validate_session_result = 1u;
    load_result = 1u;
    validate_write_result = 1u;
    validate_pma_result = 1u;
    loaded_working_type = 6u;
    mutate_parameter_on_session_complete = 0u;
    cdb_mask = 0u;
    normalize_count = 0u;
    finalize_active_count = 0u;
    finalize_existing_count = 0u;
    complete_no_sense_count = 0u;
    reset_count = 0u;
    init_coroutine_count = 0u;
    coroutine_entry = 0ul;
    yield_count = 0u;
    prepare_count = 0u;
    update_failure_count = 0u;
    unsupported_count = 0u;
    validate_media_count = 0u;
    validate_session_count = 0u;
    update_tail_count = 0u;
    clear_working_count = 0u;
    load_count = 0u;
    loaded_track = 0u;
    loaded_b = 0u;
    no_sense_count = 0u;
    sequence_error_count = 0u;
    validate_write_count = 0u;
    validate_pma_count = 0u;
    complete_session_count = 0u;
    execute_operation_count = 0u;
}

static void TestValidationAndActiveStateExit(void)
{
    ResetState();
    validate_cdb_result = 0u;
    HandleCloseTrackSessionCommand();
    assert(cdb_mask == 0xff8cd4ul);
    assert(unsupported_count == 1u && normalize_count == 0u);

    ResetState();
    memory[0x000c9eu] = 1u;
    memory[0x000c84u] = 0x88u;
    memory[0x000c85u] = 0xffu;
    HandleCloseTrackSessionCommand();
    assert(normalize_count == 1u && finalize_active_count == 1u);
    assert(memory[0x000c84u] == 0x80u);
    assert(memory[0x000c85u] == 0xfeu);
}

static void TestState2PreparationFailure(void)
{
    ResetState();
    memory[0x000c9eu] = 2u;
    memory[0x000c88u] = 0xffu;
    memory[0x000c89u] = 0xffu;
    memory[0x000c8au] = 0xffu;
    memory[0x000c84u] = 8u;
    memory[0x000b0bu] = 1u;
    prepare_result = 0u;
    memset(&memory[0x001455u], 0xff, 4u);

    HandleCloseTrackSessionCommand();

    assert(finalize_active_count == 1u && complete_no_sense_count == 1u);
    assert(reset_count == 1u && init_coroutine_count == 1u);
    assert(coroutine_entry == 0x00f84a00ul && yield_count == 1u);
    assert(prepare_count == 1u && update_failure_count == 1u);
    assert(ReadU32(0x001455u) == 0ul);
    assert(memory[0x000c88u] == 0xbfu);
    assert(memory[0x000c89u] == 0xf7u);
    assert(memory[0x000c8au] == 0xfeu);
    assert(memory[0x000c84u] == 0u && memory[0x000c85u] == 8u);
}

static void TestSessionValidationAndSpecialTailPatch(void)
{
    fw_addr_t descriptor = 0x2b8003ul + 22ul;

    ResetState();
    memory[0x000b0cu] = 2u;
    validate_session_result = 0u;
    HandleCloseTrackSessionCommand();
    assert(validate_media_count == 1u && validate_session_count == 1u);
    assert(complete_session_count == 0u);

    ResetState();
    memory[0x000b0bu] = 1u;
    memory[0x000b0cu] = 2u;
    memory[0x000c45u] = 0xc0u;
    memory[0x00dd7fu] = 99u;
    memory[0x00e4e0u] = 4u;
    memory[0x0012cau] = 22u;
    memory[descriptor] = 0xb0u;
    mutate_parameter_on_session_complete = 1u;

    HandleCloseTrackSessionCommand();

    assert(update_tail_count == 1u && complete_no_sense_count == 1u);
    assert(complete_session_count == 1u && execute_operation_count == 0u);
    assert(memory[0x000c45u] == 0xc0u);
    assert(memory[descriptor + 1u] == 0xffu);
    assert(memory[descriptor + 2u] == 0xffu);
    assert(memory[descriptor + 3u] == 0xffu);
    assert(memory[0x000c84u] == 0u && memory[0x000c85u] == 0u);
}

static void TestTrackSelectionRejections(void)
{
    ResetState();
    memory[0x000b0cu] = 1u;
    memory[0x000b0fu] = 0u;
    HandleCloseTrackSessionCommand();
    assert(unsupported_count == 1u && load_count == 0u);

    ResetState();
    memory[0x000b0cu] = 1u;
    memory[0x000b0fu] = 2u;
    memory[0x00dd7eu] = 3u;
    memory[0x00dd7fu] = 4u;
    HandleCloseTrackSessionCommand();
    assert(no_sense_count == 1u && load_count == 0u);

    ResetState();
    memory[0x000b0cu] = 1u;
    memory[0x000b0fu] = 2u;
    memory[0x00dd7eu] = 1u;
    memory[0x00dd7fu] = 4u;
    loaded_working_type = 2u;
    memory[0x000c44u] = 0u;
    HandleCloseTrackSessionCommand();
    assert(load_count == 1u && loaded_track == 2u && loaded_b == 0x5au);
    assert(sequence_error_count == 1u && execute_operation_count == 0u);
}

static void TestSuccessfulTrackCloseAndPmaFailure(void)
{
    ResetState();
    memory[0x000b0cu] = 1u;
    memory[0x000b0fu] = 2u;
    memory[0x00dd7eu] = 1u;
    memory[0x00dd7fu] = 4u;
    loaded_working_type = 6u;
    validate_pma_result = 0u;
    HandleCloseTrackSessionCommand();
    assert(validate_pma_count == 1u && execute_operation_count == 0u);

    ResetState();
    memory[0x000b0bu] = 1u;
    memory[0x000b0cu] = 1u;
    memory[0x000b0fu] = 2u;
    memory[0x00dd7eu] = 1u;
    memory[0x00dd7fu] = 4u;
    loaded_working_type = 6u;
    HandleCloseTrackSessionCommand();
    assert(clear_working_count == 1u && load_count == 1u);
    assert(validate_pma_count == 1u && complete_no_sense_count == 1u);
    assert(execute_operation_count == 1u && complete_session_count == 0u);
    assert(memory[0x000c84u] == 0u && memory[0x000c85u] == 0u);
}

static void TestOptionAndSelectorRejections(void)
{
    ResetState();
    memory[0x000b0cu] = 3u;
    HandleCloseTrackSessionCommand();
    assert(unsupported_count == 1u);

    ResetState();
    memory[0x000b0cu] = 2u;
    memory[0x000c46u] = 1u;
    HandleCloseTrackSessionCommand();
    assert(sequence_error_count == 1u && complete_session_count == 0u);
}

int main(void)
{
    TestValidationAndActiveStateExit();
    TestState2PreparationFailure();
    TestSessionValidationAndSpecialTailPatch();
    TestTrackSelectionRejections();
    TestSuccessfulTrackCloseAndPmaFailure();
    TestOptionAndSelectorRejections();
    return 0;
}
