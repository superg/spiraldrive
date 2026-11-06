#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "media/rid01_identity_block.h"
#include "platform/firmware_memory.h"

struct ConfigurationRead
{
    fw_addr_t offset;
    u16 count;
};

static u8 memory[0x220000ul];
static u8 template_bytes[0x60];
static u8 configuration_bytes[5];
static u8 configuration_status;
static struct ConfigurationRead reads[2];
static unsigned read_count;
static unsigned firmware_copy_count;
static unsigned patch_copy_count;

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

u8 DecodeAsciiHexNibbleOrZero(u8 character)
{
    if(character >= '0' && character <= '9')
        return (u8)(character - '0');
    if(character >= 'A' && character <= 'F')
        return (u8)(character - ('A' - 10));
    if(character >= 'a' && character <= 'f')
        return (u8)(character - ('a' - 10));
    return 0u;
}

void ObserveRid01CopyFirmwareBytes(fw_addr_t destination_base, fw_addr_t source, u8 byte_count)
{
    const u8 *source_bytes;

    ++firmware_copy_count;
    if(source == 0xff9269ul)
    {
        assert(byte_count == 0x60u);
        source_bytes = template_bytes;
    }
    else
    {
        assert(source == 0x00dcd5ul && byte_count == 0x10u);
        source_bytes = memory + source;
    }
    memcpy(memory + destination_base + 0x100000ul, source_bytes, byte_count);
}

void ObserveRid01CopyPatchBytes(fw_addr_t destination_base, const u8 *source, u8 byte_count)
{
    ++patch_copy_count;
    assert(byte_count == 3u);
    memcpy(memory + destination_base + 0x100000ul, source, byte_count);
}

void ObserveRid01ReadConfiguration(fw_addr_t offset, u8 *destination, u16 byte_count)
{
    assert(read_count < 2u);
    reads[read_count].offset = offset;
    reads[read_count].count = byte_count;
    ++read_count;
    if(offset == 0x1aul)
    {
        assert(byte_count == 5u);
        memcpy(destination, configuration_bytes, 5u);
    }
    else
    {
        assert(offset == 0x41ul && byte_count == 1u);
        *destination = configuration_status;
    }
}

static void ResetFixture(void)
{
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    for(index = 0u; index < sizeof(template_bytes); ++index)
        template_bytes[index] = (u8)index;
    for(index = 0u; index < 0x10u; ++index)
        memory[0xdcd5u + index] = (u8)(0xe0u + index);
    memcpy(configuration_bytes, "12345", 5u);
    configuration_status = 0u;
    memset(reads, 0, sizeof(reads));
    read_count = 0u;
    firmware_copy_count = 0u;
    patch_copy_count = 0u;
}

static void AssertCommonLayout(fw_addr_t controller_address)
{
    fw_addr_t block = controller_address + 0x100000ul;

    assert(firmware_copy_count == 2u && patch_copy_count == 1u);
    assert(read_count == 2u);
    assert(reads[0].offset == 0x1aul && reads[0].count == 5u);
    assert(reads[1].offset == 0x41ul && reads[1].count == 1u);
    assert(memcmp(memory + block, template_bytes, 0x18u) == 0);
    assert(memcmp(memory + block + 0x1bu, template_bytes + 0x1bu, 0x25u) == 0);
    assert(memcmp(memory + block + 0x40u, memory + 0xdcd5u, 0x10u) == 0);
    assert(memcmp(memory + block + 0x50u, template_bytes + 0x50u, 0x10u) == 0);
}

static void TestZeroStatusBuildsDerivedPatch(void)
{
    fw_addr_t controller_address = 0x4000ul;
    fw_addr_t block = controller_address + 0x100000ul;

    ResetFixture();
    BuildRid01RecorderIdentificationBlockAtControllerAddress(controller_address);

    AssertCommonLayout(controller_address);
    assert(memory[block + 0x18u] == 0x0cu);
    assert(memory[block + 0x19u] == 0x63u);
    assert(memory[block + 0x1au] == 0x45u);
}

static void TestNonzeroStatusKeepsNibblePackedPatch(void)
{
    fw_addr_t controller_address = 0x8000ul;
    fw_addr_t block = controller_address + 0x100000ul;

    ResetFixture();
    configuration_status = 0xffu;
    memcpy(configuration_bytes, "9AbFx", 5u);
    BuildRid01RecorderIdentificationBlockAtControllerAddress(controller_address);

    AssertCommonLayout(controller_address);
    assert(memory[block + 0x18u] == 0x09u);
    assert(memory[block + 0x19u] == 0xabu);
    assert(memory[block + 0x1au] == 0xf0u);
}

int main(void)
{
    TestZeroStatusBuildsDerivedPatch();
    TestNonzeroStatusKeepsNibblePackedPatch();
    puts("RID01 identity-block tests passed");
    return 0;
}
