#include "read_buffer_snapshot_restore.h"
#include "../platform/controller_buffer_atomic.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"

static void CopySnapshotBytesToLocal(fw_addr_t controller_offset, u8 *destination, u16 byte_count)
{
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/scsi/read_buffer_snapshot_restore_bindings.h"
#undef PLEXTOR_BINDING_SECTION
}

static void WriteRestoredStateBytes(fw_addr_t destination, const u8 *state, u8 source_offset, u8 byte_count)
{
    u8 index;

    for(index = 0u; index < byte_count; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(destination, index), state[(u8)(source_offset + index)]);
}

/* RAM:fd5875 */
void RestoreReadBufferMode1Id2Snapshot(void)
{
    u8 state[35];
    u8 source_first_track;
    u8 destination_first_track;
    u8 count_first_track;
    u8 last_track;
    u16 byte_count;

    CopySnapshotBytesToLocal(0x1aul, state, 0x23u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_0CF5, state, 0u, 4u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_0CF9, state, 4u, 2u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_FBE0, state, 6u, 2u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_FBDE, state, 8u, 2u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_F992, state, 10u, 2u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC0F, state[12]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_FC16, state[13]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BF, state[14]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C0, state[15]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12BE, state[16]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1008, state[17]);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_12C9, state[18]);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_12C1, state, 23u, 4u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_12CA, state, 19u, 2u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_12DC, state, 21u, 2u);
    WriteRestoredStateBytes(FW_NEAR_UNKNOWN_FAD3, state, 27u, 8u);

    source_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    destination_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    count_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0);
    byte_count = (u16)(((u16)last_track - count_first_track + 2u) * 6u);
    CopyReadBufferSnapshotBytesToRam(0x3dul + (u16)destination_first_track * 6u, FW_NEAR_UNKNOWN_F2B6 + (u16)source_first_track * 6u, byte_count);
    CopyReadBufferSnapshotBytesToRam(0x02a6ul, FW_HUGE_UNKNOWN_2B800B, (u16)FirmwareReadNearU16(FW_NEAR_UNKNOWN_12CA));
    CopyReadBufferSnapshotBytesToRam(0x1817ul, FW_NEAR_UNKNOWN_DD2F, 0x07c3u);
    CopyReadBufferSnapshotBytesToRam(0x1fdaul, FW_NEAR_UNKNOWN_1132, 1u);
    CopyReadBufferSnapshotBytesToRam(0x1fdbul, FW_NEAR_UNKNOWN_FB5B, 1u);
    CopyReadBufferSnapshotBytesToRam(0x1fdcul, FW_NEAR_UNKNOWN_FB7A, 0x12u);
    CopyReadBufferSnapshotBytesToRam(0x1feeul, FW_NEAR_UNKNOWN_FB53, 4u);
    CopyReadBufferSnapshotBytesToRam(0x1ff2ul, FW_NEAR_UNKNOWN_FB57, 4u);
    CopyReadBufferSnapshotBytesToRam(0x1ff6ul, FW_NEAR_UNKNOWN_1143, 0x0cu);

    source_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    destination_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    count_first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    last_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_1148);
    byte_count = (u16)(((u16)last_track - count_first_track + 2u) * 10u);
    CopyReadBufferSnapshotBytesToRam(0x2002ul + (u16)destination_first_track * 10u, FW_HUGE_UNKNOWN_2BB03B + (u16)source_first_track * 10u, byte_count);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1147) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1147, 0xffu);

    CopyReadBufferSnapshotBytesToRam(0x23f4ul, FW_HUGE_UNKNOWN_2BB42D, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) * 10u));
    CopyReadBufferSnapshotBytesToRam(0x24d0ul, FW_HUGE_UNKNOWN_2BB509, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114B) * 10u));
    CopyReadBufferSnapshotBytesToRam(0x25acul, FW_HUGE_UNKNOWN_2BB5EF, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) * 10u));
    CopyReadBufferSnapshotBytesToRam(0x2746ul, FW_HUGE_UNKNOWN_2BB789, (u16)((u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114D) * 10u));
    CopyReadBufferSnapshotBytesToRam(0x28d6ul, FW_NEAR_UNKNOWN_2BB5DB, 10u);
}
