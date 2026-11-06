#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/lead_in_out_descriptor_sequences.h"
#include "platform/firmware_memory.h"

static u8 memory[0x20000ul];
static u8 far_f7c01f;
static u8 lead_in_results[12];
static u8 lead_out_results[12];
static u8 wait_results[2];
static unsigned lead_in_count;
static unsigned lead_out_count;
static unsigned wait_count;
static u8 lead_in_initial[12];
static u8 lead_in_retry[12];
static u8 lead_out_retry[12];
static unsigned reset_count;
static unsigned clear_bit_count;
static unsigned set_sense_count;
static unsigned no_sense_count;
static unsigned fill_count;
static unsigned failure_count;
static u8 failure_code;
static unsigned queue_sense_count;
static unsigned range_count;
static s32 range_start;
static u32 range_length;
static u8 range_mode;
static u8 range_retry;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0xf7c01ful)
        return far_f7c01f;
    assert(address < sizeof(memory));
    return memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(memory));
    memory[address] = value;
}

void ObserveLeadInOutReset(void)
{
    ++reset_count;
}
void ObserveLeadInOutClearBit0035(void)
{
    ++clear_bit_count;
}

u8 ObserveLeadInSequence(u8 initial_atip, u8 retry)
{
    lead_in_initial[lead_in_count] = initial_atip;
    lead_in_retry[lead_in_count] = retry;
    return lead_in_results[lead_in_count++];
}

u8 ObserveLeadOutSequence(u8 retry)
{
    lead_out_retry[lead_out_count] = retry;
    return lead_out_results[lead_out_count++];
}

void ObserveLeadInOutSetSense(u8 sense_key, u16 asc_ascq)
{
    assert(sense_key == 0u && asc_ascq == 0u);
    ++set_sense_count;
}

void ObserveLeadInOutSetNoSense(void)
{
    ++no_sense_count;
}

u8 ObserveLeadInOutWait(void)
{
    return wait_results[wait_count++];
}

void ObserveLeadInOutFill(fw_addr_t address, u32 count, u8 value)
{
    assert(address == 0ul && count == 0x1b0000ul && value == 0u);
    ++fill_count;
}

void ObserveLeadInOutFailure(u8 code)
{
    ++failure_count;
    failure_code = code;
}

void ObserveLeadInOutQueueSense(void)
{
    ++queue_sense_count;
}

u8 ObserveLeadInOutRange(s32 start, u32 count, u8 mode, u8 retry)
{
    ++range_count;
    range_start = start;
    range_length = count;
    range_mode = mode;
    range_retry = retry;
    return 0u;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(lead_in_results, 1, sizeof(lead_in_results));
    memset(lead_out_results, 1, sizeof(lead_out_results));
    wait_results[0] = wait_results[1] = 1u;
    far_f7c01f = 0u;
    lead_in_count = lead_out_count = wait_count = 0u;
    reset_count = clear_bit_count = set_sense_count = 0u;
    no_sense_count = fill_count = failure_count = 0u;
    queue_sense_count = range_count = 0u;
    failure_code = range_mode = range_retry = 0u;
    range_start = 0;
    range_length = 0u;
    memory[0x88bau] = 1u;
    memory[0xe47u] = 0x20u;
    WriteU32(0x156au, 8u);
    WriteU32(0x1562u, 0x00123000ul);
}

static void TestRetrySuccessAndTerminalRange(void)
{
    ResetFixture();
    lead_in_results[0] = 0u;
    lead_out_results[0] = 0u;
    memory[0xfba0u] = 4u;
    memory[0x1008u] = 2u;
    memory[0xc45u] = 0xc0u;
    memory[0xc44u] = 3u;
    WriteU32(0x15f2u, 1000u);
    WriteU32(0x1153u, 2000u);

    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 1u);
    assert(reset_count == 1u && clear_bit_count == 1u);
    assert(lead_in_count == 2u && lead_in_initial[0] == 1u);
    assert(lead_in_retry[0] == 0u && lead_in_retry[1] == 1u);
    assert(set_sense_count == 1u);
    assert(wait_count == 2u);
    assert(fill_count == 1u);
    assert(ReadU16(0x129au) == 7u && ReadU16(0x129cu) == 2u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x00123000ul);
    assert(lead_out_count == 2u);
    assert(lead_out_retry[0] == 0u && lead_out_retry[1] == 1u);
    assert(memory[0xe47u] == 0x10u);
    assert(no_sense_count == 1u);
    assert(range_count == 1u && range_start == 1000l);
    assert(range_length == 4500ul && range_mode == 2u && range_retry == 1u);
    assert(memory[0x14b5u] == 1u);
    assert(failure_count == 0u && queue_sense_count == 0u);
}

static void TestSessionGateAndLeadInFailure(void)
{
    ResetFixture();
    memory[0x12c9u] = 1u;
    WriteU32(0xdd69u, 0xfffffffful);
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(lead_in_count == 0u);
    assert(failure_count == 1u && failure_code == 4u);

    ResetFixture();
    memory[0x88b7u] = 1u;
    memory[0x88bau] = 0u;
    lead_in_results[0] = 0u;
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(reset_count == 0u && lead_in_count == 1u);
    assert(failure_code == 4u);
}

static void TestWaitFailureClassification(void)
{
    ResetFixture();
    wait_results[0] = 0u;
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(queue_sense_count == 1u && failure_count == 0u);

    ResetFixture();
    wait_results[0] = 0u;
    memory[0x1293u] = 1u;
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(failure_count == 1u && failure_code == 4u);
}

static void TestLeadOutAndSecondWaitFailures(void)
{
    ResetFixture();
    lead_out_results[0] = 0u;
    memory[0xfba0u] = 3u;
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(failure_count == 1u && failure_code == 5u);

    ResetFixture();
    wait_results[1] = 0u;
    memory[0xfba0u] = 1u;
    assert(AcquireLeadInAndLeadOutRawSectorDescriptorSequences() == 0u);
    assert(failure_count == 1u && failure_code == 5u);
}

int main(void)
{
    TestRetrySuccessAndTerminalRange();
    TestSessionGateAndLeadInFailure();
    TestWaitFailureClassification();
    TestLeadOutAndSecondWaitFailures();
    puts("lead-in/out descriptor sequence tests passed");
    return 0;
}
