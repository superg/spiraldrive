#include <assert.h>
#include <math.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_f2_shared.h"

static u8 memory[1u << 24];
static fw_addr_t copy_sources[2];
static fw_addr_t copy_destinations[2];
static u16 copy_counts[2];
static unsigned copy_calls;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void ObserveVendorF2SharedCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    assert(copy_calls < 2u);
    copy_sources[copy_calls] = source;
    copy_destinations[copy_calls] = destination;
    copy_counts[copy_calls++] = byte_count;
}

static void WriteBe16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadBe16(fw_addr_t address)
{
    return (u16)((u16)memory[address] << 8) | memory[address + 1u];
}

static void WriteBe32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void WriteFloat(fw_addr_t address, float value)
{
    union
    {
        u32 bits;
        float value;
    } converted;

    converted.value = value;
    WriteBe32(address, converted.bits);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(copy_sources, 0, sizeof(copy_sources));
    memset(copy_destinations, 0, sizeof(copy_destinations));
    memset(copy_counts, 0, sizeof(copy_counts));
    copy_calls = 0u;
    WriteBe16(0x00f060u, 0x1234u);
    WriteBe16(0x00f062u, 0x5678u);
    WriteBe16(0x00f064u, 0x9abcu);
}

static void AssertCommonFields(void)
{
    assert(copy_calls == 2u);
    assert(copy_sources[0] == 0x00f11bul && copy_destinations[0] == 0x000bb0ul && copy_counts[0] == 30u);
    assert(copy_sources[1] == 0x00f139ul && copy_destinations[1] == 0x000bceul && copy_counts[1] == 30u);
    assert(ReadBe16(0x000beeu) == 0x1234u);
    assert(ReadBe16(0x000bf2u) == 0x5678u);
    assert(ReadBe16(0x000bf6u) == 0x9abcu);
}

static void TestModeThreeDirectPair(void)
{
    ResetState();
    memory[0x00ef73u] = 3u;
    WriteBe16(0x00ef85u, 0x1357u);
    WriteBe16(0x00ef87u, 0x2468u);
    BuildVendorF2SharedMeasurementResponseFields();
    AssertCommonFields();
    assert(ReadBe16(0x000bfau) == 0x1357u);
    assert(ReadBe16(0x000bfcu) == 0x2468u);
}

static void TestCalculatedPair(void)
{
    s8 table_value = -5;
    float measurement = 2.5f;
    float narrowed;
    double value;
    s32 first_rounded;
    s32 final_rounded;
    u16 reference;
    s16 delta;

    ResetState();
    memory[0x00ef73u] = 2u;
    WriteBe32(0x00f1bbu, 0x002000ul);
    memory[0x002004u] = (u8)table_value;
    WriteFloat(0x00f06eu, measurement);
    WriteBe16(0x00f074u, 1000u);
    memory[0x00f06au] = 4u;

    value = (double)table_value * 0.001 * (double)measurement;
    narrowed = (float)value;
    narrowed = (float)((double)narrowed + 4.6592);
    first_rounded = (s32)ceil((double)narrowed / 0.0043);
    reference = (u16)((u16)first_rounded + 1000u);
    delta = (s16)(reference - 1000u);
    value = ((double)delta * 0.001 - (double)measurement) + 12.0;
    value /= ((double)delta * 0.0089 + 14.3) * 0.01;
    narrowed = (float)(value + 4.0);
    final_rounded = (s32)ceil((double)narrowed);

    BuildVendorF2SharedMeasurementResponseFields();
    AssertCommonFields();
    assert(ReadBe16(0x000bfau) == (u16)(u8)final_rounded);
    assert(ReadBe16(0x000bfcu) == reference);
}

int main(void)
{
    TestModeThreeDirectPair();
    TestCalculatedPair();
    return 0;
}
