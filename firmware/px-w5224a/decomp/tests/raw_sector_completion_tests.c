#include <assert.h>
#include <string.h>
#include "media/raw_sector_completion.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long completion_bits;
static u8 p1_result;
static u8 decode_result;
static u8 phase1_result;
static u8 phase2_result;
static u8 completion_flags_result;
static u8 overhead_result;
static u32 remainder_result;
static unsigned reset_probe_count;
static unsigned initialize_count;
static unsigned update_speed_count;
static unsigned build_count;
static unsigned enqueue_count;
static unsigned timing_count;
static unsigned countdown_count;
static u16 last_status;
static u8 last_mode;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObserveRawSectorCompletionReadBit(u8 address)
{
    return (u8)((completion_bits >> address) & 1ull);
}
void ObserveRawSectorCompletionWriteBit(u8 address, u8 value)
{
    unsigned long long mask = 1ull << address;

    if(value != 0u)
        completion_bits |= mask;
    else
        completion_bits &= ~mask;
}
u8 CheckP1Bit3StableHighForCurrentMediaState(void)
{
    return p1_result;
}
u8 DecodeControllerPositionFields(u8 check_status)
{
    assert(check_status == 0u);
    return decode_result;
}
void FUN_RAM_f93a5e(void)
{
    ++reset_probe_count;
}
u8 HandleRawReadDescriptorForCompletionFlags(void)
{
    return completion_flags_result;
}
u8 BuildOrQueueRawSectorDescriptorForReadPhase1(void)
{
    return phase1_result;
}
u8 BuildOrQueueRawSectorDescriptorForReadPhase2(void)
{
    return phase2_result;
}
u8 GetFixedPacketOverheadFramesRemaining(s32 frame_position)
{
    (void)frame_position;
    return overhead_result;
}
u32 DivideS32MagnitudeRemainderNative(s32 dividend, s32 divisor)
{
    (void)dividend;
    assert(divisor == 39l);
    return remainder_result;
}
void BuildRawSectorDescriptorScratch(u16 status, u8 mode)
{
    ++build_count;
    last_status = status;
    last_mode = mode;
}
void EnqueueRawSectorDescriptor(void)
{
    ++enqueue_count;
}
void EnqueueRawSectorDescriptorWithTimingGate(u16 status, u8 mode)
{
    ++timing_count;
    last_status = status;
    last_mode = mode;
}
void ServiceRawReadCountdownAndEnqueueDescriptor(u8 kind, u16 status)
{
    assert(kind == 3u);
    ++countdown_count;
    last_status = status;
}
void InitializeControllerBufferCursorRegistersIfNeeded(void)
{
    ++initialize_count;
}
void UpdateRawReadSpeedForCurrentPipelineState(void)
{
    ++update_speed_count;
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

static void SetBit(u8 address)
{
    completion_bits |= 1ull << address;
}

static u8 GetBit(u8 address)
{
    return (u8)((completion_bits >> address) & 1ull);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    completion_bits = 0ull;
    p1_result = 0u;
    decode_result = phase1_result = phase2_result = 1u;
    completion_flags_result = 1u;
    overhead_result = 1u;
    remainder_result = 0u;
    reset_probe_count = initialize_count = update_speed_count = 0u;
    build_count = enqueue_count = timing_count = countdown_count = 0u;
    last_status = 0xffffu;
    last_mode = 0xffu;
    memory[0x000e46ul] = 0x34u;
    memory[0x000c3dul] = 20u;
    memory[0x001008ul] = 0u;
    WriteDword(0x001598ul, 100u);
    WriteDword(0x000f6eul, 100u);
    WriteWord(0x00129cul, 1u);
}

static void CheckDirectionAndAlignmentFailures(void)
{
    Reset();
    memory[0x001302ul] = 0x80u;
    ProcessControllerReadSectorCompletion();
    assert(countdown_count == 1u && last_status == 0x0200u);

    Reset();
    memory[0x000e46ul] = 0x20u;
    memory[0x000c3cul] = 1u;
    WriteDword(0x00126eul, 0x123456ul);
    ProcessControllerReadSectorCompletion();
    assert(countdown_count == 1u && last_status == 0x1106u && decode_result == 1u && build_count == 0u);
}

static void CheckControllerErrorRoutes(void)
{
    Reset();
    memory[0x000f2ful] = 0x80u;
    ProcessControllerReadSectorCompletion();
    assert(memory[0x000f2eul] == 5u && timing_count == 1u && last_status == 0x2100u && last_mode == 5u && GetBit(0x05u) == 1u);

    Reset();
    memory[0x000f2ful] = 0x40u;
    p1_result = 1u;
    ProcessControllerReadSectorCompletion();
    assert(reset_probe_count == 1u && countdown_count == 1u && last_status == 0x0281u && GetBit(0x05u) == 1u);

    Reset();
    decode_result = 0u;
    memory[0x000c3cul] = 0u;
    ProcessControllerReadSectorCompletion();
    assert(reset_probe_count == 1u && last_status == 0x1105u);
}

static void CheckPhaseOneTerminalMetadata(void)
{
    Reset();
    memory[0x000f25ul] = 1u;
    SetBit(0x0fu);
    memory[0x0015a0ul] = 0xadu;
    memory[0x0015a1ul] = 0x34u;
    memory[0x0015a2ul] = 0x56u;
    WriteWord(0x001292ul, 1u);
    ProcessControllerReadSectorCompletion();
    assert(memory[0x000f2ful] == 0xadu && memory[0x000f31ul] == 0x34u && memory[0x0015a3ul] == 0x56u && memory[0x000f2eul] == 9u);
    assert(timing_count == 1u && last_status == 0u && last_mode == 0u && enqueue_count == 0u);
}

static void CheckOrdinaryDescriptorAndBoundary(void)
{
    Reset();
    SetBit(0x04u);
    SetBit(0x20u);
    SetBit(0x24u);
    ProcessControllerReadSectorCompletion();
    assert(build_count == 1u && last_status == 0u && last_mode == 0u && enqueue_count == 1u && ReadDword(0x001598ul) == 101u);
    assert(GetBit(0x04u) == 0u && GetBit(0x20u) == 0u && GetBit(0x24u) == 0u && memory[0x000f24ul] == 20u);
    assert(ReadWord(0x000f3bul) == 1u);

    Reset();
    memory[0x00125aul] = 4u;
    WriteDword(0x0012c1ul, 1000u);
    WriteDword(0x001598ul, 1100u);
    WriteDword(0x000f6eul, 1100u);
    ProcessControllerReadSectorCompletion();
    assert(timing_count == 1u && last_status == 0x6300u && last_mode == 5u && enqueue_count == 0u && ReadDword(0x001598ul) == 1100u);
}

static void CheckFixedPacketBatchAndTablePause(void)
{
    Reset();
    SetBit(0x0cu);
    memory[0x0015a3ul] = 0xe0u;
    ProcessControllerReadSectorCompletion();
    assert(build_count == 7u && enqueue_count == 7u && last_status == 0x0600u && last_mode == 3u && ReadDword(0x001598ul) == 107u && memory[0x000f1ful] == 6u);

    Reset();
    SetBit(0x0cu);
    memory[0x0015a3ul] = 0xe0u;
    memory[0x000c9eul] = 1u;
    WriteDword(0x00fafful, 0x3000ul);
    remainder_result = 2u;
    memory[0x3002ul] = 0u;
    ProcessControllerReadSectorCompletion();
    assert(GetBit(0x25u) == 1u && initialize_count == 1u && build_count == 0u && enqueue_count == 0u);
}

static void CheckSpeedRecoveryAndQueuePressure(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x000c9eul] = 1u;
    memory[0x2ba5aful] = 1u;
    memory[0x000e43ul] = 8u;
    memory[0x000e46ul] = 8u;
    memory[0x000f3aul] = 0x28u;
    memory[0x000c37ul] = 0x28u;
    memory[0x001037ul] = 5u;
    WriteWord(0x000f3bul, 9000u);
    WriteDword(0x001598ul, 1500u);
    WriteDword(0x000f6eul, 1500u);
    WriteDword(0x000f3dul, 1401u);
    ProcessControllerReadSectorCompletion();
    assert(GetBit(0x20u) == 1u && GetBit(0x27u) == 1u && memory[0x000e46ul] == 0x18u && memory[0x000e44ul] == 0x18u && memory[0x000f45ul] == 0x18u && memory[0x000e49ul] == 0x18u);
    assert(initialize_count == 1u && update_speed_count == 0u);

    Reset();
    memory[0x0015e2ul] = 10u;
    memory[0x000e43ul] = 0x28u;
    memory[0x000e46ul] = 0x28u;
    WriteWord(0x00129cul, 100u);
    WriteWord(0x001292ul, 26u);
    ProcessControllerReadSectorCompletion();
    assert(initialize_count == 1u && memory[0x000f45ul] == 8u && memory[0x000e46ul] == 8u && memory[0x000e49ul] == 8u);
}

int main(void)
{
    CheckDirectionAndAlignmentFailures();
    CheckControllerErrorRoutes();
    CheckPhaseOneTerminalMetadata();
    CheckOrdinaryDescriptorAndBoundary();
    CheckFixedPacketBatchAndTablePause();
    CheckSpeedRecoveryAndQueuePressure();
    return 0;
}
