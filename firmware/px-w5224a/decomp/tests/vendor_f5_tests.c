#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_f5.h"

#define MEMORY_SIZE (1u << 24)
#define CDB 0x000b0aul

static u8 memory[MEMORY_SIZE];
static unsigned unsupported_count;
static unsigned no_sense_count;
static unsigned transfer_count;
static fw_addr_t transfer_source;
static u32 transfer_bytes;
static u8 transfer_result;
static u16 descriptor_address;
static unsigned selector09_count;
static unsigned strategy_count;
static unsigned fixed52_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u16 MinU16(u16 value, u16 limit)
{
    return value < limit ? value : limit;
}

void HandleUnsupportedScsiCommand(void)
{
    ++unsupported_count;
}

void SetNoSenseData(void)
{
    ++no_sense_count;
}

u8 CopyResponseToControllerBufferAndExecuteDataIn(fw_addr_t source, u32 byte_count)
{
    ++transfer_count;
    transfer_source = source;
    transfer_bytes = byte_count;
    return transfer_result;
}

u16 GetRawReadConsumerDescriptorAddressOrZero(void)
{
    return descriptor_address;
}

void BuildVendorF5Selector09ControllerStateResponse(void)
{
    ++selector09_count;
}

void BuildVendorF5MediaStrategyResponse(void)
{
    ++strategy_count;
}

void TransferFixedResponse52Bytes(void)
{
    ++fixed52_count;
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

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    unsupported_count = 0u;
    no_sense_count = 0u;
    transfer_count = 0u;
    transfer_source = 0ul;
    transfer_bytes = 0ul;
    transfer_result = 5u;
    descriptor_address = 0u;
    selector09_count = 0u;
    strategy_count = 0u;
    fixed52_count = 0u;
    WriteU32(0x00155aul, 0x00160000ul);
}

static void SetCommand(u8 selector, u16 allocation_length)
{
    memory[CDB + 3u] = selector;
    WriteU16(CDB + 8u, allocation_length);
}

static void TestDirectMemoryAndCapacity(void)
{
    ResetState();
    SetCommand(0u, 0x123u);
    WriteU32(CDB + 4u, 0x00123456ul);
    HandleScsiOpcodeF5();
    assert(transfer_count == 1u);
    assert(transfer_source == 0x00123456ul);
    assert(transfer_bytes == 0x123u);
    assert(no_sense_count == 1u);

    ResetState();
    SetCommand(1u, 12u);
    WriteU32(0x00155aul, 0x001affc2ul);
    HandleScsiOpcodeF5();
    assert(unsupported_count == 1u);
    assert(transfer_count == 0u);
    assert(no_sense_count == 0u);
}

static void TestFixedAndDynamicWindows(void)
{
    static const u8 selectors[] = { 1u, 5u, 6u, 7u, 0x80u, 0x81u };
    static const fw_addr_t sources[] = { 0x00fb6aul, 0x00dd2ful, 0x00fb7aul, 0x00116eul, 0x00e806ul, 0x00f157ul };
    static const u16 lengths[] = { 12u, 87u, 18u, 92u, 0x6a4u, 0x60u };
    unsigned index;

    for(index = 0u; index < sizeof(selectors); ++index)
    {
        ResetState();
        memory[0x00dd7ful] = 3u;
        SetCommand(selectors[index], 0xffffu);
        HandleScsiOpcodeF5();
        assert(transfer_count == 1u);
        assert(transfer_source == sources[index]);
        assert(transfer_bytes == lengths[index]);
        assert(no_sense_count == 1u);
    }
}

static void TestLocalStatusAndDescriptorPayload(void)
{
    ResetState();
    memory[0x00fb9aul] = 0xa5u;
    SetCommand(3u, 2u);
    HandleScsiOpcodeF5();
    assert(transfer_source == 0x007000ul);
    assert(transfer_bytes == 2u);
    assert(memory[0x007000ul] == 0xa5u);
    assert(memory[0x007001ul] == 0u);

    ResetState();
    descriptor_address = 0x2000u;
    WriteU16(0x2002u, 0x3000u);
    WriteU16(0x3000u, 0x0012u);
    SetCommand(4u, 3u);
    HandleScsiOpcodeF5();
    assert(transfer_source == 0x00122004ul);
    assert(transfer_bytes == 3u);
}

static void TestRetainedResponse(void)
{
    ResetState();
    SetCommand(8u, 4u);
    memory[CDB + 4u] = 3u;
    WriteU16(0x00fbd7ul, 0x1234u);
    memory[0x00fbd9ul] = 0x56u;
    HandleScsiOpcodeF5();
    assert(transfer_source == 0x000b2aul);
    assert(transfer_bytes == 4u);
    assert(ReadU16(0x000b2aul) == 0x1234u);
    assert(ReadU16(0x000b2cul) == 0x0056u);
    assert(ReadU16(0x00fbd7ul) == 0u);
    assert(memory[0x00fbd9ul] == 0u);
    assert(no_sense_count == 1u);

    ResetState();
    transfer_result = 4u;
    SetCommand(8u, 4u);
    memory[CDB + 4u] = 3u;
    WriteU16(0x00fbd7ul, 0x789au);
    memory[0x00fbd9ul] = 0xbcu;
    HandleScsiOpcodeF5();
    assert(ReadU16(0x00fbd7ul) == 0x789au);
    assert(memory[0x00fbd9ul] == 0xbcu);
    assert(no_sense_count == 0u);

    ResetState();
    SetCommand(8u, 4u);
    memory[CDB + 4u] = 1u;
    WriteU16(0x00fbd7ul, 0x1234u);
    memory[0x00fbd9ul] = 0x56u;
    HandleScsiOpcodeF5();
    assert(transfer_count == 0u);
    assert(ReadU16(0x00fbd7ul) == 0u);
    assert(memory[0x00fbd9ul] == 0u);
    assert(no_sense_count == 1u);
}

static void TestDelegatesAndRejections(void)
{
    ResetState();
    SetCommand(9u, 0u);
    HandleScsiOpcodeF5();
    assert(selector09_count == 1u);
    assert(no_sense_count == 0u);

    ResetState();
    SetCommand(0x0au, 0u);
    HandleScsiOpcodeF5();
    assert(strategy_count == 1u);

    ResetState();
    SetCommand(0x0bu, 0u);
    HandleScsiOpcodeF5();
    assert(fixed52_count == 1u);

    ResetState();
    SetCommand(2u, 0u);
    HandleScsiOpcodeF5();
    assert(unsupported_count == 1u);

    ResetState();
    SetCommand(0x7fu, 0u);
    HandleScsiOpcodeF5();
    assert(unsupported_count == 1u);
}

static void TestZeroAllocationAndTransferStatus(void)
{
    ResetState();
    SetCommand(1u, 0u);
    HandleScsiOpcodeF5();
    assert(transfer_count == 0u);
    assert(no_sense_count == 1u);

    ResetState();
    transfer_result = 4u;
    SetCommand(1u, 12u);
    HandleScsiOpcodeF5();
    assert(transfer_count == 1u);
    assert(no_sense_count == 0u);
}

int main(void)
{
    TestDirectMemoryAndCapacity();
    TestFixedAndDynamicWindows();
    TestLocalStatusAndDescriptorPayload();
    TestRetainedResponse();
    TestDelegatesAndRejections();
    TestZeroAllocationAndTransferStatus();
    return 0;
}
