#include <assert.h>
#include <string.h>
#include "platform/firmware_memory.h"
#include "scsi/vendor_ec_record_list.h"

#define PRIMARY_TABLE 0x00ff8c54ul
#define ALTERNATE_TABLE 0x00ff8c86ul
#define RESPONSE 0x000b2aul

static u8 memory[0x1000000ul];

u8 FirmwareRead8(fw_addr_t address)
{
    return memory[address & 0xfffffful];
}
void FirmwareWrite8(fw_addr_t address, u8 value)
{
    memory[address & 0xfffffful] = value;
}

static void Reset(void)
{
    memset(memory, 0, sizeof(memory));
}

static void SetU32(fw_addr_t address, u32 value)
{
    memory[address] = (u8)(value >> 24);
    memory[address + 1u] = (u8)(value >> 16);
    memory[address + 2u] = (u8)(value >> 8);
    memory[address + 3u] = (u8)value;
}

static void SetRecord(fw_addr_t table, u8 index, u8 flags, u8 byte1, u8 byte2, u8 byte3, u8 byte4)
{
    fw_addr_t record = table + 5u + (u16)index * 5u;

    memory[record] = flags;
    memory[record + 1u] = byte1;
    memory[record + 2u] = byte2;
    memory[record + 3u] = byte3;
    memory[record + 4u] = byte4;
}

static u16 ReadResponseLengthField(void)
{
    return (u16)(((u16)memory[RESPONSE] << 8) | memory[RESPONSE + 1u]);
}

static void AssertPayload(u8 offset, u8 byte1, u8 byte2, u8 byte3, u8 byte4)
{
    assert(memory[RESPONSE + offset] == byte1);
    assert(memory[RESPONSE + offset + 1u] == byte2);
    assert(memory[RESPONSE + offset + 2u] == byte3);
    assert(memory[RESPONSE + offset + 3u] == byte4);
}

static void TestControllerActiveForcesAllPrimaryRecords(void)
{
    Reset();
    memory[0x000faeul] = 1u;
    memory[PRIMARY_TABLE] = 2u;
    SetRecord(PRIMARY_TABLE, 0u, 0u, 1u, 2u, 3u, 4u);
    SetRecord(PRIMARY_TABLE, 1u, 0u, 5u, 6u, 7u, 8u);
    assert(BuildVendorEcRecordListResponse(0u, 0u) == 12u);
    assert(ReadResponseLengthField() == 10u);
    assert(memory[RESPONSE + 2u] == 0u && memory[RESPONSE + 3u] == 1u);
    AssertPayload(4u, 1u, 2u, 3u, 4u);
    AssertPayload(8u, 5u, 6u, 7u, 8u);
}

static void TestUnknownStateForcesAllOnlyForPrimary(void)
{
    Reset();
    memory[PRIMARY_TABLE] = 1u;
    SetRecord(PRIMARY_TABLE, 0u, 0u, 1u, 2u, 3u, 4u);
    assert(BuildVendorEcRecordListResponse(0u, 0u) == 8u);
    assert(memory[RESPONSE + 2u] == 1u && memory[RESPONSE + 3u] == 1u);

    Reset();
    memory[ALTERNATE_TABLE] = 2u;
    SetRecord(ALTERNATE_TABLE, 0u, 0u, 1u, 0x80u, 3u, 4u);
    SetRecord(ALTERNATE_TABLE, 1u, 0u, 5u, 0u, 7u, 8u);
    assert(BuildVendorEcRecordListResponse(0u, 1u) == 8u);
    assert(memory[RESPONSE + 2u] == 1u && memory[RESPONSE + 3u] == 0u);
    AssertPayload(4u, 1u, 0x80u, 3u, 4u);
}

static void TestStateOneUsesRecordBit7AndTransformsType(void)
{
    Reset();
    memory[0x001008ul] = 1u;
    memory[PRIMARY_TABLE] = 2u;
    SetRecord(PRIMARY_TABLE, 0u, 0x80u, 0x11u, 0u, 0x33u, 0x44u);
    SetRecord(PRIMARY_TABLE, 1u, 0x40u, 0x55u, 0x80u, 0x77u, 0x88u);
    assert(BuildVendorEcRecordListResponse(0u, 0u) == 8u);
    assert(ReadResponseLengthField() == 6u);
    assert(memory[RESPONSE + 2u] == 2u && memory[RESPONSE + 3u] == 0u);
    AssertPayload(4u, 0x11u, 0x80u, 0x33u, 0x44u);
}

static void TestStateOneCapacityOverrideUsesRecordBit6(void)
{
    Reset();
    memory[0x001008ul] = 1u;
    memory[0x00fbecul] = 1u;
    SetU32(0x001153ul, 0x00030000ul);
    memory[PRIMARY_TABLE] = 2u;
    SetRecord(PRIMARY_TABLE, 0u, 0x40u, 0x11u, 0u, 0x33u, 0x44u);
    SetRecord(PRIMARY_TABLE, 1u, 0x80u, 0x55u, 0x80u, 0x77u, 0x88u);
    assert(BuildVendorEcRecordListResponse(0u, 0u) == 8u);
    AssertPayload(4u, 0x11u, 0x80u, 0x33u, 0x44u);
}

static void TestStateTwoAppliesMediaTypeMask(void)
{
    Reset();
    memory[0x001008ul] = 2u;
    memory[0x00fb87ul] = 0x40u;
    memory[0x00fb80ul] = 2u;
    memory[PRIMARY_TABLE] = 3u;
    SetRecord(PRIMARY_TABLE, 0u, 2u, 0x11u, 0x40u, 0x33u, 0x44u);
    SetRecord(PRIMARY_TABLE, 1u, 1u, 0x55u, 0x40u, 0x77u, 0x88u);
    SetRecord(PRIMARY_TABLE, 2u, 2u, 0x99u, 0u, 0xbbu, 0xccu);
    assert(BuildVendorEcRecordListResponse(0u, 0u) == 8u);
    assert(memory[RESPONSE + 2u] == 3u);
    AssertPayload(4u, 0x11u, 0x40u, 0x33u, 0x44u);
}

static void TestAlternateStateTwoUsesOnlyEligibilityMask(void)
{
    Reset();
    memory[0x001008ul] = 2u;
    memory[0x00fb80ul] = 2u;
    memory[ALTERNATE_TABLE] = 2u;
    SetRecord(ALTERNATE_TABLE, 0u, 0u, 0x11u, 0x40u, 0x33u, 0x44u);
    SetRecord(ALTERNATE_TABLE, 1u, 0xffu, 0x55u, 0u, 0x77u, 0x88u);
    assert(BuildVendorEcRecordListResponse(0u, 1u) == 8u);
    AssertPayload(4u, 0x11u, 0x40u, 0x33u, 0x44u);

    assert(BuildVendorEcRecordListResponse(1u, 1u) == 12u);
    assert(memory[RESPONSE + 3u] == 1u);
    AssertPayload(8u, 0x55u, 0u, 0x77u, 0x88u);
}

int main(void)
{
    TestControllerActiveForcesAllPrimaryRecords();
    TestUnknownStateForcesAllOnlyForPrimary();
    TestStateOneUsesRecordBit7AndTransformsType();
    TestStateOneCapacityOverrideUsesRecordBit6();
    TestStateTwoAppliesMediaTypeMask();
    TestAlternateStateTwoUsesOnlyEligibilityMask();
    return 0;
}
