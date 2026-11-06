#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform/controller_record_workspace_page.h"
#include "platform/firmware_memory.h"

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

static void ResetFixture(void)
{
    memset(memory, 0, sizeof(memory));
    memset(memory + 0x2ba62fu, 0xa5, 0x0801u);
}

static void SetTableRecord(u8 segment_index, u8 byte0, u8 byte1, u8 byte2, u8 byte3)
{
    fw_addr_t address = 0xf84590ul + (fw_addr_t)segment_index * 4u;

    memory[address] = byte0;
    memory[address + 1u] = byte1;
    memory[address + 2u] = byte2;
    memory[address + 3u] = byte3;
}

static void AssertValidRecords(u8 expected_byte1, u8 expected_byte2)
{
    unsigned index;

    for(index = 0u; index < 0x100u; ++index)
    {
        fw_addr_t record = 0x2ba64ful + (fw_addr_t)index * 6u;

        assert(memory[record] == 0x20u);
        assert(memory[record + 1u] == 0u);
        assert(memory[record + 2u] == 0u);
        assert(memory[record + 3u] == expected_byte1);
        assert(memory[record + 4u] == expected_byte2);
        assert(memory[record + 5u] == 0u);
    }
}

static void TestValidPageLayout(void)
{
    ResetFixture();
    memory[0x2ba588u] = 11u;
    SetTableRecord(10u, 0x11u, 0x22u, 0x33u, 0x44u);

    BuildControllerRecordWorkspacePage(1u, 2u);

    assert(memory[0x2ba62fu] == 0x4du);
    assert(memory[0x2ba630u] == 0x44u);
    assert(memory[0x2ba631u] == 0x54u);
    assert(memory[0x2ba632u] == 0x12u);
    assert(memory[0x2ba637u] == 1u);
    assert(memory[0x2ba638u] == 0u);
    assert(memory[0x2ba639u] == 0u);
    assert(memory[0x2ba63au] == 0x20u);
    assert(memory[0x2ba63eu] == 0xffu);
    assert(memory[0x2ba640u] == 0xffu);
    assert(memory[0x2ba641u] == 0xffu);
    assert(memory[0x2ba642u] == 0xffu);
    assert(memory[0x2ba644u] == 0xffu);
    assert(memory[0x2ba645u] == 0xffu);
    assert(memory[0x2ba646u] == 0xffu);
    assert(memory[0x2ba648u] == 0x22u);
    assert(memory[0x2ba649u] == 0x33u);
    assert(memory[0x2ba64au] == 0u);
    assert(memory[0x2ba64cu] == 0x22u);
    assert(memory[0x2ba64du] == 0x33u);
    assert(memory[0x2ba64eu] == 0xffu);
    AssertValidRecords(0x22u, 0x33u);
    assert(memory[0x2bae2eu] == 0u);
    assert(memory[0x2bae2fu] == 0xa5u);
}

static void TestInvalidPageSummary(void)
{
    static const fw_addr_t summary_addresses[6] = { 0x2ba648ul, 0x2ba649ul, 0x2ba64aul, 0x2ba64cul, 0x2ba64dul, 0x2ba64eul };
    unsigned index;

    ResetFixture();
    memory[0x2ba588u] = 10u;
    BuildControllerRecordWorkspacePage(1u, 2u);

    assert(memory[0x2ba637u] == 0u);
    for(index = 0u; index < sizeof(summary_addresses) / sizeof(summary_addresses[0]); ++index)
        assert(memory[summary_addresses[index]] == 0xffu);
    assert(memory[0x2ba64fu] == 0u);
    assert(memory[0x2bae2eu] == 0u);
    assert(memory[0x2bae2fu] == 0xa5u);
}

static void TestByteIndexWrapDiffersFromHeaderGate(void)
{
    ResetFixture();
    memory[0x2ba588u] = 1u;
    SetTableRecord(0u, 0x91u, 0x92u, 0x93u, 0x94u);

    BuildControllerRecordWorkspacePage(32u, 0u);

    assert(memory[0x2ba632u] == 0u);
    assert(memory[0x2ba637u] == 0u);
    assert(memory[0x2ba648u] == 0x92u);
    assert(memory[0x2ba649u] == 0x93u);
    AssertValidRecords(0x92u, 0x93u);
}

int main(void)
{
    TestValidPageLayout();
    TestInvalidPageSummary();
    TestByteIndexWrapDiffersFromHeaderGate();
    puts("controller record workspace page tests passed");
    return 0;
}
