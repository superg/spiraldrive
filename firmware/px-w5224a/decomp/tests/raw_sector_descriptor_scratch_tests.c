#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "platform/raw_sector_descriptor_scratch.h"

static u8 memory_bytes[0x20000ul];
static u8 bit_001f;
static u8 controller_read_value;
static u32 controller_read_address;
static unsigned controller_read_count;

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

u8 ObserveRawSectorDescriptorBit001F(void)
{
    return bit_001f;
}

u8 ObserveRawSectorDescriptorControllerRead(u32 controller_address)
{
    controller_read_address = controller_address;
    ++controller_read_count;
    return controller_read_value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory_bytes[address] = (u8)(value >> 24);
    memory_bytes[address + 1u] = (u8)(value >> 16);
    memory_bytes[address + 2u] = (u8)(value >> 8);
    memory_bytes[address + 3u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory_bytes[address] << 8) | memory_bytes[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory_bytes[address] << 24) | ((u32)memory_bytes[address + 1u] << 16) | ((u32)memory_bytes[address + 2u] << 8) | memory_bytes[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory_bytes, 0, sizeof(memory_bytes));
    bit_001f = 0u;
    controller_read_value = 0u;
    controller_read_address = 0ul;
    controller_read_count = 0u;
    WriteU32(0x1598u, 0x01020304ul);
    WriteU32(0x126eu, 0x10203040ul);
    WriteU32(0x1262u, 0x00001234ul);
    WriteU32(0x1266u, 0x00000007ul);
}

static void TestSpecialDescriptorInputsAndVirtualOverride(void)
{
    ResetFixture();
    memory_bytes[0x127eu] = 1u;
    bit_001f = 1u;
    WriteU32(0x0f00u, 0xa1a2a3a4ul);
    WriteU32(0x126au, 0x50607080ul);
    memory_bytes[0x0f30u] = 0x31u;
    memory_bytes[0x15a3u] = 0x32u;
    memory_bytes[0x0f2eu] = 0x33u;
    memory_bytes[0x0f2fu] = 0x34u;
    memory_bytes[0x0f31u] = 0x35u;

    BuildRawSectorDescriptorScratch(0xabcd, 5u);

    assert(ReadU32(0x12a2u) == 0xa1a2a3a4ul);
    assert(ReadU32(0x12a6u) == 0x10203040ul);
    assert(ReadU16(0x12aau) == 0x1234u);
    assert(ReadU32(0x12acu) == 0x50607080ul);
    assert(ReadU16(0x12b0u) == 0xabcdu);
    assert(memory_bytes[0x12b2u] == 5u);
    assert(memory_bytes[0x12b3u] == 0x31u);
    assert(memory_bytes[0x12b4u] == 0x32u);
    assert(memory_bytes[0x12b5u] == 0x33u);
    assert(memory_bytes[0x12b6u] == 0x34u);
    assert(memory_bytes[0x12b7u] == 0x35u);
    assert(controller_read_count == 0u);
}

static void TestOrdinaryPrimaryPacketAndLatchedTerminalFlag(void)
{
    unsigned index;

    ResetFixture();
    for(index = 0u; index < 10u; ++index)
        memory_bytes[0x1272u + index] = (u8)(0x40u + index);
    memory_bytes[0x1285u] = 0u;
    memory_bytes[0x127cu] = 1u;
    memory_bytes[0x12b6u] = 0x15u;

    BuildRawSectorDescriptorScratch(0xffffu, 0xffu);

    assert(ReadU32(0x12a2u) == 0x01020304ul);
    assert(ReadU32(0x12a6u) == 0x10203040ul);
    assert(ReadU16(0x12aau) == 0x1234u);
    for(index = 0u; index < 10u; ++index)
        assert(memory_bytes[0x12acu + index] == (u8)(0x40u + index));
    assert(memory_bytes[0x12b6u] == 0x95u);
    assert(memory_bytes[0x12b7u] == 0u);
    assert(controller_read_count == 0u);
}

static void TestSecondarySlotMetadataControlsTerminalFlag(void)
{
    ResetFixture();
    memory_bytes[0x1285u] = 1u;
    WriteU32(0x156eu, 0x00008000ul);
    controller_read_value = 0x80u;
    memory_bytes[0x12b6u] = 0x22u;

    BuildRawSectorDescriptorScratch(0u, 0u);

    assert(ReadU16(0x12aau) == 7u);
    assert(controller_read_count == 1u);
    assert(controller_read_address == 0x0000806ful);
    assert(memory_bytes[0x127cu] == 1u);
    assert(memory_bytes[0x12b6u] == 0xa2u);
    assert(memory_bytes[0x12b7u] == 1u);
}

static void TestType0DControllerAndOneShotFlags(void)
{
    ResetFixture();
    memory_bytes[0x1285u] = 0x0du;
    memory_bytes[0x12b6u] = 0x03u;
    memory_bytes[0x14d0u] = 1u;
    controller_read_value = 0x20u;

    BuildRawSectorDescriptorScratch(0u, 0u);

    assert(controller_read_count == 1u);
    assert(controller_read_address == 0x10203052ul);
    assert(memory_bytes[0x12b6u] == 0xc3u);
    assert(memory_bytes[0x14d0u] == 0u);
    assert(memory_bytes[0x12b7u] == 0x0du);
}

static void TestSignedPositionGateAndTypeTwoControllerBypass(void)
{
    ResetFixture();
    memory_bytes[0x1285u] = 2u;
    memory_bytes[0x14cfu] = 1u;
    memory_bytes[0x1281u] = 2u;
    memory_bytes[0x12b6u] = 0x31u;
    WriteU32(0x14d1u, 99ul);
    WriteU32(0x1598u, 100ul);

    BuildRawSectorDescriptorScratch(0u, 0u);

    assert(controller_read_count == 0u);
    assert(memory_bytes[0x12b6u] == 0xb1u);
    assert(memory_bytes[0x12b7u] == 2u);

    ResetFixture();
    memory_bytes[0x1285u] = 2u;
    memory_bytes[0x14cfu] = 1u;
    memory_bytes[0x1281u] = 2u;
    memory_bytes[0x127cu] = 0u;
    memory_bytes[0x12b6u] = 0xf1u;
    WriteU32(0x14d1u, 1ul);
    WriteU32(0x1598u, 0x80000000ul);

    BuildRawSectorDescriptorScratch(0u, 0u);
    assert(memory_bytes[0x12b6u] == 0x71u);
}

int main(void)
{
    TestSpecialDescriptorInputsAndVirtualOverride();
    TestOrdinaryPrimaryPacketAndLatchedTerminalFlag();
    TestSecondarySlotMetadataControlsTerminalFlag();
    TestType0DControllerAndOneShotFlags();
    TestSignedPositionGateAndTypeTwoControllerBypass();
    puts("raw-sector descriptor-scratch tests passed");
    return 0;
}
