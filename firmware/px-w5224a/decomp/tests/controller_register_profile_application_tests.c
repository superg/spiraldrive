#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_register_profile_application.h"
#include "platform/firmware_memory.h"

typedef struct
{
    fw_addr_t address;
    u8 value;
} WriteEvent;

static u8 memory[0x1000000ul];
static WriteEvent writes[128];
static unsigned write_count;
static char bit_events[8];
static unsigned bit_event_count;
static u8 fixed_code;
static unsigned fixed_count;
static unsigned derived_count;
static u8 e8_value;
static unsigned e8_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    writes[write_count].address = address;
    writes[write_count].value = value;
    ++write_count;
}

u32 MultiplyU32Low(u32 left, u32 right)
{
    return left * right;
}

void ObserveControllerRegisterProfileClearEa(void)
{
    bit_events[bit_event_count++] = 'c';
}
void ObserveControllerRegisterProfileSetEa(void)
{
    bit_events[bit_event_count++] = 'a';
}
void ObserveControllerRegisterProfileSetEc(void)
{
    bit_events[bit_event_count++] = 'e';
}
void ObserveControllerRegisterProfileSetA9Bit2(void)
{
    bit_events[bit_event_count++] = '9';
}
void ObserveApplyFixedControllerRegisterProfile(u8 operating_code)
{
    fixed_code = operating_code;
    ++fixed_count;
}
void ObserveUpdateControllerProfileDerivedValue(void)
{
    ++derived_count;
}
void ObserveWriteControllerProfileE8Value(u8 value)
{
    e8_value = value;
    ++e8_count;
}

static void StoreWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1ul] = (u8)value;
}

static u16 LoadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1ul]);
}

static fw_addr_t RecordAddress(u8 index)
{
    return 0x00e806ul + (fw_addr_t)((u16)index * 34u);
}

static void SetRecordSource(u8 index, u8 coordinate)
{
    memory[RecordAddress(index) + 33ul] = coordinate;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(writes, 0, sizeof(writes));
    memset(bit_events, 0, sizeof(bit_events));
    write_count = bit_event_count = 0u;
    fixed_code = 0u;
    fixed_count = derived_count = e8_count = 0u;
    e8_value = 0u;
    memory[0x00fa01ul] = 0xffu;
    memory[0x00fa04ul] = 0x20u;
    memory[0x00fab5ul] = 0x03u;
}

static void TestGeneratedExactMatchAndWriteOrder(void)
{
    fw_addr_t record;
    unsigned event;
    u8 column;
    u8 offset;

    ResetFixture();
    SetRecordSource(0u, 10u);
    SetRecordSource(1u, 20u);
    SetRecordSource(2u, 0xffu);
    record = RecordAddress(1u);
    for(offset = 0u; offset != 33u; ++offset)
        memory[record + offset] = (u8)(0x40u + offset);
    memory[0x001008ul] = 1u;
    memory[0x000c46ul] = 1u;
    memory[0x000fcaul] = 0x91u;
    memory[0x00fa02ul] = 3u;

    ApplyControllerRegisterProfile(20u, 0xffu);

    assert(memory[0x00fa01ul] == 0u);
    assert(memory[0x000e51ul] == 2u);
    assert(memory[0x000fcaul] == 0x81u);
    assert(memory[0x018002ul] == 0x81u);
    assert(bit_event_count == 4u);
    assert(memcmp(bit_events, "cae9", 4u) == 0);
    event = 2u;
    assert(writes[0].address == 0x00fa01ul && writes[0].value == 1u);
    assert(writes[1].address == 0x000fcaul && writes[1].value == 0x99u);
    assert(writes[2].address == 0x018002ul && writes[2].value == 0x99u);
    ++event;
    assert(writes[event].address == 0x018081ul && writes[event].value == 0x40u);
    ++event;
    for(column = 0u; column != 9u; ++column)
    {
        assert(writes[event].address == 0x018082ul + column);
        assert(writes[event++].value == (u8)(0x41u + column));
        assert(writes[event].address == 0x01808bul + column);
        assert(writes[event++].value == (u8)(0x4au + column));
        assert(writes[event].address == 0x018094ul + column);
        assert(writes[event++].value == (u8)(0x53u + column));
    }
    assert(writes[event].address == 0x0180aaul && writes[event++].value == 0x5cu);
    assert(writes[event].address == 0x0180a9ul && writes[event++].value == 0x5du);
    assert(writes[event].address == 0x0180a6ul && writes[event++].value == 0x5eu);
    assert(writes[event].address == 0x0180a7ul && writes[event++].value == 0x5fu);
    assert(writes[event].address == 0x0180a8ul && writes[event++].value == 0x60u);
    assert(writes[event].address == 0x000fcaul && writes[event++].value == 0x81u);
    assert(writes[event].address == 0x018002ul && writes[event++].value == 0x81u);
    assert(derived_count == 1u && e8_count == 0u);
}

static void TestGeneratedSearchAndSignedNormalization(void)
{
    ResetFixture();
    SetRecordSource(0u, 0u);
    SetRecordSource(1u, 50u);
    SetRecordSource(2u, 70u);
    memory[0x00fa02ul] = 4u;
    ApplyControllerRegisterProfile(60u, 0xffu);
    assert(memory[0x00fa01ul] == 1u);
    assert(memory[0x000e51ul] == 4u);

    ResetFixture();
    SetRecordSource(0u, 0u);
    SetRecordSource(1u, 50u);
    SetRecordSource(2u, 0xffu);
    memory[0x000e51ul] = 0x77u;
    ApplyControllerRegisterProfile(0xffu, 0xffu);
    assert(memory[0x00fa01ul] == 1u);
    assert(memory[0x000e51ul] == 0x12u);
}

static void TestGeneratedMismatchStillAdjusts(void)
{
    ResetFixture();
    memory[0x00fa01ul] = 1u;
    SetRecordSource(1u, 40u);
    memory[0x001008ul] = 1u;
    StoreWord(0x00eeaaul + 82ul, 30000u);
    StoreWord(0x00f9e2ul, 1000u);
    StoreWord(0x00fad1ul, 25u);
    memory[0x000e51ul] = 0x66u;

    ApplyControllerRegisterProfile(41u, 0xffu);

    assert(LoadWord(0x00facful) == 3025u);
    assert(LoadWord(0x00fbc3ul) == 3025u);
    assert(memory[0x000e51ul] == 0x66u);
    assert(bit_event_count == 0u);
    assert(e8_count == 1u && e8_value == 0x23u);
}

static void TestFixedProfilesAndUnknownCode(void)
{
    static const u8 codes[5] = { 0x14u, 0x18u, 0x20u, 0x28u, 0x34u };
    u8 index;

    for(index = 0u; index != 5u; ++index)
    {
        ResetFixture();
        StoreWord(0x00e752ul + (fw_addr_t)((u16)index * 2u), (u16)(100u + index));
        StoreWord(0x00f9e2ul, 10000u);
        StoreWord(0x00fad1ul, 7u);
        ApplyControllerRegisterProfile(0u, codes[index]);
        assert(fixed_count == 1u && fixed_code == codes[index]);
        assert(LoadWord(0x00facful) == (u16)(107u + index));
        assert(LoadWord(0x00fbc3ul) == (u16)(107u + index));
        assert(memory[0x000e51ul] == codes[index]);
        assert(derived_count == 1u && e8_count == 1u);
    }

    ResetFixture();
    StoreWord(0x00facful, 0x4567u);
    ApplyControllerRegisterProfile(0u, 0x33u);
    assert(fixed_count == 0u);
    assert(LoadWord(0x00facful) == 0x4567u);
    assert(memory[0x000e51ul] == 0x33u);
}

static void TestAdjustmentGates(void)
{
    ResetFixture();
    memory[0x00fa01ul] = 1u;
    SetRecordSource(1u, 40u);
    memory[0x001008ul] = 1u;
    memory[0x000c46ul] = 1u;
    StoreWord(0x00facful, 0x1234u);
    ApplyControllerRegisterProfile(41u, 0xffu);
    assert(LoadWord(0x00facful) == 0x1234u);
    assert(e8_count == 0u);

    ResetFixture();
    memory[0x00fa01ul] = 1u;
    SetRecordSource(1u, 40u);
    memory[0x001008ul] = 1u;
    memory[0x00f971ul] = 2u;
    StoreWord(0x00facful, 0x5678u);
    ApplyControllerRegisterProfile(41u, 0xffu);
    assert(LoadWord(0x00facful) == 0x5678u);
    assert(e8_count == 1u);
}

int main(void)
{
    TestGeneratedExactMatchAndWriteOrder();
    TestGeneratedSearchAndSignedNormalization();
    TestGeneratedMismatchStillAdjusts();
    TestFixedProfilesAndUnknownCode();
    TestAdjustmentGates();
    puts("controller_register_profile_application_tests: ok");
    return 0;
}
