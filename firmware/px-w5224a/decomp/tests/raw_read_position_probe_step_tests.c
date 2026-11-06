#include <assert.h>
#include <string.h>
#include "media/raw_read_position_probe.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u8 terminal_bit;
static u8 mark_count;
static u8 throttle_count;
static u8 dispatch_count;
static u8 yield_count;
static u8 p1_result;
static s32 converted_position;
static u8 request_results[8];
static u8 request_count;
static u8 initialize_count;
static u8 service_count;
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
u8 ReadBit0009Atomic(void)
{
    return terminal_bit;
}
void SetBit0017AndClearBit0016ForPositionProbeAtomic(void)
{
    ++mark_count;
}
void ThrottleRawReadProducerForQueueCapacity(void)
{
    ++throttle_count;
}
void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    ++dispatch_count;
}
void YieldCoroutine2(void)
{
    ++yield_count;
}
u8 CheckP1Bit3StableHighForCurrentMediaState(void)
{
    return p1_result;
}
void CopyBytesWithinFarBanks(fw_addr_t source, fw_addr_t destination, u16 count)
{
    assert(source == 0x000f64ul && destination == 0x001245ul && count == 10u);
}
s32 ConvertBinaryMsfToAdjustedFramePosition(fw_addr_t address)
{
    assert(address == 0x000f6bul);
    return converted_position;
}
void EnqueueRawSectorDescriptorWithTimingGate(u16 status, u8 mode)
{
    descriptor_status = status;
    descriptor_mode = mode;
    ++descriptor_count;
}
u8 RequestReadAcquisitionAndWait(s32 target, s32 adjustment)
{
    assert(target == 1000);
    assert(adjustment == (s32)(2000u - 2u * (request_count + 1u)));
    return request_results[request_count++];
}
void InitializeRawReadControllerForPositionProbe(void)
{
    ++initialize_count;
    memory[0x000f04ul] = 4u;
    memory[0x000f76ul] = 1u;
}
void ServiceParsedQRawReadCountdownAndEnqueueDescriptor(u16 status)
{
    assert(status == 0x82u);
    ++service_count;
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
    memset(request_results, 0, sizeof(request_results));
    terminal_bit = mark_count = throttle_count = dispatch_count = 0u;
    yield_count = request_count = initialize_count = service_count = 0u;
    descriptor_count = 0u;
    descriptor_status = 0u;
    descriptor_mode = 0u;
    p1_result = 1u;
    converted_position = 1000;
    WriteDword(0x001598ul, 1000u);
    WriteDword(0x00159cul, 2000u);
}

static void CheckTerminalAndSuccessfulProbe(void)
{
    Reset();
    terminal_bit = 1u;
    assert(ProcessRawReadPositionProbeStep() == 0u);
    assert(throttle_count == 0u && memory[0x000f23ul] == 0x19u);

    Reset();
    memory[0x000f07ul] = 1u;
    memory[0x000f64ul] = 1u;
    assert(ProcessRawReadPositionProbeStep() == 1u);
    assert(dispatch_count == 1u && request_count == 1u && initialize_count == 1u);
    assert((memory[0x000ff9ul] & 4u) == 0u && descriptor_count == 0u);
}

static void CheckAcquisitionAndPositionExhaustion(void)
{
    Reset();
    memory[0x000f07ul] = 1u;
    memory[0x000e41ul] = 1u;
    assert(ProcessRawReadPositionProbeStep() == 0u);
    assert(dispatch_count == 6u && descriptor_count == 1u);
    assert(descriptor_status == 0x200u && descriptor_mode == 3u);

    Reset();
    memory[0x000f07ul] = 1u;
    memory[0x000f64ul] = 1u;
    converted_position = 849;
    assert(ProcessRawReadPositionProbeStep() == 0u);
    assert(dispatch_count == 6u && descriptor_status == 0x283u);
}

static void CheckP1AndRequestExhaustion(void)
{
    Reset();
    memory[0x000f07ul] = 1u;
    p1_result = 0u;
    WriteDword(0x001598ul, 100u);
    assert(ProcessRawReadPositionProbeStep() == 0u);
    assert(dispatch_count == 6u && descriptor_status == 0x800bu && descriptor_mode == 9u && mark_count == 1u);

    Reset();
    memset(request_results, 1, sizeof(request_results));
    assert(ProcessRawReadPositionProbeStep() == 0u);
    assert(request_count == 6u && descriptor_status == 0x280u && descriptor_mode == 3u);
}

int main(void)
{
    CheckTerminalAndSuccessfulProbe();
    CheckAcquisitionAndPositionExhaustion();
    CheckP1AndRequestExhaustion();
    return 0;
}
