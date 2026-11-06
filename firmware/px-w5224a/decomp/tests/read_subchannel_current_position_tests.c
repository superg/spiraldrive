#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_subchannel_current_position.h"

static u8 memory[0x1000000ul];
static u8 capture_bit;
static u8 yield_count;
static u16 descriptor_address;
static u8 disable_count;
static u8 enable_count;
static fw_addr_t converted_sources[4];
static s32 converted_values[4];
static u8 converted_count;

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
u8 ObserveReadSubChannelCaptureBit(void)
{
    return capture_bit;
}
void YieldCoroutine0(void)
{
    yield_count++;
    memory[0x000e40ul] = 0u;
    memory[0x001303ul] |= 4u;
}
u16 GetRawReadConsumerDescriptorAddressOrZero(void)
{
    return descriptor_address;
}
void DisableExternalInterrupt0IfPipelineModeNonzero(void)
{
    disable_count++;
}
void EnableExternalInterrupt0IfPipelineModeNonzero(void)
{
    enable_count++;
}
u8 ReadControllerBufferByteAtomic(fw_addr_t offset)
{
    return memory[0x100000ul + offset];
}
u8 PackedBcdByteToBinary(u8 value)
{
    return (u8)((value >> 4) * 10u + (value & 0x0fu));
}
s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t source)
{
    u8 index = converted_count;
    converted_sources[index] = source;
    converted_count++;
    return converted_values[index];
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    capture_bit = 0u;
    yield_count = 0u;
    descriptor_address = 0x0200u;
    disable_count = 0u;
    enable_count = 0u;
    converted_count = 0u;
    memset(converted_sources, 0, sizeof(converted_sources));
    memset(converted_values, 0, sizeof(converted_values));
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void CheckUnavailableAndWait(void)
{
    unsigned index;
    Reset();
    BuildReadSubChannelCurrentPositionPayloadC(0x12fff8ul);
    assert(memory[0x12fff8ul] == 1u);
    for(index = 1u; index < 8u; ++index)
        assert(memory[0x12fff8ul + index] == 0u);
    for(index = 0u; index < 4u; ++index)
        assert(memory[0x120000ul + index] == 0u);

    Reset();
    memory[0x000faf] = 2u;
    memory[0x000e40ul] = 1u;
    memory[0x001245ul] = 0x41u;
    memory[0x001246ul] = 3u;
    memory[0x001247ul] = 1u;
    memory[0x000b0bul] = 2u;
    memory[0x00124cul] = 1u;
    memory[0x00124dul] = 2u;
    memory[0x00124eul] = 3u;
    memory[0x001248ul] = 4u;
    memory[0x001249ul] = 5u;
    memory[0x00124aul] = 6u;
    BuildReadSubChannelCurrentPositionPayloadC(0x3000ul);
    assert(yield_count >= 1u && (memory[0x001303ul] & 2u) == 0u);
    assert(memory[0x3001ul] == 0x14u && memory[0x3002ul] == 3u);
    assert(memory[0x3005ul] == 1u && memory[0x300bul] == 6u);
}

static void CheckQueuedPacket(void)
{
    u8 index;
    Reset();
    capture_bit = 1u;
    memory[0x001292ul] = 0u;
    memory[0x001293ul] = 9u;
    memory[descriptor_address + 8u] = 0u;
    memory[descriptor_address + 9u] = 2u;
    memory[descriptor_address + 0x10u] = 0u;
    memory[0x100000ul + 0x158eul] = 0x41u;
    for(index = 1u; index < 10u; ++index)
        memory[0x100000ul + 0x158eul + index] = (u8)(0x10u + index);
    memory[0x000b0bul] = 2u;
    BuildReadSubChannelCurrentPositionPayloadC(0x4000ul);
    assert(disable_count == 1u && enable_count == 1u);
    assert(memory[0x001245ul] == 0x41u && memory[0x001246ul] == 11u);
    assert(memory[0x4001ul] == 0x14u && memory[0x4002ul] == 11u);
}

static void CheckSpecialLbaAndLeadOut(void)
{
    Reset();
    memory[0x001303ul] = 4u;
    memory[0x000c8cul] = 2u;
    memory[0x00faf2ul] = 0x21u;
    memory[0x00faf3ul] = 7u;
    memory[0x00faf4ul] = 0u;
    converted_values[0] = 1000l;
    converted_values[1] = 200l;
    BuildReadSubChannelCurrentPositionPayloadC(0x5000ul);
    assert(converted_sources[0] == 0x00faf9ul && converted_sources[1] == 0x00faf5ul);
    assert(ReadDword(0x5004ul) == 850ul);
    assert(ReadDword(0x5008ul) == (u32)-200l);

    Reset();
    memory[0x001303ul] = 4u;
    memory[0x000fbaul] = 1u;
    memory[0x001246ul] = 0xaau;
    memory[0x00f578ul] = 0x12u;
    memory[0x00f579ul] = 0xaau;
    memory[0x00f57aul] = 1u;
    memory[0x000b0bul] = 2u;
    memory[0x00f57ful] = 9u;
    memory[0x00f580ul] = 8u;
    memory[0x00f581ul] = 7u;
    BuildReadSubChannelCurrentPositionPayloadC(0x6000ul);
    assert(memory[0x6001ul] == 0x21u && memory[0x6002ul] == 0xaau);
    assert(memory[0x6005ul] == 9u && memory[0x6007ul] == 7u);
}

int main(void)
{
    CheckUnavailableAndWait();
    CheckQueuedPacket();
    CheckSpecialLbaAndLeadOut();
    puts("read_subchannel_current_position_tests: ok");
    return 0;
}
