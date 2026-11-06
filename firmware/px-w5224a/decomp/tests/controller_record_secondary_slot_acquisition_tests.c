#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_secondary_slot_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned resume_count;
static unsigned convert_count;
static s32 converted_frames[6];
static unsigned submit_count;
static unsigned block_count;
static unsigned build_count;
static unsigned copy_workspace_count;
static unsigned marker_count;
static unsigned payload_copy_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned advance_q_count;
static unsigned final_wait_count;
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;
static u8 final_results[2];
static u8 observed_banks[64];
static u8 observed_pages[64];
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

fw_addr_t ObserveSecondarySlotResumeRing(void)
{
    return 0x2000ul + (fw_addr_t)(resume_count++ * 0x10000ul);
}

void ObserveSecondarySlotConvert(s32 frame_position, fw_addr_t destination)
{
    assert(convert_count < sizeof(converted_frames) / sizeof(converted_frames[0]));
    converted_frames[convert_count++] = frame_position;
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObserveSecondarySlotPackBcd(u16 tuple_address)
{
    assert(tuple_address == 0x7000u);
}

void ObserveSecondarySlotCopyTuple(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    assert(source == 0x7000ul && byte_count == 3u);
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObserveSecondarySlotWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObserveSecondarySlotSubmit(fw_addr_t cursor, u8 parameter)
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

void ObserveSecondarySlotBuildWorkspace(u8 bank_index, u8 page_index)
{
    assert(build_count < 64u);
    observed_banks[build_count] = bank_index;
    observed_pages[build_count] = page_index;
    ++build_count;
}

void ObserveSecondarySlotCopyWorkspace(u8 bank_index, u8 page_index)
{
    assert(copy_workspace_count < 64u);
    observed_banks[copy_workspace_count] = bank_index;
    observed_pages[copy_workspace_count] = page_index;
    ++copy_workspace_count;
}

void ObserveSecondarySlotWriteMarker(void)
{
    ++marker_count;
}

void ObserveSecondarySlotCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count)
{
    unsigned within_attempt = payload_copy_count % 32u;
    unsigned attempt = payload_copy_count / 32u;
    fw_addr_t expected_cursor = 0x2000ul + (fw_addr_t)(attempt * 0x10000ul) + (fw_addr_t)((within_attempt + 5u) * 0x930ul);

    assert(destination == expected_cursor + 0x18ul);
    assert(source == 0x2ba62ful);
    assert(dword_count == 0x200u);
    ++payload_copy_count;
}

void ObserveSecondarySlotUpdateQ(void)
{
    ++update_q_count;
}

void ObserveSecondarySlotEnqueue(void)
{
    ++enqueue_count;
}

void ObserveSecondarySlotIncrement(u16 tuple_address)
{
    assert(tuple_address == 0x7000u || tuple_address == 0x1428u);
}

fw_addr_t ObserveSecondarySlotAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}

void ObserveSecondarySlotAdvanceQ(u8 reverse_relative)
{
    assert(reverse_relative == 0u);
    ++advance_q_count;
}

u8 ObserveSecondarySlotWaitForRead(void)
{
    return final_results[final_wait_count++];
}

void ObserveSecondarySlotLatchFailure(void)
{
    ++latch_count;
}

void ObserveSecondarySlotSetNoSense(void)
{
    ++no_sense_count;
}

void ObserveSecondarySlotReportFailure(u8 failure_code)
{
    ++report_count;
    report_code = failure_code;
}

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(converted_frames, 0, sizeof(converted_frames));
    memset(final_results, 0, sizeof(final_results));
    memset(observed_banks, 0, sizeof(observed_banks));
    memset(observed_pages, 0, sizeof(observed_pages));
    memset(prefix_modes, 0, sizeof(prefix_modes));
    memset(suffix_modes, 0, sizeof(suffix_modes));
    resume_count = convert_count = submit_count = block_count = 0u;
    build_count = copy_workspace_count = marker_count = 0u;
    payload_copy_count = update_q_count = enqueue_count = 0u;
    advance_q_count = final_wait_count = latch_count = 0u;
    no_sense_count = report_count = report_code = 0u;
    terminal_mode = 0u;
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

static void CheckDescriptorTotals(unsigned attempts)
{
    assert(resume_count == attempts);
    assert(submit_count == attempts * 40u);
    assert(block_count == attempts * 40u);
    assert(marker_count == attempts * 32u);
    assert(payload_copy_count == attempts * 32u);
    assert(update_q_count == attempts * 40u);
    assert(enqueue_count == attempts * 40u);
    assert(advance_q_count == attempts * 40u);
}

static void CheckPageSequence(unsigned offset, u8 bank)
{
    static const u8 expected[32] = { 0u, 1u, 2u, 3u, 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 4u, 5u, 6u, 7u, 0u, 1u, 0u, 1u, 2u, 3u, 2u, 3u, 4u, 5u, 4u, 5u, 6u, 7u, 6u, 7u };
    unsigned index;

    for(index = 0u; index != 32u; ++index)
    {
        assert(observed_banks[offset + index] == bank);
        assert(observed_pages[offset + index] == expected[index]);
    }
}

static void TestBuildModeCompletesFirstAttempt(void)
{
    ResetFixture();
    final_results[0] = 1u;
    assert(AcquireSecondaryType2ControllerRecordSlot(0u, 6u, 1000l) == 1u);
    CheckDescriptorTotals(1u);
    assert(build_count == 32u && copy_workspace_count == 0u);
    CheckPageSequence(0u, 6u);
    assert(final_wait_count == 1u && no_sense_count == 1u);
    assert(latch_count == 0u && report_count == 0u);
    assert(convert_count == 3u);
    assert(converted_frames[0] == 1000l);
    assert(converted_frames[1] == 850l);
    assert(converted_frames[2] == 1000l);
    assert(ReadU32(0x1598u) == 1040ul);
    assert(memory[0x1272u] == 0x71u && memory[0x1273u] == 1u && memory[0x1274u] == 1u && memory[0x1278u] == 0u);
    assert(prefix_modes[0] == 0x22u && prefix_modes[4] == 0xa2u);
    assert(suffix_modes[0] == 0xc2u && suffix_modes[1] == 0xe2u);
    assert(terminal_mode == 0xa2u);
    assert(memory[0x18021u] == 0u);
}

static void TestCopyModeRetriesOnce(void)
{
    ResetFixture();
    final_results[0] = 0u;
    final_results[1] = 1u;
    assert(AcquireSecondaryType2ControllerRecordSlot(9u, 3u, -25l) == 1u);
    CheckDescriptorTotals(2u);
    assert(build_count == 0u && copy_workspace_count == 64u);
    CheckPageSequence(0u, 3u);
    CheckPageSequence(32u, 3u);
    assert(final_wait_count == 2u && latch_count == 1u);
    assert(no_sense_count == 1u && report_count == 0u);
    assert(convert_count == 6u);
    assert(converted_frames[0] == -25l);
    assert(converted_frames[1] == -175l);
    assert(converted_frames[3] == -25l);
    assert(ReadU16(0x1298u) == 0u && ReadU16(0x1294u) == 0u);
    assert(ReadU16(0x1296u) == 0u && ReadU16(0x1292u) == 0u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x000088bbul);
    assert(ReadU32(0x126au) == 0x00112233ul);
}

static void TestTwoFailuresReportCodeFive(void)
{
    ResetFixture();
    assert(AcquireSecondaryType2ControllerRecordSlot(0u, 7u, 2000l) == 0u);
    CheckDescriptorTotals(2u);
    assert(build_count == 64u && copy_workspace_count == 0u);
    assert(final_wait_count == 2u && latch_count == 2u);
    assert(no_sense_count == 0u);
    assert(report_count == 1u && report_code == 5u);
    assert(memory[0xfba0u] == 0u && memory[0x127du] == 0u);
}

int main(void)
{
    TestBuildModeCompletesFirstAttempt();
    TestCopyModeRetriesOnce();
    TestTwoFailuresReportCodeFive();
    puts("controller record secondary slot acquisition tests passed");
    return 0;
}
