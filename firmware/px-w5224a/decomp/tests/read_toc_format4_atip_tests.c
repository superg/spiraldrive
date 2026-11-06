#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/read_toc_format4_atip.h"

static u8 memory[0x1000000ul];
static u8 rejected;
static u8 transfer_result;
static u8 no_sense;
static fw_addr_t transfer_source;
static u32 transfer_count;
static u16 min_value;
static u16 min_limit;

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

void ObserveRejectReadTocFormat4Command(void)
{
    rejected = 1u;
}

u16 ObserveReadTocFormat4MinU16(u16 value, u16 limit)
{
    min_value = value;
    min_limit = limit;
    return value < limit ? value : limit;
}

u8 ObserveTransferReadTocFormat4Response(fw_addr_t source_address, u32 byte_count)
{
    transfer_source = source_address;
    transfer_count = byte_count;
    return transfer_result;
}

void ObserveSetReadTocFormat4NoSense(void)
{
    no_sense = 1u;
}

static void Reset(void)
{
    memset(memory, 0xa5, sizeof(memory));
    rejected = 0u;
    transfer_result = 4u;
    no_sense = 0u;
    transfer_source = 0u;
    transfer_count = 0u;
    min_value = 0u;
    min_limit = 0u;
    memory[0x000b10ul] = 0u;
    memory[0x000b11ul] = 0u;
    memory[0x000b12ul] = 0xffu;
    memory[0x001008ul] = 1u;
    memory[0x00dd79ul] = 0u;
}

static void CheckReject(void)
{
    Reset();
    memory[0x000b10ul] = 1u;
    HandleReadTocFormat4Atip();
    assert(rejected == 1u && transfer_count == 0u);
}

static void CheckUnavailable(void)
{
    Reset();
    memory[0x001008ul] = 0u;
    memory[0x000b12ul] = 3u;
    transfer_result = 5u;
    HandleReadTocFormat4Atip();
    assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 2u);
    assert(memory[0x000b2cul] == 0u && memory[0x000b2dul] == 0u);
    assert(min_value == 3u && min_limit == 4u);
    assert(transfer_source == 0x000b2aul && transfer_count == 3u);
    assert(no_sense == 1u);

    Reset();
    memory[0x00dd79ul] = 1u;
    memory[0x000c88ul] = 2u;
    HandleReadTocFormat4Atip();
    assert(min_limit == 4u && transfer_count == 4u);
}

static void FillBaseFields(void)
{
    memory[0x00fb5bul] = 0u;
    memory[0x00fb7aul] = 1u;
    memory[0x00fb7bul] = 2u;
    memory[0x00fb7cul] = 0xffu;
    memory[0x00fb7dul] = 8u;
    memory[0x00fb7eul] = 0u;
    memory[0x00113ful] = 0x11u;
    memory[0x001140ul] = 0x22u;
    memory[0x001141ul] = 0x33u;
    memory[0x00113bul] = 0x44u;
    memory[0x00113cul] = 0x55u;
    memory[0x00113dul] = 0x66u;
}

static void CheckBaseResponse(void)
{
    unsigned address;

    Reset();
    FillBaseFields();
    HandleReadTocFormat4Atip();
    assert(memory[0x000b2aul] == 0u && memory[0x000b2bul] == 0x1au);
    assert(memory[0x000b2cul] == 0u && memory[0x000b2dul] == 0u);
    assert(memory[0x000b2eul] == 0x83u);
    assert(memory[0x000b2ful] == 0x40u);
    assert(memory[0x000b30ul] == 0x88u);
    assert(memory[0x000b32ul] == 0x11u && memory[0x000b34ul] == 0x33u);
    assert(memory[0x000b36ul] == 0x44u && memory[0x000b38ul] == 0x66u);
    for(address = 0x000b39u; address <= 0x000b45u; ++address)
        assert(memory[address] == 0u);
    assert(memory[0x000b31ul] == 0u && memory[0x000b35ul] == 0u);
    assert(min_value == 0xffu && min_limit == 0x1cu);
    assert(transfer_count == 0x1cu && no_sense == 0u);
}

static void CheckAllPackedGroups(void)
{
    Reset();
    FillBaseFields();
    memory[0x00fb7eul] = 7u;
    memory[0x00fb7ful] = 1u;
    memory[0x00fb80ul] = 0x10u;
    memory[0x00fb81ul] = 2u;
    memory[0x00fb82ul] = 0x20u;
    memory[0x00fb83ul] = 4u;
    memory[0x00fb84ul] = 8u;
    memory[0x00fb85ul] = 0xabu;
    memory[0x00fb86ul] = 0x5du;
    memory[0x00fb87ul] = 0x36u;
    memory[0x00fb89ul] = 0xb6u;
    memory[0x00fb8aul] = 0x55u;
    memory[0x00fb8bul] = 0x9du;
    HandleReadTocFormat4Atip();
    assert(memory[0x000b30ul] == 0x8fu);
    assert(memory[0x000b3aul] == 0x11u);
    assert(memory[0x000b3bul] == 0x22u);
    assert(memory[0x000b3cul] == 0x8cu);
    assert(memory[0x000b3eul] == 0x35u);
    assert(memory[0x000b3ful] == 0xd6u);
    assert(memory[0x000b40ul] == 0x6cu);
    assert(memory[0x000b42ul] == 0x4du);
    assert(memory[0x000b43ul] == 0xd5u);
    assert(memory[0x000b44ul] == 0xe9u);
}

static void CheckFallbackEnableBits(void)
{
    Reset();
    FillBaseFields();
    memory[0x00fb5bul] = 0x1cu;
    HandleReadTocFormat4Atip();
    assert((memory[0x000b30ul] & 7u) == 7u);
}

int main(void)
{
    CheckReject();
    CheckUnavailable();
    CheckBaseResponse();
    CheckAllPackedGroups();
    CheckFallbackEnableBits();
    puts("read_toc_format4_atip_tests: ok");
    return 0;
}
