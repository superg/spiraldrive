#include <assert.h>
#include <string.h>
#include "media/primary_raw_read_pipeline_step.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long pipeline_bits;
static u8 inherited_retry;
static u8 fixed_overhead;
static u8 selected_interval;
static u8 acquisition_result;
static u8 p1_stable;
static u16 scan_result;
static u8 set_descriptor_state_on_yield;
static unsigned capture_count;
static unsigned yield_count;
static unsigned dispatch_count;
static unsigned build_count;
static unsigned enqueue_count;
static unsigned advance_count;
static unsigned prepare_count;
static unsigned initialize_count;
static unsigned throttle_count;
static unsigned selector_update_count;
static unsigned service_count;
static u16 last_build_status;
static u8 last_build_mode;
static u16 last_service_status;
static u8 last_batch_count;
static s32 last_request_target;
static s32 last_request_adjustment;
static u8 last_scan_force;
static s32 last_scan_start;
static u32 last_scan_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObservePrimaryRawReadPipelineBit(u8 address)
{
    return (u8)((pipeline_bits >> address) & 1ull);
}
void ObserveWritePrimaryRawReadPipelineBit(u8 address, u8 value)
{
    unsigned long long mask = 1ull << address;

    if(value != 0u)
        pipeline_bits |= mask;
    else
        pipeline_bits &= ~mask;
}
u8 ObserveCaptureInheritedR15ForPrimaryRawReadPipelineStep(void)
{
    ++capture_count;
    return inherited_retry;
}
void YieldCoroutine2(void)
{
    ++yield_count;
    if(set_descriptor_state_on_yield != 0u)
        memory[0x000f04ul] = set_descriptor_state_on_yield;
}
void DispatchReadAcquisitionRequest5ForPipelineTarget(void)
{
    ++dispatch_count;
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
void AdvanceRawReadRingCursors(void)
{
    ++advance_count;
}
u8 GetFixedPacketOverheadFramesRemaining(s32 position)
{
    (void)position;
    return fixed_overhead;
}
void PrepareAndSubmitPrimaryRawReadCaptureBatch(u8 count)
{
    ++prepare_count;
    last_batch_count = count;
}
u8 SelectDriveNumericTable0IntervalForReadPipelineTarget(void)
{
    return selected_interval;
}
u16 ScanControllerRecordBanksForFrameRange(u8 force, s32 start, u32 count)
{
    last_scan_force = force;
    last_scan_start = start;
    last_scan_count = count;
    return scan_result;
}
void ThrottleRawReadProducerForQueueCapacity(void)
{
    ++throttle_count;
}
u8 RequestReadAcquisitionAndWait(s32 target, s32 adjustment)
{
    last_request_target = target;
    last_request_adjustment = adjustment;
    return acquisition_result;
}
void InitializeRawReadControllerForPositionProbe(void)
{
    ++initialize_count;
}
u8 CheckP1Bit3StableHighForCurrentMediaState(void)
{
    return p1_stable;
}
void ServiceParsedQRawReadCountdownAndEnqueueDescriptor(u16 status)
{
    ++service_count;
    last_service_status = status;
}
void UpdatePrimaryRawReadOperatingCodeSelector(void)
{
    ++selector_update_count;
}

static void WriteWord(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
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

static void SetBit(u8 address)
{
    pipeline_bits |= 1ull << address;
}

static u8 GetBit(u8 address)
{
    return (u8)((pipeline_bits >> address) & 1ull);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    pipeline_bits = 0ull;
    inherited_retry = 4u;
    fixed_overhead = selected_interval = acquisition_result = 0u;
    p1_stable = scan_result = set_descriptor_state_on_yield = 0u;
    capture_count = yield_count = dispatch_count = build_count = 0u;
    enqueue_count = advance_count = prepare_count = initialize_count = 0u;
    throttle_count = selector_update_count = service_count = 0u;
    last_build_status = last_service_status = 0xffffu;
    last_build_mode = last_batch_count = 0xffu;
    last_request_target = last_request_adjustment = 0L;
    last_scan_force = 0xffu;
    last_scan_start = 0L;
    last_scan_count = 0ul;
    WriteDword(0x00156aul, 64ul);
    WriteDword(0x00fafful, 0x00002000ul);
}

static void CheckEntryAndTerminalGate(void)
{
    Reset();
    SetBit(0x09u);
    memory[0x000f3bul] = 0xaau;
    memory[0x000f3cul] = 0xbbu;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(capture_count == 1u && memory[0x000f23ul] == 0x19u);
    assert(memory[0x000f3bul] == 0u && memory[0x000f3cul] == 0u);
    assert(throttle_count == 0u);
}

static void CheckFixedPacketDescriptors(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x2ba5aful] = 1u;
    WriteDword(0x2ba58dul, 1000ul);
    WriteDword(0x001598ul, 100ul);
    fixed_overhead = 2u;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(memory[0x000f2eul] == 10u && build_count == 2u && enqueue_count == 2u && advance_count == 2u);
    assert(last_build_status == 0x0600u && last_build_mode == 3u);
    assert(ReadDword(0x001598ul) == 102ul);
}

static void CheckSelectedIntervalPublication(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x2ba5aful] = 1u;
    WriteDword(0x2ba58dul, 1000ul);
    WriteDword(0x001598ul, 100ul);
    WriteDword(0x000f00ul, 500ul);
    selected_interval = 1u;
    WriteDword(0xf84694ul, 300ul);
    memory[0x2000ul + ((300u - 145u) % 39u)] = 1u;
    SetBit(0x09u);
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(ReadDword(0x001598ul) == 300ul && GetBit(0x25u) == 0u);
}

static void CheckThirtyTwoFrameCaptureBatch(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    WriteDword(0x001598ul, 150ul);
    SetBit(0x25u);
    memory[0x2000ul + ((182u - 138u) % 39u)] = 1u;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(prepare_count == 1u && last_batch_count == 32u);
    assert(build_count == 32u && enqueue_count == 32u && advance_count == 32u && ReadDword(0x001598ul) == 182ul);
    assert(GetBit(0x25u) == 0u);
}

static void CheckOrdinaryAcquisitionAndProfileAdjustment(void)
{
    Reset();
    WriteDword(0x001598ul, 123ul);
    WriteDword(0x00159cul, 100ul);
    memory[0x000f06ul] = 1u;
    WriteDword(0xff9578ul, 5ul);
    memory[0xff9589ul] = 0x44u;
    memory[0x000f76ul] = 1u;
    set_descriptor_state_on_yield = 4u;
    assert(ProcessPrimaryRawReadPipelineStep() == 1u);
    assert(throttle_count == 1u && initialize_count == 1u);
    assert(last_request_target == 123L && last_request_adjustment == 103L);
    assert(ReadDword(0x000f10ul) == 5ul && memory[0x000f20ul] == 0x44u);
    assert(yield_count == 1u && service_count == 0u);
}

static void CheckInheritedRetryExpiration(void)
{
    Reset();
    inherited_retry = 0u;
    WriteDword(0x001598ul, 50ul);
    memory[0x000f76ul] = 0u;
    set_descriptor_state_on_yield = 4u;
    assert(ProcessPrimaryRawReadPipelineStep() == 1u);
    assert(service_count == 1u && last_service_status == 0x0082u);
}

static void CheckAcquisitionFailurePaths(void)
{
    Reset();
    acquisition_result = 2u;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(GetBit(0x07u) == 1u && build_count == 0u);

    Reset();
    acquisition_result = 2u;
    SetBit(0x07u);
    WriteWord(0xf8446cul, 0x0200u);
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(memory[0x000f2eul] == 5u && build_count == 1u && last_build_status == 0x0280u && last_build_mode == 3u && enqueue_count == 1u && GetBit(0x09u) == 1u);

    Reset();
    acquisition_result = 3u;
    SetBit(0x07u);
    WriteWord(0xf8446eul, 0x1234u);
    memory[0xf84457ul] = 6u;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(last_build_status == 0x1234u && last_build_mode == 6u);
}

static void CheckSpecialRecordScan(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x000ca1ul] = 1u;
    memory[0x000c3dul] = 0x2au;
    WriteDword(0x001598ul, 400ul);
    scan_result = 1u;
    assert(ProcessPrimaryRawReadPipelineStep() == 0u);
    assert(last_scan_force == 0u && last_scan_start == 400L && last_scan_count == 1ul);
    assert(GetBit(0x20u) == 1u && memory[0x000f2eul] == 0x0bu);
    assert(last_build_status == 0x0200u && last_build_mode == 3u);
    assert(ReadDword(0x001598ul) == 401ul && memory[0x000f24ul] == 0x2au);
}

int main(void)
{
    CheckEntryAndTerminalGate();
    CheckFixedPacketDescriptors();
    CheckSelectedIntervalPublication();
    CheckThirtyTwoFrameCaptureBatch();
    CheckOrdinaryAcquisitionAndProfileAdjustment();
    CheckInheritedRetryExpiration();
    CheckAcquisitionFailurePaths();
    CheckSpecialRecordScan();
    return 0;
}
