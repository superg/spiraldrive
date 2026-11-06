#include <assert.h>
#include <string.h>
#include "platform/controller_position_average.h"
#include "platform/firmware_memory.h"

static u8 memory[1u << 24];
static u8 ready_values[32];
static u16 sample_values[10];
static s16 word_values[16];
static unsigned ready_count, ready_index, sample_index, sample_byte;
static unsigned word_count, word_index, write_count, yield_count;
static unsigned signed_divide_count, unsigned_divide_count;
static fw_addr_t write_addresses[10];
static u8 write_values[10];
static s32 signed_dividend, signed_divisor;
static u32 unsigned_dividend, unsigned_divisor;

u8 FirmwareRead8(fw_addr_t address)
{
    if(address == 0x0180ccul)
    {
        assert(ready_index < ready_count);
        return ready_values[ready_index++];
    }
    return memory[address & 0xfffffful];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

void WaitControllerPortReadyAndWriteByte(fw_addr_t address, u8 value)
{
    write_addresses[write_count] = address;
    write_values[write_count++] = value;
}

u8 WaitControllerPortReadyAndReadByte(fw_addr_t address)
{
    u16 sample = sample_values[sample_index];

    if(sample_byte == 0u)
    {
        assert(address == 0x0180cful);
        sample_byte = 1u;
        return (u8)(sample >> 8);
    }
    assert(address == 0x0180ceul);
    sample_byte = 0u;
    ++sample_index;
    return (u8)sample;
}

u16 ReadControllerWord(u16 selector)
{
    assert(selector == 0x0233u);
    assert(word_index < word_count);
    return (u16)word_values[word_index++];
}

void YieldCoroutine1(void)
{
    ++yield_count;
}

s32 DivideS32Quotient(s32 dividend, s32 divisor)
{
    ++signed_divide_count;
    signed_dividend = dividend;
    signed_divisor = divisor;
    return dividend / divisor;
}

u32 DivideU32Quotient(u32 dividend, u32 divisor)
{
    ++unsigned_divide_count;
    unsigned_dividend = dividend;
    unsigned_divisor = divisor;
    return dividend / divisor;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static void ResetState(void)
{
    unsigned index;

    memset(memory, 0, sizeof(memory));
    memset(ready_values, 0, sizeof(ready_values));
    memset(word_values, 0, sizeof(word_values));
    for(index = 0u; index < 10u; ++index)
        sample_values[index] = (u16)(0x1000u + index);
    ready_count = 10u;
    ready_index = sample_index = sample_byte = 0u;
    word_count = word_index = write_count = yield_count = 0u;
    signed_divide_count = unsigned_divide_count = 0u;
    signed_dividend = signed_divisor = 0l;
    unsigned_dividend = unsigned_divisor = 0ul;
}

static void AssertCommonSequence(void)
{
    unsigned index;

    assert(write_count == 10u);
    assert(ready_index == ready_count);
    assert(sample_index == 10u && sample_byte == 0u);
    assert(yield_count == 10u);
    assert(unsigned_divide_count == 1u);
    assert(unsigned_dividend == 0xa02dul);
    assert(unsigned_divisor == 10ul);
    for(index = 0u; index < 10u; ++index)
    {
        assert(write_addresses[index] == 0x0180ccul);
        assert(write_values[index] == 0x12u);
    }
}

static void TestAverageWithoutVariation(void)
{
    ResetState();
    assert(MeasureAverageControllerPositionWord() == 0x1004u);
    AssertCommonSequence();
    assert(word_index == 0u);
    assert(signed_divide_count == 0u);
    assert(ReadU16(0x00fc30ul) == 0u);
}

static void TestLateSampleVariationAndThreshold(void)
{
    static const s16 words[10] = { 100, 180, -100, -20, 32760, 32767, -32768, -32600, 0, 100 };
    unsigned ready = 0u;
    unsigned index;

    ResetState();
    memory[0x00fc32ul] = 1u;
    for(index = 0u; index < 5u; ++index)
        ready_values[ready++] = 0u;
    for(index = 0u; index < 5u; ++index)
    {
        ready_values[ready++] = 0x10u;
        ready_values[ready++] = 0x10u;
        ready_values[ready++] = 0u;
    }
    ready_count = ready;
    memcpy(word_values, words, sizeof(words));
    word_count = 10u;

    assert(MeasureAverageControllerPositionWord() == 0x1004u);
    AssertCommonSequence();
    assert(word_index == 10u);
    assert(signed_divide_count == 1u);
    assert(signed_dividend == 435l && signed_divisor == 5l);
    assert(ReadU16(0x00fc30ul) == 87u);
    assert((memory[0x00fc32ul] & 3u) == 3u);
}

static void TestVariationBelowThreshold(void)
{
    unsigned ready = 0u;
    unsigned index;

    ResetState();
    memory[0x00fc32ul] = 1u;
    for(index = 0u; index < 5u; ++index)
        ready_values[ready++] = 0u;
    for(index = 0u; index < 5u; ++index)
    {
        ready_values[ready++] = 0x10u;
        ready_values[ready++] = 0u;
        word_values[index] = 40;
    }
    ready_count = ready;
    word_count = 5u;

    assert(MeasureAverageControllerPositionWord() == 0x1004u);
    assert(ReadU16(0x00fc30ul) == 0u);
    assert((memory[0x00fc32ul] & 2u) == 0u);
}

int main(void)
{
    TestAverageWithoutVariation();
    TestLateSampleVariationAndThreshold();
    TestVariationBelowThreshold();
    return 0;
}
