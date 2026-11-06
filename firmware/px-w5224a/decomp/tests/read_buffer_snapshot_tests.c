#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "scsi/read_buffer_snapshot.h"

#define FIRMWARE_SIZE 0x2bc000u
#define SNAPSHOT_BASE 0x00160000ul
#define SNAPSHOT_LENGTH 0x28e2u

typedef struct
{
    fw_addr_t destination;
    fw_addr_t source;
    u16 count;
} CopyCall;

static u8 firmware[FIRMWARE_SIZE];
static u8 snapshot[SNAPSHOT_LENGTH];
static CopyCall copy_calls[17];
static size_t copy_call_count;
static size_t atomic_write_count;

u8 FirmwareRead8(fw_addr_t address)
{
    assert(address < FIRMWARE_SIZE);
    return firmware[address];
}

void FirmwareWrite8(fw_addr_t address, u8 value)
{
    assert(address < FIRMWARE_SIZE);
    firmware[address] = value;
}

u8 FirmwareReadSfr8(u8 address)
{
    (void)address;
    return 0u;
}

void FirmwareWriteSfr8(u8 address, u8 value)
{
    (void)address;
    (void)value;
}

void WriteControllerBufferWindowByteAtomic(fw_addr_t destination, u8 value)
{
    assert(destination >= SNAPSHOT_BASE);
    assert(destination < SNAPSHOT_BASE + SNAPSHOT_LENGTH);
    snapshot[destination - SNAPSHOT_BASE] = value;
    ++atomic_write_count;
}

void CopyRamBytesToReadBufferSnapshot(fw_addr_t destination, fw_addr_t source, u16 byte_count)
{
    u16 index;

    assert(copy_call_count < sizeof(copy_calls) / sizeof(copy_calls[0]));
    copy_calls[copy_call_count].destination = destination;
    copy_calls[copy_call_count].source = source;
    copy_calls[copy_call_count].count = byte_count;
    ++copy_call_count;
    for(index = 0u; index < byte_count; ++index)
        WriteControllerBufferWindowByteAtomic(destination + index, FirmwareRead8(source + index));
}

u16 CalculateControllerBufferChecksum28E0(fw_addr_t start_offset)
{
    u16 checksum = 0u;
    u16 index;

    assert(start_offset == SNAPSHOT_BASE);
    for(index = 0u; index < 0x28e0u; ++index)
        checksum = (u16)(checksum + snapshot[index]);
    return checksum;
}

static void WriteU32(fw_addr_t address, u32 value)
{
    firmware[address] = (u8)(value >> 24);
    firmware[address + 1u] = (u8)(value >> 16);
    firmware[address + 2u] = (u8)(value >> 8);
    firmware[address + 3u] = (u8)value;
}

static void FillPattern(fw_addr_t address, u16 count, u8 seed)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        firmware[address + index] = (u8)(seed + index * 13u);
}

static void InitializeState(u8 first_toc_track)
{
    memset(firmware, 0, sizeof(firmware));
    memset(snapshot, 0xa5, sizeof(snapshot));
    memset(copy_calls, 0, sizeof(copy_calls));
    copy_call_count = 0u;
    atomic_write_count = 0u;

    WriteU32(0x00155aul, SNAPSHOT_BASE);
    FillPattern(0x00dccdul, 24u, 0x21u);
    FillPattern(0x000cf5ul, 6u, 0x31u);
    FillPattern(0x00fbe0ul, 2u, 0x41u);
    FillPattern(0x00fbdeul, 2u, 0x51u);
    FillPattern(0x00f992ul, 2u, 0x61u);
    firmware[0x00fc0ful] = 0x71u;
    firmware[0x00fc16ul] = 0x72u;
    firmware[0x0012bful] = 2u;
    firmware[0x0012c0ul] = 4u;
    firmware[0x0012beul] = 0x75u;
    firmware[0x001008ul] = 0x76u;
    firmware[0x0012c9ul] = 0x77u;
    FillPattern(0x0012c1ul, 4u, 0x81u);
    firmware[0x0012caul] = 0u;
    firmware[0x0012cbul] = 0x16u;
    firmware[0x0012dcul] = 0x91u;
    firmware[0x0012ddul] = 0x92u;
    FillPattern(0x00fad3ul, 8u, 0xa1u);

    FillPattern(0x00f2c2ul, 24u, 0xb1u);
    FillPattern(0x2b800bul, 0x16u, 0xc1u);
    FillPattern(0x00dd2ful, 0x07c3u, 0xd1u);
    firmware[0x001132ul] = 0xe1u;
    firmware[0x00fb5bul] = 0xe2u;
    FillPattern(0x00fb7aul, 0x12u, 0xe3u);
    FillPattern(0x00fb53ul, 4u, 0xf1u);
    FillPattern(0x00fb57ul, 4u, 0xf5u);
    FillPattern(0x001143ul, 0x0cu, 0x11u);
    firmware[0x001147ul] = first_toc_track;
    firmware[0x001148ul] = 2u;
    FillPattern(0x2bb03bul, 40u, 0x22u);
    firmware[0x00114aul] = 2u;
    firmware[0x00114bul] = 1u;
    firmware[0x00114cul] = 3u;
    firmware[0x00114dul] = 0u;
    FillPattern(0x2bb42dul, 20u, 0x33u);
    FillPattern(0x2bb509ul, 10u, 0x44u);
    FillPattern(0x2bb5eful, 30u, 0x55u);
    FillPattern(0x2bb5dbul, 10u, 0x66u);
}

static void AssertBytes(u16 destination_offset, fw_addr_t source, u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        assert(snapshot[destination_offset + index] == firmware[source + index]);
}

static void AssertCopyCall(size_t index, u16 destination_offset, fw_addr_t source, u16 count)
{
    assert(copy_calls[index].destination == SNAPSHOT_BASE + destination_offset);
    assert(copy_calls[index].source == source);
    assert(copy_calls[index].count == count);
}

static void TestSnapshotContentsAndCopyLayout(void)
{
    static const u8 expected_state[35] = { 0x31u, 0x3eu, 0x4bu, 0x58u, 0x65u, 0x72u, 0x41u, 0x4eu, 0x51u, 0x5eu, 0x61u, 0x6eu, 0x71u, 0x72u, 0x02u, 0x04u, 0x75u, 0x76u, 0x77u, 0x00u, 0x16u, 0x91u,
        0x92u, 0x81u, 0x8eu, 0x9bu, 0xa8u, 0xa1u, 0xaeu, 0xbbu, 0xc8u, 0xd5u, 0xe2u, 0xefu, 0xfcu };
    u16 checksum = 0u;
    u16 index;

    InitializeState(0xffu);
    assert(BuildReadBufferMode1Id2Snapshot() == SNAPSHOT_LENGTH);
    assert(copy_call_count == 17u);
    assert(atomic_write_count > SNAPSHOT_LENGTH);
    assert(snapshot[0] == 0x28u);
    assert(snapshot[1] == 0xe2u);
    AssertBytes(2u, 0x00dccdul, 8u);
    AssertBytes(0x0au, 0x00dcd5ul, 0x10u);
    assert(memcmp(snapshot + 0x1au, expected_state, sizeof(expected_state)) == 0);

    AssertCopyCall(0u, 2u, 0x00dccdul, 8u);
    AssertCopyCall(1u, 0x0au, 0x00dcd5ul, 0x10u);
    AssertCopyCall(2u, 0x49u, 0x00f2c2ul, 24u);
    AssertCopyCall(3u, 0x02a6u, 0x2b800bul, 0x16u);
    AssertCopyCall(4u, 0x1817u, 0x00dd2ful, 0x07c3u);
    AssertCopyCall(5u, 0x1fdau, 0x001132ul, 1u);
    AssertCopyCall(6u, 0x1fdbu, 0x00fb5bul, 1u);
    AssertCopyCall(7u, 0x1fdcu, 0x00fb7aul, 0x12u);
    AssertCopyCall(8u, 0x1feeu, 0x00fb53ul, 4u);
    AssertCopyCall(9u, 0x1ff2u, 0x00fb57ul, 4u);
    AssertCopyCall(10u, 0x1ff6u, 0x001143ul, 0x0cu);
    AssertCopyCall(11u, 0x2002u, 0x2bb03bul, 40u);
    AssertCopyCall(12u, 0x23f4u, 0x2bb42dul, 20u);
    AssertCopyCall(13u, 0x24d0u, 0x2bb509ul, 10u);
    AssertCopyCall(14u, 0x25acu, 0x2bb5eful, 30u);
    AssertCopyCall(15u, 0x2746u, 0x2bb789ul, 0u);
    AssertCopyCall(16u, 0x28d6u, 0x2bb5dbul, 10u);

    AssertBytes(0x49u, 0x00f2c2ul, 24u);
    AssertBytes(0x02a6u, 0x2b800bul, 0x16u);
    AssertBytes(0x1817u, 0x00dd2ful, 0x07c3u);
    assert(snapshot[0x1ffau] == 0u);
    AssertBytes(0x2002u, 0x2bb03bul, 40u);
    AssertBytes(0x23f4u, 0x2bb42dul, 20u);
    AssertBytes(0x24d0u, 0x2bb509ul, 10u);
    AssertBytes(0x25acu, 0x2bb5eful, 30u);
    AssertBytes(0x28d6u, 0x2bb5dbul, 10u);
    assert(firmware[0x001147ul] == 0xffu);

    for(index = 0u; index < 0x28e0u; ++index)
        checksum = (u16)(checksum + snapshot[index]);
    assert(snapshot[0x28e0u] == (u8)(checksum >> 8));
    assert(snapshot[0x28e1u] == (u8)checksum);
    assert(snapshot[0x1000u] == 0u);
}

static void TestZeroFirstTrackAlsoNormalizesToFf(void)
{
    InitializeState(0u);
    assert(BuildReadBufferMode1Id2Snapshot() == SNAPSHOT_LENGTH);
    assert(firmware[0x001147ul] == 0xffu);
    assert(snapshot[0x1ffau] == 0u);
}

int main(void)
{
    TestSnapshotContentsAndCopyLayout();
    TestZeroFirstTrackAlsoNormalizesToFf();
    puts("read buffer snapshot tests passed");
    return 0;
}
