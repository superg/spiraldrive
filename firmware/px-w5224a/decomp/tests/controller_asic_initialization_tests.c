#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_asic_initialization.h"
#include "platform/mcs251.h"

typedef struct
{
    fw_addr_t address;
    u8 value;
} WriteEvent;

static u8 memory[0x1000000ul];
static WriteEvent controller_writes[160];
static unsigned controller_write_count;
static u16 delays[2];
static unsigned delay_count;
static unsigned register01_count;
static u8 register01_value;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    address &= 0xfffffful;
    memory[address] = value;
    if(address >= 0x018000ul && address <= 0x0180fful)
    {
        assert(controller_write_count < sizeof(controller_writes) / sizeof(controller_writes[0]));
        controller_writes[controller_write_count].address = address;
        controller_writes[controller_write_count].value = value;
        controller_write_count++;
    }
}

void DelayTimerUnits(u16 delay_units)
{
    assert(delay_count < 2u);
    delays[delay_count++] = delay_units;
}

void WriteControllerRegister01Clamped(u8 value)
{
    register01_count++;
    register01_value = value;
    FirmwareWrite8(0x018001ul, value < 0x14u ? (u8)(value & 0x3fu) : 0x18u);
}

static u16 ReadBigEndianU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadBigEndianU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetCase(void)
{
    memset(memory, 0, 0x20000u);
    memset(controller_writes, 0, sizeof(controller_writes));
    controller_write_count = 0u;
    memset(delays, 0, sizeof(delays));
    delay_count = 0u;
    register01_count = 0u;
    register01_value = 0u;
}

static void TestModeTwoExactControllerWriteTrace(void)
{
    static const WriteEvent expected[] = {
        { 0x18002u, 0x39u },
        { 0x18031u, 0x40u },
        { 0x1800bu, 0xd2u },
        { 0x18001u, 0x71u },
        { 0x1803cu, 0x0du },
        { 0x1803du, 0x60u },
        { 0x18008u, 0x41u },
        { 0x18009u, 0x00u },
        { 0x1800au, 0xa0u },
        { 0x1803au, 0x42u },
        { 0x1806eu, 0x02u },
        { 0x18047u, 0x50u },
        { 0x1803eu, 0x27u },
        { 0x1803fu, 0x00u },
        { 0x18023u, 0x10u },
        { 0x1803bu, 0x88u },
        { 0x18025u, 0x62u },
        { 0x18002u, 0x21u },
        { 0x18003u, 0x00u },
        { 0x1802du, 0x7fu },
        { 0x18001u, 0x01u },
        { 0x18002u, 0x29u },
        { 0x1800au, 0xa0u },
        { 0x1803au, 0x83u },
        { 0x1806eu, 0x02u },
        { 0x1800bu, 0xd2u },
        { 0x18001u, 0x71u },
        { 0x1800fu, 0x04u },
        { 0x18010u, 0x00u },
        { 0x18011u, 0x00u },
        { 0x18012u, 0x00u },
        { 0x18013u, 0x00u },
        { 0x18014u, 0x60u },
        { 0x18015u, 0x12u },
        { 0x1802au, 0x00u },
        { 0x1802bu, 0x50u },
        { 0x1802cu, 0x0bu },
        { 0x1802du, 0x00u },
        { 0x1802fu, 0x0du },
        { 0x18016u, 0x00u },
        { 0x18017u, 0x61u },
        { 0x18018u, 0x12u },
        { 0x18019u, 0x00u },
        { 0x1801au, 0xaeu },
        { 0x1801bu, 0x14u },
        { 0x1801cu, 0x00u },
        { 0x1801du, 0x02u },
        { 0x1801fu, 0x00u },
        { 0x18020u, 0x6fu },
        { 0x18021u, 0x15u },
        { 0x18022u, 0xe0u },
        { 0x18023u, 0x10u },
        { 0x18048u, 0x00u },
        { 0x18049u, 0x00u },
        { 0x1804au, 0x00u },
        { 0x18040u, 0x00u },
        { 0x18043u, 0x01u },
        { 0x18041u, 0xffu },
        { 0x18042u, 0x01u },
        { 0x18002u, 0x21u },
        { 0x18006u, 0x01u },
        { 0x18004u, 0x00u },
        { 0x18005u, 0x00u },
        { 0x1802au, 0x03u },
        { 0x18032u, 0x00u },
        { 0x18033u, 0x00u },
        { 0x18034u, 0x00u },
        { 0x18035u, 0x00u },
        { 0x18007u, 0x00u },
        { 0x18008u, 0x00u },
        { 0x18009u, 0x00u },
        { 0x18018u, 0x06u },
        { 0x18019u, 0x60u },
        { 0x1801cu, 0x00u },
        { 0x1801du, 0x00u },
        { 0x18002u, 0x29u },
        { 0x1804bu, 0x2au },
        { 0x18005u, 0x00u },
        { 0x18006u, 0x00u },
        { 0x1800eu, 0x08u },
        { 0x18005u, 0x82u },
        { 0x18006u, 0x80u },
        { 0x18007u, 0x01u },
        { 0x1800eu, 0x88u },
        { 0x18080u, 0x00u },
        { 0x180bcu, 0x00u },
        { 0x180bbu, 0x00u },
        { 0x180bau, 0x00u },
        { 0x180b9u, 0x00u },
        { 0x180bdu, 0x01u },
        { 0x18078u, 0x20u },
        { 0x18079u, 0x10u },
        { 0x1807au, 0x00u },
        { 0x1807bu, 0x00u },
        { 0x1807cu, 0x57u },
        { 0x1807du, 0x00u },
        { 0x1807eu, 0x00u },
        { 0x1807fu, 0x00u },
        { 0x18002u, 0x21u },
        { 0x1802bu, 0x80u },
        { 0x1807fu, 0x00u },
        { 0x1802eu, 0x08u },
        { 0x1807eu, 0x00u },
        { 0x18002u, 0x29u },
        { 0x180c0u, 0x00u },
        { 0x180c1u, 0x14u },
        { 0x180c2u, 0xe1u },
        { 0x180c3u, 0x60u },
        { 0x180c6u, 0x23u },
        { 0x180cfu, 0x04u },
        { 0x18002u, 0x21u }
    };
    unsigned index;

    ResetCase();
    memory[0x18040u] = 2u;
    memory[0x1803fu] = 5u;
    memory[0x0fcau] = 0x31u;
    InitializeControllerAsic();

    assert(controller_write_count == sizeof(expected) / sizeof(expected[0]));
    for(index = 0u; index < controller_write_count; ++index)
    {
        assert(controller_writes[index].address == expected[index].address);
        assert(controller_writes[index].value == expected[index].value);
    }
    assert(delay_count == 2u && delays[0] == 12u && delays[1] == 9u);
    assert(register01_count == 1u && register01_value == 1u);
}

static void TestPublishedGeometryAndAlternateModes(void)
{
    ResetCase();
    memory[0x18040u] = 1u;
    memory[0x1803fu] = 4u;
    InitializeControllerAsic();

    assert(memory[0x1157u] == 1u && memory[0x1158u] == 4u);
    assert(ReadBigEndianU16(0x1552u) == 0x0082u);
    assert(ReadBigEndianU32(0x1556u) == 0x001b0000ul);
    assert(ReadBigEndianU32(0x155au) == 0x00160000ul);
    assert(ReadBigEndianU32(0x155eu) == 0x00126000ul);
    assert(ReadBigEndianU32(0x1562u) == 0x00126100ul);
    assert(ReadBigEndianU32(0x1566u) == 0x0014ae00ul);
    assert(ReadBigEndianU32(0x156au) == 0x00000200ul);
    assert(ReadBigEndianU32(0x156eu) == 0x00156f00ul);
    assert(ReadBigEndianU32(0x1572u) == 0x000b5000ul);
    assert(ReadBigEndianU32(0x1576u) == 0x000d9c00ul);
    assert(memory[0x18031u] == 0x80u);
    assert(memory[0x1803cu] == 1u && memory[0x1803du] == 0x10u);
    assert(memory[0x180cfu] == 0u);
    assert(memory[0x0fcau] == 0u && memory[0x18002u] == 0u);
}

int main(void)
{
    TestModeTwoExactControllerWriteTrace();
    TestPublishedGeometryAndAlternateModes();
    puts("controller ASIC initialization tests passed");
    return 0;
}
