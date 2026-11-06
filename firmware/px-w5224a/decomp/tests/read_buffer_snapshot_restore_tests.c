#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "platform/mcs251.h"
#include "scsi/read_buffer_snapshot_restore.h"

#define FIRMWARE_SIZE 0x2bc000u
#define SNAPSHOT_LENGTH 0x28e2u

typedef struct
{
    fw_addr_t source;
    fw_addr_t destination;
    u16 count;
} CopyCall;

static u8 firmware[FIRMWARE_SIZE];
static u8 snapshot[SNAPSHOT_LENGTH];
static CopyCall copy_calls[15];
static size_t copy_call_count;
static fw_addr_t atomic_read_offsets[35];
static u8 atomic_read_count;

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

u8 ReadControllerBufferByteAtomic(fw_addr_t controller_offset)
{
    assert(controller_offset < SNAPSHOT_LENGTH);
    assert(atomic_read_count < sizeof(atomic_read_offsets) / sizeof(atomic_read_offsets[0]));
    atomic_read_offsets[atomic_read_count++] = controller_offset;
    return snapshot[controller_offset];
}

void CopyReadBufferSnapshotBytesToRam(fw_addr_t controller_offset, fw_addr_t ram_destination, u16 byte_count)
{
    u16 index;

    assert(copy_call_count < sizeof(copy_calls) / sizeof(copy_calls[0]));
    copy_calls[copy_call_count].source = controller_offset;
    copy_calls[copy_call_count].destination = ram_destination;
    copy_calls[copy_call_count].count = byte_count;
    ++copy_call_count;
    for(index = 0u; index < byte_count; ++index)
    {
        assert(controller_offset + index < SNAPSHOT_LENGTH);
        assert(ram_destination + index < FIRMWARE_SIZE);
        firmware[ram_destination + index] = snapshot[controller_offset + index];
    }
}

static void FillSnapshotPattern(u16 offset, u16 count, u8 seed)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        snapshot[offset + index] = (u8)(seed + index * 11u);
}

static void InitializeSnapshot(u8 first_toc_track, u8 highest_adr1_track)
{
    u8 index;

    memset(firmware, 0xa5, sizeof(firmware));
    memset(snapshot, 0, sizeof(snapshot));
    memset(copy_calls, 0, sizeof(copy_calls));
    memset(atomic_read_offsets, 0, sizeof(atomic_read_offsets));
    copy_call_count = 0u;
    atomic_read_count = 0u;

    for(index = 0u; index < 35u; ++index)
        snapshot[0x1au + index] = (u8)(0x30u + index);
    snapshot[0x1au + 14u] = 2u;
    snapshot[0x1au + 15u] = 4u;
    snapshot[0x1au + 19u] = 0u;
    snapshot[0x1au + 20u] = 0x16u;

    FillSnapshotPattern(0x49u, 24u, 0x61u);
    FillSnapshotPattern(0x02a6u, 0x16u, 0x71u);
    FillSnapshotPattern(0x1817u, 0x07c3u, 0x81u);
    snapshot[0x1fdau] = 0x91u;
    snapshot[0x1fdbu] = 0x92u;
    FillSnapshotPattern(0x1fdcu, 0x12u, 0x93u);
    FillSnapshotPattern(0x1feeu, 4u, 0xa1u);
    FillSnapshotPattern(0x1ff2u, 4u, 0xa5u);
    FillSnapshotPattern(0x1ff6u, 0x0cu, 0xb1u);
    snapshot[0x1ffau] = first_toc_track;
    snapshot[0x1ffbu] = highest_adr1_track;
    snapshot[0x1ffdu] = 2u;
    snapshot[0x1ffeu] = 1u;
    snapshot[0x1fffu] = 3u;
    snapshot[0x2000u] = 0u;
    FillSnapshotPattern((u16)(0x2002u + first_toc_track * 10u), (u16)((highest_adr1_track - first_toc_track + 2u) * 10u), 0xc1u);
    FillSnapshotPattern(0x23f4u, 20u, 0xd1u);
    FillSnapshotPattern(0x24d0u, 10u, 0xe1u);
    FillSnapshotPattern(0x25acu, 30u, 0xf1u);
    FillSnapshotPattern(0x28d6u, 10u, 0x21u);
}

static void AssertCopyCall(size_t index, u16 source_offset, fw_addr_t destination, u16 count)
{
    assert(copy_calls[index].source == source_offset);
    assert(copy_calls[index].destination == destination);
    assert(copy_calls[index].count == count);
}

static void AssertRestoredBytes(fw_addr_t destination, u16 source_offset, u16 count)
{
    u16 index;

    for(index = 0u; index < count; ++index)
        assert(firmware[destination + index] == snapshot[source_offset + index]);
}

static void TestCompleteRestoreWithZeroTrackNormalization(void)
{
    u8 index;

    InitializeSnapshot(0u, 2u);
    RestoreReadBufferMode1Id2Snapshot();
    assert(atomic_read_count == 35u);
    for(index = 0u; index < 35u; ++index)
        assert(atomic_read_offsets[index] == 0x1au + index);
    assert(copy_call_count == 15u);

    AssertRestoredBytes(0x000cf5ul, 0x1au, 4u);
    AssertRestoredBytes(0x000cf9ul, 0x1eu, 2u);
    AssertRestoredBytes(0x00fbe0ul, 0x20u, 2u);
    AssertRestoredBytes(0x00fbdeul, 0x22u, 2u);
    AssertRestoredBytes(0x00f992ul, 0x24u, 2u);
    AssertRestoredBytes(0x0012c1ul, 0x31u, 4u);
    AssertRestoredBytes(0x0012caul, 0x2du, 2u);
    AssertRestoredBytes(0x0012dcul, 0x2fu, 2u);
    AssertRestoredBytes(0x00fad3ul, 0x35u, 8u);

    AssertCopyCall(0u, 0x49u, 0x00f2c2ul, 24u);
    AssertCopyCall(1u, 0x02a6u, 0x2b800bul, 0x16u);
    AssertCopyCall(2u, 0x1817u, 0x00dd2ful, 0x07c3u);
    AssertCopyCall(3u, 0x1fdau, 0x001132ul, 1u);
    AssertCopyCall(4u, 0x1fdbu, 0x00fb5bul, 1u);
    AssertCopyCall(5u, 0x1fdcu, 0x00fb7aul, 0x12u);
    AssertCopyCall(6u, 0x1feeu, 0x00fb53ul, 4u);
    AssertCopyCall(7u, 0x1ff2u, 0x00fb57ul, 4u);
    AssertCopyCall(8u, 0x1ff6u, 0x001143ul, 0x0cu);
    AssertCopyCall(9u, 0x2002u, 0x2bb03bul, 40u);
    AssertCopyCall(10u, 0x23f4u, 0x2bb42dul, 20u);
    AssertCopyCall(11u, 0x24d0u, 0x2bb509ul, 10u);
    AssertCopyCall(12u, 0x25acu, 0x2bb5eful, 30u);
    AssertCopyCall(13u, 0x2746u, 0x2bb789ul, 0u);
    AssertCopyCall(14u, 0x28d6u, 0x2bb5dbul, 10u);

    AssertRestoredBytes(0x00f2c2ul, 0x49u, 24u);
    AssertRestoredBytes(0x2b800bul, 0x02a6u, 0x16u);
    AssertRestoredBytes(0x00dd2ful, 0x1817u, 0x07c3u);
    AssertRestoredBytes(0x2bb03bul, 0x2002u, 40u);
    AssertRestoredBytes(0x2bb42dul, 0x23f4u, 20u);
    AssertRestoredBytes(0x2bb509ul, 0x24d0u, 10u);
    AssertRestoredBytes(0x2bb5eful, 0x25acu, 30u);
    AssertRestoredBytes(0x2bb5dbul, 0x28d6u, 10u);
    assert(firmware[0x001147ul] == 0xffu);
}

static void TestNonzeroFirstTrackControlsPmaWindow(void)
{
    InitializeSnapshot(3u, 4u);
    RestoreReadBufferMode1Id2Snapshot();
    AssertCopyCall(9u, 0x2020u, 0x2bb059ul, 30u);
    AssertRestoredBytes(0x2bb059ul, 0x2020u, 30u);
    assert(firmware[0x001147ul] == 3u);
}

int main(void)
{
    TestCompleteRestoreWithZeroTrackNormalization();
    TestNonzeroFirstTrackControlsPmaWindow();
    puts("read buffer snapshot restore tests passed");
    return 0;
}
