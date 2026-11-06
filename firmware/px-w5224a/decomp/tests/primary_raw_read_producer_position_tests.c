#include <assert.h>
#include <string.h>
#include "media/primary_raw_read_producer_position.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long producer_bits;
static u8 decode_result;
static unsigned decode_count;
static unsigned service_count;
static unsigned select_count;
static unsigned speed_update_count;
static u16 last_status;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObservePrimaryProducerPositionReadBit(u8 address)
{
    return (u8)((producer_bits >> address) & 1ull);
}
void ObservePrimaryProducerPositionWriteBit(u8 address, u8 value)
{
    unsigned long long mask = 1ull << address;

    if(value != 0u)
        producer_bits |= mask;
    else
        producer_bits &= ~mask;
}
u8 DecodeControllerPositionFields(u8 check_status)
{
    assert(check_status == 1u);
    ++decode_count;
    return decode_result;
}
void ServiceParsedQRawReadCountdownAndEnqueueDescriptor(u16 status)
{
    ++service_count;
    last_status = status;
}
void SelectRecentReadAcquisitionProbeTarget(void)
{
    ++select_count;
}
void UpdateRawReadSpeedForAcquisitionPhase(void)
{
    ++speed_update_count;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void SetBit(u8 address)
{
    producer_bits |= 1ull << address;
}

static u8 GetBit(u8 address)
{
    return (u8)((producer_bits >> address) & 1ull);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    producer_bits = 0ull;
    decode_result = 1u;
    decode_count = service_count = select_count = speed_update_count = 0u;
    last_status = 0xffffu;
    memory[0x000f1eul] = 30u;
    WriteDword(0x001598ul, 100u);
    WriteDword(0x000f6eul, 100u);
    WriteDword(0x001235ul, 100u);
}

static void CheckCountdownAndDecodeFailures(void)
{
    Reset();
    memory[0x000f2ful] = 0x80u;
    memory[0x000f1eul] = 0u;
    ProcessPrimaryRawReadProducerPosition();
    assert(memory[0x000f1eul] == 0xffu && service_count == 1u && last_status == 0x0081u && decode_count == 0u);

    Reset();
    memory[0x000f21ul] = 1u;
    ProcessPrimaryRawReadProducerPosition();
    assert(memory[0x000f21ul] == 0u && decode_count == 0u);

    Reset();
    decode_result = 0u;
    memory[0x000f2ful] = 0x40u;
    memory[0x000f1eul] = 1u;
    ProcessPrimaryRawReadProducerPosition();
    assert(memory[0x000f1eul] == 0u && service_count == 1u && last_status == 0x0082u);
}

static void CheckAlignedControllerProgramming(void)
{
    Reset();
    SetBit(0x20u);
    memory[0x000f06ul] = 1u;
    WriteWord(0x001296ul, 2u);
    WriteWord(0x000b00ul, 0x0a56u);
    WriteDword(0x000f87ul, 11u);
    WriteDword(0x000f8bul, 22u);
    WriteDword(0x000f8ful, 33u);
    WriteDword(0x000f93ul, 44u);
    ProcessPrimaryRawReadProducerPosition();
    assert(ReadDword(0x000f83ul) == 11u && ReadDword(0x000f87ul) == 22u && ReadDword(0x000f8bul) == 33u && ReadDword(0x000f8ful) == 44u && ReadDword(0x000f93ul) == 100u);
    assert(memory[0x01801cul] == 2u && memory[0x01801dul] == 0u && memory[0x018019ul] == 0x80u);
    assert(memory[0x018007ul] == 0x60u && memory[0x018008ul] == 0x12u && memory[0x018009ul] == 0u);
    assert(memory[0x018005ul] == 0x4cu && memory[0x018004ul] == 12u && memory[0x000f04ul] == 2u && GetBit(0x26u) == 0u);

    Reset();
    memory[0x000ca1ul] = 1u;
    WriteWord(0x001296ul, 1u);
    WriteDword(0x000f7ful, 0x100u);
    ProcessPrimaryRawReadProducerPosition();
    assert(memory[0x018007ul] == 0x20u && memory[0x018008ul] == 0x38u && memory[0x018009ul] == 0u && memory[0x018004ul] == 8u);
}

static void CheckProbeAndPositionRetries(void)
{
    Reset();
    SetBit(0x26u);
    memory[0x000f9bul] = 1u;
    WriteDword(0x000f97ul, 200u);
    ProcessPrimaryRawReadProducerPosition();
    assert(GetBit(0x26u) == 0u && service_count == 1u && last_status == 0x0083u);

    Reset();
    WriteDword(0x001598ul, 110u);
    memory[0x000f2cul] = 76u;
    ProcessPrimaryRawReadProducerPosition();
    assert(memory[0x000f2cul] == 0u && service_count == 1u && last_status == 0x0083u && memory[0x000f04ul] == 0u);

    Reset();
    SetBit(0x20u);
    WriteDword(0x000f10ul, 0xfffffffful);
    ProcessPrimaryRawReadProducerPosition();
    assert(GetBit(0x26u) == 1u && select_count == 1u && service_count == 1u && last_status == 0x0082u);

    Reset();
    SetBit(0x20u);
    WriteDword(0x001598ul, 90u);
    WriteDword(0x000f6eul, 100u);
    memory[0x000f9cul] = 6u;
    ProcessPrimaryRawReadProducerPosition();
    assert(speed_update_count == 1u && memory[0x000f9cul] == 7u && service_count == 1u && last_status == 0x0082u);
}

int main(void)
{
    CheckCountdownAndDecodeFailures();
    CheckAlignedControllerProgramming();
    CheckProbeAndPositionRetries();
    return 0;
}
