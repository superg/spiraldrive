#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_page_reconciliation.h"
#include "platform/firmware_memory.h"

struct FindCall
{
    u32 key;
    u8 mode;
};

static u8 memory[0x300000ul];
static u32 find_results[8];
static struct FindCall find_calls[8];
static unsigned find_result_count;
static unsigned find_count;
static unsigned compact_count;
static u8 compact_argument;
static unsigned clear_count;
static u16 clear_group;
static u16 clear_page;
static unsigned copy_from_count;
static unsigned copy_to_count;

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

void ObserveRecordPageHeaderCompact(u8 include_type_zero_and_one)
{
    ++compact_count;
    compact_argument = include_type_zero_and_one;
}

void ObserveRecordPageHeaderCopyFrom(fw_addr_t source_base, fw_addr_t destination, u8 byte_count)
{
    u8 index;

    ++copy_from_count;
    for(index = 0u; index < byte_count; ++index)
        memory[destination + index] = memory[source_base + 0x100000ul + index];
}

void ObserveRecordPageHeaderCopyTo(fw_addr_t destination_base, fw_addr_t source, u8 byte_count)
{
    u8 index;

    ++copy_to_count;
    for(index = 0u; index < byte_count; ++index)
        memory[destination_base + 0x100000ul + index] = memory[source + index];
}

u32 ObserveRecordPageHeaderFind(u32 address_key, u8 search_mode)
{
    assert(find_count < find_result_count);
    find_calls[find_count].key = address_key;
    find_calls[find_count].mode = search_mode;
    return find_results[find_count++];
}

void ObserveRecordPageHeaderClearPresence(u16 group_index, u16 page_index)
{
    ++clear_count;
    clear_group = group_index;
    clear_page = page_index;
}

static u16 ReadU16(fw_addr_t address)
{
    return (u16)(((u16)memory[address] << 8) | memory[address + 1u]);
}

static u32 ReadU32(fw_addr_t address)
{
    return ((u32)memory[address] << 24) | ((u32)memory[address + 1u] << 16) | ((u32)memory[address + 2u] << 8) | memory[address + 3u];
}

static u32 ControllerPage(u8 group, u8 page)
{
    return 0x094000ul + (u32)group * 0x4000ul + (u16)page * 0x0800u;
}

static void SetBytes(fw_addr_t address, u8 a, u8 b, u8 c)
{
    memory[address] = a;
    memory[address + 1u] = b;
    memory[address + 2u] = c;
}

static void ResetFixture(u8 group, u8 page)
{
    u32 controller_page = ControllerPage(group, page);
    unsigned index;

    memset(memory, 0xa5, sizeof(memory));
    for(index = 0u; index < 0x20u; ++index)
        memory[controller_page + 0x100000ul + index] = (u8)(0x80u + index);
    find_result_count = 0u;
    find_count = 0u;
    compact_count = 0u;
    compact_argument = 0u;
    clear_count = 0u;
    clear_group = 0u;
    clear_page = 0u;
    copy_from_count = 0u;
    copy_to_count = 0u;
}

static void AssertFindModes(const u8 *modes, unsigned count)
{
    unsigned index;

    assert(find_count == count);
    for(index = 0u; index < count; ++index)
    {
        assert(find_calls[index].key == 0ul);
        assert(find_calls[index].mode == modes[index]);
    }
}

static void TestBothRecordTypesRebuildAllHeaderFields(void)
{
    static const u8 modes[] = { 0x20u, 0x31u, 0x20u };
    u32 controller_page = ControllerPage(2u, 3u);
    u32 record_start = controller_page + 0x20ul;
    u32 type20 = record_start + 0x30ul;
    u32 type31 = record_start + 0x90ul;

    ResetFixture(2u, 3u);
    find_results[0] = type20;
    find_results[1] = type31;
    find_results[2] = type20;
    find_result_count = 3u;
    SetBytes(record_start + 0x100000ul, 0xabu, 0xcdu, 0xefu);
    SetBytes(type20 + 3u + 0x100000ul, 1u, 2u, 3u);
    SetBytes(type31 - 3u + 0x100000ul, 4u, 5u, 6u);
    SetBytes(type20 - 6u + 0x100000ul, 7u, 8u, 9u);

    RebuildControllerBufferRecordPageHeader(2u, 3u);

    assert(compact_count == 1u && compact_argument == 1u);
    assert(ReadU32(0x2ba51cu) == record_start);
    assert(ReadU32(0x2b9710u) == type20);
    assert(ReadU16(0xfa12u) == 0x50u);
    assert(memcmp(memory + 0xfa21u, (u8[]){ 1u, 2u, 3u }, 3u) == 0);
    assert(memcmp(memory + 0xfa25u, (u8[]){ 4u, 5u, 6u }, 3u) == 0);
    assert(memory[0xfa19u] == 0x0bu && memory[0xfa1au] == 0xcdu && memory[0xfa1bu] == 0xefu);
    assert(memcmp(memory + 0xfa1du, (u8[]){ 7u, 8u, 9u }, 3u) == 0);
    assert(clear_count == 0u);
    assert(copy_from_count == 5u && copy_to_count == 1u);
    assert(memcmp(memory + controller_page + 0x100000ul, memory + 0xfa08u, 0x20u) == 0);
    AssertFindModes(modes, 3u);
}

static void TestMissingType20ClearsPresenceAndUsesEndFallback(void)
{
    static const u8 modes[] = { 0x20u, 0x20u, 0x31u };
    u32 controller_page = ControllerPage(4u, 5u);
    u32 record_start = controller_page + 0x20ul;

    ResetFixture(4u, 5u);
    find_results[0] = 0ul;
    find_results[1] = 0ul;
    find_results[2] = 0ul;
    find_result_count = 3u;
    SetBytes(record_start + 0x100000ul, 0x2eu, 0x22u, 0x33u);
    SetBytes(record_start + 0x5faul + 0x100000ul, 0x44u, 0x55u, 0x66u);

    RebuildControllerBufferRecordPageHeader(4u, 5u);

    assert(ReadU16(0xfa12u) == 0u);
    assert(memory[0xfa21u] == 0xffu && memory[0xfa22u] == 0xffu && memory[0xfa23u] == 0xffu);
    assert(memory[0xfa25u] == 0xffu && memory[0xfa26u] == 0xffu && memory[0xfa27u] == 0xffu);
    assert(clear_count == 1u && clear_group == 4u && clear_page == 5u);
    assert(memory[0xfa19u] == 0x0eu && memory[0xfa1au] == 0x22u && memory[0xfa1bu] == 0x33u);
    assert(memcmp(memory + 0xfa1du, (u8[]){ 0x44u, 0x55u, 0x66u }, 3u) == 0);
    AssertFindModes(modes, 3u);
}

static void TestType31FallbacksSupplyBothTrailingTriplets(void)
{
    static const u8 modes[] = { 0x20u, 0x31u, 0x20u, 0x31u };
    u32 controller_page = ControllerPage(1u, 6u);
    u32 record_start = controller_page + 0x20ul;
    u32 type20 = record_start + 0x18ul;
    u32 type31 = record_start + 0x60ul;

    ResetFixture(1u, 6u);
    find_results[0] = type20;
    find_results[1] = 0ul;
    find_results[2] = 0ul;
    find_results[3] = type31;
    find_result_count = 4u;
    SetBytes(type20 + 3u + 0x100000ul, 1u, 1u, 1u);
    SetBytes(record_start + 0x5fdul + 0x100000ul, 2u, 2u, 2u);
    SetBytes(type31 - 6u + 0x100000ul, 3u, 3u, 3u);

    RebuildControllerBufferRecordPageHeader(1u, 6u);

    assert(memcmp(memory + 0xfa25u, (u8[]){ 2u, 2u, 2u }, 3u) == 0);
    assert(memcmp(memory + 0xfa1du, (u8[]){ 3u, 3u, 3u }, 3u) == 0);
    AssertFindModes(modes, 4u);
}

int main(void)
{
    TestBothRecordTypesRebuildAllHeaderFields();
    TestMissingType20ClearsPresenceAndUsesEndFallback();
    TestType31FallbacksSupplyBothTrailingTriplets();
    puts("controller-record page header tests passed");
    return 0;
}
