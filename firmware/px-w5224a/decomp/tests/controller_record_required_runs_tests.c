#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_required_runs.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned acquire_count;
static u16 acquired_last;
static u8 acquired_mode;
static unsigned divide_count;
static unsigned recalculate_count;
static unsigned build_count;
static u8 build_selector;
static unsigned base_count;
static u8 inject_status_fault;
static u8 inject_run_latch;

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
static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}
static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

void ObserveRequiredRunsAcquire(u16 last_index, u8 operation_mode)
{
    ++acquire_count;
    acquired_last = last_index;
    acquired_mode = operation_mode;
    if(inject_status_fault != 0u)
        memory[0x0c89u] |= 4u;
    if(inject_run_latch != 0u)
        memory[0x2ba5b1u] = 1u;
}
u32 ObserveRequiredRunsDivide(u32 dividend, u32 divisor)
{
    ++divide_count;
    assert(divisor == 39ul);
    return dividend / divisor;
}
void ObserveRequiredRunsRecalculate(void)
{
    ++recalculate_count;
}
void ObserveRequiredRunsBuildLeadIn(u8 selector)
{
    ++build_count;
    build_selector = selector;
}
void ObserveRequiredRunsAcquireBaseWindow(void)
{
    ++base_count;
}

static void SetBoundary(u16 segment, u16 index)
{
    WriteU32(0xf84490ul + segment * 4u, (u32)index * 39ul + 145ul);
}
static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    acquire_count = divide_count = recalculate_count = build_count = base_count = 0u;
    acquired_last = 0u;
    acquired_mode = build_selector = 0u;
    inject_status_fault = inject_run_latch = 0u;
    WriteU16(0x2ba5d9u, 600u);
    WriteU16(0x2ba5dbu, 500u);
    memory[0x2ba588u] = 3u;
    memory[0x0c88u] = 0xffu;
    SetBoundary(0u, 100u);
    SetBoundary(1u, 300u);
    SetBoundary(2u, 500u);
}

static void TestOrdinaryAndModeThreeFullRanges(void)
{
    ResetFixture();
    assert(AcquireRequiredControllerRecordIndexRuns(0u) == 1u);
    assert(acquire_count == 1u && acquired_last == 599u && acquired_mode == 0u);
    assert(divide_count == 0u && memory[0x2ba5afu] == 1u);
    assert((memory[0x0c88u] & 0x40u) == 0u);

    ResetFixture();
    assert(AcquireRequiredControllerRecordIndexRuns(3u) == 1u);
    assert(acquired_last == 599u && acquired_mode == 3u && divide_count == 1u);
}

static void TestModeThreeSearchesBackward(void)
{
    ResetFixture();
    WriteU16(0x2ba5dbu, 200u);
    assert(AcquireRequiredControllerRecordIndexRuns(3u) == 1u);
    assert(acquired_last == 275u && divide_count == 3u);
}

static void TestStatusFaultRunsRecovery(void)
{
    ResetFixture();
    inject_status_fault = 1u;
    memory[0x0c89u] = 0xa5u;
    assert(AcquireRequiredControllerRecordIndexRuns(2u) == 0u);
    assert((memory[0x0c89u] & 4u) == 0u && (memory[0x0c88u] & 0x40u) == 0u);
    assert(recalculate_count == 1u && build_count == 1u && build_selector == 0u && base_count == 1u);
}

static void TestRunLatchPublishesRetry(void)
{
    ResetFixture();
    inject_run_latch = 1u;
    memory[0x2ba5b0u] = 1u;
    memory[0x1588u] = 0x80u;
    assert(AcquireRequiredControllerRecordIndexRuns(1u) == 0u);
    assert(memory[0x1588u] == 0x84u && memory[0x2ba5b0u] == 0u);
    assert(recalculate_count == 0u && base_count == 0u);
}

int main(void)
{
    TestOrdinaryAndModeThreeFullRanges();
    TestModeThreeSearchesBackward();
    TestStatusFaultRunsRecovery();
    TestRunLatchPublishesRetry();
    puts("controller record required-runs tests passed");
    return 0;
}
