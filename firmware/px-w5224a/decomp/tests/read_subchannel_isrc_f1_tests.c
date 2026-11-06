#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_subchannel_isrc_f1.h"

static u8 memory[0x1000000ul];

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

static void FillIdentification(void)
{
    memory[0x000f51ful] = 0x12u;
    memory[0x000f520ul] = 0x34u;
    memory[0x000f521ul] = 0x56u;
    memory[0x000f522ul] = 0x78u;
    memory[0x000f523ul] = 0x9au;
    memory[0x000f524ul] = 0xbcu;
    memory[0x000f525ul] = 0xdeu;
    memory[0x000f526ul] = 0xf1u;
}

static void CheckUnavailableAndLow16Wrap(void)
{
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    memory[0x001259ul] = 0u;
    BuildReadSubChannelIsrcOrF1PayloadC(3u, 0x12fff8ul);
    for(index = 0u; index < 8u; ++index)
    {
        assert(memory[0x12fff8ul + index] == 0u);
        assert(memory[0x120000ul + index] == 0u);
    }
    assert(memory[0x130000ul] == 0xa5u);
}

static void CheckStandardIsrcPayload(void)
{
    static const u8 expected[16] = { 0x80u, '4', 'S', 'A', 'F', 'N', '9', ':', ';', '<', '=', '>', '?', 0u, 0u, 0u };

    memset(memory, 0, sizeof(memory));
    memory[0x001259ul] = 4u;
    FillIdentification();
    BuildReadSubChannelIsrcOrF1PayloadC(3u, 0x002000ul);
    assert(memcmp(&memory[0x002000ul], expected, sizeof(expected)) == 0);
}

static void CheckVendorF1Payload(void)
{
    static const u8 expected[16] = { 0x80u, '4', 'S', 'A', 'F', 'N', '9', ':', 0x0bu, 0xcdu, 0xefu, 0u, 0u, 0u, 0u, 0u };

    memset(memory, 0, sizeof(memory));
    memory[0x001259ul] = 4u;
    FillIdentification();
    BuildReadSubChannelIsrcOrF1PayloadC(0xf1u, 0x003000ul);
    assert(memcmp(&memory[0x003000ul], expected, sizeof(expected)) == 0);
}

int main(void)
{
    CheckUnavailableAndLow16Wrap();
    CheckStandardIsrcPayload();
    CheckVendorF1Payload();
    puts("read_subchannel_isrc_f1_tests: ok");
    return 0;
}
