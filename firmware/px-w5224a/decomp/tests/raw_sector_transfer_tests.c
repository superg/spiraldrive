#include <assert.h>
#include <string.h>
#include "media/raw_sector_transfer.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 bit_000d;
static u16 descriptor_address;
static unsigned sense_calls;
static u8 sense_key;
static u16 sense_code;
static unsigned information_calls;
static u32 sense_information;
static unsigned no_sense_calls;
static unsigned configure_calls;
static unsigned normalize_calls;
static u32 normalize_address;
static u8 normalize_type;
static unsigned prepare_calls;
static u16 prepared_size;
static u32 prepared_result;
static unsigned start_calls;
static u32 started_value;
static unsigned yield_calls;
static fw_addr_t port_write_addresses[64];
static u8 port_write_values[64];
static unsigned port_write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    address &= 0xfffffful;
    memory[address] = value;
    if(address >= 0x01800dul && address <= 0x018021ul)
    {
        port_write_addresses[port_write_count] = address;
        port_write_values[port_write_count++] = value;
    }
}

u8 ObserveRawSectorTransferBit000D(void)
{
    return bit_000d;
}
u16 ObserveRawSectorTransferGetDescriptor(void)
{
    return descriptor_address;
}
void ObserveRawSectorTransferSetSense(u8 key, u16 code)
{
    ++sense_calls;
    sense_key = key;
    sense_code = code;
}
void ObserveRawSectorTransferSetInformation(u32 information)
{
    ++information_calls;
    sense_information = information;
}
void ObserveRawSectorTransferNoSense(void)
{
    ++no_sense_calls;
}
void ObserveRawSectorTransferConfigureAta(void)
{
    ++configure_calls;
}
void ObserveRawSectorTransferNormalize(u32 address, u8 header_type)
{
    ++normalize_calls;
    normalize_address = address;
    normalize_type = header_type;
}
u32 ObserveRawSectorTransferPrepareSize(u16 block_size)
{
    ++prepare_calls;
    prepared_size = block_size;
    return prepared_result;
}
void ObserveRawSectorTransferStart(u32 transfer_value)
{
    ++start_calls;
    started_value = transfer_value;
}
void ObserveRawSectorTransferYield(void)
{
    ++yield_calls;
    memory[0x001462u] = 0u;
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

static u16 ReadBe16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadBe32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static u32 ControllerAddress(void)
{
    return (u32)memory[0x018010u] | ((u32)memory[0x018011u] << 8) | ((u32)memory[0x018012u] << 16);
}

static u16 ControllerLength(void)
{
    return (u16)((u16)memory[0x01800du] | ((u16)memory[0x01800eu] << 8));
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(port_write_addresses, 0, sizeof(port_write_addresses));
    memset(port_write_values, 0, sizeof(port_write_values));
    bit_000d = 0u;
    descriptor_address = 0x1000u;
    sense_calls = 0u;
    sense_key = 0u;
    sense_code = 0u;
    information_calls = 0u;
    sense_information = 0u;
    no_sense_calls = 0u;
    configure_calls = 0u;
    normalize_calls = 0u;
    normalize_address = 0u;
    normalize_type = 0u;
    prepare_calls = 0u;
    prepared_size = 0u;
    prepared_result = 0x89abcdeful;
    start_calls = 0u;
    started_value = 0u;
    yield_calls = 0u;
    port_write_count = 0u;
    WriteBe32(0x000b02u, 1000ul);
    WriteBe32(0x000b06u, 20ul);
    WriteBe32(0x001459u, 10ul);
    WriteBe16(0x001463u, 2u);
    WriteBe32(descriptor_address + 4u, 0x00123400ul);
    WriteBe32(descriptor_address + 10u, 0x00567800ul);
}

static void AssertTransferStarted(u16 size, u32 completed)
{
    assert(configure_calls == 1u);
    assert(prepare_calls == 1u && prepared_size == size);
    assert(start_calls == 1u && started_value == prepared_result);
    assert(yield_calls == 1u && memory[0x001462u] == 0u);
    assert(ReadBe32(0x001459u) == completed);
}

static void TestNormalLayoutRejections(void)
{
    struct Case
    {
        u8 type;
        u16 size;
    } cases[] = {
        { 0u, 0x0808u },
        { 0u, 0x0914u },
        { 1u, 0x0804u },
        { 2u, 0x0914u },
        { 3u, 0x080cu }
    };
    unsigned index;
    u32 expected_information;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetState();
        memory[descriptor_address + 0x13u] = cases[index].type;
        WriteBe16(0x00145du, cases[index].size);
        WriteBe16(0x006402u, 0x2200u);
        WriteBe16(0x002200u, 0x1234u);
        WriteBe32(0x000f0cu, 0x100ul);
        expected_information = 0x12346400ul - 0x100ul - 150ul;
        assert(ConfigureAndTransferRawReadSector() == 0u);
        assert(sense_calls == 1u && sense_key == 5u);
        assert(sense_code == 0x6400u && information_calls == 1u);
        assert(sense_information == expected_information);
        assert(configure_calls == 0u && start_calls == 0u);
        assert(ReadBe32(0x000f14u) == 1170ul);
    }
}

static void TestNormalAddressLayouts(void)
{
    struct Case
    {
        u8 type;
        u16 size;
        u32 adjustment;
        u16 controller_length;
        u8 command;
    } cases[] = {
        { 0u, 0x0800u, 0x10u, 0x0800u, 0x88u },
        { 1u, 0x0800u, 0x18u, 0x0800u, 0x88u },
        { 2u, 0x0808u, 0x10u, 0x0808u, 0x88u },
        { 4u, 0x0804u, 0x0cu, 0x0804u, 0x88u },
        { 4u, 0x080cu, 0x0cu, 0x080cu, 0x88u },
        { 4u, 0x0914u, 0x18u, 0x0914u, 0x88u },
        { 4u, 0x0918u, 0x18u, 0x0918u, 0x88u },
        { 4u, 0x0920u, 0x10u, 0x0920u, 0x88u },
        { 4u, 0x0924u, 0x0cu, 0x0924u, 0x88u },
        { 4u, 0x0990u, 0u,    0x0990u, 0x88u }
    };
    unsigned index;

    for(index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    {
        ResetState();
        memory[descriptor_address + 0x13u] = cases[index].type;
        WriteBe16(0x00145du, cases[index].size);
        assert(ConfigureAndTransferRawReadSector() == 1u);
        assert(ControllerAddress() == 0x00123400ul + cases[index].adjustment);
        assert(ControllerLength() == cases[index].controller_length);
        assert(memory[0x018020u] == cases[index].command);
        AssertTransferStarted(cases[index].size, 12ul);
    }
}

static void TestNormalA56SplitLayout(void)
{
    unsigned index;
    unsigned address_write_count = 0u;

    ResetState();
    memory[descriptor_address + 0x13u] = 4u;
    WriteBe16(0x00145du, 0x0a56u);
    assert(ConfigureAndTransferRawReadSector() == 1u);
    assert(memory[0x018020u] == 0x8au);
    assert(ControllerLength() == 0x0126u);
    assert(ControllerAddress() == 0x00567800ul);
    for(index = 0u; index < port_write_count; ++index)
        if(port_write_addresses[index] == 0x018010ul)
            ++address_write_count;
    assert(address_write_count == 2u);
    AssertTransferStarted(0x0a56u, 12ul);
}

static void ConfigureRawFormatTables(u8 type, u8 option, u16 main_size, u8 command_index, u8 command)
{
    u8 layout = (u8)(((u8)(type & 0xfeu) << 4) + option);

    memory[0xff8ebau + layout] = 1u;
    memory[0xff8efau + layout] = 0x20u;
    WriteBe16(0xff8f3au + (fw_addr_t)type * 64u + (fw_addr_t)option * 2u, main_size);
    memory[0xff903au + command_index] = command;
}

static void TestRawFormatValidationAndSequence(void)
{
    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    assert(ConfigureAndTransferRawReadSector() == 0u);
    assert(sense_key == 5u && sense_code == 0x6400u);

    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    ConfigureRawFormatTables(2u, 1u, 0x0800u, 5u, 0x88u);
    memory[0x000f0au] = 1u;
    memory[0x000f09u] = 9u;
    assert(ConfigureAndTransferRawReadSector() == 0u);
    assert(memory[0x000f09u] == 0u && sense_key == 5u);

    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    ConfigureRawFormatTables(2u, 1u, 0x0800u, 5u, 0x88u);
    memory[0x000f09u] = 1u;
    WriteBe32(0x000b02u, 1000u);
    WriteBe32(0x000f14u, 1152u);
    assert(ConfigureAndTransferRawReadSector() == 1u);
    assert(sense_key == 3u && sense_code == 0x1500u);
    assert(configure_calls == 0u && start_calls == 0u);

    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    ConfigureRawFormatTables(2u, 1u, 0u, 0u, 0u);
    assert(ConfigureAndTransferRawReadSector() == 0u);
    assert(no_sense_calls == 1u && sense_calls == 0u);
}

static void TestRawFormatComponentLayouts(void)
{
    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    ConfigureRawFormatTables(2u, 1u, 0x0800u, 5u, 0x87u);
    assert(ConfigureAndTransferRawReadSector() == 1u);
    assert(ControllerAddress() == 0x00123420ul);
    assert(ControllerLength() == 0x0800u);
    assert(memory[0x018020u] == 0x87u);
    assert(memory[0x000f09u] == 4u);
    assert(ReadBe32(0x000f14u) == 1170ul);
    AssertTransferStarted(0x0800u, 12ul);

    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 10u;
    memory[0x000f06u] = 2u;
    ConfigureRawFormatTables(2u, 1u, 8u, 17u, 0x91u);
    WriteBe32(0x00156eu, 0x00200000ul);
    WriteBe32(0x001556u, 0x00300000ul);
    assert(ConfigureAndTransferRawReadSector() == 1u);
    assert(memory[0x018020u] == 0x91u);
    assert(ControllerAddress() == 0x002fffe0ul);
    assert(ControllerLength() == 0x10u);
    assert(normalize_calls == 1u);
    assert(normalize_address == 0x00200010ul && normalize_type == 0x1au);
    assert(ReadBe16(0x001463u) == 1u);
    AssertTransferStarted(0x013eu, 11ul);

    ResetState();
    bit_000d = 1u;
    memory[descriptor_address + 0x13u] = 2u;
    memory[0x000f0bu] = 8u;
    memory[0x000f06u] = 1u;
    ConfigureRawFormatTables(2u, 1u, 0u, 1u, 0x92u);
    WriteBe32(0x001566u, 0x00400000ul);
    assert(ConfigureAndTransferRawReadSector() == 1u);
    assert(ControllerAddress() == 0x00400060ul);
    assert(memory[0x018020u] == 0x92u);
    assert(normalize_calls == 0u);
    AssertTransferStarted(0x60u, 12ul);
}

int main(void)
{
    TestNormalLayoutRejections();
    TestNormalAddressLayouts();
    TestNormalA56SplitLayout();
    TestRawFormatValidationAndSequence();
    TestRawFormatComponentLayouts();
    return 0;
}
