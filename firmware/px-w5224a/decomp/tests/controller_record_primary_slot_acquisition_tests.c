#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_primary_slot_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static unsigned resume_count;
static unsigned convert_count;
static s32 converted_frames[2];
static unsigned submit_count;
static unsigned block_count;
static unsigned q_copy_count;
static u16 q_sources[80];
static unsigned q_overlay_count;
static unsigned build_count;
static unsigned copy_workspace_count;
static u8 observed_banks[64];
static u8 observed_pages[64];
static unsigned payload_copy_count;
static unsigned update_q_count;
static unsigned enqueue_count;
static unsigned increment_count;
static unsigned advance_count;
static unsigned increment_record_count;
static u8 increment_record_slots[2];
static unsigned final_wait_count;
static u8 final_results[2];
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;
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

fw_addr_t ObservePrimarySlotResumeRing(void)
{
    return 0x2000ul + (fw_addr_t)(resume_count++ * 0x10000ul);
}

void ObservePrimarySlotConvert(s32 frame_position, fw_addr_t destination)
{
    assert(convert_count < 2u);
    converted_frames[convert_count++] = frame_position;
    memory[destination] = (u8)((u32)frame_position >> 16);
    memory[destination + 1u] = (u8)((u32)frame_position >> 8);
    memory[destination + 2u] = (u8)frame_position;
}

void ObservePrimarySlotPackBcd(u16 tuple_address)
{
    assert(tuple_address == 0x7000u);
}

void ObservePrimarySlotCopyFar(fw_addr_t source, fw_addr_t destination, u16 byte_count)
{
    assert(source == 0x7000ul && byte_count == 3u);
    assert(destination == 0x1428ul || destination == 0x1275ul);
    if(destination == 0x1275ul)
        ++q_overlay_count;
    while(byte_count-- != 0u)
        memory[destination++] = memory[source++];
}

void ObservePrimarySlotWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
    ++block_count;
}

void ObservePrimarySlotSubmit(fw_addr_t cursor, u8 parameter)
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

void ObservePrimarySlotCopyQ(u16 destination, u16 source, u8 byte_count)
{
    u8 index;

    assert(destination == 0x1272u && byte_count == 10u);
    assert(q_copy_count < 80u);
    q_sources[q_copy_count++] = source;
    for(index = 0u; index != byte_count; ++index)
        memory[(u16)(destination + index)] = memory[(u16)(source + index)];
}

void ObservePrimarySlotBuildWorkspace(u8 bank_index, u8 page_index)
{
    assert(build_count < 64u);
    observed_banks[build_count] = bank_index;
    observed_pages[build_count] = page_index;
    ++build_count;
}

void ObservePrimarySlotCopyWorkspace(u8 bank_index, u8 page_index)
{
    assert(copy_workspace_count < 64u);
    observed_banks[copy_workspace_count] = bank_index;
    observed_pages[copy_workspace_count] = page_index;
    ++copy_workspace_count;
}

void ObservePrimarySlotCopyPayload(fw_addr_t destination, fw_addr_t source, u16 dword_count)
{
    unsigned within_attempt = payload_copy_count % 32u;
    unsigned attempt = payload_copy_count / 32u;
    fw_addr_t expected_cursor = 0x2000ul + (fw_addr_t)(attempt * 0x10000ul) + (fw_addr_t)((within_attempt + 5u) * 0x930ul);

    assert(destination == expected_cursor + 0x18ul);
    assert(source == 0x2ba62ful && dword_count == 0x200u);
    ++payload_copy_count;
}

void ObservePrimarySlotUpdateQ(void)
{
    ++update_q_count;
}

void ObservePrimarySlotEnqueue(void)
{
    ++enqueue_count;
}

void ObservePrimarySlotIncrement(u16 tuple_address)
{
    assert(tuple_address == 0x7000u || tuple_address == 0x1428u);
    ++increment_count;
}

fw_addr_t ObservePrimarySlotAdvanceRing(fw_addr_t cursor)
{
    ++advance_count;
    return cursor + 0x930ul;
}

void ObservePrimarySlotIncrementRecord(u8 record_slot_index)
{
    assert(increment_record_count < 2u);
    increment_record_slots[increment_record_count++] = record_slot_index;
}

u8 ObservePrimarySlotWaitForRead(void)
{
    return final_results[final_wait_count++];
}

void ObservePrimarySlotLatchFailure(void)
{
    ++latch_count;
}

void ObservePrimarySlotSetNoSense(void)
{
    ++no_sense_count;
}

void ObservePrimarySlotReportFailure(u8 failure_code)
{
    ++report_count;
    report_code = failure_code;
}

static void ResetFixture(void)
{
    unsigned record;

    memset(memory, 0, sizeof(memory));
    memset(converted_frames, 0, sizeof(converted_frames));
    memset(q_sources, 0, sizeof(q_sources));
    memset(observed_banks, 0, sizeof(observed_banks));
    memset(observed_pages, 0, sizeof(observed_pages));
    memset(increment_record_slots, 0, sizeof(increment_record_slots));
    memset(final_results, 0, sizeof(final_results));
    memset(prefix_modes, 0, sizeof(prefix_modes));
    memset(suffix_modes, 0, sizeof(suffix_modes));
    resume_count = convert_count = submit_count = block_count = 0u;
    q_copy_count = q_overlay_count = build_count = 0u;
    copy_workspace_count = payload_copy_count = update_q_count = 0u;
    enqueue_count = increment_count = advance_count = 0u;
    increment_record_count = final_wait_count = latch_count = 0u;
    no_sense_count = report_count = report_code = 0u;
    terminal_mode = 0u;
    WriteU16(0x8008u, 1u);
    WriteU16(0x800au, 3u);
    for(record = 0u; record != 4u; ++record)
    {
        memory[0x8012u + record * 10u] = 0x11u;
        memory[0x8013u + record * 10u] = (u8)record;
    }
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
    assert(resume_count == attempts && convert_count == attempts);
    assert(submit_count == attempts * 40u);
    assert(block_count == attempts * 40u);
    assert(q_copy_count == attempts * 40u);
    assert(q_overlay_count == attempts * 40u);
    assert(payload_copy_count == attempts * 32u);
    assert(update_q_count == attempts * 40u);
    assert(enqueue_count == attempts * 40u);
    assert(increment_count == attempts * 80u);
    assert(advance_count == attempts * 40u);
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

static void CheckQSequence(unsigned offset)
{
    static const u8 groups[4] = { 0u, 2u, 1u, 3u };
    unsigned index;

    for(index = 0u; index != 39u; ++index)
    {
        u8 record = groups[(index / 3u) & 3u];
        assert(q_sources[offset + index] == (u16)(0x8012u + record * 10u));
    }
    assert(q_sources[offset + 39u] == 0x8012u);
}

static void TestBuildModeCompletesFirstAttempt(void)
{
    ResetFixture();
    final_results[0] = 1u;
    assert(AcquirePrimaryType2ControllerRecordSlot(0u, 6u, 1000l) == 1u);
    CheckTotals(1u);
    assert(build_count == 32u && copy_workspace_count == 0u);
    CheckPageSequence(0u, 6u);
    CheckQSequence(0u);
    assert(increment_record_count == 0u);
    assert(converted_frames[0] == 1000l);
    assert(ReadU32(0x1598u) == 1040ul);
    assert(final_wait_count == 1u && no_sense_count == 1u);
    assert(latch_count == 0u && report_count == 0u);
    assert(prefix_modes[0] == 0x22u && prefix_modes[4] == 0xa2u);
    assert(suffix_modes[0] == 0xc2u && suffix_modes[1] == 0xe2u);
    assert(terminal_mode == 0xa2u);
}

static void TestModeOneCopiesAndRetriesOnce(void)
{
    ResetFixture();
    final_results[0] = 0u;
    final_results[1] = 1u;
    assert(AcquirePrimaryType2ControllerRecordSlot(1u, 3u, -25l) == 1u);
    CheckTotals(2u);
    assert(build_count == 0u && copy_workspace_count == 64u);
    CheckPageSequence(0u, 3u);
    CheckPageSequence(32u, 3u);
    CheckQSequence(0u);
    CheckQSequence(40u);
    assert(increment_record_count == 2u);
    assert(increment_record_slots[0] == 3u && increment_record_slots[1] == 3u);
    assert(converted_frames[0] == -25l && converted_frames[1] == -25l);
    assert(final_wait_count == 2u && latch_count == 1u);
    assert(no_sense_count == 1u && report_count == 0u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x129eu) == 0x000088bbul);
    assert(ReadU32(0x126au) == 0x00112233ul);
}

static void TestOtherNonzeroModeDoesNotIncrementAndReportsFour(void)
{
    ResetFixture();
    assert(AcquirePrimaryType2ControllerRecordSlot(2u, 7u, 2000l) == 0u);
    CheckTotals(2u);
    assert(build_count == 0u && copy_workspace_count == 64u);
    assert(increment_record_count == 0u);
    assert(final_wait_count == 2u && latch_count == 2u);
    assert(no_sense_count == 0u);
    assert(report_count == 1u && report_code == 4u);
    assert(memory[0xfba0u] == 0u && memory[0x127du] == 0u);
}

int main(void)
{
    TestBuildModeCompletesFirstAttempt();
    TestModeOneCopiesAndRetriesOnce();
    TestOtherNonzeroModeDoesNotIncrementAndReportsFour();
    puts("controller record primary slot acquisition tests passed");
    return 0;
}
