#include <assert.h>
#include <string.h>
#include "platform/controller_word0251_sweep.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u16 d8_result, d9_result;
static unsigned drain_count, submit_count, fb_count;
static u16 submitted_value, submitted_selector;
static u8 fb_value;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

u16 DrainFourSfrD8OrD9Bytes(u8 select_sfr_d9)
{
    ++drain_count;
    return select_sfr_d9 == 0u ? d8_result : d9_result;
}

void SubmitControllerWordPair(u16 value, u16 selector)
{
    ++submit_count;
    submitted_value = value;
    submitted_selector = selector;
}

void WriteControllerParameterFBValueWithBit0Set(u8 value)
{
    ++fb_count;
    fb_value = value;
}

static void WriteU16(fw_addr_t address, u16 value)
{
    memory[address] = (u8)(value >> 8);
    memory[address + 1u] = (u8)value;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    memset(memory, 0, sizeof(memory));
    d8_result = d9_result = 0u;
    drain_count = submit_count = fb_count = 0u;
    submitted_value = submitted_selector = 0u;
    fb_value = 0u;
}

static void TestSubmissionAndWindowAverages(void)
{
    ResetState();
    memory[0x0015c9ul] = 14u;
    memory[0x00f848c7ul + 14u] = 3u;
    WriteU16(0x0015cbul, 96u);
    WriteU16(0x0015cful, 96u);
    d8_result = 64u;
    d9_result = 32u;

    ServiceControllerWord0251SweepSampleTick();
    assert(submit_count == 1u && submitted_value == 0x0120u);
    assert(submitted_selector == 0x0251u && drain_count == 2u);
    assert(ReadU16(0x0015cbul) == 10u);
    assert(ReadU16(0x0015cful) == 8u);
    assert(ReadU16(0x0015d3ul) == 9u);
    assert(memory[0x0015c9ul] == 15u);

    memory[0x0015c9ul] = 38u;
    WriteU16(0x0015cdul, 144u);
    WriteU16(0x0015d1ul, 112u);
    d8_result = d9_result = 16u;
    ServiceControllerWord0251SweepSampleTick();
    assert(ReadU16(0x0015cdul) == 10u);
    assert(ReadU16(0x0015d1ul) == 8u);
    assert(ReadU16(0x0015d5ul) == 9u);
}

static void SeedCompletedWindows(void)
{
    memory[0x0015c9ul] = 39u;
    WriteU16(0x0015cbul, 0x0100u);
    WriteU16(0x0015cdul, 1u);
    WriteU16(0x0015cful, 20u);
    WriteU16(0x0015d1ul, 50u);
    WriteU16(0x0015d3ul, 10u);
    WriteU16(0x0015d5ul, 14u);
}

static void TestBaselineAndIndexedPassPublication(void)
{
    ResetState();
    SeedCompletedWindows();
    memory[0x0015caul] = 1u;
    ServiceControllerWord0251SweepSampleTick();
    assert(memory[0x00f9b7ul] == 0xffu);
    assert(memory[0x00f9cbul] == 30u);
    assert(ReadU16(0x00100aul) == 12u);
    assert(ReadU16(0x0015d3ul) == 12u);
    assert(memory[0x00fc16ul] == 0xb8u);
    assert(fb_count == 1u && fb_value == 0xb8u);

    ResetState();
    SeedCompletedWindows();
    memory[0x0015caul] = 2u;
    memory[0x00fc16ul] = 0x58u;
    ServiceControllerWord0251SweepSampleTick();
    assert(memory[0x00f9b8ul] == 0xffu);
    assert(memory[0x00f9ccul] == 30u);
    assert(ReadU16(0x00100cul) == 12u);
    assert(memory[0x00fc16ul] == 0x50u);
    assert(fb_count == 1u && fb_value == 0x50u);

    ResetState();
    SeedCompletedWindows();
    memory[0x0015caul] = 2u;
    memory[0x00fc16ul] = 0x50u;
    memory[0x001301ul] = 0xffu;
    ServiceControllerWord0251SweepSampleTick();
    assert(memory[0x001301ul] == 0xfbu);
    assert(memory[0x00fc16ul] == 0x50u);
}

static void TestPassZeroAndPhaseWrap(void)
{
    ResetState();
    SeedCompletedWindows();
    ServiceControllerWord0251SweepSampleTick();
    assert(fb_count == 0u && memory[0x0015c9ul] == 40u);

    memory[0x0015c9ul] = 47u;
    memory[0x0015caul] = 0xffu;
    WriteU16(0x0015cbul, 1u);
    WriteU16(0x0015cdul, 2u);
    WriteU16(0x0015cful, 3u);
    WriteU16(0x0015d1ul, 4u);
    WriteU16(0x0015d3ul, 5u);
    WriteU16(0x0015d5ul, 6u);
    ServiceControllerWord0251SweepSampleTick();
    assert(memory[0x0015c9ul] == 0u && memory[0x0015caul] == 0u);
    assert(ReadU16(0x0015cbul) == 0u && ReadU16(0x0015cdul) == 0u);
    assert(ReadU16(0x0015cful) == 0u && ReadU16(0x0015d1ul) == 0u);
    assert(ReadU16(0x0015d3ul) == 0u && ReadU16(0x0015d5ul) == 0u);
}

int main(void)
{
    TestSubmissionAndWindowAverages();
    TestBaselineAndIndexedPassPublication();
    TestPassZeroAndPhaseWrap();
    return 0;
}
