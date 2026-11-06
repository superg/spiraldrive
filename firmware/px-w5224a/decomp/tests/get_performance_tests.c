#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/get_performance.h"

typedef struct AppendCall
{
    u8 kind;
    u8 speed;
    u16 limit;
    u16 length;
    fw_addr_t record;
    u16 inherited_wr14;
} AppendCall;

static u8 memory[1u << 24];
static AppendCall append_calls[32];
static unsigned append_count;
static unsigned validate_calls;
static fw_addr_t validate_mask;
static u8 validate_result;
static unsigned unsupported_calls;
static unsigned transfer_calls;
static fw_addr_t transfer_source;
static u32 transfer_count;
static u8 transfer_result;
static unsigned no_sense_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
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

u8 ObserveGetPerformanceValidateCdb(fw_addr_t mask_descriptor)
{
    ++validate_calls;
    validate_mask = mask_descriptor;
    return validate_result;
}

void ObserveGetPerformanceUnsupported(void)
{
    ++unsupported_calls;
}

static u16 RecordAppend(u8 kind, u8 speed, u16 limit, u16 length, fw_addr_t record, u16 inherited_wr14)
{
    AppendCall *call = &append_calls[append_count++];

    call->kind = kind;
    call->speed = speed;
    call->limit = limit;
    call->length = length;
    call->record = record;
    call->inherited_wr14 = inherited_wr14;
    return (u16)(length + 16u);
}

u16 ObserveGetPerformanceAppendNominal(u8 speed, u16 limit, u16 response_length)
{
    return RecordAppend(1u, speed, limit, response_length, 0u, 0u);
}

u16 ObserveGetPerformanceAppendWrite(u8 speed, u16 response_length, u16 limit)
{
    return RecordAppend(2u, speed, limit, response_length, 0u, 0u);
}

u16 ObserveGetPerformanceAppendTable(fw_addr_t record, u16 response_length, u16 inherited_wr14)
{
    return RecordAppend(3u, 0u, 0u, response_length, record, inherited_wr14);
}

u8 ObserveGetPerformanceTransfer(fw_addr_t source, u32 byte_count)
{
    ++transfer_calls;
    transfer_source = source;
    transfer_count = byte_count;
    return transfer_result;
}

void ObserveGetPerformanceNoSense(void)
{
    ++no_sense_calls;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(append_calls, 0, sizeof(append_calls));
    append_count = 0u;
    validate_calls = 0u;
    validate_mask = 0u;
    validate_result = 1u;
    unsupported_calls = 0u;
    transfer_calls = 0u;
    transfer_source = 0u;
    transfer_count = 0u;
    transfer_result = 0u;
    no_sense_calls = 0u;
    WriteBe16(0x000b12u, 3u);
}

static void AssertResponse(u16 length, u8 type)
{
    assert(ReadBe32(0x000b2au) == (u16)(length - 4u));
    assert(memory[0x000b2eu] == type);
    assert(memory[0x000b2fu] == 0u && memory[0x000b30u] == 0u && memory[0x000b31u] == 0u);
    assert(transfer_calls == 1u && transfer_source == 0x000b2aul && transfer_count == length);
}

static void TestValidationAndUnsupportedRoutes(void)
{
    ResetState();
    validate_result = 0u;
    HandleGetPerformanceCommand();
    assert(validate_calls == 1u && validate_mask == 0x00ff8eadul);
    assert(unsupported_calls == 1u && transfer_calls == 0u);

    ResetState();
    memory[0x000b14u] = 1u;
    HandleGetPerformanceCommand();
    assert(unsupported_calls == 1u && append_count == 0u && transfer_calls == 0u);

    ResetState();
    memory[0x000b0bu] = 2u;
    HandleGetPerformanceCommand();
    assert(unsupported_calls == 1u && transfer_calls == 0u);
}

static void TestNominalRoutesAndCompletion(void)
{
    ResetState();
    memory[0x000faeu] = 0u;
    memory[0x00fc1fu] = 40u;
    HandleGetPerformanceCommand();
    assert(append_count == 1u && append_calls[0].kind == 1u);
    assert(append_calls[0].speed == 40u && append_calls[0].limit == 3u && append_calls[0].length == 8u);
    AssertResponse(24u, 0u);
    assert(no_sense_calls == 1u);

    ResetState();
    memory[0x000faeu] = 1u;
    memory[0x000c43u] = 24u;
    transfer_result = 4u;
    HandleGetPerformanceCommand();
    assert(append_calls[0].speed == 24u);
    AssertResponse(24u, 0u);
    assert(no_sense_calls == 0u);
}

static void TestWriteSpeedSelection(void)
{
    struct Case
    {
        u8 available;
        u8 media_state;
        u8 mode;
        u8 family;
        u8 flags;
        u8 expected;
    } cases[] = {
        { 0u, 0u, 0u, 0u, 0u,    52u },
        { 1u, 0u, 0u, 0u, 0u,    52u },
        { 1u, 1u, 1u, 0u, 0u,    52u },
        { 1u, 1u, 0u, 0u, 0u,    8u  },
        { 1u, 2u, 0u, 2u, 0u,    24u },
        { 1u, 2u, 0u, 0u, 0x40u, 10u },
        { 1u, 2u, 0u, 0u, 0u,    4u  }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetState();
        memory[0x000b0bu] = 4u;
        memory[0x000faeu] = cases[index].available ? 0u : 1u;
        memory[0x001008u] = cases[index].media_state;
        memory[0x00fbecu] = cases[index].mode;
        memory[0x00fb88u] = cases[index].family;
        memory[0x00fb87u] = cases[index].flags;
        HandleGetPerformanceCommand();
        assert(append_count == 1u && append_calls[0].kind == 2u);
        assert(append_calls[0].speed == cases[index].expected);
        assert(append_calls[0].length == 8u && append_calls[0].limit == 3u);
        AssertResponse(24u, 2u);
    }
}

static void SetTableRecord(u8 index, u8 byte0, u8 byte2)
{
    u8 offset = (u8)(5u + index * 5u);
    fw_addr_t record = 0x00ff8c54ul + offset;

    memory[record] = byte0;
    memory[record + 2u] = byte2;
}

static void TestTableFiltersAndLimit(void)
{
    ResetState();
    memory[0x000b14u] = 3u;
    memory[0x000faeu] = 1u;
    memory[0x00ff8c54u] = 4u;
    SetTableRecord(0u, 0u, 0x80u);
    SetTableRecord(1u, 0u, 0u);
    SetTableRecord(2u, 0u, 0x80u);
    SetTableRecord(3u, 0u, 0x80u);
    WriteBe16(0x000b12u, 2u);
    HandleGetPerformanceCommand();
    assert(append_count == 2u);
    assert(append_calls[0].record == 0x00ff8c59ul);
    assert(append_calls[0].inherited_wr14 == 0x0500u);
    assert(append_calls[1].record == 0x00ff8c63ul);
    assert(append_calls[1].inherited_wr14 == 0x0f02u);
    AssertResponse(40u, 0u);

    ResetState();
    memory[0x000b14u] = 3u;
    memory[0x000faeu] = 0u;
    memory[0x001008u] = 1u;
    memory[0x00fbecu] = 1u;
    WriteBe32(0x001153u, 0x00030000ul);
    memory[0x00ff8c54u] = 2u;
    SetTableRecord(0u, 0x40u, 0u);
    SetTableRecord(1u, 0u, 0x80u);
    HandleGetPerformanceCommand();
    assert(append_count == 1u && append_calls[0].record == 0x00ff8c59ul);

    ResetState();
    memory[0x000b14u] = 3u;
    memory[0x000faeu] = 0u;
    memory[0x001008u] = 2u;
    memory[0x00fb88u] = 2u;
    memory[0x00ff8c54u] = 2u;
    SetTableRecord(0u, 4u, 0u);
    SetTableRecord(1u, 2u, 0u);
    HandleGetPerformanceCommand();
    assert(append_count == 1u && append_calls[0].record == 0x00ff8c59ul);

    ResetState();
    memory[0x000b14u] = 3u;
    WriteBe16(0x000b12u, 0u);
    HandleGetPerformanceCommand();
    assert(append_count == 0u);
    AssertResponse(8u, 0u);
}

int main(void)
{
    TestValidationAndUnsupportedRoutes();
    TestNominalRoutesAndCompletion();
    TestWriteSpeedSelection();
    TestTableFiltersAndLimit();
    return 0;
}
