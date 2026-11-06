#include <assert.h>
#include <string.h>
#include "media/primary_raw_read_pipeline_coroutine.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned long long coroutine_bits;
static u8 step_result;
static u8 step_sets_terminal;
static u8 step_descriptor_state;
static u8 yield_sets_event;
static u8 p1_result;
static unsigned speed_count;
static unsigned step_count;
static unsigned yield_count;
static unsigned p1_count;
static unsigned build_count;
static unsigned enqueue_count;
static unsigned service_count;
static unsigned selector_count;
static unsigned finalize_count;
static u16 last_build_status;
static u8 last_build_mode;
static u8 last_service_kind;
static u16 last_service_status;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}
u8 ObservePrimaryRawReadCoroutineBit(u8 address)
{
    return (u8)((coroutine_bits >> address) & 1ull);
}
void ObserveWritePrimaryRawReadCoroutineBit(u8 address, u8 value)
{
    unsigned long long mask = 1ull << address;

    if(value != 0u)
        coroutine_bits |= mask;
    else
        coroutine_bits &= ~mask;
}
u8 ObserveCallPrimaryRawReadPipelineStep(void)
{
    ++step_count;
    memory[0x000f04ul] = step_descriptor_state;
    if(step_sets_terminal != 0u)
        ObserveWritePrimaryRawReadCoroutineBit(0x09u, 1u);
    return step_result;
}
void ObservePrimaryRawReadCoroutineYield(void)
{
    ++yield_count;
    if(yield_sets_event != 0u)
        ObserveWritePrimaryRawReadCoroutineBit(0x05u, 1u);
}
u8 ObservePrimaryRawReadCoroutineCheckP1(void)
{
    ++p1_count;
    return p1_result;
}
void ObservePrimaryRawReadCoroutineFinalize(void)
{
    ++finalize_count;
}
void UpdateRawReadSpeedForCurrentPipelineState(void)
{
    ++speed_count;
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
void ServiceRawReadCountdownAndEnqueueDescriptor(u8 kind, u16 status)
{
    ++service_count;
    last_service_kind = kind;
    last_service_status = status;
}
void UpdatePrimaryRawReadOperatingCodeSelector(void)
{
    ++selector_count;
}

static void WriteDword(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u16 ReadWord(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadDword(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void SetBit(u8 address)
{
    coroutine_bits |= 1ull << address;
}

static u8 GetBit(u8 address)
{
    return (u8)((coroutine_bits >> address) & 1ull);
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
    coroutine_bits = 0ull;
    step_result = step_descriptor_state = yield_sets_event = p1_result = 0u;
    step_sets_terminal = 1u;
    speed_count = step_count = yield_count = p1_count = 0u;
    build_count = enqueue_count = service_count = selector_count = 0u;
    finalize_count = 0u;
    last_build_status = last_service_status = 0xffffu;
    last_build_mode = last_service_kind = 0xffu;
    WriteDword(0x00156aul, 10ul);
    WriteDword(0x001562ul, 0x12345678ul);
}

static void CheckOrdinaryInitialization(void)
{
    Reset();
    memory[0x000ff9ul] = 0xffu;
    memory[0x000f3bul] = 0xaau;
    memory[0x000f3cul] = 0xbbu;
    memory[0x000c3dul] = 0x2au;
    WriteDword(0x001598ul, 100ul);
    RunPrimaryRawReadPipelineCoroutine2();
    assert(memory[0x000f05ul] == 2u && memory[0x000e44ul] == 1u);
    assert(memory[0x000f24ul] == 0x2au && memory[0x000f08ul] == 0x32u);
    assert(memory[0x000ff9ul] == 0xf9u && memory[0x000f3bul] == 0u && memory[0x000f3cul] == 0u);
    assert(GetBit(0x08u) == 1u && GetBit(0x04u) == 0u && GetBit(0x07u) == 0u && GetBit(0x18u) == 0u);
    assert(speed_count == 1u && step_count == 1u && finalize_count == 1u);
    assert(ReadWord(0x00129aul) == 9u && ReadWord(0x00129cul) == 4u);
    assert(ReadDword(0x00129eul) == 0x000088bbul);
    assert(ReadDword(0x00126aul) == 0x12345678ul);
    assert(ReadDword(0x001598ul) == 101ul && memory[0x001591ul] == 0u);
}

static void CheckSpecialInitialization(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x000ca1ul] = 1u;
    WriteDword(0x001598ul, 200ul);
    WriteDword(0x000ce1ul, 500ul);
    WriteDword(0x2b9704ul, 0xaaaaaaaau);
    WriteDword(0x000f7ful, 0xbbbbbbbbu);
    SetBit(0x25u);
    SetBit(0x26u);
    SetBit(0x1fu);
    RunPrimaryRawReadPipelineCoroutine2();
    assert(memory[0x000fa2ul] == 2u && ReadDword(0x000f00ul) == 232ul && memory[0x001037ul] == 0u);
    assert(ReadDword(0x2b9704ul) == 0u && ReadDword(0x000f7ful) == 0u);
    assert(GetBit(0x25u) == 0u && GetBit(0x26u) == 0u && GetBit(0x1fu) == 0u);
}

static void CheckActiveDescriptorFailure(void)
{
    Reset();
    memory[0x0015d7ul] = 1u;
    memory[0x000f76ul] = 0u;
    step_result = 1u;
    step_sets_terminal = 0u;
    step_descriptor_state = 4u;
    yield_sets_event = 1u;
    p1_result = 0u;
    RunPrimaryRawReadPipelineCoroutine2();
    assert(yield_count == 1u && p1_count == 1u);
    assert(build_count == 1u && enqueue_count == 1u && last_build_status == 0x2100u && last_build_mode == 5u);
    assert(GetBit(0x09u) == 1u && memory[0x000f04ul] == 0u);
    assert(service_count == 1u && last_service_kind == 3u && last_service_status == 0x0281u);
    assert(selector_count == 1u && finalize_count == 1u);
}

int main(void)
{
    CheckOrdinaryInitialization();
    CheckSpecialInitialization();
    CheckActiveDescriptorFailure();
    return 0;
}
