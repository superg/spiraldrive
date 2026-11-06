#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "scsi/raw_sector_batched_host_transfer.h"

#define MEMORY_SIZE 0x20000u
#define ROM_BASE 0x00ff95e2ul
#define ROM_SIZE 0x20u

static u8 memory[MEMORY_SIZE];
static u8 rom[ROM_SIZE];
static u8 bit_000d;
static u8 bit_0045;
static u16 descriptor_address;
static unsigned configure_count;
static unsigned normalize_count;
static u32 normalize_address;
static u8 normalize_type;
static unsigned multiply_count;
static u16 multiply_value;
static u32 multiply_result;
static unsigned start_count;
static u32 start_value;
static u32 address_writes[8];
static unsigned address_write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address >= ROM_BASE && address < ROM_BASE + ROM_SIZE)
        return rom[address - ROM_BASE];
    assert(address < MEMORY_SIZE);
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < MEMORY_SIZE);
    memory[address] = value;
    if(address == 0x018012ul)
    {
        assert(address_write_count < 8u);
        address_writes[address_write_count++] = (u32)memory[0x18010u] | ((u32)memory[0x18011u] << 8) | ((u32)memory[0x18012u] << 16);
    }
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

u8 FirmwareReadBit000D(void)
{
    return bit_000d;
}

u8 FirmwareReadBit0045(void)
{
    return bit_0045;
}

u16 GetRawReadSecondaryCursorDescriptorAddress(void)
{
    return descriptor_address;
}

void ConfigureAtaDataTransferStatus(void)
{
    ++configure_count;
}

void CopyAndNormalizeRawReadHeaderToLinkedBuffer(u32 address, u8 type)
{
    ++normalize_count;
    normalize_address = address;
    normalize_type = type;
}

u32 FUN_RAM_fde9cf(u16 value)
{
    ++multiply_count;
    multiply_value = value;
    return multiply_result;
}

void StartRawReadHostTransferFromStackAddress(u32 value)
{
    ++start_count;
    start_value = value;
}

static void StoreU16(u32 address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static void StoreU32(u32 address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u16 ControllerLength(void)
{
    return (u16)((u16)memory[0x1800du] | ((u16)memory[0x1800eu] << 8));
}

static u32 ControllerAddress(void)
{
    return (u32)memory[0x18010u] | ((u32)memory[0x18011u] << 8) | ((u32)memory[0x18012u] << 16);
}

static void Reset(void)
{
    memset(memory, 0xa5, sizeof(memory));
    memset(rom, 0, sizeof(rom));
    bit_000d = 0u;
    bit_0045 = 0u;
    descriptor_address = 0x3000u;
    configure_count = 0u;
    normalize_count = 0u;
    normalize_address = 0ul;
    normalize_type = 0u;
    multiply_count = 0u;
    multiply_value = 0u;
    multiply_result = 0x12345678ul;
    start_count = 0u;
    start_value = 0ul;
    address_write_count = 0u;
    StoreU32(descriptor_address + 4u, 0x00100000ul);
    StoreU16(descriptor_address + 8u, 3u);
    StoreU32(descriptor_address + 10u, 0x00110000ul);
    StoreU32(0x1556u, 0x001b0000ul);
    StoreU32(0x1566u, 0x0014ae00ul);
    StoreU32(0x156eu, 0x00156f00ul);
    StoreU16(0x145du, 0x0930u);
    rom[0] = 0x70u;
    rom[1] = 0x71u;
    rom[2] = 0x72u;
    rom[3] = 0x73u;
    rom[4] = 0x74u;
    rom[7] = 0x77u;
    rom[8] = 0x78u;
    rom[9] = 0x79u;
    rom[10] = 0x7au;
    rom[11] = 0x7bu;
}

static void AssertCommon(void)
{
    assert(configure_count == 1u);
    assert(memory[0x1801fu] == 0u && memory[0x18021u] == 0u);
    assert(multiply_count == 1u && multiply_value == 0x0930u);
    assert(memory[0x0f47u] == 1u);
    assert(start_count == 1u && start_value == multiply_result);
}

static void TestFormatZeroMainTransfer(void)
{
    Reset();
    memory[0x0f06u] = 0u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x70u);
    assert(ControllerLength() == 0xa5a5u);
    assert(ControllerAddress() == 0x100110ul);
    assert(address_write_count == 1u && address_writes[0] == 0x100110ul);
    AssertCommon();
}

static void TestSubchannelFormats(void)
{
    u8 format;

    for(format = 2u; format <= 4u; format = (u8)(format + 2u))
    {
        Reset();
        memory[0x0f06u] = format;
        StartBatchedRawReadHostTransferFromSecondaryCursor();
        assert(memory[0x18020u] == rom[format]);
        assert(ControllerLength() == 0x0060u);
        assert(ControllerAddress() == 0x14af20ul);
        assert(address_write_count == 2u);
        assert(address_writes[0] == 0x100110ul);
        assert(address_writes[1] == 0x14af20ul);
        AssertCommon();
    }

    Reset();
    memory[0x0f06u] = 3u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x73u);
    assert(ControllerLength() == 0xa5a5u);
    assert(ControllerAddress() == 0x14af20ul);
    assert(address_write_count == 1u);
}

static void TestMetadataAndLinkedHeader(void)
{
    Reset();
    memory[0x0f06u] = 1u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(ControllerLength() == 0x0010u);
    assert(ControllerAddress() == 0x156f30ul);
    assert(memory[0x1463u] == 0u && memory[0x1464u] == 1u);
    assert(normalize_count == 0u);

    Reset();
    memory[0x0f06u] = 1u;
    bit_000d = 1u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(normalize_count == 1u);
    assert(normalize_address == 0x00156f30ul && normalize_type == 0x1au);
    assert(ControllerLength() == 0x0010u);
    assert(ControllerAddress() == 0x1affe0ul);
}

static void TestC2SelectionAndAddressOrder(void)
{
    Reset();
    memory[0x0f06u] = 2u;
    memory[0x0f0bu] = 2u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x79u);
    assert(ControllerLength() == 0x0060u);
    assert(address_write_count == 3u);
    assert(address_writes[0] == 0x100110ul);
    assert(address_writes[1] == 0x110022ul);
    assert(address_writes[2] == 0x14af20ul);

    Reset();
    memory[0x0f06u] = 3u;
    memory[0x0f0bu] = 2u;
    bit_0045 = 1u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x82u);
    assert(ControllerLength() == 0x0060u);
    assert(address_write_count == 2u);
    assert(address_writes[0] == 0x110022ul);
    assert(address_writes[1] == 0x14af20ul);
}

static void TestBit45MainSelector(void)
{
    Reset();
    memory[0x0f06u] = 2u;
    bit_0045 = 1u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x80u);
    assert(address_write_count == 1u);
    assert(ControllerAddress() == 0x14af20ul);

    Reset();
    memory[0x0f06u] = 0u;
    memory[0x0f0bu] = 2u;
    bit_0045 = 1u;
    StartBatchedRawReadHostTransferFromSecondaryCursor();
    assert(memory[0x18020u] == 0x80u);
    assert(ControllerLength() == 0x0126u);
    assert(ControllerAddress() == 0x110022ul);
}

int main(void)
{
    TestFormatZeroMainTransfer();
    TestSubchannelFormats();
    TestMetadataAndLinkedHeader();
    TestC2SelectionAndAddressOrder();
    TestBit45MainSelector();
    puts("raw sector batched host transfer tests passed");
    return 0;
}
