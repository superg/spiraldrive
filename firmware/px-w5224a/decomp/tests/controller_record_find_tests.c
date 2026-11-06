#include <assert.h>
#include <string.h>
#include "platform/controller_record_find.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static fw_addr_t reads[256];
static unsigned read_count;

u8 FirmwareRead8(fw_addr_t address)
{
    address &= 0xfffffful;
    reads[read_count++] = address;
    return memory[address];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void WriteAddress(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void WriteRecord(u32 controller_address, u8 type, u32 key)
{
    fw_addr_t cpu_address = (controller_address + 0x100000ul) & 0xfffffful;

    memory[cpu_address] = (u8)(type << 4);
    memory[(cpu_address + 1u) & 0xfffffful] = 0xa1u;
    memory[(cpu_address + 2u) & 0xfffffful] = 0xa2u;
    memory[(cpu_address + 3u) & 0xfffffful] = (u8)(0xb0u | (key >> 16));
    memory[(cpu_address + 4u) & 0xfffffful] = (u8)(key >> 8);
    memory[(cpu_address + 5u) & 0xfffffful] = (u8)key;
}

static void Reset(u32 start, u32 end)
{
    memset(memory, 0, sizeof(memory));
    memset(reads, 0, sizeof(reads));
    read_count = 0u;
    WriteAddress(0x2ba51cul, start);
    WriteAddress(0x2ba520ul, end);
}

static void TestTypeSearches(void)
{
    Reset(0x002000ul, 0x00200cul);
    WriteRecord(0x002000ul, 1u, 0x12345ul);
    WriteRecord(0x002006ul, 2u, 0x23456ul);
    WriteRecord(0x00200cul, 3u, 0x34567ul);
    assert(FindControllerBufferSixByteRecord(0ul, 0x20u) == 0x002006ul);
    assert(reads[0] == 0x2ba51eul && reads[1] == 0x2ba51ful && reads[2] == 0x2ba51cul && reads[3] == 0x2ba51dul);

    read_count = 0u;
    assert(FindControllerBufferSixByteRecord(0ul, 0x31u) == 0x00200cul);
}

static void TestPackedKeyAndTypeGate(void)
{
    Reset(0x003000ul, 0x00300cul);
    WriteRecord(0x003000ul, 3u, 0xabcdeul);
    WriteRecord(0x003006ul, 2u, 0xabcdfu);
    WriteRecord(0x00300cul, 2u, 0xabcdeul);
    assert(FindControllerBufferSixByteRecord(0xabcdeul, 0u) == 0x00300cul);
    assert(FindControllerBufferSixByteRecord(0x10abcdeul, 0u) == 0ul);
}

static void TestInclusiveBoundAndTwentyFourBitWrap(void)
{
    Reset(0xeffffcul, 0xeffffcul);
    WriteRecord(0xeffffcul, 2u, 0x54321ul);
    assert(FindControllerBufferSixByteRecord(0x54321ul, 7u) == 0xeffffcul);

    Reset(0x004000ul, 0x003ffful);
    WriteRecord(0x004000ul, 2u, 0x11111ul);
    assert(FindControllerBufferSixByteRecord(0ul, 0x20u) == 0x004000ul);
}

int main(void)
{
    TestTypeSearches();
    TestPackedKeyAndTypeGate();
    TestInclusiveBoundAndTwentyFourBitWrap();
    return 0;
}
