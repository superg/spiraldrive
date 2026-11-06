#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/send_diagnostic.h"

typedef struct EventRecord
{
    u8 event;
    u32 argument0;
    u32 argument1;
} EventRecord;

static u8 memory[0x1000000ul];
static EventRecord events[32];
static unsigned event_count;
static u8 cdb_valid;
static u8 boot_valid;
static u8 controller_buffer_valid;
static u8 data_out_status;

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

u32 ObserveSendDiagnostic(u8 event, u32 argument0, u32 argument1)
{
    assert(event_count < sizeof(events) / sizeof(events[0]));
    events[event_count].event = event;
    events[event_count].argument0 = argument0;
    events[event_count].argument1 = argument1;
    ++event_count;
    if(event == SEND_DIAGNOSTIC_VALIDATE_CDB)
        return cdb_valid;
    if(event == SEND_DIAGNOSTIC_VALIDATE_BOOT)
        return boot_valid;
    if(event == SEND_DIAGNOSTIC_TEST_CONTROLLER_BUFFER)
        return controller_buffer_valid;
    if(event == SEND_DIAGNOSTIC_DATA_OUT)
        return data_out_status;
    if(event == SEND_DIAGNOSTIC_ZERO_WORKSPACE)
        memset(&memory[argument0], 0, (size_t)argument1);
    return 0ul;
}

static unsigned CountEvent(u8 event)
{
    unsigned index;
    unsigned count = 0u;

    for(index = 0u; index < event_count; ++index)
        if(events[index].event == event)
            ++count;
    return count;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(events, 0, sizeof(events));
    event_count = 0u;
    cdb_valid = 1u;
    boot_valid = 1u;
    controller_buffer_valid = 1u;
    data_out_status = 0u;
}

static void SetLength(u16 length)
{
    memory[0xb0du] = (u8)(length >> 8);
    memory[0xb0eu] = (u8)length;
}

static void AssertPreparationPrefix(void)
{
    static const u8 expected[] = { SEND_DIAGNOSTIC_VALIDATE_CDB, SEND_DIAGNOSTIC_RESET_RAW_READ, SEND_DIAGNOSTIC_INITIALIZE_COROUTINE2, SEND_DIAGNOSTIC_DISPATCH_REQUEST5,
        SEND_DIAGNOSTIC_YIELD_COROUTINE0, SEND_DIAGNOSTIC_ZERO_WORKSPACE };
    unsigned index;

    assert(event_count >= sizeof(expected));
    for(index = 0u; index < sizeof(expected); ++index)
        assert(events[index].event == expected[index]);
    assert(events[2].argument0 == 0xf84a00ul);
    assert(events[5].argument0 == 0xdcbdul && events[5].argument1 == 8ul);
}

static void TestValidationAndEmptyCommandRoutes(void)
{
    ResetFixture();
    cdb_valid = 0u;
    HandleSendDiagnosticCommand();
    assert(event_count == 2u && events[1].event == SEND_DIAGNOSTIC_REJECT);

    ResetFixture();
    HandleSendDiagnosticCommand();
    assert(event_count == 2u && events[1].event == SEND_DIAGNOSTIC_SET_NO_SENSE);

    ResetFixture();
    memory[0xb0bu] = 4u;
    SetLength(8u);
    HandleSendDiagnosticCommand();
    assert(event_count == 2u && events[1].event == SEND_DIAGNOSTIC_REJECT);
}

static void TestSelfTestSuccessAndFailurePriority(void)
{
    ResetFixture();
    memory[0xb0bu] = 4u;
    memset(&memory[0xdcbdu], 0xa5, 8u);
    HandleSendDiagnosticCommand();
    AssertPreparationPrefix();
    assert(CountEvent(SEND_DIAGNOSTIC_VALIDATE_BOOT) == 1u);
    assert(CountEvent(SEND_DIAGNOSTIC_TEST_CONTROLLER_BUFFER) == 1u);
    assert(CountEvent(SEND_DIAGNOSTIC_SET_NO_SENSE) == 1u);
    assert(memory[0xc2au] == 7u && memory[0xdcbdu] == 0u);

    ResetFixture();
    memory[0xb0bu] = 4u;
    boot_valid = 0u;
    memory[0x1009u] = 1u;
    controller_buffer_valid = 0u;
    HandleSendDiagnosticCommand();
    AssertPreparationPrefix();
    assert(CountEvent(SEND_DIAGNOSTIC_SET_SENSE) == 1u);
    assert(events[event_count - 1u].argument0 == 4u && events[event_count - 1u].argument1 == 0x4000u);
    assert(CountEvent(SEND_DIAGNOSTIC_SET_NO_SENSE) == 0u);
    assert(memory[0xc2au] == 7u);
}

static void TestParameterLengthAndDataOutFailures(void)
{
    ResetFixture();
    SetLength(7u);
    HandleSendDiagnosticCommand();
    AssertPreparationPrefix();
    assert(events[event_count - 1u].event == SEND_DIAGNOSTIC_REJECT);

    ResetFixture();
    SetLength(8u);
    data_out_status = 4u;
    HandleSendDiagnosticCommand();
    AssertPreparationPrefix();
    assert(events[event_count - 1u].event == SEND_DIAGNOSTIC_DATA_OUT);
    assert(events[event_count - 1u].argument0 == 0xb2aul && events[event_count - 1u].argument1 == 8u);
    assert(memory[0xc2au] == 0u);
}

static void TestParameterDiagnosticResultPublication(void)
{
    ResetFixture();
    SetLength(8u);
    memory[0xb2cu] = 1u;
    memory[0xb2du] = 1u;
    memory[0x1009u] = 1u;
    boot_valid = 2u;

    HandleSendDiagnosticCommand();

    AssertPreparationPrefix();
    assert(memory[0xdcbfu] == 1u && memory[0xdcc0u] == 1u && memory[0xdcc1u] == 0u && memory[0xc2au] == 7u);
    assert(CountEvent(SEND_DIAGNOSTIC_DATA_OUT) == 1u);
    assert(CountEvent(SEND_DIAGNOSTIC_SET_NO_SENSE) == 1u);
}

int main(void)
{
    TestValidationAndEmptyCommandRoutes();
    TestSelfTestSuccessAndFailurePriority();
    TestParameterLengthAndDataOutFailures();
    TestParameterDiagnosticResultPublication();
    puts("send diagnostic tests passed");
    return 0;
}
