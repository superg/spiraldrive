#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/write_buffer.h"

static u8 memory[0x20000];
static u8 sfr[256];
static u8 bits[6];
static u8 validate_cdb_result;
static u8 data_out_results[4];
static u8 data_out_calls;
static u32 data_out_lengths[4];
static u8 buffer_result;
static u8 buffer_calls;
static u32 buffer_addresses[4];
static u32 buffer_lengths[4];
static u8 unsupported_calls;
static u8 no_sense_calls;
static u8 sense_calls;
static u8 sense_key;
static u16 sense_code;
static u8 status_sense_calls;
static u8 status_sense_value;
static u8 sequence_error_calls;
static u8 prepare_calls;
static u8 reset_queue_calls;
static u8 initialize2_calls;
static u8 initialize3_calls;
static u32 initialize2_entry;
static u32 initialize3_entry;
static u8 yield_calls;
static u8 revision_result;
static u8 header_result;
static u8 checksum_result;
static u8 checksum_calls;
static u32 checksum_end;
static u8 reset_media_calls;
static u8 stage_configuration_calls;
static u8 snapshot_result;
static u32 snapshot_count;
static u8 programmer3_result;
static u8 programmer5_result;
static u8 programmer3_calls;
static u8 programmer5_calls;
static u8 yield_release_media3;
static u8 yield_complete_quiesce;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    return sfr[address];
}
void FirmwareWriteSfr8(u8 address, u8 value)
{
    sfr[address] = value;
}
void FirmwareWriteBit0480(u8 value)
{
    bits[0] = value;
}
void FirmwareWriteBit0481(u8 value)
{
    bits[1] = value;
}
void FirmwareWriteBit0546(u8 value)
{
    bits[2] = value;
}
void FirmwareWriteBit0547(u8 value)
{
    bits[3] = value;
}
void FirmwareWriteBit0580(u8 value)
{
    bits[4] = value;
}
void FirmwareWriteBit0581(u8 value)
{
    bits[5] = value;
}

u8 ValidateScsiCdbFieldMasks(fw_addr_t mask)
{
    assert(mask == 0xff8e3ful);
    return validate_cdb_result;
}

void HandleUnsupportedScsiCommand(void)
{
    unsupported_calls++;
}
void SetNoSenseData(void)
{
    no_sense_calls++;
}
void SetSenseData(u8 key, u16 code)
{
    sense_calls++;
    sense_key = key;
    sense_code = code;
}
void SetSenseDataFromStatusCode(u8 status)
{
    status_sense_calls++;
    status_sense_value = status;
}
void SetCommandSequenceErrorSense(void)
{
    sequence_error_calls++;
}
void PLEXTOR_NEAR_FUNCTION PrepareBufferDataTransfer(void)
{
    prepare_calls++;
}
u8 ExecuteControllerDataOutAndMapStatus(u32 length)
{
    data_out_lengths[data_out_calls] = length;
    return data_out_results[data_out_calls++];
}
u8 ExecuteBufferDataTransfer(u32 address, u32 length, u8 opcode)
{
    assert(opcode == 0x3bu);
    buffer_addresses[buffer_calls] = address;
    buffer_lengths[buffer_calls] = length;
    buffer_calls++;
    return buffer_result;
}
void ResetRawReadQueueAndTransferState(void)
{
    reset_queue_calls++;
}
void InitializeCoroutine2(u32 entry)
{
    initialize2_calls++;
    initialize2_entry = entry;
}
void InitializeCoroutine3(u32 entry)
{
    initialize3_calls++;
    initialize3_entry = entry;
}
void YieldCoroutine0(void)
{
    yield_calls++;
    if(yield_release_media3 != 0u)
        memory[0x0fae] = 0u;
    if(yield_complete_quiesce != 0u)
        memory[0x15c3] = 1u;
}
u8 PLEXTOR_NEAR_FUNCTION ValidateStagedFirmwareRevisionPolicy(u8 id)
{
    assert(id == memory[0x0b0c]);
    return revision_result;
}
u8 ValidateStagedMainFirmwareHeader(void)
{
    return header_result;
}
u8 ValidateStagedFirmwareChecksum(u32 end, u32 start)
{
    assert(start == 0ul);
    checksum_calls++;
    checksum_end = end;
    return checksum_result;
}
void ResetMediaProcessingCoroutines(void)
{
    reset_media_calls++;
}
void StagePrimaryDriveConfigurationForAuxiliaryUpdate(void)
{
    stage_configuration_calls++;
}
u8 ValidateWriteBufferSnapshot(u32 count)
{
    snapshot_count = count;
    return snapshot_result;
}
u8 InvokeRuntimeMainFlashProgrammerOperation3(void)
{
    programmer3_calls++;
    return programmer3_result;
}
u8 InvokeRuntimeMainFlashProgrammerOperation5(void)
{
    programmer5_calls++;
    return programmer5_result;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void SetCdb(u8 mode, u8 id, u32 offset, u32 length)
{
    memory[0x0b0b] = mode;
    memory[0x0b0c] = id;
    memory[0x0b0d] = (u8)(offset >> 16);
    memory[0x0b0e] = (u8)(offset >> 8);
    memory[0x0b0f] = (u8)offset;
    memory[0x0b10] = (u8)(length >> 16);
    memory[0x0b11] = (u8)(length >> 8);
    memory[0x0b12] = (u8)length;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(sfr, 0, sizeof(sfr));
    memset(bits, 1, sizeof(bits));
    validate_cdb_result = 1u;
    memset(data_out_results, 0, sizeof(data_out_results));
    data_out_calls = 0u;
    memset(data_out_lengths, 0, sizeof(data_out_lengths));
    buffer_result = 0u;
    buffer_calls = 0u;
    unsupported_calls = no_sense_calls = sense_calls = 0u;
    status_sense_calls = sequence_error_calls = prepare_calls = 0u;
    reset_queue_calls = initialize2_calls = initialize3_calls = 0u;
    yield_calls = checksum_calls = reset_media_calls = 0u;
    stage_configuration_calls = 0u;
    revision_result = header_result = checksum_result = snapshot_result = 1u;
    programmer3_result = programmer5_result = 0u;
    programmer3_calls = programmer5_calls = 0u;
    yield_release_media3 = yield_complete_quiesce = 0u;
    WriteDword(0x1556u, 0x1000ul);
    memory[0x0c85] = 2u;
}

static void TestAdmissionAndMode0(void)
{
    ResetState();
    SetCdb(0u, 0u, 0ul, 3ul);
    memory[0x0fae] = 7u;
    HandleWriteBufferCommand();
    assert(status_sense_calls == 1u && status_sense_value == 7u);

    ResetState();
    SetCdb(4u, 0u, 0ul, 1ul);
    validate_cdb_result = 0u;
    HandleWriteBufferCommand();
    assert(unsupported_calls == 1u && buffer_calls == 0u);

    ResetState();
    SetCdb(0u, 0u, 0ul, 3ul);
    memory[0x0c85] = 0u;
    HandleWriteBufferCommand();
    assert(sense_calls == 1u && sense_key == 2u && sense_code == 0x0401u);

    ResetState();
    SetCdb(0u, 0u, 0ul, 3ul);
    HandleWriteBufferCommand();
    assert(data_out_calls == 1u && data_out_lengths[0] == 3ul);
    assert(no_sense_calls == 1u && (memory[0x0c83] & 0x20u) == 0u);

    ResetState();
    SetCdb(0u, 0u, 0ul, 9ul);
    HandleWriteBufferCommand();
    assert(prepare_calls == 1u && data_out_lengths[0] == 4ul);
    assert(buffer_calls == 1u && buffer_addresses[0] == 0ul && buffer_lengths[0] == 5ul);
}

static void TestOrdinaryOffsetModes(void)
{
    ResetState();
    SetCdb(1u, 0u, 0xff0ul, 0x14ul);
    HandleWriteBufferCommand();
    assert(buffer_calls == 1u);
    assert(buffer_addresses[0] == 0xff0ul && buffer_lengths[0] == 0x10ul);

    ResetState();
    SetCdb(2u, 0u, 0x100ul, 0x20ul);
    HandleWriteBufferCommand();
    assert(prepare_calls == 1u && buffer_calls == 1u);
    assert(buffer_addresses[0] == 0x100ul && buffer_lengths[0] == 0x20ul);

    ResetState();
    SetCdb(4u, 1u, 0x100ul, 0x20ul);
    HandleWriteBufferCommand();
    assert(unsupported_calls == 1u && no_sense_calls == 0u);

    ResetState();
    SetCdb(3u, 0u, 0ul, 0ul);
    HandleWriteBufferCommand();
    assert(unsupported_calls == 1u);
}

static void TestSnapshotMode(void)
{
    ResetState();
    SetCdb(5u, 2u, 0ul, 0x100ul);
    memory[0x0fae] = 1u;
    snapshot_result = 0u;
    HandleWriteBufferCommand();
    assert(initialize2_calls == 1u && initialize2_entry == 0xf84a00ul);
    assert(buffer_calls == 1u && snapshot_count == 0x100ul);
    assert(sense_key == 5u && sense_code == 0x2600u);

    ResetState();
    SetCdb(5u, 2u, 0ul, 1ul);
    memory[0x0fae] = 2u;
    HandleWriteBufferCommand();
    assert(sequence_error_calls == 1u);
}

static void TestPrimaryProgrammingValidationFailure(void)
{
    ResetState();
    SetCdb(5u, 0u, 0x20ul, 0x200ul);
    revision_result = 0u;
    HandleWriteBufferCommand();
    assert(reset_queue_calls == 1u);
    assert(initialize2_entry == 0xf84a00ul && initialize3_entry == 0xf84b00ul);
    assert(buffer_addresses[0] == 0x20ul && buffer_lengths[0] == 0x200ul);
    assert(sense_key == 4u && sense_code == 0x40f5u);
    assert(programmer3_calls == 0u);

    ResetState();
    SetCdb(5u, 0u, 0ul, 0x200ul);
    header_result = 0u;
    yield_complete_quiesce = 1u;
    sfr[0xa9] = 0x10u;
    HandleWriteBufferCommand();
    assert(sense_key == 4u && sense_code == 0x40f5u);
    assert(bits[2] == 1u && bits[3] == 1u);
    assert(sfr[0xa9] == 0x14u);
    assert(programmer3_calls == 0u);

    ResetState();
    SetCdb(5u, 0u, 0ul, 0x200ul);
    checksum_result = 0u;
    yield_complete_quiesce = 1u;
    HandleWriteBufferCommand();
    assert(checksum_end == 0x79ffeul);
    assert(sense_code == 0x40f6u && programmer3_calls == 0u);
}

static void TestPrimaryProgrammingSuccess(void)
{
    ResetState();
    SetCdb(5u, 0u, 0ul, 0x200ul);
    yield_complete_quiesce = 1u;
    HandleWriteBufferCommand();
    assert(header_result == 1u && checksum_calls == 1u);
    assert(checksum_end == 0x79ffeul);
    assert(reset_media_calls == 1u);
    assert(bits[0] == 0u && bits[1] == 0u && bits[3] == 0u);
    assert(bits[4] == 0u && bits[5] == 0u);
    assert(memory[0x1312] == 0u && memory[0x1313] == 0xffu && memory[0x1314] == 0x40u && memory[0x1315] == 0u);
    assert(programmer3_calls == 1u && programmer5_calls == 0u);
    assert(no_sense_calls == 1u);
}

static void TestAuxiliaryProgramming(void)
{
    ResetState();
    SetCdb(5u, 0xffu, 0ul, 0x100ul);
    memory[0x0fae] = 3u;
    yield_release_media3 = 1u;
    yield_complete_quiesce = 1u;
    HandleWriteBufferCommand();
    assert(checksum_end == 0xfffeul);
    assert(stage_configuration_calls == 1u);
    assert(programmer3_calls == 0u && programmer5_calls == 1u);
}

int main(void)
{
    TestAdmissionAndMode0();
    TestOrdinaryOffsetModes();
    TestSnapshotMode();
    TestPrimaryProgrammingValidationFailure();
    TestPrimaryProgrammingSuccess();
    TestAuxiliaryProgramming();
    puts("write buffer tests passed");
    return 0;
}
