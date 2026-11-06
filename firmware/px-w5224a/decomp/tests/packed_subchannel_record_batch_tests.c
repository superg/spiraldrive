#include <assert.h>
#include <string.h>
#include "media/packed_subchannel_record_batch.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 bit0003;
static u8 bit001e;
static u8 bit0009;
static u8 initialize_count;
static u8 parsed_status_count;
static u8 service_count;
static u8 pack_count;
static fw_addr_t pack_address;
static u16 descriptor_status;
static u8 descriptor_mode;
static u8 descriptor_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address] = value;
}
u8 ReadBit0003ForPackedSubchannelBatch(void)
{
    return bit0003;
}
void SetBit0003ForPackedSubchannelBatch(void)
{
    bit0003 = 1u;
}
u8 ReadBit001EForPackedSubchannelBatch(void)
{
    return bit001e;
}
void SetBit0009Atomic(void)
{
    bit0009 = 1u;
}
void InitializeControllerBufferCursorRegistersIfNeeded(void)
{
    ++initialize_count;
}
void EnqueueRawSectorDescriptorForParsedQStatus(void)
{
    ++parsed_status_count;
}
void ServiceParsedQDescriptorCountdownAndPositionAdvance(void)
{
    ++service_count;
}
u32 MultiplyU32Low(u32 a, u32 b)
{
    return a * b;
}
void PackControllerSubchannelSymbolsTo72Bytes(fw_addr_t address)
{
    ++pack_count;
    pack_address = address;
}
void EnqueueRawSectorDescriptorWithTimingGate(u16 status, u8 mode)
{
    descriptor_status = status;
    descriptor_mode = mode;
    ++descriptor_count;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    bit0003 = bit001e = bit0009 = 0u;
    initialize_count = parsed_status_count = service_count = pack_count = 0u;
    descriptor_count = descriptor_mode = 0u;
    descriptor_status = 0u;
    WriteDword(0x001262ul, 0u);
    WriteDword(0x001566ul, 0x1000ul);
    WriteDword(0x000f56ul, 0x2000ul);
    memory[0x001300ul] = 1u;
    memory[0x000faful] = 2u;
}

static void CheckNoSlotAndInactiveRoutes(void)
{
    Reset();
    WriteDword(0x001262ul, 0xffffu);
    memory[0x000f22ul] = 10u;
    ProcessPackedSubchannelRecordBatch();
    assert(memory[0x000f22ul] == 11u && initialize_count == 1u && parsed_status_count == 1u && bit0009 == 1u);

    Reset();
    memory[0x001300ul] = 0u;
    ProcessPackedSubchannelRecordBatch();
    assert(service_count == 1u && pack_count == 0u);
}

static void CheckPipelineReset(void)
{
    Reset();
    memory[0x2bb919ul] = 0x10u;
    WriteDword(0x000f52ul, 100u);
    WriteDword(0x00156aul, 10u);
    WriteDword(0x001562ul, 0x3456u);
    ProcessPackedSubchannelRecordBatch();
    assert(service_count == 1u && memory[0x000f4bul] == 1u);
    assert(ReadDword(0x000f52ul) == 175u);
    assert(ReadDword(0x001262ul) == 0u && ReadDword(0x001266ul) == 0u);
    assert(ReadWord(0x00129aul) == 9u && ReadWord(0x00129cul) == 4u);
    assert(ReadDword(0x00126aul) == 0x3456u);
}

static void CheckAcceptedRecordCopy(void)
{
    u8 index;

    Reset();
    memory[0x2bb919ul] = 0x80u;
    memory[0x2bb91bul] = 0u;
    memory[0x2bb91cul] = 0u;
    for(index = 1u; index < 18u; ++index)
        memory[0x2bb919ul + index] = index;
    memory[0x2bb919ul] = 0x80u;
    memory[0x2bb91bul] = 0u;
    memory[0x2bb91cul] = 0u;
    ProcessPackedSubchannelRecordBatch();
    assert(pack_count == 1u && pack_address == 0x1000ul);
    assert(bit0003 == 1u && memory[0x2000ul] == 0x80u && memory[0x2011ul] == 17u);
    assert(ReadDword(0x000f56ul) == 0x2012ul);
    assert(memory[0x000f4dul] == 1u && ReadWord(0x000f50ul) == 18u);
}

static void CheckMissingTypeTerminal(void)
{
    Reset();
    WriteWord(0x000f4eul, 32u);
    ProcessPackedSubchannelRecordBatch();
    assert(ReadDword(0x000f56ul) == 0x2004ul);
    assert(ReadDword(0x001598ul) == 0x96ul);
    assert(memory[0x000f2eul] == 5u && descriptor_count == 1u);
    assert(descriptor_status == 0x2400u && descriptor_mode == 5u);
}

static void CheckResynchronizationTableHit(void)
{
    u8 index;

    Reset();
    bit001e = 1u;
    WriteWord(0x000f50ul, 36u);
    WriteWord(0x00f58cul, 1u);
    WriteWord(0x00f58eul, 2u);
    for(index = 0u; index < 18u; ++index)
        memory[0x2bb919ul + index] = (u8)(0x80u + index);
    memory[0x2bb91bul] = 0u;
    memory[0x2bb91cul] = 0u;
    ProcessPackedSubchannelRecordBatch();
    assert(ReadWord(0x00f58eul) == 0xffffu);
    assert(memory[0x2000ul] == 0x80u && memory[0x2011ul] == 0x91u);
    assert(ReadDword(0x000f56ul) == 0x2012ul);
    assert(ReadWord(0x000f50ul) == 54u);
}

int main(void)
{
    CheckNoSlotAndInactiveRoutes();
    CheckPipelineReset();
    CheckAcceptedRecordCopy();
    CheckResynchronizationTableHit();
    CheckMissingTypeTerminal();
    return 0;
}
