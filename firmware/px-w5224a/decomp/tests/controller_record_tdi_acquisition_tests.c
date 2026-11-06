#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_tdi_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned resume_count;
static unsigned submit_count;
static unsigned block_count;
static unsigned payload_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned advance_q_count;
static unsigned service_count;
static unsigned yield_count;
static unsigned wait_count;
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;
static u8 wait_results[3];

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

fw_addr_t ObserveTdiResumeRing(void)
{
    return 0x2000ul + (fw_addr_t)(resume_count++ * 0x100ul);
}

void ObserveTdiConvert(s32 frame_position, fw_addr_t destination)
{
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObserveTdiPackBcd(u16 tuple_address)
{
    (void)tuple_address;
}

void ObserveTdiCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObserveTdiWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObserveTdiSubmit(fw_addr_t cursor, u8 parameter)
{
    assert(parameter == 0u && cursor >= 0x2000ul);
    ++submit_count;
}

void ObserveTdiCopyPayload(fw_addr_t destination, fw_addr_t source, u8 byte_count)
{
    unsigned within_attempt = payload_count % 138u;
    fw_addr_t attempt_cursor = 0x2000ul + (fw_addr_t)((resume_count - 1u) * 0x100ul);

    assert(destination == attempt_cursor + (fw_addr_t)(5u + within_attempt) * 0x930ul + 0x18ul);
    assert(source == 0x143cul && byte_count == 13u);
    ++payload_count;
}

void ObserveTdiUpdateQ(void)
{
    ++update_q_count;
}
void ObserveTdiEnqueue(void)
{
    ++enqueue_count;
}
void ObserveTdiIncrement(u16 tuple_address)
{
    (void)tuple_address;
}
fw_addr_t ObserveTdiAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}

void ObserveTdiAdvanceQ(u8 reverse_relative)
{
    assert(reverse_relative == 1u);
    ++advance_q_count;
}

void ObserveTdiServiceHost(void)
{
    ++service_count;
}
void ObserveTdiYield(void)
{
    ++yield_count;
}

u8 ObserveTdiWaitForRead(void)
{
    return wait_results[wait_count++];
}

void ObserveTdiLatchFailure(void)
{
    ++latch_count;
}
void ObserveTdiSetNoSense(void)
{
    ++no_sense_count;
}

void ObserveTdiReportFailure(u8 failure_code)
{
    ++report_count;
    report_code = failure_code;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(wait_results, 0, sizeof(wait_results));
    resume_count = submit_count = block_count = payload_count = 0u;
    update_q_count = enqueue_count = advance_q_count = 0u;
    service_count = yield_count = wait_count = latch_count = 0u;
    no_sense_count = report_count = report_code = 0u;
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

static void TestCompleteSequence(void)
{
    static const u8 expected_payload[13] = { 0x54u, 0x44u, 0x49u, 0x01u, 0x50u, 0x01u, 0x01u, 0x01u, 0x01u, 0x91u, 0x00u, 0x00u, 0x32u };

    ResetFixture();
    wait_results[0] = 1u;
    assert(AcquireType2TdiControllerRecordSequence() == 1u);
    assert(resume_count == 1u && submit_count == 146u && block_count == 146u);
    assert(payload_count == 138u && service_count == 138u && yield_count == 138u);
    assert(update_q_count == 146u && enqueue_count == 146u && advance_q_count == 146u);
    assert(wait_count == 1u && latch_count == 0u && no_sense_count == 1u);
    assert(report_count == 0u);
    assert(memcmp(&memory[0x143cu], expected_payload, sizeof(expected_payload)) == 0);
    assert(memory[0x1272u] == 0x71u && memory[0x1273u] == 1u && memory[0x1274u] == 0u);
    assert(memory[0x1275u] == 0u && memory[0x1276u] == 0u && memory[0x1277u] == 0x95u);
    assert(ReadU32(0x1598u) == 146ul && ReadU32(0x2ba59fu) == 145ul);
}

static void TestRetryThenSuccess(void)
{
    ResetFixture();
    wait_results[0] = 0u;
    wait_results[1] = 1u;
    assert(AcquireType2TdiControllerRecordSequence() == 1u);
    assert(resume_count == 2u && submit_count == 292u);
    assert(payload_count == 276u && service_count == 276u);
    assert(wait_count == 2u && latch_count == 1u);
    assert(no_sense_count == 1u && report_count == 0u);
}

static void TestThreeFailuresReportZero(void)
{
    ResetFixture();
    assert(AcquireType2TdiControllerRecordSequence() == 0u);
    assert(resume_count == 3u && submit_count == 438u);
    assert(wait_count == 3u && latch_count == 3u);
    assert(report_count == 1u && report_code == 0u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x88bbul);
    assert(ReadU32(0x126au) == 0x00111111ul);
    assert(memory[0xfba0u] == 0u && memory[0x127du] == 0u);
}

int main(void)
{
    TestCompleteSequence();
    TestRetryThenSuccess();
    TestThreeFailuresReportZero();
    puts("controller record TDI acquisition tests passed");
    return 0;
}
