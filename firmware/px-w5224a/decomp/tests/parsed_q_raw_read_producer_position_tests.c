#include <assert.h>
#include <string.h>
#include "media/parsed_q_raw_read_producer_position.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long producer_bits;
static u8 decode_result;
static unsigned decode_count;
static unsigned service_count;
static u16 last_status;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObserveParsedQProducerPositionReadBit(u8 address)
{
    return (u8)((producer_bits >> address) & 1ull);
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

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    producer_bits = 0ull;
    decode_result = 1u;
    decode_count = service_count = 0u;
    last_status = 0xffffu;
    memory[0x000f1eul] = 30u;
    WriteDword(0x001598ul, 100u);
    WriteDword(0x000f6eul, 100u);
    WriteDword(0x001235ul, 100u);
}

static void CheckCountdownAndDecodeGates(void)
{
    Reset();
    memory[0x000f2ful] = 0x80u;
    memory[0x000f1eul] = 0u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f1eul] == 0xffu && decode_count == 0u && service_count == 1u && last_status == 0x0081u);

    Reset();
    memory[0x000f21ul] = 1u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f21ul] == 0u && decode_count == 0u && memory[0x000f04ul] == 0u);

    Reset();
    decode_result = 0u;
    memory[0x000f2ful] = 0x40u;
    memory[0x000f1eul] = 16u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f1eul] == 15u && service_count == 1u && last_status == 0x0082u);
}

static void CheckPositionPublicationAndBitBypass(void)
{
    Reset();
    WriteDword(0x001235ul, 270u);
    ProcessParsedQRawReadProducerPosition();
    assert(ReadDword(0x00159cul) == 170u && memory[0x000f04ul] == 2u);

    Reset();
    WriteDword(0x00159cul, 0x11223344ul);
    WriteDword(0x001235ul, 271u);
    ProcessParsedQRawReadProducerPosition();
    assert(ReadDword(0x00159cul) == 0x11223344ul);

    Reset();
    SetBit(0x13u);
    WriteDword(0x001598ul, 1000u);
    memory[0x000f2cul] = 9u;
    ProcessParsedQRawReadProducerPosition();
    assert(service_count == 0u && memory[0x000f2cul] == 0u && memory[0x000f04ul] == 2u);
}

static void CheckControllerProgramming(void)
{
    Reset();
    memory[0x000f06ul] = 1u;
    WriteWord(0x001296ul, 2u);
    WriteWord(0x000b00ul, 0x0a56u);
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x01801cul] == 2u && memory[0x01801dul] == 0u && memory[0x000efful] == 0x80u && memory[0x018019ul] == 0x80u);
    assert(memory[0x000efcul] == 0xccu && memory[0x018005ul] == 0xccu);
    assert(memory[0x018007ul] == 0x60u && memory[0x018008ul] == 0x12u && memory[0x018009ul] == 0u && memory[0x018004ul] == 12u && memory[0x000f04ul] == 2u);

    Reset();
    memory[0x000f06ul] = 0xffu;
    WriteWord(0x001296ul, 1u);
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x01801cul] == 0u && memory[0x01801dul] == 0u && memory[0x018007ul] == 0x30u && memory[0x018008ul] == 9u && memory[0x018004ul] == 8u);
}

static void CheckPositiveRetries(void)
{
    Reset();
    WriteDword(0x001598ul, 110u);
    memory[0x000ee4ul] = 10u;
    memory[0x000f2cul] = 30u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f2cul] == 31u && service_count == 0u && memory[0x000f04ul] == 0u);

    Reset();
    WriteDword(0x001598ul, 110u);
    memory[0x000ee4ul] = 10u;
    memory[0x000f2cul] = 31u;
    memory[0x000f23ul] = 17u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f2cul] == 0u && memory[0x000f23ul] == 1u && service_count == 1u && last_status == 0x0083u);

    Reset();
    WriteDword(0x001598ul, 110u);
    memory[0x001008ul] = 1u;
    ProcessParsedQRawReadProducerPosition();
    assert(memory[0x000f2cul] == 0u && service_count == 0u && memory[0x000f04ul] == 0u);

    Reset();
    WriteDword(0x001598ul, 250u);
    memory[0x000f07ul] = 1u;
    memory[0x000f23ul] = 15u;
    ProcessParsedQRawReadProducerPosition();
    assert(service_count == 1u && last_status == 0x0083u && memory[0x000f2cul] == 1u);
}

static void CheckNegativeWindow(void)
{
    Reset();
    memory[0x000f07ul] = 1u;
    WriteDword(0x001598ul, 96u);
    ProcessParsedQRawReadProducerPosition();
    assert(service_count == 0u && memory[0x000f04ul] == 2u);

    Reset();
    memory[0x000f07ul] = 1u;
    WriteDword(0x001598ul, 95u);
    ProcessParsedQRawReadProducerPosition();
    assert(service_count == 1u && last_status == 0x0082u && memory[0x000f04ul] == 0u);
}

int main(void)
{
    CheckCountdownAndDecodeGates();
    CheckPositionPublicationAndBitBypass();
    CheckControllerProgramming();
    CheckPositiveRetries();
    CheckNegativeWindow();
    return 0;
}
