#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_buffer_chunk_fill.h"
#include "platform/firmware_memory.h"

typedef struct WriteRecord
{
    fw_addr_t address;
    u8 value;
} WriteRecord;

static u8 firmware_memory[0x1000000ul];
static WriteRecord writes[64];
static unsigned write_count;
static unsigned completion_zero_reads;
static unsigned completion_read_count;
static u8 mutate_shadow_at_completion;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < sizeof(firmware_memory));
    if(address == 0x0014daul)
    {
        ++completion_read_count;
        if(completion_read_count <= completion_zero_reads)
            return 0u;
        if(mutate_shadow_at_completion != 0u)
            firmware_memory[0x0fadu] = 0xffu;
        return 1u;
    }
    return firmware_memory[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < sizeof(firmware_memory));
    assert(write_count < sizeof(writes) / sizeof(writes[0]));
    firmware_memory[address] = value;
    writes[write_count].address = address;
    writes[write_count].value = value;
    ++write_count;
}

static void ResetFixture(void)
{
    memset(firmware_memory, 0, sizeof(firmware_memory));
    memset(writes, 0, sizeof(writes));
    write_count = 0u;
    completion_zero_reads = 0u;
    completion_read_count = 0u;
    mutate_shadow_at_completion = 0u;
}

static void AssertWrite(unsigned index, fw_addr_t address, u8 value)
{
    assert(index < write_count);
    assert(writes[index].address == address);
    assert(writes[index].value == value);
}

static void TestRepeatedProgrammingAndSpecialCommand(void)
{
    unsigned repetition;

    ResetFixture();
    firmware_memory[0x0fadu] = 0x15u;
    firmware_memory[0x0ca0u] = 1u;
    completion_zero_reads = 3u;
    mutate_shadow_at_completion = 1u;

    FillControllerBufferChunkAndWait(0x00a1b2c3ul, 0x1234u, 0x5au);

    assert(write_count == 39u);
    AssertWrite(0u, 0x018020ul, 0xe5u);
    for(repetition = 0u; repetition < 5u; ++repetition)
    {
        unsigned base = 1u + repetition * 3u;

        AssertWrite(base, 0x018010ul, 0xc3u);
        AssertWrite(base + 1u, 0x018011ul, 0xb2u);
        AssertWrite(base + 2u, 0x018012ul, 0xa1u);
    }
    for(repetition = 0u; repetition < 5u; ++repetition)
    {
        unsigned base = 16u + repetition * 2u;

        AssertWrite(base, 0x01800dul, 0x34u);
        AssertWrite(base + 1u, 0x01800eul, 0x12u);
    }
    AssertWrite(26u, 0x018020ul, 0xe0u);
    AssertWrite(27u, 0x01801ful, 0x80u);
    AssertWrite(28u, 0x018029ul, 0x5au);
    AssertWrite(29u, 0x018010ul, 0xc3u);
    AssertWrite(30u, 0x018011ul, 0xb2u);
    AssertWrite(31u, 0x018012ul, 0xa1u);
    AssertWrite(32u, 0x01800dul, 0x34u);
    AssertWrite(33u, 0x01800eul, 0x12u);
    AssertWrite(34u, 0x000fadul, 0x55u);
    AssertWrite(35u, 0x018003ul, 0x55u);
    AssertWrite(36u, 0x018022ul, 0x88u);
    AssertWrite(37u, 0x000fadul, 0xbfu);
    AssertWrite(38u, 0x018003ul, 0xbfu);
    assert(completion_read_count == 4u);
}

static void TestDefaultCommandAndImmediateCompletion(void)
{
    ResetFixture();
    firmware_memory[0x0fadu] = 0xc3u;
    firmware_memory[0x0ca0u] = 2u;

    FillControllerBufferChunkAndWait(0x00123456ul, 0x8000u, 0xa5u);

    assert(write_count == 39u);
    AssertWrite(36u, 0x018022ul, 0x80u);
    AssertWrite(34u, 0x000fadul, 0xc3u);
    AssertWrite(37u, 0x000fadul, 0x83u);
    AssertWrite(38u, 0x018003ul, 0x83u);
    assert(completion_read_count == 1u);
}

int main(void)
{
    TestRepeatedProgrammingAndSpecialCommand();
    TestDefaultCommandAndImmediateCompletion();
    puts("controller-buffer chunk fill tests passed");
    return 0;
}
