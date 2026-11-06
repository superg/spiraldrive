#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/get_event_status_notification.h"

static u8 memory[0x1000000ul];
static u8 validate_result;
static u8 execute_result;
static u8 rejected;
static u8 calls[8];
static unsigned call_count;
static u32 descriptors[5];
static fw_addr_t staged_source;
static u32 staged_offset;
static u32 staged_count;
static u16 min_value;
static u16 min_limit;

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

u8 ObserveValidateGetEventStatusCdb(fw_addr_t mask_descriptor)
{
    assert(mask_descriptor == 0xff8e97ul);
    return validate_result;
}

void ObserveRejectGetEventStatusCommand(void)
{
    rejected = 1u;
}

void ObserveStageGetEventStatusResponse(fw_addr_t source_address, u32 controller_offset, u32 byte_count)
{
    staged_source = source_address;
    staged_offset = controller_offset;
    staged_count = byte_count;
}

u16 ObserveGetEventStatusMinU16(u16 value, u16 limit)
{
    min_value = value;
    min_limit = limit;
    return value < limit ? value : limit;
}

u8 ObserveExecuteGetEventStatusDataIn(u32 transfer_length)
{
    memory[0x20ul] = (u8)transfer_length;
    return execute_result;
}

u32 BuildOperationalChangeEventStatusDescriptor(void)
{
    calls[call_count++] = 2u;
    return descriptors[0];
}

u32 BuildMediaEventStatusDescriptor(void)
{
    calls[call_count++] = 4u;
    return descriptors[1];
}

u32 BuildEmptyExternalRequestEventStatusDescriptor(void)
{
    calls[call_count++] = 8u;
    return descriptors[2];
}

u32 BuildGetEventStatusMediaEventDescriptor(u8 inherited_event)
{
    assert(inherited_event == 0x10u);
    calls[call_count++] = 0x10u;
    return descriptors[3];
}

u32 BuildDeviceBusyEventStatusDescriptor(void)
{
    calls[call_count++] = 0x40u;
    return descriptors[4];
}

static void Reset(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(calls, 0, sizeof(calls));
    memset(descriptors, 0, sizeof(descriptors));
    validate_result = 1u;
    execute_result = 4u;
    rejected = 0u;
    call_count = 0u;
    staged_source = 0u;
    staged_offset = 0u;
    staged_count = 0u;
    min_value = 0u;
    min_limit = 0u;
    memory[0x000b0bul] = 1u;
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 0xffu;
    memory[0x00155aul] = 0x12u;
    memory[0x00155bul] = 0x34u;
    memory[0x00155cul] = 0x56u;
    memory[0x00155dul] = 0x78u;
}

static void CheckRejectedCommands(void)
{
    Reset();
    validate_result = 0u;
    HandleGetEventStatusNotificationCommand();
    assert(rejected == 1u && call_count == 0u && staged_count == 0u);

    Reset();
    memory[0x000b0bul] = 0u;
    HandleGetEventStatusNotificationCommand();
    assert(rejected == 1u && call_count == 0u && staged_count == 0u);
}

static void CheckExactOperationalClass(void)
{
    Reset();
    memory[0x000b0eul] = 2u;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 1u && calls[0] == 2u);
    assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 6u);
    assert(memory[0x000b2cul] == 1u && memory[0x000b2dul] == 0x5eu);
    assert(memory[0x000b2eul] == 0u && memory[0x000b31ul] == 0u);
    assert(staged_source == 0x000b2aul && staged_offset == 0x12345678ul);
    assert(staged_count == 8u && min_value == 0xffu && min_limit == 8u);
    assert(memory[0x20ul] == 8u && (memory[0x000c87ul] & 0x10u) != 0u);
}

static void CheckPriorityAndLengthAccumulation(void)
{
    Reset();
    memory[0x000b0eul] = 0x46u;
    descriptors[1] = 0x01020304ul;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 2u && calls[0] == 2u && calls[1] == 4u);
    assert(memory[0x000b2bul] == 10u && memory[0x000b2cul] == 2u);
    assert(memory[0x000b2eul] == 1u && memory[0x000b2ful] == 2u);
    assert(memory[0x000b30ul] == 3u && memory[0x000b31ul] == 4u);
    assert(staged_count == 12u);
}

static void CheckSupportedClassFallback(void)
{
    Reset();
    memory[0x000b0eul] = 0x58u;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 3u && calls[0] == 8u && calls[1] == 0x10u && calls[2] == 0x40u);
    assert(memory[0x000b2cul] == 8u);
    assert(memory[0x000b2bul] == 14u && staged_count == 16u);
}

static void CheckNoRequestedClass(void)
{
    Reset();
    memory[0x000b0eul] = 0x20u;
    memory[0x000b2eul] = 0x7au;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 0u && memory[0x000b2cul] == 0x80u);
    assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 2u);
    assert(memory[0x000b2eul] == 0x7au && staged_count == 4u);
}

static void CheckMediaSeedBusyAndNoSense(void)
{
    Reset();
    memory[0x000b0eul] = 0x50u;
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 5u;
    descriptors[3] = 0x06000200ul;
    execute_result = 3u;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 1u && calls[0] == 0x10u);
    assert(memory[0x000b2cul] == 4u && staged_count == 8u);
    assert(min_value == 5u && min_limit == 8u && memory[0x20ul] == 5u);
    assert(memory[0x000b18ul] == 0x70u && memory[0x000b1aul] == 0u);
    assert(memory[0x000b24ul] == 0u && memory[0x000b25ul] == 0u);
    assert(memory[0x000b1bul] == 0u && memory[0x000b1eul] == 0u);

    Reset();
    memory[0x000b0eul] = 0x40u;
    descriptors[4] = 0x01031234ul;
    HandleGetEventStatusNotificationCommand();
    assert(call_count == 1u && calls[0] == 0x40u);
    assert(memory[0x000b2cul] == 6u && memory[0x000b31ul] == 0x34u);
}

int main(void)
{
    CheckRejectedCommands();
    CheckExactOperationalClass();
    CheckPriorityAndLengthAccumulation();
    CheckSupportedClassFallback();
    CheckNoRequestedClass();
    CheckMediaSeedBusyAndNoSense();
    puts("get_event_status_notification_tests: ok");
    return 0;
}
