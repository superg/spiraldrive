#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/write_partial_range.h"

static u8 memory[1u << 24];
static u8 requires_record, preservation_result;
static unsigned requires_count, initialize_count, fill_count;
static s32 requires_address, preservation_address;
static u8 requires_option;
static fw_addr_t fill_address;
static u32 fill_length;
static u8 fill_value;
static unsigned preservation_count, decrement_count, enqueue_count;
static u16 decrement_addresses[2];
static unsigned sense_count;
static u8 sense_key;
static u16 sense_code;
static u8 c88_during_preservation, c3c_during_preservation;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u8 DoesWriteStartRequireControllerRecordPreparation(s32 address, u8 option)
{
    ++requires_count;
    requires_address = address;
    requires_option = option;
    return requires_record;
}
void InitializeReadVerifyWorkingTrackState(void)
{
    ++initialize_count;
}
void FillControllerBufferRange(fw_addr_t address, u32 length, u8 value)
{
    ++fill_count;
    fill_address = address;
    fill_length = length;
    fill_value = value;
}
u8 AcquireWritePreservationRawReadAtAddress(s32 address, u8 mode)
{
    assert(mode == 0u);
    ++preservation_count;
    preservation_address = address;
    c88_during_preservation = memory[0x000c88ul];
    c3c_during_preservation = memory[0x000c3cul];
    return preservation_result;
}
void DecrementPackedBcdMsf(u16 address)
{
    assert(decrement_count < 2u);
    decrement_addresses[decrement_count++] = address;
}
void EnqueueThirtyTwoWritePreservationDescriptors(void)
{
    ++enqueue_count;
    memory[0x001298ul] = 0x55u;
    memory[0x00126eul] = 0x66u;
}
void SetSenseData(u8 key, u16 code)
{
    ++sense_count;
    sense_key = key;
    sense_code = code;
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

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    requires_record = 0u;
    preservation_result = 1u;
    requires_count = initialize_count = fill_count = 0u;
    requires_address = preservation_address = 0;
    requires_option = 0u;
    fill_address = fill_length = 0ul;
    fill_value = 0xffu;
    preservation_count = decrement_count = enqueue_count = 0u;
    decrement_addresses[0] = decrement_addresses[1] = 0u;
    sense_count = 0u;
    sense_key = 0u;
    sense_code = 0u;
    c88_during_preservation = c3c_during_preservation = 0u;
    WriteU32(0x001598ul, 0x12345678ul);
    WriteU32(0x002bc703ul, 33ul);
    WriteU32(0x00156aul, 16ul);
    WriteU32(0x001562ul, 0x00abc000ul);
}

static void TestBoundaryGate(void)
{
    ResetState();
    memory[0x000c8aul] = 0xffu;
    assert(PreparePartialWriteControllerBufferRange(31ul) == 1u);
    assert(requires_count == 0u && fill_count == 0u);
    assert(memory[0x000c8aul] == 0xefu);
    assert(ReadU32(0x001598ul) == 0x12345678ul);
}

static void TestPreservationSuccessAndRestoration(void)
{
    ResetState();
    requires_record = 1u;
    memory[0x000c8aul] = 0x04u;
    memory[0x000c88ul] = 0x25u;
    memory[0x000c3cul] = 0xa5u;
    assert(PreparePartialWriteControllerBufferRange(30ul) == 1u);
    assert(requires_count == 1u && requires_address == 33);
    assert(requires_option == 1u);
    assert(initialize_count == 1u);
    assert(fill_count == 1u && fill_address == 0ul);
    assert(fill_length == 0x00019940ul && fill_value == 0u);
    assert(preservation_count == 1u && preservation_address == 32);
    assert((c88_during_preservation & 0x80u) != 0u);
    assert((c3c_during_preservation & 1u) == 0u);
    assert(memory[0x000c88ul] == 0x25u);
    assert(memory[0x000c3cul] == 0xa5u);
    assert(memory[0x000ca1ul] == 0u && memory[0x0015d7ul] == 0u);
    assert(memory[0x000c2aul] == 6u);
    assert((memory[0x000c8aul] & 0x10u) != 0u);
    assert(ReadU32(0x001598ul) == 0x12345678ul);
    assert(ReadU16(0x00129aul) == 15u);
    assert(ReadU16(0x00129cul) == 10u);
    assert(ReadU32(0x00129eul) == 0x000088bbul);
    assert(ReadU32(0x00126aul) == 0x00abc000ul);
}

static void TestPreservationFailure(void)
{
    ResetState();
    requires_record = 1u;
    preservation_result = 0u;
    assert(PreparePartialWriteControllerBufferRange(30ul) == 0u);
    assert(sense_count == 1u && sense_key == 3u && sense_code == 0x0c07u);
    assert(ReadU32(0x001598ul) == 0x12345678ul);
}

static void TestSyntheticDescriptors(void)
{
    ResetState();
    assert(PreparePartialWriteControllerBufferRange(30ul) == 1u);
    assert(fill_count == 1u && fill_address == 0ul);
    assert(fill_length == 0x0000cca0ul && fill_value == 0u);
    assert(decrement_count == 2u);
    assert(decrement_addresses[0] == 0x1275u);
    assert(decrement_addresses[1] == 0x1279u);
    assert(enqueue_count == 1u);
    assert(ReadU16(0x001298ul) == 0u);
    assert(ReadU32(0x00126eul) == 0ul);
    assert(ReadU32(0x001598ul) == 0x12345678ul);
    assert((memory[0x000c8aul] & 0x10u) == 0u);
}

int main(void)
{
    TestBoundaryGate();
    TestPreservationSuccessAndRestoration();
    TestPreservationFailure();
    TestSyntheticDescriptors();
    return 0;
}
