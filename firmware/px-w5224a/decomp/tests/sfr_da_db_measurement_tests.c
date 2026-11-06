#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/sfr_da_db_measurement.h"

static u8 memory[0x200000];
static u8 sfr[256];
static u8 bits[256];
static unsigned divide_calls;
static u32 divide_dividend;
static u32 divide_divisor;
static unsigned adjustment_calls;

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

u8 FirmwareReadSfr8(u8 address)
{
    return sfr[address];
}
u8 ObserveSfrDaDbMeasurementBit(u8 address)
{
    return bits[address];
}
void ObserveWriteSfrDaDbMeasurementBit(u8 address, u8 value)
{
    bits[address] = (u8)(value != 0u);
}

u32 DivideU32Quotient(u32 dividend, u32 divisor)
{
    ++divide_calls;
    divide_dividend = dividend;
    divide_divisor = divisor;
    return divisor == 0ul ? 0ul : dividend / divisor;
}

void ApplySfrDaStrategyAdjustment(void)
{
    ++adjustment_calls;
}

static void WriteU16(u16 address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(u16 address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteU32(u16 address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(u16 address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    memset(sfr, 0, sizeof(sfr));
    memset(bits, 0, sizeof(bits));
    divide_calls = 0u;
    divide_dividend = 0ul;
    divide_divisor = 0ul;
    adjustment_calls = 0u;
    memory[0x0e4f] = 0x30u;
}

static void TestSfrDbRingAndThreshold(void)
{
    ResetState();
    sfr[0xdb] = 0xaeu;
    ProcessSfrDaDbMeasurementSample();
    assert(memory[0x102c] == 1u && ReadU16(0x100au) == 0x00bfu);
    assert((memory[0x102d] & 1u) != 0u);

    memory[0x102c] = 15u;
    memory[0x102d] = 1u;
    sfr[0xdb] = 0x50u;
    ProcessSfrDaDbMeasurementSample();
    assert(memory[0x102c] == 0u && ReadU16(0x1028u) == 0x00c8u);
}

static void TestWindowInitialization(void)
{
    ResetState();
    memory[0x0e4f] = 0x20u;
    memory[0x0e52] = 0xffu;
    WriteU32(0x1235u, 54000ul);
    WriteU16(0xfbc7u, 625u);
    WriteU16(0xfbceu, 9u);
    WriteU32(0xfbcau, 1234ul);

    ProcessSfrDaDbMeasurementSample();

    assert(memory[0x0ee4] == 12u && bits[0x2c] == 1u);
    assert(ReadU16(0xfbc7u) == 0u && ReadU16(0xfbceu) == 0u);
    assert(ReadU32(0xfbcau) == 0ul);
}

static void TestFirstWindowCollection(void)
{
    ResetState();
    memory[0x0e4f] = 0x20u;
    bits[0x2c] = 1u;
    bits[0x39] = 1u;
    memory[0x0ee4] = 10u;
    WriteU16(0xfbc7u, 159u);
    WriteU16(0xfbceu, 1u);
    WriteU32(0xfbcau, 100ul);
    sfr[0xda] = 20u;

    ProcessSfrDaDbMeasurementSample();

    assert(bits[0x2d] == 0u && ReadU16(0xfbceu) == 2u);
    assert(ReadU32(0xfbcau) == 120ul && memory[0xfbd2] == 20u);
    assert(divide_calls == 0u);
}

static void TestFirstWindowCompletionStartsSecond(void)
{
    ResetState();
    memory[0x0e4f] = 0x20u;
    memory[0x0ee4] = 10u;
    bits[0x2c] = 1u;
    bits[0x39] = 1u;
    WriteU16(0xfbc7u, 160u);
    WriteU16(0xfbceu, 4u);
    WriteU32(0xfbcau, 400ul);
    WriteU32(0x1235u, 0xfffffffful);
    sfr[0xda] = 20u;

    ProcessSfrDaDbMeasurementSample();

    assert(divide_calls == 1u && divide_dividend == 400ul && divide_divisor == 4ul);
    assert(memory[0xfbd0] == 100u && memory[0xfbd1] == 100u);
    assert(bits[0x39] == 0u && bits[0x2d] == 1u);
    assert(memory[0xfbc9] == 10u);
    assert(ReadU16(0xfbc7u) == 0u && ReadU16(0xfbceu) == 1u);
    assert(ReadU32(0xfbcau) == 20ul);
}

static void TestSecondWindowCompletionAndAdjustment(void)
{
    ResetState();
    memory[0x0e4f] = 0x20u;
    bits[0x2c] = 1u;
    bits[0x2d] = 1u;
    bits[0x28] = 1u;
    memory[0xfbc9] = 5u;
    memory[0x1008] = 1u;
    WriteU16(0xfbc7u, 80u);
    WriteU16(0xfbceu, 4u);
    WriteU32(0xfbcau, 200ul);
    WriteU32(0x1235u, 27000ul);

    ProcessSfrDaDbMeasurementSample();

    assert(divide_calls == 1u && memory[0xfbd3] == 50u);
    assert(ReadU16(0xfbc7u) == 0u && ReadU16(0xfbceu) == 0u);
    assert(ReadU32(0xfbcau) == 0ul && memory[0xfbc9] == 11u);
    assert(adjustment_calls == 1u);
}

int main(void)
{
    TestSfrDbRingAndThreshold();
    TestWindowInitialization();
    TestFirstWindowCollection();
    TestFirstWindowCompletionStartsSecond();
    TestSecondWindowCompletionAndAdjustment();
    puts("SFR DA/DB measurement tests passed");
    return 0;
}
