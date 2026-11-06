#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_leadin_acquisition.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static u16 q_sources[128];
static unsigned q_copy_count;
static unsigned tuple_copy_to_relative_count;
static unsigned submit_count;
static unsigned submit_in_attempt;
static unsigned capacity_count;
static unsigned capacity_false_once;
static unsigned wait_count;
static u8 wait_results[3];
static unsigned latch_count;
static unsigned no_sense_count;
static unsigned report_count;
static u8 report_code;

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

fw_addr_t ObserveLeadInResumeRing(void)
{
    submit_in_attempt = 0u;
    return 0x2000ul;
}
void ObserveLeadInConvert(s32 frame, fw_addr_t destination)
{
    memory[destination] = (u8)((u32)frame >> 16);
    memory[destination + 1u] = (u8)((u32)frame >> 8);
    memory[destination + 2u] = (u8)frame;
}
void ObserveLeadInPackBcd(u16 address)
{
    (void)address;
}
void ObserveLeadInCopyTuple(fw_addr_t source, fw_addr_t destination, u16 count)
{
    if(destination == 0x1275ul)
        ++tuple_copy_to_relative_count;
    while(count-- != 0u)
        memory[destination++] = memory[source++];
}
void ObserveLeadInWriteControllerBlock(fw_addr_t source)
{
    assert(source == 0x1428ul);
}
void ObserveLeadInSubmit(fw_addr_t cursor, u8 parameter)
{
    assert(cursor == 0x2000ul + submit_in_attempt * 0x930ul);
    assert(parameter == 0u);
    ++submit_in_attempt;
    ++submit_count;
}
void ObserveLeadInCopyQ(u16 destination, u16 source, u8 count)
{
    assert(destination == 0x1272u && count == 10u);
    assert(q_copy_count < 128u);
    q_sources[q_copy_count++] = source;
    memcpy(&memory[destination], &memory[source], count);
}
void ObserveLeadInUpdateQ(void) {}
void ObserveLeadInEnqueue(void) {}
void ObserveLeadInIncrement(u16 address)
{
    (void)address;
}
fw_addr_t ObserveLeadInAdvanceRing(fw_addr_t cursor)
{
    return cursor + 0x930ul;
}
u8 ObserveLeadInWaitForCapacity(void)
{
    ++capacity_count;
    if(capacity_false_once != 0u)
    {
        capacity_false_once = 0u;
        return 0u;
    }
    return 1u;
}
u8 ObserveLeadInWaitForRead(void)
{
    return wait_results[wait_count++];
}
void ObserveLeadInLatchFailure(void)
{
    ++latch_count;
}
void ObserveLeadInSetNoSense(void)
{
    ++no_sense_count;
}
void ObserveLeadInReportFailure(u8 code)
{
    ++report_count;
    report_code = code;
}
s32 ObserveLeadInDivide(s32 dividend, s32 divisor)
{
    return dividend / divisor;
}
u32 ObserveLeadInMultiply(u32 left, u32 right)
{
    return left * right;
}

static void ResetFixture(void)
{
    unsigned index;
    memset(memory, 0, sizeof(memory));
    memset(q_sources, 0, sizeof(q_sources));
    memset(wait_results, 0, sizeof(wait_results));
    q_copy_count = tuple_copy_to_relative_count = submit_count = 0u;
    submit_in_attempt = 0u;
    capacity_count = capacity_false_once = wait_count = latch_count = 0u;
    no_sense_count = report_count = report_code = 0u;
    WriteU32(0x114fu, 100ul);
    WriteU32(0x2ba577u, (u32)-54l);
    WriteU32(0x156au, 16ul);
    WriteU32(0x1562u, 0x00111111ul);
    WriteU16(0x8008u, 2u);
    WriteU16(0x800au, 4u);
    for(index = 0u; index != 5u; ++index)
    {
        memory[0x8012u + index * 10u] = (u8)(index == 1u ? 0x11u : 0x12u);
        memory[0xf84890u + index] = (u8)(index + 1u);
    }
    memory[0xf84895u] = 6u;
    memory[0xf84896u] = 7u;
}

static void TestCompleteGeometryAndQSelection(void)
{
    static const u16 expected_sources[18] = { 0x8012u, 0x8012u, 0x8012u, 0x8030u, 0x8030u, 0x8030u, 0x801cu, 0x801cu, 0x801cu, 0x803au, 0x803au, 0x803au, 0x8026u, 0x8026u, 0x8026u, 0x8030u, 0x8030u,
        0x8030u };
    ResetFixture();
    wait_results[0] = 1u;
    assert(AcquireType2LeadInControllerRecordSequence() == 1u);
    assert(submit_count == 18u && q_copy_count == 18u);
    assert(memcmp(q_sources, expected_sources, sizeof(expected_sources)) == 0);
    assert(capacity_count == 10u && wait_count == 1u);
    assert(tuple_copy_to_relative_count == 3u);
    assert(ReadU32(0x1598u) == 118ul && ReadU32(0x8000u) == 100ul);
    assert(no_sense_count == 1u && latch_count == 0u && report_count == 0u);
}

static void TestTransientCapacityMissAdvancesSelection(void)
{
    ResetFixture();
    capacity_false_once = 1u;
    wait_results[0] = 1u;
    assert(AcquireType2LeadInControllerRecordSequence() == 1u);
    assert(capacity_count == 11u && submit_count == 18u);
    assert(q_sources[5] == 0x801cu);
}

static void TestThreeFinalFailures(void)
{
    ResetFixture();
    assert(AcquireType2LeadInControllerRecordSequence() == 0u);
    assert(submit_count == 54u && wait_count == 3u && latch_count == 3u);
    assert(report_count == 1u && report_code == 4u);
    assert(ReadU16(0x129au) == 15u && ReadU16(0x129cu) == 10u);
    assert(ReadU32(0x126au) == 0x00111111ul && memory[0xfba0u] == 0u);
}

int main(void)
{
    TestCompleteGeometryAndQSelection();
    TestTransientCapacityMissAdvancesSelection();
    TestThreeFinalFailures();
    puts("controller record lead-in acquisition tests passed");
    return 0;
}
