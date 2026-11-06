#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "startup/indexed_read_state_configuration.h"

static u8 memory_bytes[0x14000u];
static fw_addr_t converted_address;
static unsigned convert_count;
static unsigned divide_count;
static s32 divided_values[3];
static u8 configuration_byte;
static unsigned initialize_count;
static unsigned advance_count;
static unsigned reset_count;
static u16 reset_seed;
static u8 bit37;
static u8 p3_bit0;
static u8 p3_bit1;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory_bytes));
    return memory_bytes[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory_bytes));
    memory_bytes[address] = value;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

s32 ObserveIndexedReadConvertMsf(fw_addr_t tuple_address)
{
    converted_address = tuple_address;
    convert_count++;
    return (s32)(tuple_address ^ 0x0055aa55ul);
}

s32 ObserveIndexedReadDivide(s32 dividend, s32 divisor)
{
    assert(divisor == 75l);
    divided_values[divide_count++] = dividend;
    return dividend / divisor;
}

u16 ObserveIndexedReadMapSeconds(u16 frame_seconds)
{
    return (u16)(0x1000u + frame_seconds);
}

void ObserveIndexedReadConfigurationByte(fw_addr_t offset, u8 *destination, u16 count)
{
    assert(offset == 0x3eul && count == 1u);
    *destination = configuration_byte;
}

void ObserveIndexedReadInitializeSubsystem(void)
{
    initialize_count++;
}

void ObserveIndexedReadAdvanceStateMachine(void)
{
    advance_count++;
}

void ObserveIndexedReadResetPseudoRandom(u16 seed)
{
    reset_count++;
    reset_seed = seed;
}

void ObserveIndexedReadWriteBit(u16 bit_address, u8 value)
{
    if(bit_address == 0x37u)
        bit37 = value;
    else if(bit_address == 0x580u)
        p3_bit0 = value;
    else
    {
        assert(bit_address == 0x581u);
        p3_bit1 = value;
    }
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    converted_address = 0u;
    convert_count = divide_count = 0u;
    memset(divided_values, 0, sizeof(divided_values));
    configuration_byte = 0u;
    initialize_count = advance_count = reset_count = 0u;
    reset_seed = 0u;
    bit37 = p3_bit0 = p3_bit1 = 0xffu;
    memory_bytes[0x1301u] = 0xffu;
    memory_bytes[0x1302u] = 0xffu;
}

static void PrepareActiveState(u8 state, u8 media_type)
{
    ResetFixture();
    memory_bytes[0x1008u] = media_type;
    memory_bytes[0x1132u] = 0x40u;
    memory_bytes[0x1147u] = 2u;
    WriteDword(0x1153u, 7500ul);
    memory_bytes[0x1308u] = 1u;
    memory_bytes[0x130au] = state;
}

static void TestTargetAndEndpointSelection(void)
{
    ResetFixture();
    memory_bytes[0x1008u] = 0u;
    memory_bytes[0x1132u] = 0xffu;
    memory_bytes[0x12bfu] = 7u;
    WriteDword(0x12c1u, 7500ul);
    memory_bytes[0x12beu] = 1u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0x1132u] == 0xe7u);
    assert(memory_bytes[0xfbbu] == 7u && memory_bytes[0xe42u] == 7u);
    assert(converted_address == 0xf2b9ul + 42u && convert_count == 1u);
    assert(ReadDword(0xe3bu) == ((0xf2b9ul + 42u) ^ 0x0055aa55ul));
    assert(divide_count == 1u && divided_values[0] == 7500l);
    assert(ReadWord(0xff7u) == 0x1064u);
    assert(memory_bytes[0xfafu] == 3u && memory_bytes[0xe44u] == 4u);
    assert(memory_bytes[0x1301u] == 0xbfu);
    assert(reset_count == 1u && reset_seed == 1u);

    ResetFixture();
    memory_bytes[0x1008u] = 1u;
    memory_bytes[0x1132u] = 0x40u;
    memory_bytes[0x1147u] = 4u;
    WriteDword(0x1153u, 15000ul);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(converted_address == 0x2bb042ul + 40u);
    assert(memory_bytes[0xfbbu] == 4u && memory_bytes[0xe42u] == 4u);
    assert(divide_count == 1u && divided_values[0] == 15000l);
    assert(ReadWord(0xff7u) == 0x10c8u);

    ResetFixture();
    memory_bytes[0x1008u] = 1u;
    memory_bytes[0x1132u] = 0xc0u;
    WriteDword(0x1153u, 22500ul);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0x1132u] == 0xd0u);
    assert(ReadDword(0xe3bu) == 0u && convert_count == 0u);
    assert(ReadWord(0xff7u) == 0x112cu);

    ResetFixture();
    memory_bytes[0x1008u] = 2u;
    memory_bytes[0x1132u] = 0x08u;
    memory_bytes[0x12bfu] = 3u;
    WriteDword(0x1153u, 750ul);
    WriteDword(0x12c1u, 1500ul);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0x1132u] == 0u);
    assert(divide_count == 2u && divided_values[0] == 750l && divided_values[1] == 1500l);
    assert(ReadWord(0xff7u) == 0x1014u);
}

static void TestOrdinaryAndSpecialStateDispatch(void)
{
    static const u8 unchanged_states[] = { 1u, 2u, 3u, 4u, 7u, 9u, 0x0eu, 0x14u, 0x15u, 0x16u, 0x17u, 0xf2u, 0xf3u, 0xf4u, 0xfdu, 0xffu };
    unsigned index;

    for(index = 0u; index < sizeof(unchanged_states); ++index)
    {
        PrepareActiveState(unchanged_states[index], 1u);
        ConfigureMediaReadStateForIndexedDriveState();
        assert(memory_bytes[0xfafu] == 3u);
        if(unchanged_states[index] == 0x17u)
            assert((memory_bytes[0x1302u] & 4u) != 0u);
    }

    PrepareActiveState(0u, 1u);
    configuration_byte = 0u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfafu] == 2u && ReadDword(0xe3bu) == 0x4ce78ul && memory_bytes[0xe44u] == 0u);
    PrepareActiveState(0u, 1u);
    configuration_byte = 0x81u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(ReadDword(0xe3bu) == 0x47694ul);

    PrepareActiveState(5u, 1u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(ReadDword(0xe3bu) == 45000ul && memory_bytes[0xfafu] == 1u);
    PrepareActiveState(6u, 1u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xf9e1u] == 1u && ReadWord(0xf9dfu) == 0x1eu && (memory_bytes[0x1302u] & 4u) == 0u);

    PrepareActiveState(0x0au, 1u);
    WriteDword(0x11f0u, 0x12345678ul);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(ReadDword(0x11f0u) == 0u && memory_bytes[0xfafu] == 1u);
    PrepareActiveState(0x0cu, 1u);
    memory_bytes[0xfbbu] = memory_bytes[0xe42u] = 0xa5u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfbbu] == 0u && memory_bytes[0xe42u] == 0u);
    PrepareActiveState(0x11u, 1u);
    memory_bytes[0x1302u] = 0u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0x1302u] == 4u);
    PrepareActiveState(0x12u, 1u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(initialize_count == 1u && memory_bytes[0xfbbu] == 0u && memory_bytes[0xe42u] == 0u);
    PrepareActiveState(0x13u, 1u);
    memory_bytes[0xfae6u] = 0xa5u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfae6u] == 0u);

    PrepareActiveState(8u, 1u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfafu] == 1u);
}

static void TestState31SpeedPinsAndFinalOverride(void)
{
    unsigned index;

    PrepareActiveState(0x1fu, 1u);
    for(index = 0u; index < 12u; ++index)
        memory_bytes[0xf9abu + index] = 0xa5u;
    memory_bytes[0x1303u] = 8u;
    ConfigureMediaReadStateForIndexedDriveState();
    for(index = 0u; index < 12u; ++index)
        assert(memory_bytes[0xf9abu + index] == 0u);
    assert(memory_bytes[0xf977u] == 3u && bit37 == 1u && advance_count == 1u);

    PrepareActiveState(0x1fu, 2u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xf977u] == 1u && memory_bytes[0xf9e1u] == 0xffu && ReadWord(0xf9dfu) == 0x1eu && advance_count == 0u);

    PrepareActiveState(0xf6u, 1u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfafu] == 5u && memory_bytes[0xe47u] == 12u);
    PrepareActiveState(0xf6u, 2u);
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xe47u] == 2u);

    PrepareActiveState(0xf9u, 1u);
    memory_bytes[0x1301u] = 0x15u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfafu] == 1u && memory_bytes[0x1301u] == 0xd5u && p3_bit0 == 0u && p3_bit1 == 1u);

    PrepareActiveState(5u, 1u);
    memory_bytes[0x1309u] = 1u;
    memory_bytes[0x12beu] = 1u;
    ConfigureMediaReadStateForIndexedDriveState();
    assert(memory_bytes[0xfafu] == 1u);
}

int main(void)
{
    TestTargetAndEndpointSelection();
    TestOrdinaryAndSpecialStateDispatch();
    TestState31SpeedPinsAndFinalOverride();
    puts("indexed_read_state_configuration_tests: ok");
    return 0;
}
