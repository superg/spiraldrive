#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_toc_format3_pma.h"

typedef struct
{
    fw_addr_t source;
    u32 offset;
    u32 count;
    u8 data[11];
} StageCall;

static u8 memory[0x1000000ul];
static StageCall stage_calls[12];
static unsigned stage_call_count;
static u8 unsupported;
static u8 no_sense;
static u8 combined_result;
static u8 execute_result;
static fw_addr_t combined_source;
static u32 combined_count;
static u32 execute_count;
static u8 bcd_values[8];
static unsigned bcd_call_count;

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

void HandleUnsupportedScsiCommand(void)
{
    unsupported = 1u;
}
void SetNoSenseData(void)
{
    no_sense = 1u;
}

u16 MinU16(u16 value, u16 limit)
{
    return value < limit ? value : limit;
}

u8 BinaryByteToPackedBcd(u8 value)
{
    assert(bcd_call_count < sizeof(bcd_values));
    bcd_values[bcd_call_count++] = value;
    return (u8)(((value / 10u) << 4) | (value % 10u));
}

u32 CopyToControllerBufferWindow(fw_addr_t source, u32 offset, u32 count)
{
    StageCall *call;
    u32 index;

    assert(stage_call_count < sizeof(stage_calls) / sizeof(stage_calls[0]));
    assert(count <= sizeof(stage_calls[0].data));
    call = &stage_calls[stage_call_count++];
    call->source = source;
    call->offset = offset;
    call->count = count;
    for(index = 0ul; index < count; ++index)
        call->data[index] = FirmwareRead8(source + index);
    return offset + count;
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 count)
{
    combined_source = source;
    combined_count = count;
    return combined_result;
}

u8 ExecuteControllerDataInAndMapStatus(u32 count)
{
    execute_count = count;
    return execute_result;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(stage_calls, 0, sizeof(stage_calls));
    memset(bcd_values, 0, sizeof(bcd_values));
    stage_call_count = 0u;
    unsupported = 0u;
    no_sense = 0u;
    combined_result = 4u;
    execute_result = 4u;
    combined_source = 0u;
    combined_count = 0u;
    execute_count = 0u;
    bcd_call_count = 0u;
    memory[0x000b11ul] = 1u;
    memory[0x000b12ul] = 0u;
    memory[0x001008ul] = 1u;
    memory[0x001149ul] = 1u;
    memory[0x00155aul] = 0x12u;
    memory[0x00155bul] = 0x34u;
    memory[0x00155cul] = 0x56u;
    memory[0x00155dul] = 0x78u;
}

static void CheckRejectionAndZeroAllocation(void)
{
    Reset();
    memory[0x000b10ul] = 1u;
    HandleReadTocFormat3Pma();
    assert(unsupported == 1u && no_sense == 0u && stage_call_count == 0u);

    Reset();
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 0u;
    HandleReadTocFormat3Pma();
    assert(unsupported == 0u && no_sense == 1u && stage_call_count == 0u);
}

static void CheckUnavailableResponse(void)
{
    Reset();
    memory[0x001008ul] = 0u;
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 3u;
    combined_result = 3u;
    HandleReadTocFormat3Pma();
    assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 2u);
    assert(memory[0x000b2cul] == 0u && memory[0x000b2dul] == 0u);
    assert(combined_source == 0x000b2aul && combined_count == 3u);
    assert(no_sense == 1u && stage_call_count == 0u);

    Reset();
    memory[0x001149ul] = 0u;
    HandleReadTocFormat3Pma();
    assert(combined_count == 4u && no_sense == 0u);
}

static void FillFullRecordSet(void)
{
    fw_addr_t record;

    memory[0x001149ul] = 2u;
    memory[0x00114eul] = 1u;
    memory[0x00114aul] = 1u;
    memory[0x00114bul] = 1u;
    memory[0x00114cul] = 1u;
    memory[0x00114dul] = 1u;

    memory[0x2bb5dbul] = 0x12u;
    memory[0x2bb5deul] = 12u;
    memory[0x2bb5dful] = 34u;
    memory[0x2bb5e0ul] = 56u;
    memory[0x2bb5e3ul] = 0x77u;

    record = 0x2bb423ul + 10u;
    memory[record] = 0x23u;
    memory[record + 2u] = 0xa3u;
    memory[record + 3u] = 0xb3u;

    record = 0x2bb4fful + 10u;
    memory[record] = 0x24u;
    memory[record + 2u] = 0xa4u;
    memory[record + 3u] = 0xb4u;

    record = 0x2bb5e5ul + 10u;
    memory[record] = 0x25u;
    memory[record + 2u] = 0xa5u;
    memory[record + 3u] = 0x15u;
    memory[record + 4u] = 0x25u;
    memory[record + 5u] = 0x35u;
    memory[record + 7u] = 0x45u;
    memory[record + 8u] = 0x55u;
    memory[record + 9u] = 0x65u;

    record = 0x2bb77ful + 10u;
    memory[record] = 0x26u;
    memory[record + 2u] = 0xa6u;
    memory[record + 3u] = 0x16u;
    memory[record + 4u] = 0x26u;
    memory[record + 5u] = 0x36u;
    memory[record + 7u] = 0x46u;
    memory[record + 8u] = 0x56u;
    memory[record + 9u] = 0x66u;

    record = 0x2bb03bul + 10u;
    memory[record] = 0x21u;
    memory[record + 2u] = 0xa1u;
    memory[record + 3u] = 0x11u;
    memory[record + 4u] = 0x21u;
    memory[record + 5u] = 0x31u;
    memory[record + 7u] = 0x41u;
    memory[record + 8u] = 0x51u;
    memory[record + 9u] = 0x61u;
}

static void CheckDescriptor(const StageCall *call, u8 control, u8 point, u8 m, u8 s, u8 f, u8 pm, u8 ps, u8 pf)
{
    assert(call->source == 0x000b2aul && call->count == 11u);
    assert(call->data[0] == 0u && call->data[1] == control);
    assert(call->data[2] == 0u && call->data[3] == point);
    assert(call->data[4] == m && call->data[5] == s && call->data[6] == f);
    assert(call->data[7] == 0u);
    assert(call->data[8] == pm && call->data[9] == ps && call->data[10] == pf);
}

static void CheckAllRecordFamilies(void)
{
    Reset();
    FillFullRecordSet();
    HandleReadTocFormat3Pma();

    assert(stage_call_count == 7u);
    assert(stage_calls[0].source == 0x000b2aul);
    assert(stage_calls[0].offset == 0x12345678ul && stage_calls[0].count == 4u);
    assert(stage_calls[0].data[0] == 0u && stage_calls[0].data[1] == 0x44u);
    assert(stage_calls[0].data[2] == 0u && stage_calls[0].data[3] == 0u);
    assert(stage_calls[1].offset == 0x1234567cul);
    CheckDescriptor(&stage_calls[1], 0x21u, 0u, 0x12u, 0x34u, 0x56u, 0u, 0x77u, 0u);
    CheckDescriptor(&stage_calls[2], 0x32u, 0xa3u, 0xb3u, 0u, 0u, 0u, 0u, 0u);
    CheckDescriptor(&stage_calls[3], 0x42u, 0xa4u, 0xb4u, 0u, 0u, 0u, 0u, 0u);
    CheckDescriptor(&stage_calls[4], 0x52u, 0xa5u, 0x15u, 0x25u, 0x35u, 0x45u, 0x55u, 0x65u);
    CheckDescriptor(&stage_calls[5], 0x62u, 0xa6u, 0x16u, 0x26u, 0x36u, 0x46u, 0x56u, 0x66u);
    CheckDescriptor(&stage_calls[6], 0x12u, 0xa1u, 0x11u, 0x21u, 0x31u, 0x41u, 0x51u, 0x61u);
    assert(stage_calls[6].offset == 0x123456b3ul);
    assert(execute_count == 70u && no_sense == 0u);
    assert(bcd_call_count == 3u);
    assert(bcd_values[0] == 12u && bcd_values[1] == 34u && bcd_values[2] == 56u);
}

static void CheckAllocationClampAndCompletion(void)
{
    fw_addr_t record;

    Reset();
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 13u;
    record = 0x2bb03bul + 10u;
    memory[record + 2u] = 1u;
    execute_result = 2u;
    HandleReadTocFormat3Pma();
    assert(stage_call_count == 2u && stage_calls[0].data[1] == 13u);
    assert(execute_count == 13u && no_sense == 1u);
}

int main(void)
{
    CheckRejectionAndZeroAllocation();
    CheckUnavailableResponse();
    CheckAllRecordFamilies();
    CheckAllocationClampAndCompletion();
    puts("read_toc_format3_pma_tests: ok");
    return 0;
}
