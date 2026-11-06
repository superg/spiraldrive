#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_full_toc_response.h"

typedef struct
{
    fw_addr_t source;
    u32 offset;
    u32 count;
    u8 data[11];
} StageCall;

static u8 memory[0x1000000ul];
static StageCall stage_calls[8];
static unsigned stage_call_count;
static unsigned type2_build_count;
static u8 type2_build_argument;
static u8 unsupported;
static u8 no_sense;
static u8 execute_result;
static u32 execute_count;
static u8 bcd_inputs[16];
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

void BuildType2LeadInTocRecordSequence(u8 argument)
{
    ++type2_build_count;
    type2_build_argument = argument;
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
    assert(bcd_call_count < sizeof(bcd_inputs));
    bcd_inputs[bcd_call_count++] = value;
    return (u8)(value + 0x40u);
}

void CopyBytesWith8BitCount(fw_addr_t destination, fw_addr_t source, u8 count)
{
    u8 index;

    for(index = 0u; index < count; ++index)
        FirmwareWrite8(destination + index, FirmwareRead8(source + index));
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

u8 ExecuteControllerDataInAndMapStatus(u32 count)
{
    execute_count = count;
    return execute_result;
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static fw_addr_t Descriptor(u8 index)
{
    return 0x2b8000ul + (fw_addr_t)((u16)index * 11u);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    memset(stage_calls, 0, sizeof(stage_calls));
    memset(bcd_inputs, 0, sizeof(bcd_inputs));
    stage_call_count = 0u;
    type2_build_count = 0u;
    type2_build_argument = 0u;
    unsupported = 0u;
    no_sense = 0u;
    execute_result = 4u;
    execute_count = 0u;
    bcd_call_count = 0u;
    memory[0x0012c9ul] = 1u;
    WriteBe16(0x0012caul, 11u);
    WriteBe16(0x000b11ul, 0xffffu);
    memory[0x00155aul] = 0x12u;
    memory[0x00155bul] = 0x34u;
    memory[0x00155cul] = 0x56u;
    memory[0x00155dul] = 0x78u;
    memory[Descriptor(1u)] = 1u;
}

static void FillDescriptor(u8 index, u8 session, u8 control_adr, u8 point)
{
    fw_addr_t descriptor = Descriptor(index);
    u8 offset;

    for(offset = 0u; offset < 11u; ++offset)
        memory[descriptor + offset] = offset;
    memory[descriptor] = session;
    memory[descriptor + 1u] = control_adr;
    memory[descriptor + 3u] = point;
}

static void CheckUnsupportedAndRegeneration(void)
{
    Reset();
    memory[0x0012c9ul] = 0u;
    ExecuteReadFullTocResponse(0u);
    assert(unsupported == 1u && type2_build_count == 0u);

    Reset();
    memory[0x000c9eul] = 2u;
    memory[0x0012c9ul] = 3u;
    WriteBe16(0x00800cul, 1u);
    FillDescriptor(1u, 1u, 0x41u, 1u);
    ExecuteReadFullTocResponse(0u);
    assert(type2_build_count == 1u && type2_build_argument == 1u);
    assert(memory[0x0012c9ul] == 3u);
    assert(memory[0x0012caul] == 0u && memory[0x0012cbul] == 11u);
    assert(execute_count == 15u);

    Reset();
    memory[0x000c9eul] = 1u;
    memory[0x0012c9ul] = 0u;
    WriteBe16(0x00800cul, 1u);
    FillDescriptor(1u, 1u, 0x41u, 1u);
    ExecuteReadFullTocResponse(0u);
    assert(type2_build_count == 1u && memory[0x0012c9ul] == 0u);
}

static void CheckRequestedSessionValidation(void)
{
    Reset();
    memory[0x000b10ul] = 2u;
    ExecuteReadFullTocResponse(0u);
    assert(unsupported == 1u && stage_call_count == 0u);
}

static void CheckShortHeaderTransfer(void)
{
    Reset();
    WriteBe16(0x000b11ul, 3u);
    execute_result = 2u;
    ExecuteReadFullTocResponse(0u);
    assert(stage_call_count == 1u);
    assert(stage_calls[0].source == 0x000b2aul);
    assert(stage_calls[0].offset == 0x12345678ul);
    assert(stage_calls[0].count == 3u);
    assert(stage_calls[0].data[0] == 0u && stage_calls[0].data[1] == 13u);
    assert(stage_calls[0].data[2] == 1u);
    assert(execute_count == 3u && no_sense == 1u);
    assert(memory[0x0012c9ul] == 1u);
}

static void CheckSessionSelectionAndStreaming(void)
{
    Reset();
    memory[0x0012c9ul] = 2u;
    WriteBe16(0x0012caul, 33u);
    memory[0x000b10ul] = 2u;
    FillDescriptor(1u, 1u, 0x11u, 1u);
    FillDescriptor(2u, 2u, 0x42u, 2u);
    FillDescriptor(3u, 2u, 0x53u, 3u);
    ExecuteReadFullTocResponse(0u);

    assert(stage_call_count == 3u);
    assert(stage_calls[0].count == 4u);
    assert(stage_calls[0].data[0] == 0u && stage_calls[0].data[1] == 24u);
    assert(stage_calls[0].data[2] == 1u && stage_calls[0].data[3] == 2u);
    assert(stage_calls[1].offset == 0x1234567cul && stage_calls[1].count == 11u);
    assert(stage_calls[1].data[0] == 2u && stage_calls[1].data[1] == 0x24u && stage_calls[1].data[3] == 2u);
    assert(stage_calls[2].offset == 0x12345687ul && stage_calls[2].data[0] == 2u && stage_calls[2].data[1] == 0x35u);
    assert(execute_count == 26u && bcd_call_count == 0u);
}

static void CheckConversionCase(u8 control_adr, u8 point, unsigned conversion_mask, u8 byte4)
{
    unsigned offset;

    Reset();
    FillDescriptor(1u, 1u, control_adr, point);
    memory[Descriptor(1u) + 4u] = byte4;
    ExecuteReadFullTocResponse(1u);
    assert(stage_call_count == 2u && stage_calls[1].count == 11u);
    assert(stage_calls[1].data[1] == (u8)((control_adr << 4) | (control_adr >> 4)));
    for(offset = 3u; offset <= 10u; ++offset)
    {
        u8 source = memory[Descriptor(1u) + offset];
        u8 expected = (conversion_mask & (1u << offset)) != 0u ? (u8)(source + 0x40u) : source;

        if((control_adr & 0x0fu) == 1u && point < 0xa0u && offset == 7u)
            expected = 0u;
        assert(stage_calls[1].data[offset] == expected);
    }
}

static void CheckPackedBcdMatrix(void)
{
    CheckConversionCase(0x51u, 0x01u, 0x0778u, 4u);
    CheckConversionCase(0x51u, 0xa0u, 0x0570u, 4u);
    CheckConversionCase(0x51u, 0xa1u, 0x0170u, 4u);
    CheckConversionCase(0x51u, 0xa2u, 0x0770u, 4u);
    CheckConversionCase(0x51u, 0xa3u, 0u, 4u);
    CheckConversionCase(0x52u, 0x01u, 0x0778u, 4u);
    CheckConversionCase(0x52u, 0xb0u, 0x07f0u, 4u);
    CheckConversionCase(0x52u, 0xb0u, 0x0780u, 0xffu);
    CheckConversionCase(0x52u, 0xb2u, 0x07f0u, 4u);
    CheckConversionCase(0x52u, 0xc0u, 0x0700u, 4u);
    CheckConversionCase(0x52u, 0xc1u, 0u, 4u);
}

int main(void)
{
    CheckUnsupportedAndRegeneration();
    CheckRequestedSessionValidation();
    CheckShortHeaderTransfer();
    CheckSessionSelectionAndStreaming();
    CheckPackedBcdMatrix();
    puts("read_full_toc_response_tests: ok");
    return 0;
}
