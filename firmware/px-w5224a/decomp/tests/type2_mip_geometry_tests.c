#include <assert.h>
#include <string.h>
#include "media/type2_mip_geometry.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static unsigned multiply_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u32 MultiplyU32Low(u32 left, u32 right)
{
    ++multiply_count;
    return left * right;
}

static void WriteU24(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 16);
    memory[address + 1u] = (u8)(value >> 8);
    memory[address + 2u] = (u8)value;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static u32 MapCoordinate(u32 coordinate)
{
    return (coordinate * 39ul >> 5) - 0xa7ul;
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    multiply_count = 0u;
    memory[0x000c89ul] = 0xffu;
}

static void TestInactiveTimerTail(void)
{
    ResetState();
    WriteU32(0x002ba52eul, 0x12345678ul);
    DeriveMipControllerRecordGeometryFromPayload();
    assert(ReadU32(0x002ba52eul) == 0x12345678ul);
    assert(multiply_count == 0u);
    assert(memory[0x000c89ul] == 0xf7u);
    assert(memory[0x00122bul] == 0x14u);

    ResetState();
    memory[0x00e750ul] = 1u;
    DeriveMipControllerRecordGeometryFromPayload();
    assert(memory[0x00122bul] == 0x3cu);
}

static void TestEveryGridPoint(void)
{
    u8 row;
    u8 column;

    for(row = 0u; row != 3u; ++row)
    {
        for(column = 0u; column != 8u; ++column)
        {
            u32 primary = 500ul + ((u32)row << 8) + ((u32)(column + 1u) << 5);
            u32 mapped = MapCoordinate(primary);

            ResetState();
            memory[0x000c9eul] = 1u;
            WriteU32(0x002ba57ful, 500ul);
            WriteU24(0x002bb931ul, primary);
            WriteU24(0x002bb934ul, 1000ul);
            DeriveMipControllerRecordGeometryFromPayload();
            assert(memory[0x00fc4cul] == 0u);
            assert(ReadU32(0x002ba52eul) == primary);
            assert(ReadU32(0x002ba59ful) == mapped);
            assert(ReadU32(0x002ba532ul) == 1000ul);
            assert(ReadU32(0x002ba5a3ul) == MapCoordinate(1000ul));
            assert(ReadU32(0x002ba58dul) == mapped + 0x15ful);
            assert(ReadU32(0x002ba595ul) == mapped + 0x138ul);
            assert(ReadU32(0x002ba599ul) == mapped);
            assert(multiply_count == 2u);
        }
    }
}

static void TestFallbackAndMissingSecondary(void)
{
    u32 mapped = MapCoordinate(532ul);

    ResetState();
    memory[0x000c9eul] = 1u;
    WriteU32(0x002ba57ful, 500ul);
    WriteU24(0x002bb931ul, 999ul);
    WriteU24(0x002bb934ul, 0ul);
    WriteU32(0x002ba532ul, 0xfffffffful);
    WriteU32(0x002ba5a3ul, 0xfffffffful);
    DeriveMipControllerRecordGeometryFromPayload();
    assert(memory[0x00fc4cul] == 1u);
    assert(ReadU32(0x002ba52eul) == 532ul);
    assert(ReadU32(0x002ba59ful) == mapped);
    assert(ReadU32(0x002ba532ul) == 0ul);
    assert(ReadU32(0x002ba5a3ul) == 0ul);
    assert(ReadU32(0x002ba599ul) == mapped);
    assert(multiply_count == 1u);
}

int main(void)
{
    TestInactiveTimerTail();
    TestEveryGridPoint();
    TestFallbackAndMissingSecondary();
    return 0;
}
