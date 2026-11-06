#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_window_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned resume_count;
static unsigned convert_count;
static s32 converted_frames[9];
static unsigned submit_count;
static unsigned block_count;
static unsigned mip_count;
static unsigned marker_count;
static unsigned payload_copy_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned advance_q_count;
static unsigned final_wait_count;
static u8 final_results[3];
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;
static u8 retry_start_override;
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

fw_addr_t ObserveRecordWindowResumeRing(void)
{
    return 0x2000ul + (fw_addr_t)(resume_count++ * 0x10000ul);
}

void ObserveRecordWindowConvert(s32 frame_position, fw_addr_t destination)
{
    assert(convert_count < 9u);
    converted_frames[convert_count++] = frame_position;
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObserveRecordWindowPackBcd(u16 tuple_address)
{
    assert(tuple_address == 0x7000u);
}

void ObserveRecordWindowCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    assert(source == 0x7000ul && byte_count == 3u);
    assert(destination == 0x1428ul || destination == 0x1275ul || destination == 0x1279ul);
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObserveRecordWindowWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObserveRecordWindowSubmit(fw_addr_t cursor, u8 parameter)
{
    unsigned within_attempt = submit_count % 40u;
    fw_addr_t attempt_base = 0x2000ul + (fw_addr_t)((submit_count / 40u) * 0x10000ul);

    assert(parameter == 0u);
    assert(cursor == attempt_base + (fw_addr_t)(within_attempt * 0x930ul));
    if(within_attempt < 5u)
        prefix_modes[within_attempt] = memory[0x18028u];
    else if(within_attempt == 37u)
        suffix_modes[0] = memory[0x18028u];
    else if(within_attempt == 38u)
        suffix_modes[1] = memory[0x18028u];
    else if(within_attempt == 39u)
        terminal_mode = memory[0x18028u];
    ++submit_count;
}

void ObserveRecordWindowBuildMip(void)
{
    ++mip_count;
}

void ObserveRecordWindowWriteMarker(void)
{
    ++marker_count;
}

void ObserveRecordWindowCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count)
{
    unsigned within_attempt = payload_copy_count % 32u;
    unsigned attempt = payload_copy_count / 32u;
    fw_addr_t expected_cursor = 0x2000ul + (fw_addr_t)(attempt * 0x10000ul) + (fw_addr_t)((within_attempt + 5u) * 0x930ul);

    assert(destination == expected_cursor + 0x18ul);
    assert(source == 0x2ba5dful && dword_count == 0x14u);
    ++payload_copy_count;
}

void ObserveRecordWindowUpdateQ(void)
{
    ++update_q_count;
}

void ObserveRecordWindowEnqueue(void)
{
    ++enqueue_count;
}

void ObserveRecordWindowIncrement(u16 tuple_address)
{
    assert(tuple_address == 0x7000u || tuple_address == 0x1428u);
}

fw_addr_t ObserveRecordWindowAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}

void ObserveRecordWindowAdvanceQ(u8 reverse_relative)
{
    assert(reverse_relative == 0u);
    ++advance_q_count;
}

u8 ObserveRecordWindowWaitForRead(void)
{
    u8 result = final_results[final_wait_count++];

    if(result == 0u && retry_start_override != 0u)
        WriteU32(0x2ba595u, 1200ul);
    return result;
}

void ObserveRecordWindowLatchFailure(void)
{
    ++latch_count;
}

void ObserveRecordWindowSetNoSense(void)
{
    ++no_sense_count;
}

void ObserveRecordWindowReportFailure(u8 failure_code)
{
    ++report_count;
    report_code = failure_code;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(converted_frames, 0, sizeof(converted_frames));
    memset(final_results, 0, sizeof(final_results));
    memset(prefix_modes, 0, sizeof(prefix_modes));
    memset(suffix_modes, 0, sizeof(suffix_modes));
    resume_count = convert_count = submit_count = block_count = 0u;
    mip_count = marker_count = payload_copy_count = 0u;
    update_q_count = enqueue_count = advance_q_count = 0u;
    final_wait_count = latch_count = no_sense_count = 0u;
    report_count = report_code = 0u;
    retry_start_override = 0u;
    terminal_mode = 0u;
    WriteU32(0x2ba595u, 1000ul);
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x00112233ul);
    memory[0xf84890u] = 1u;
    memory[0xf84891u] = 2u;
    memory[0xf84892u] = 3u;
    memory[0xf84893u] = 4u;
    memory[0xf84894u] = 5u;
    memory[0xf84895u] = 6u;
    memory[0xf84896u] = 7u;
}

static void CheckTotals(unsigned attempts)
{
    assert(resume_count == attempts && convert_count == attempts * 3u);
    assert(submit_count == attempts * 40u);
    assert(block_count == attempts * 40u);
    assert(mip_count == attempts * 32u);
    assert(marker_count == attempts * 32u);
    assert(payload_copy_count == attempts * 32u);
    assert(update_q_count == attempts * 40u);
    assert(enqueue_count == attempts * 40u);
    assert(advance_q_count == attempts * 40u);
}

static void TestCompletesFirstAttempt(void)
{
    ResetFixture();
    final_results[0] = 1u;
    assert(AcquireFortyFrameType2ControllerRecordWindow() == 1u);
    CheckTotals(1u);
    assert(converted_frames[0] == 1000l);
    assert(converted_frames[1] == 850l);
    assert(converted_frames[2] == 1000l);
    assert(ReadU32(0x1598u) == 1040ul);
    assert(ReadU32(0x8000u) == 1000ul);
    assert(memory[0x1272u] == 0x71u && memory[0x1273u] == 1u && memory[0x1274u] == 1u && memory[0x1278u] == 0u);
    assert(prefix_modes[0] == 0x22u && prefix_modes[4] == 0xa2u);
    assert(suffix_modes[0] == 0xc2u && suffix_modes[1] == 0xe2u);
    assert(terminal_mode == 0xa2u);
    assert(final_wait_count == 1u && no_sense_count == 1u);
    assert(latch_count == 0u && report_count == 0u);
}

static void TestRetriesAndReReadsStartPosition(void)
{
    ResetFixture();
    final_results[0] = 0u;
    final_results[1] = 1u;
    retry_start_override = 1u;
    assert(AcquireFortyFrameType2ControllerRecordWindow() == 1u);
    CheckTotals(2u);
    assert(converted_frames[0] == 1000l && converted_frames[3] == 1200l);
    assert(final_wait_count == 2u && latch_count == 1u);
    assert(no_sense_count == 1u && report_count == 0u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x000088bbul);
    assert(ReadU32(0x126au) == 0x00112233ul);
}

static void TestThreeFailuresReportFive(void)
{
    ResetFixture();
    assert(AcquireFortyFrameType2ControllerRecordWindow() == 0u);
    CheckTotals(3u);
    assert(final_wait_count == 3u && latch_count == 3u);
    assert(no_sense_count == 0u);
    assert(report_count == 1u && report_code == 5u);
    assert(memory[0xfba0u] == 0u && memory[0x127du] == 0u);
}

int main(void)
{
    TestCompletesFirstAttempt();
    TestRetriesAndReReadsStartPosition();
    TestThreeFailuresReportFive();
    puts("controller record window acquisition tests passed");
    return 0;
}
