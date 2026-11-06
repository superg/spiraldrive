#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/type2_leadin_toc_sequence.h"

static u8 memory[0x300000ul];
static fw_addr_t write_addresses[512];
static u8 write_values[512];
static unsigned write_count;
static u8 bcd_inputs[16];
static unsigned bcd_count;
static unsigned msf_calls;
static s32 msf_position;
static fw_addr_t msf_destination;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    assert(write_count < sizeof(write_addresses) / sizeof(write_addresses[0]));
    write_addresses[write_count] = address;
    write_values[write_count] = value;
    write_count++;
    memory[address] = value;
}

u8 BinaryByteToPackedBcd(u8 value)
{
    assert(bcd_count < sizeof(bcd_inputs));
    bcd_inputs[bcd_count++] = value;
    return (u8)(value + (value / 10u) * 6u);
}

void ConvertFramePositionToBinaryMsfTuple(s32 frame_position, fw_addr_t output_tuple_address)
{
    msf_calls++;
    msf_position = frame_position;
    msf_destination = output_tuple_address;
    FirmwareWrite8(output_tuple_address, 9u);
    FirmwareWrite8(output_tuple_address + 1u, 17u);
    FirmwareWrite8(output_tuple_address + 2u, 68u);
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

static fw_addr_t RawRecord(unsigned index)
{
    return 0x00008012ul + (fw_addr_t)index * 10u;
}

static fw_addr_t FullDescriptor(unsigned record_number)
{
    return 0x2b8000ul + (fw_addr_t)record_number * 11u;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(memory + 0x8011u, 0xa5, 72u);
    memset(memory + 0x2b8000u, 0xa5, 89u);
    write_count = 0u;
    bcd_count = 0u;
    msf_calls = 0u;
    msf_position = 0;
    msf_destination = 0u;

    WriteU32(0x2ba58du, 0x89abcdeful);
    memory[0x113bu] = 59u;
    memory[0x113cu] = 58u;
    memory[0x113du] = 74u;
    memory[0x113fu] = 42u;
    memory[0x1140u] = 31u;
    memory[0x1141u] = 20u;
    memory[0xfb7au] = 0x01u;
    memory[0xfb7bu] = 0x04u;
    memory[0xfb7cu] = 0x11u;
    memory[0xfb7du] = 0x20u;
    memory[0xfb7eu] = 0x03u;
    memory[0xfb7fu] = 0x40u;
    memory[0xfb80u] = 0x01u;
    memory[0xfb81u] = 0x08u;
    memory[0xfb82u] = 0x02u;
    memory[0xfb83u] = 0x21u;
}

static void AssertPayload(fw_addr_t address, const u8 expected[10])
{
    assert(memcmp(memory + address, expected, 10u) == 0);
}

static void AssertInitialClearTrace(void)
{
    unsigned index;

    assert(write_count >= 60u);
    for(index = 0u; index < 60u; ++index)
    {
        assert(write_addresses[index] == 0x8012ul + index);
        assert(write_values[index] == 0u);
    }
}

static void TestRawQLayoutAndBcdConversions(void)
{
    static const u8 expected[7][10] = {
        { 0x71, 0x00, 0xa0, 0,    0,    0,    0, 0x01, 0x20, 0x00 },
        { 0x71, 0x00, 0xa1, 0,    0,    0,    0, 0x01, 0x00, 0x00 },
        { 0x71, 0x00, 0xa2, 0,    0,    0,    0, 0x09, 0x17, 0x68 },
        { 0x71, 0x00, 0x01, 0,    0,    0,    0, 0x00, 0x02, 0x00 },
        { 0x75, 0x00, 0xb0, 0xff, 0xff, 0xff, 3, 0x59, 0x58, 0x74 },
        { 0x75, 0x00, 0xc0, 0x0a, 0x22, 0x46, 0, 0x42, 0x31, 0x20 },
        { 0x75, 0x00, 0xc1, 0x82, 0x14, 0x42, 0, 0x00, 0x00, 0x00 }
    };
    static const u8 expected_bcd_inputs[9] = { 9u, 17u, 68u, 59u, 58u, 74u, 42u, 31u, 20u };
    unsigned index;

    ResetFixture();
    BuildType2LeadInTocRecordSequence(0u);

    AssertInitialClearTrace();
    assert(msf_calls == 1u);
    assert(msf_position == (s32)0x89abcdeful);
    assert(msf_destination == RawRecord(2u) + 7u);
    assert(bcd_count == 9u);
    assert(memcmp(bcd_inputs, expected_bcd_inputs, sizeof(expected_bcd_inputs)) == 0);
    for(index = 0u; index < 7u; ++index)
        AssertPayload(RawRecord(index), expected[index]);
    assert(memory[0x8011u] == 0xa5u);
    assert(memory[0x8058u] == 0xa5u);
    assert(memory[0x2b8000u] == 0xa5u);
    assert(ReadU16(0x8008u) == 3u);
    assert(ReadU16(0x800au) == 6u);
    assert(ReadU16(0x800cu) == 7u);
}

static void TestFullTocDescriptorLayoutRetainsBinaryMsf(void)
{
    static const u8 expected[7][10] = {
        { 0x71, 0x00, 0xa0, 0,    0,    0,    0, 0x01, 0x20, 0x00 },
        { 0x71, 0x00, 0xa1, 0,    0,    0,    0, 0x01, 0x00, 0x00 },
        { 0x71, 0x00, 0xa2, 0,    0,    0,    0, 9,    17,   68   },
        { 0x71, 0x00, 0x01, 0,    0,    0,    0, 0,    2,    0    },
        { 0x75, 0x00, 0xb0, 0xff, 0xff, 0xff, 3, 59,   58,   74   },
        { 0x75, 0x00, 0xc0, 0x0a, 0x22, 0x46, 0, 42,   31,   20   },
        { 0x75, 0x00, 0xc1, 0x82, 0x14, 0x42, 0, 0,    0,    0    }
    };
    unsigned record_number;

    ResetFixture();
    BuildType2LeadInTocRecordSequence(0xffu);

    AssertInitialClearTrace();
    assert(msf_calls == 1u);
    assert(msf_destination == FullDescriptor(3u) + 8u);
    assert(bcd_count == 0u);
    assert(memory[0x2b8000u] == 0xa5u);
    for(record_number = 1u; record_number <= 7u; ++record_number)
    {
        fw_addr_t descriptor = FullDescriptor(record_number);

        assert(memory[descriptor] == 1u);
        AssertPayload(descriptor + 1u, expected[record_number - 1u]);
    }
    assert(memory[FullDescriptor(7u) + 11u] == 0xa5u);
    for(record_number = 0u; record_number < 60u; ++record_number)
        assert(memory[0x8012u + record_number] == 0u);
    assert(memory[0x804eu] == 0xa5u);
    assert(ReadU16(0x8008u) == 3u);
    assert(ReadU16(0x800au) == 6u);
    assert(ReadU16(0x800cu) == 7u);
}

int main(void)
{
    TestRawQLayoutAndBcdConversions();
    TestFullTocDescriptorLayoutRetainsBinaryMsf();
    puts("type-two lead-in TOC sequence tests passed");
    return 0;
}
