#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_leadout_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned resume_count;
static unsigned submit_count;
static unsigned block_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned advance_q_count;
static unsigned capacity_count;
static unsigned capacity_in_attempt;
static unsigned final_wait_count;
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;
static unsigned capacity_limit;
static u8 final_results[3];
static u8 prefix_modes[5];
static u8 suffix_modes[2];
static u8 terminal_mode;

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

static u16 ReadU16(fw_addr_t address)
{
    return ((u16)memory[address] << 8) | memory[address + 1u];
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static void WriteU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

fw_addr_t ObserveLeadOutResumeRing(void)
{
    capacity_in_attempt = 0u;
    return 0x2000ul + (fw_addr_t)(resume_count++ * 0x100ul);
}

void ObserveLeadOutConvert(s32 frame_position, fw_addr_t destination)
{
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObserveLeadOutPackBcd(u16 tuple_address)
{
    (void)tuple_address;
}

void ObserveLeadOutCopy(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObserveLeadOutWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObserveLeadOutSubmit(fw_addr_t cursor, u8 parameter)
{
    unsigned within_attempt = submit_count % 6751u;

    assert(parameter == 0u);
    assert(cursor >= 0x2000ul);
    if(within_attempt < 5u)
        prefix_modes[within_attempt] = memory[0x018028u];
    else if(within_attempt == 6748u)
        suffix_modes[0] = memory[0x018028u];
    else if(within_attempt == 6749u)
        suffix_modes[1] = memory[0x018028u];
    else if(within_attempt == 6750u)
        terminal_mode = memory[0x018028u];
    ++submit_count;
}

void ObserveLeadOutUpdateQ(void)
{
    ++update_q_count;
}

void ObserveLeadOutEnqueue(void)
{
    ++enqueue_count;
}

void ObserveLeadOutIncrement(u16 tuple_address)
{
    (void)tuple_address;
}

fw_addr_t ObserveLeadOutAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}

void ObserveLeadOutAdvanceQ(u8 reverse_relative)
{
    assert(reverse_relative == 0u);
    ++advance_q_count;
}

u8 ObserveLeadOutWaitForCapacity(void)
{
    ++capacity_count;
    if(capacity_in_attempt++ == capacity_limit)
    {
        memory[0x00fba0u] = 1u;
        return 0u;
    }
    return 1u;
}

u8 ObserveLeadOutWaitForRead(void)
{
    return final_results[final_wait_count++];
}

void ObserveLeadOutLatchFailure(void)
{
    ++latch_count;
}

void ObserveLeadOutSetNoSense(void)
{
    ++no_sense_count;
}

void ObserveLeadOutReportFailure(u8 failure_code)
{
    ++report_count;
    report_code = failure_code;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(final_results, 0, sizeof(final_results));
    memset(prefix_modes, 0, sizeof(prefix_modes));
    memset(suffix_modes, 0, sizeof(suffix_modes));
    resume_count = submit_count = block_count = update_q_count = 0u;
    enqueue_count = advance_q_count = capacity_count = 0u;
    capacity_in_attempt = 0u;
    final_wait_count = latch_count = no_sense_count = 0u;
    report_count = report_code = 0u;
    terminal_mode = 0u;
    capacity_limit = 0x1a57u;
    WriteU32(0x2ba58du, 0x00123456ul);
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x00111111ul);
    memory[0xf84890u] = 1u;
    memory[0xf84891u] = 2u;
    memory[0xf84892u] = 3u;
    memory[0xf84893u] = 4u;
    memory[0xf84894u] = 5u;
    memory[0xf84895u] = 6u;
    memory[0xf84896u] = 7u;
}

static void TestCompleteFirstAttempt(void)
{
    ResetFixture();
    final_results[0] = 1u;
    assert(AcquireType2LeadOutControllerRecordSequence() == 1u);
    assert(resume_count == 1u && capacity_count == 0x1a57u);
    assert(submit_count == 6751u && block_count == 6751u);
    assert(update_q_count == 6751u && enqueue_count == 6751u && advance_q_count == 6751u);
    assert(final_wait_count == 1u && no_sense_count == 1u);
    assert(latch_count == 0u && report_count == 0u);
    assert(ReadU32(0x1598u) == 0x00123456ul + 6751ul);
    assert(ReadU32(0x8000u) == 0x00123456ul);
    assert(memory[0x1272u] == 0x71u && memory[0x1273u] == 0xaau && memory[0x1274u] == 1u);
    assert(prefix_modes[0] == 0x22u && prefix_modes[4] == 0xa2u);
    assert(suffix_modes[0] == 0xc2u && suffix_modes[1] == 0xe2u);
    assert(terminal_mode == 0xa2u);
}

static void TestCapacityFailureRetriesThreeTimes(void)
{
    ResetFixture();
    capacity_limit = 2u;
    assert(AcquireType2LeadOutControllerRecordSequence() == 0u);
    assert(resume_count == 3u && capacity_count == 9u);
    assert(submit_count == 21u);
    assert(final_wait_count == 0u && latch_count == 3u);
    assert(report_count == 1u && report_code == 5u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x00111111ul);
    assert(memory[0x00fba0u] == 0u && memory[0x127du] == 0u);
}

static void TestFinalWaitFailureThenSuccess(void)
{
    ResetFixture();
    final_results[0] = 0u;
    final_results[1] = 1u;
    assert(AcquireType2LeadOutControllerRecordSequence() == 1u);
    assert(resume_count == 2u && submit_count == 13502u);
    assert(final_wait_count == 2u && latch_count == 1u);
    assert(no_sense_count == 1u && report_count == 0u);
}

int main(void)
{
    TestCompleteFirstAttempt();
    TestCapacityFailureRetriesThreeTimes();
    TestFinalWaitFailureThenSuccess();
    puts("controller record lead-out acquisition tests passed");
    return 0;
}
