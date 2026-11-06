#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/startup_media_state_initialization.h"

static u8 memory_bytes[0x20000ul];
static u16 default_radial_step_scale;
static u8 bit_values[0x40u];
static u8 indexed_result;
static unsigned event_count;
static u8 events[16];
static u8 e8e9_argument;
static u8 indexed_argument;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x00f848feul)
        return (u8)(default_radial_step_scale >> 8);
    if(address == 0x00f848fful)
        return (u8)default_radial_step_scale;
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

void ObserveStartupMediaWriteBit(u8 bit_address, u8 value)
{
    assert(bit_address < sizeof(bit_values));
    bit_values[bit_address] = value;
}

void ObserveStartupMediaRebuildRadialTable(void)
{
    events[event_count++] = 1u;
}

void ObserveStartupMediaClearStateFlags(void)
{
    events[event_count++] = 2u;
}

void ObserveStartupMediaWriteE8E9(u8 value)
{
    e8e9_argument = value;
    events[event_count++] = 3u;
}

void ObserveStartupMediaClearSpeedReduction(void)
{
    events[event_count++] = 4u;
}

void ObserveStartupMediaInitializeDiscControl(void)
{
    events[event_count++] = 5u;
}

u8 ObserveStartupMediaIndexedState(u8 state_code)
{
    indexed_argument = state_code;
    events[event_count++] = 6u;
    return indexed_result;
}

void ObserveStartupMediaCapturePosition(void)
{
    events[event_count++] = 7u;
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0xff, sizeof(memory_bytes));
    memset(bit_values, 0xff, sizeof(bit_values));
    memset(events, 0, sizeof(events));
    default_radial_step_scale = 0x2468u;
    indexed_result = 0u;
    event_count = 0u;
    e8e9_argument = indexed_argument = 0xffu;
}

static void AssertZeroRange(fw_addr_t address, unsigned count)
{
    unsigned index;

    for(index = 0u; index < count; ++index)
        assert(memory_bytes[address + index] == 0u);
}

static void TestFullInitializationWithRetainedConfiguration(void)
{
    ResetFixture();
    InitializeStartupMediaState();

    AssertZeroRange(0x100au, 32u);
    assert(memory_bytes[0x102cu] == 0u);
    assert(memory_bytes[0x102du] == 0xfeu);
    assert(memory_bytes[0x1008u] == 0u);
    assert(bit_values[0x28u] == 0u && bit_values[0x31u] == 0u);
    assert(bit_values[0x36u] == 0u && bit_values[0x37u] == 0u);
    assert(bit_values[0x38u] == 0u && bit_values[0x3au] == 0u);
    assert(ReadDword(0xcf5u) == 0x00014cccul);
    assert(ReadWord(0xcf9u) == 0x0640u);
    assert(ReadDword(0xfc37u) == 0u && ReadDword(0xfc33u) == 0u);
    assert(ReadWord(0x1000u) == 0x2468u);
    assert(ReadDword(0xef1u) == 0u && memory_bytes[0xed5u] == 0u);

    assert(memory_bytes[0x1301u] == 0xddu);
    assert(memory_bytes[0x1302u] == 0x66u);
    assert(memory_bytes[0x1303u] == 0x0cu);
    assert(memory_bytes[0x1304u] == 0x6du);
    assert(memory_bytes[0x1305u] == 0xa2u);
    assert(memory_bytes[0x1306u] == 0u);
    assert(memory_bytes[0x1307u] == 0xfcu);
    assert(memory_bytes[0xff1u] == 0u && ReadWord(0xff5u) == 0xffb7u);
    assert(memory_bytes[0xfab0u] == 0u && memory_bytes[0xfab1u] == 0u && memory_bytes[0xfab2u] == 0u);
    assert(memory_bytes[0xfc0bu] == 0u && memory_bytes[0xfb98u] == 0u);
    assert(ReadDword(0x14d6u) == 0u);
    assert(memory_bytes[0xe4cu] == 0x10u && memory_bytes[0xf971u] == 0u);
    assert(memory_bytes[0xfb9u] == 0xfeu);
    assert(ReadWord(0xfaacu) == 0x1ec3u);
    assert(memory_bytes[0xf79bu] == 0u);

    assert(ReadWord(0xfad1u) == 0u && ReadWord(0xfacfu) == 0u);
    assert(memory_bytes[0xfc1fu] == 0x34u);
    assert(memory_bytes[0xfc21u] == 0xf0u && ReadWord(0xfc22u) == 0u);
    assert(memory_bytes[0x1132u] == 8u);
    assert(memory_bytes[0xf9a8u] == 0u && memory_bytes[0x158eu] == 0xf0u);
    assert(memory_bytes[0x158bu] == 0u && ReadWord(0x1589u) == 0u);
    assert(memory_bytes[0xfb99u] == 0u);
    AssertZeroRange(0xfcbu, 6u);
    assert(memory_bytes[0xfaeau] == 0u && memory_bytes[0xfafeu] == 0u);
}

static void TestTailStateAndHelperOrder(void)
{
    ResetFixture();
    InitializeStartupMediaState();

    assert(ReadWord(0xfb96u) == 0u);
    assert(memory_bytes[0xfaedu] == 0xe4u);
    assert(memory_bytes[0xfaeeu] == 0u);
    assert(memory_bytes[0xfaefu] == 0x85u);
    assert(memory_bytes[0xfaf0u] == 0u);
    assert(memory_bytes[0x102fu] == 0u && memory_bytes[0x102eu] == 0u);
    AssertZeroRange(0xcfbu, 0x140u);
    assert(ReadDword(0xfbedu) == 0u);
    assert(memory_bytes[0xf9e8u] == 100u && memory_bytes[0xf9e7u] == 100u);
    assert(memory_bytes[0xf9e9u] == 0xf8u);
    assert(memory_bytes[0xfaecu] == 0u && ReadWord(0xfc30u) == 0u);
    assert(memory_bytes[0xfc32u] == 0xfcu);
    assert(event_count == 6u);
    assert(events[0] == 1u && events[1] == 2u && events[2] == 3u && events[3] == 4u && events[4] == 5u && events[5] == 6u);
    assert(e8e9_argument == 0u && indexed_argument == 4u);
}

static void TestConfigurationAndConditionalBranches(void)
{
    ResetFixture();
    memory_bytes[0x11dcu] = 0u;
    memory_bytes[0xfaedu] = 4u;
    memory_bytes[0xc88u] = 0u;
    memory_bytes[0xc43u] = 0u;
    indexed_result = 1u;
    InitializeStartupMediaState();
    assert(ReadWord(0xfaacu) == 0x18c3u);
    assert(memory_bytes[0xfae9u] == 0u);
    assert(ReadWord(0x11dau) == 0x0960u && memory_bytes[0x11dcu] == 2u);
    assert(memory_bytes[0xc43u] == 0x28u && memory_bytes[0xfaedu] == 0u);
    assert(event_count == 7u && events[6] == 7u);

    ResetFixture();
    memory_bytes[0xfaedu] = 4u;
    memory_bytes[0xc88u] = 4u;
    memory_bytes[0xc43u] = 0x5au;
    memory_bytes[0xfaefu] = 0x05u;
    InitializeStartupMediaState();
    assert(memory_bytes[0xc43u] == 0x5au);
    assert(memory_bytes[0xfaefu] == 0x04u);
}

int main(void)
{
    TestFullInitializationWithRetainedConfiguration();
    TestTailStateAndHelperOrder();
    TestConfigurationAndConditionalBranches();
    puts("startup_media_state_initialization_tests: ok");
    return 0;
}
