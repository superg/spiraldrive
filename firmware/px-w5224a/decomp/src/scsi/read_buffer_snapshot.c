#include "read_buffer_snapshot.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define SNAPSHOT_LENGTH 0x28e2u
#define SNAPSHOT_CHECKSUM_OFFSET 0x28e0u

static fw_addr_t ReadReservedControllerBufferBase(void)
{
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_buffer_snapshot_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void CopyLocalBytesToSnapshot(fw_addr_t destination, const u8 *source, u16 byte_count)
{
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/scsi/read_buffer_snapshot_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void ReadSnapshotStateBytes(u8 *state, u8 destination_offset, fw_addr_t source, u8 byte_count)
{
    u8 index;

    for(index = 0u; index < byte_count; ++index)
        state[(u8)(destination_offset + index)] = FirmwareRead8(FirmwareAddressIncrement24(source, index));
}

/* RAM:fd50ae */
u16 BuildReadBufferMode1Id2Snapshot(void)
{
    fw_addr_t base = ReadReservedControllerBufferBase();
    fw_addr_t cursor = base;
    u16 index;
    u8 state[35];
    u8 first_track;
    u8 checksum_bytes[2];
    u16 checksum;

    for(index = 0u; index < SNAPSHOT_LENGTH; ++index)
    {
        WriteControllerBufferWindowByteAtomic(cursor, 0u);
        cursor = FirmwareAddressIncrement24(cursor, 1u);
    }

    checksum_bytes[0] = (u8)(SNAPSHOT_LENGTH >> 8);
    checksum_bytes[1] = (u8)SNAPSHOT_LENGTH;
    CopyLocalBytesToSnapshot(base, checksum_bytes, 2u);
    CopyRamBytesToReadBufferSnapshot(base + 2u, FW_NEAR_UNKNOWN_DCCD, 8u);
    CopyRamBytesToReadBufferSnapshot(base + 0x0au, FW_NEAR_UNKNOWN_DCD5, 0x10u);

    ReadSnapshotStateBytes(state, 0u, FW_NEAR_UNKNOWN_0CF5, 4u);
    ReadSnapshotStateBytes(state, 4u, FW_NEAR_UNKNOWN_0CF9, 2u);
    ReadSnapshotStateBytes(state, 6u, FW_NEAR_UNKNOWN_FBE0, 2u);
    ReadSnapshotStateBytes(state, 8u, FW_NEAR_UNKNOWN_FBDE, 2u);
    ReadSnapshotStateBytes(state, 10u, FW_NEAR_UNKNOWN_F992, 2u);
    state[12] = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC0F);
    state[13] = FirmwareReadNear8(FW_NEAR_UNKNOWN_FC16);
    state[14] = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    state[15] = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    state[16] = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BE);
    state[17] = FirmwareReadNear8(FW_NEAR_UNKNOWN_1008);
    state[18] = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9);
    ReadSnapshotStateBytes(state, 23u, FW_NEAR_UNKNOWN_12C1, 4u);
    ReadSnapshotStateBytes(state, 19u, FW_NEAR_UNKNOWN_12CA, 2u);
    ReadSnapshotStateBytes(state, 21u, FW_NEAR_UNKNOWN_12DC, 2u);
    ReadSnapshotStateBytes(state, 27u, FW_NEAR_UNKNOWN_FAD3, 8u);
    CopyLocalBytesToSnapshot(base + 0x1au, state, 0x23u);

    first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    CopyRamBytesToReadBufferSnapshot(base + 0x3du + (u16)first_track * 6u, FW_NEAR_UNKNOWN_F2B6 + (u16)first_track * 6u, (u16)(((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) - first_track + 2u) * 6u));
    CopyRamBytesToReadBufferSnapshot(base + 0x02a6u, FW_HUGE_UNKNOWN_2B800B, (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12CA));
    CopyRamBytesToReadBufferSnapshot(base + 0x1817u, FW_NEAR_UNKNOWN_DD2F, 0x07c3u);
    CopyRamBytesToReadBufferSnapshot(base + 0x1fdau, FW_NEAR_UNKNOWN_1132, 1u);
    CopyRamBytesToReadBufferSnapshot(base + 0x1fdbu, FW_NEAR_UNKNOWN_FB5B, 1u);
    CopyRamBytesToReadBufferSnapshot(base + 0x1fdcu, FW_NEAR_UNKNOWN_FB7A, 0x12u);
    CopyRamBytesToReadBufferSnapshot(base + 0x1feeu, FW_NEAR_UNKNOWN_FB53, 4u);
    CopyRamBytesToReadBufferSnapshot(base + 0x1ff2u, FW_NEAR_UNKNOWN_FB57, 4u);

    first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    if(first_track == 0xffu)
    {
        first_track = 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 0u);
    }
    CopyRamBytesToReadBufferSnapshot(base + 0x1ff6u, FW_NEAR_UNKNOWN_1143, 0x0cu);
    CopyRamBytesToReadBufferSnapshot(base + 0x2002u + (u16)first_track * 10u, FW_HUGE_UNKNOWN_2BB03B + (u16)first_track * 10u,
        (u16)(((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_1148) - first_track + 2u) * 10u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 0xffu);

    CopyRamBytesToReadBufferSnapshot(base + 0x23f4u, FW_HUGE_UNKNOWN_2BB42D, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) * 10u));
    CopyRamBytesToReadBufferSnapshot(base + 0x24d0u, FW_HUGE_UNKNOWN_2BB509, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114B) * 10u));
    CopyRamBytesToReadBufferSnapshot(base + 0x25acu, FW_HUGE_UNKNOWN_2BB5EF, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) * 10u));
    CopyRamBytesToReadBufferSnapshot(base + 0x2746u, FW_HUGE_UNKNOWN_2BB789, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114D) * 10u));
    CopyRamBytesToReadBufferSnapshot(base + 0x28d6u, FW_NEAR_UNKNOWN_2BB5DB, 10u);

    checksum = CalculateControllerBufferChecksum28E0(base);
    checksum_bytes[0] = (u8)(checksum >> 8);
    checksum_bytes[1] = (u8)checksum;
    CopyLocalBytesToSnapshot(base + SNAPSHOT_CHECKSUM_OFFSET, checksum_bytes, 2u);
    return SNAPSHOT_LENGTH;
}
