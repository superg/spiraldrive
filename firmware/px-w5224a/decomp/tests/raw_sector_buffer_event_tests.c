#include <assert.h>
#include <string.h>
#include "media/raw_sector_buffer_event.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 bit000e;
static u8 bit0013;
static u8 bit0018;
static u8 bit0019;
static u8 bit001a;
static u8 high_water_result;
static unsigned service_count;
static unsigned build_count;
static unsigned enqueue_count;
static unsigned initialize_count;
static unsigned advance_q_count;
static u16 last_build_status;
static u8 last_build_mode;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 FirmwareReadBit000E(void)
{
    return bit000e;
}
u8 FirmwareReadBit0013(void)
{
    return bit0013;
}
u8 FirmwareReadBit001A(void)
{
    return bit001a;
}
void FirmwareWriteBit001A(u8 value)
{
    bit001a = value != 0u;
}
u8 ReadBit0018ForRawSectorBufferEvent(void)
{
    return bit0018;
}
u8 ReadBit0019ForRawSectorBufferEvent(void)
{
    return bit0019;
}
void ServiceParsedQDescriptorCountdownAndPositionAdvance(void)
{
    ++service_count;
}
void BuildRawSectorDescriptorScratch(u16 status, u8 mode)
{
    ++build_count;
    last_build_status = status;
    last_build_mode = mode;
}
void EnqueueRawSectorDescriptor(void)
{
    ++enqueue_count;
}
void InitializeControllerBufferCursorRegistersIfNeeded(void)
{
    ++initialize_count;
}
void AdvanceRawSectorQPositionByOneFrame(void)
{
    ++advance_q_count;
}
u8 IsRawReadQueueAtHighWater(void)
{
    return high_water_result;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
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

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    bit000e = bit0018 = bit0019 = bit001a = 0u;
    bit0013 = 1u;
    high_water_result = 0u;
    service_count = build_count = enqueue_count = initialize_count = 0u;
    advance_q_count = 0u;
    last_build_status = 0xffffu;
    last_build_mode = 0xffu;
    memory[0x001300ul] = 1u;
    memory[0x000faful] = 2u;
    memory[0x001008ul] = 1u;
}

static void CheckDirectionAndModuloGates(void)
{
    Reset();
    memory[0x001302ul] = 0x80u;
    ProcessRawSectorBufferEvent();
    assert(service_count == 1u && build_count == 0u);

    Reset();
    bit0013 = 0u;
    bit000e = 1u;
    memory[0x000f2ful] = 1u;
    memory[0x000e46ul] = 0x20u;
    memory[0x000f22ul] = 10u;
    WriteDword(0x00126eul, 0x00123456ul);
    ProcessRawSectorBufferEvent();
    assert(service_count == 1u && build_count == 0u);

    Reset();
    bit0013 = 0u;
    memory[0x000f2ful] = 1u;
    memory[0x000e46ul] = 0x34u;
    WriteDword(0x001235ul, 100u);
    WriteDword(0x001598ul, 103u);
    ProcessRawSectorBufferEvent();
    assert(service_count == 1u && memory[0x000f22ul] == 1u);

    Reset();
    bit0013 = 0u;
    memory[0x000f2ful] = 1u;
    memory[0x000e46ul] = 0x34u;
    WriteDword(0x001235ul, 100u);
    WriteDword(0x001598ul, 93u);
    ProcessRawSectorBufferEvent();
    assert(service_count == 0u && build_count == 1u && ReadDword(0x001598ul) == 94u);
}

static void CheckTerminalDescriptor(void)
{
    Reset();
    bit0013 = 0u;
    memory[0x000f2ful] = 1u;
    memory[0x000e46ul] = 0x34u;
    memory[0x000f22ul] = 10u;
    memory[0x001008ul] = 0u;
    memory[0x00125aul] = 4u;
    WriteDword(0x0012c1ul, 1000u);
    WriteDword(0x001598ul, 1100u);
    ProcessRawSectorBufferEvent();
    assert(build_count == 1u && last_build_status == 0x6300u && last_build_mode == 5u && enqueue_count == 1u && initialize_count == 1u && ReadDword(0x001598ul) == 1100u);
}

static void CheckMetadataCopyAndRepair(void)
{
    u8 index;
    fw_addr_t slot = 0x101010ul;

    Reset();
    bit000e = bit001a = 1u;
    memory[0x000f31ul] = 1u;
    WriteWord(0x001292ul, 1u);
    WriteDword(0x00156eul, 0x1000ul);
    WriteDword(0x001262ul, 1u);
    for(index = 0u; index < 10u; ++index)
        memory[slot + index] = (u8)(0x41u + index);
    ProcessRawSectorBufferEvent();
    for(index = 0u; index < 10u; ++index)
        assert(memory[0x0015a4ul + index] == (u8)(0x41u + index));
    assert(ReadDword(0x0015d8ul) == slot + 10u && memory[0x000f30ul] == 1u && bit001a == 0u && memory[0x01807cul] == 0u && advance_q_count == 0u);

    Reset();
    bit000e = 1u;
    WriteDword(0x00156eul, 0x1000ul);
    WriteDword(0x001262ul, 1u);
    for(index = 0u; index < 10u; ++index)
        memory[0x0015a4ul + index] = (u8)(0xa0u + index);
    ProcessRawSectorBufferEvent();
    for(index = 0u; index < 10u; ++index)
        assert(memory[slot + index] == (u8)(0xa0u + index));
    assert(advance_q_count == 1u && ReadDword(0x0015d8ul) == slot + 10u);
}

static void CheckCommonHighWaterPath(void)
{
    Reset();
    high_water_result = 1u;
    WriteDword(0x001598ul, 50u);
    memory[0x0015e2ul] = 10u;
    memory[0x000e43ul] = 0x28u;
    memory[0x000e46ul] = 0x28u;
    WriteWord(0x00129cul, 100u);
    WriteWord(0x001292ul, 26u);
    ProcessRawSectorBufferEvent();
    assert(ReadDword(0x001598ul) == 51u && memory[0x000f22ul] == 0u && memory[0x000f23ul] == 0x19u && initialize_count == 2u);
    assert(memory[0x000f45ul] == 8u && memory[0x000e46ul] == 8u && memory[0x000e49ul] == 8u && memory[0x000f61ul] == 4u);
}

static void CheckScanTargetAdjustments(void)
{
    Reset();
    bit0018 = 1u;
    WriteWord(0x000f5eul, 19u);
    WriteDword(0x001598ul, 100u);
    WriteDword(0x000f36ul, 500u);
    ProcessRawSectorBufferEvent();
    assert(ReadWord(0x000f5eul) == 0u && ReadDword(0x001598ul) == 211u && ReadDword(0x000f36ul) == 390u && initialize_count == 1u);

    Reset();
    bit0018 = bit0019 = 1u;
    WriteWord(0x000f5eul, 19u);
    WriteDword(0x001598ul, 100u);
    WriteDword(0x000f36ul, 500u);
    ProcessRawSectorBufferEvent();
    assert(ReadDword(0x001598ul) == 0xffffffe5ul && ReadDword(0x000f36ul) == 390u && initialize_count == 1u);
}

int main(void)
{
    CheckDirectionAndModuloGates();
    CheckTerminalDescriptor();
    CheckMetadataCopyAndRepair();
    CheckCommonHighWaterPath();
    CheckScanTargetAdjustments();
    return 0;
}
