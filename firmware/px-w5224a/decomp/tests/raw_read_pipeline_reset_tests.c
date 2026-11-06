#include <assert.h>
#include <string.h>
#include "media/raw_read_pipeline_reset.h"
#include "platform/firmware_memory.h"

static u8 memory[0x1000000ul];
static fw_addr_t writes[2300];
static unsigned write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
    writes[write_count++] = address;
}

static void TestCompleteReset(void)
{
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    memset(writes, 0, sizeof(writes));
    write_count = 0u;
    ResetRawReadPipelineBuffersAndState();

    for(index = 0u; index < 10u; ++index)
        assert(memory[0x001272ul + index] == 0u);
    for(index = 0u; index < 16u; ++index)
        assert(memory[0x001262ul + index] == 0u);
    for(index = 0u; index < 18u; ++index)
        assert(memory[0x00127cul + index] == (index == 5u ? 2u : 0u));
    for(index = 0u; index < 4u; ++index)
        assert(memory[0x008000ul + index] == 0u);
    for(index = 0u; index < 10u; ++index)
        assert(memory[0x008008ul + index] == 0u);
    for(index = 0u; index < 2200u; ++index)
        assert(memory[0x008012ul + index] == 0u);
    for(index = 0u; index < 16u; ++index)
        assert(memory[0x0088aaul + index] == 0u);
    assert(memory[0x0014ccul] == 0xbbu);
    assert(memory[0x001554ul] == 0u && memory[0x001555ul] == 0u);
    assert(memory[0x008004ul] == 0xa5u && memory[0x008006ul] == 0xa5u);

    assert(write_count == 2282u);
    assert(writes[45] == 0x008000ul && writes[46] == 0x008001ul && writes[47] == 0x008002ul && writes[48] == 0x008003ul);
    assert(writes[49] == 0x008000ul && writes[50] == 0x008001ul && writes[51] == 0x008002ul && writes[52] == 0x008003ul);
}

int main(void)
{
    TestCompleteReset();
    return 0;
}
