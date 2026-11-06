#include "track_state.h"
#include "encoding.h"
#include "firmware_memory.h"
#include "leaf_workers.h"
#include "media_interrupt_leaves.h"
#include "status_mapping.h"

/* Reconstruction helper; no direct Ghidra function address. */
static void FirmwareWriteBigEndianU32TrackState(fw_addr_t destination, u32 value)
{
    FirmwareWriteU32(destination, value);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u32 FirmwareReadBigEndianU32TrackState(fw_addr_t source)
{
    return FirmwareReadU32(source);
}

/* Reconstruction helper; no direct Ghidra function address. */
static u16 FirmwareReadBigEndianU16TrackState(fw_addr_t source)
{
    return FirmwareReadU16(source);
}

/* RAM:fb5129 */
void PLEXTOR_NEAR_FUNCTION ClearControllerBufferWordsC119AndC517(void)
{
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC119, 0);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, 0);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0);
}

/* RAM:fb5468 */
void ClearWorkingTrackInformation(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD33, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD30, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD51, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD4D, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD49, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD45, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD5B, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD57, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD61, 0);
}

/* RAM:fb54ef */
void InitializeWorkingTrackFromTrackTableTail(void)
{
    u8 tail_index;
    u16 tail_offset;
    u32 tail_frame;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD63, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD64, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64) + 1u));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1306) & 2u) != 0u)
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_CURRENT_FRAME_POSITION,
            FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_FB90) + FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_FB8C) - 2250ul);

    tail_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, tail_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7E, tail_index);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) != 0xc0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD79, 1u);
    tail_offset = (u16)((u16)(tail_index << 4) + (u16)(tail_index << 1) + tail_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD87 + tail_offset, 0u);

    tail_frame = FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD75);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD65, tail_frame);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD69, tail_frame);
    tail_frame += 4500ul;
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD6D, tail_frame);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD7A, tail_frame);
    ClearWorkingTrackInformation();
    SnapshotTrackTableTailState();
}

/* Reconstruction helper; emits a primary staged raw-Q record in the native
 * control/point/tuple/zero-field store order. */
static void WritePrimaryStagedRawQRecord(fw_addr_t destination, u8 control, u8 point, u8 tuple0, u8 tuple1, u8 tuple2)
{
    FirmwareWrite8(destination, control);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 2u), point);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 7u), tuple0);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 8u), tuple1);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 9u), tuple2);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 6u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 1u), 0u);
}

/* RAM:fb87de */
u8 PLEXTOR_NEAR_FUNCTION IsPmaQRecordStateComplete(void)
{
    u8 index;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u)
    {
        u32 atip_start = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_114F) << 24;
        u32 pma_start;

        atip_start |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1150) << 16;
        atip_start |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1151) << 8;
        atip_start |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1152);
        pma_start = (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1143) << 24;
        pma_start |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1144) << 16;
        pma_start |= (u32)FirmwareReadNear8(FW_NEAR_UNKNOWN_1145) << 8;
        pma_start |= FirmwareReadNear8(FW_NEAR_UNKNOWN_1146);

        if((s32)pma_start < (s32)(atip_start - 0x3dful) || FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_114B) != 0u
            || FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_114D) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) | 0x80u));
            return 1u;
        }
        return 0u;
    }

    index = FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12BF) == 0u)
        index = FirmwareReadNear8(FW_NEAR_UNKNOWN_1147);
    for(; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_1149); ++index)
    {
        if(FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2BB03D, (u16)((u16)index * 10u))) == 0u)
        {
            fw_addr_t summary = FirmwareAddressIncrement24(FW_NEAR_UNKNOWN_F2B6, (u16)((u16)index * 6u));

            if(FirmwareRead8(summary) == 0u || FirmwareRead8(summary) == 0xaau)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x7fu));
                return 0u;
            }
        }
    }

    for(index = 1u; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_114A); ++index)
        if(FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2BB425, (u16)((u16)index * 10u))) == 0u)
            return 0u;
    for(index = 1u; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_114B); ++index)
        if(FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2BB501, (u16)((u16)index * 10u))) == 0u)
            return 0u;
    for(index = 1u; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_114C); ++index)
        if(FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2BB5E7, (u16)((u16)index * 10u))) == 0u)
            return 0u;
    for(index = 1u; index <= FirmwareReadNear8(FW_NEAR_UNKNOWN_114D); ++index)
        if(FirmwareRead8(FirmwareAddressIncrement24(FW_HUGE_UNKNOWN_2BB781, (u16)((u16)index * 10u))) == 0u)
            return 0u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1132, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1132) & 0x7fu));
    return 1u;
}

/* RAM:fb5594 */
void PLEXTOR_NEAR_FUNCTION BuildTrackTablePrimaryStagedRawQRecordSequence(void)
{
    fw_addr_t destination = FW_NEAR_UNKNOWN_2BC11B;
    u8 control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15F7) | 1u);
    u8 first_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E);
    u8 high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    u8 index;

    WritePrimaryStagedRawQRecord(destination, control, 0xa0u, BinaryByteToPackedBcd(first_index), FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A), 0u);
    destination = FirmwareAddressIncrement24(destination, 10u);
    WritePrimaryStagedRawQRecord(destination, control, 0xa1u, BinaryByteToPackedBcd((u8)(high_index - 1u)), 0u, 0u);
    destination = FirmwareAddressIncrement24(destination, 10u);

    FirmwareWrite8(destination, control);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 2u), 0xa2u);
    ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD71), FirmwareAddressIncrement24(destination, 7u));
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 7u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 7u))));
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 8u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 8u))));
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 9u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 9u))));
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 6u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 1u), 0u);
    destination = FirmwareAddressIncrement24(destination, 10u);

    for(index = first_index; index < high_index; ++index)
    {
        fw_addr_t source = FW_NEAR_UNKNOWN_DD80 + (fw_addr_t)((u16)(index << 4) + (u16)(index << 1) + index);

        WritePrimaryStagedRawQRecord(destination, FirmwareRead8(source), BinaryByteToPackedBcd(index), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(source, 1u))),
            BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(source, 2u))), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(source, 3u))));
        destination = FirmwareAddressIncrement24(destination, 10u);
    }

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC119, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, (u8)((u8)(high_index - first_index) + 3u));
}

/* RAM:fb6038 */
void CollectNonzeroPmaAdr3TupleBytes(fw_addr_t output_address)
{
    u8 record_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114A);
    u8 output_count = 0u;
    u8 record_index;

    FirmwareWrite8(output_address, 0u);
    if(record_count == 0u)
        return;
    for(record_index = 1u; record_index <= record_count; ++record_index)
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2AB42D + (fw_addr_t)((u16)((record_index - 1u) << 3) + (u16)((record_index - 1u) << 1));
        u8 tuple_index;

        for(tuple_index = 0u; tuple_index < 6u; ++tuple_index)
        {
            u8 tuple_offset = (u8)(tuple_index + 3u);
            u8 value = FirmwareRead8(FirmwareAddressIncrement24(record, tuple_index < 3u ? tuple_offset : (u8)(tuple_offset + 1u)));

            if(value == 0u)
                continue;
            ++output_count;
            FirmwareWrite8(FirmwareAddressIncrement24(output_address, output_count), value);
            if(output_count == 21u)
            {
                FirmwareWrite8(output_address, output_count);
                return;
            }
        }
    }
    FirmwareWrite8(output_address, output_count);
}

/* RAM:fb5cdc */
void BuildPmaAdr3Adr5RawQRecordSequence(fw_addr_t destination_address, fw_addr_t record_count_address)
{
    u8 adr3_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114A);
    u8 adr5_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114C);
    u8 adr3_tuples[22];
    u8 adr3_output_count = 0u;
    u8 group_count;
    u8 group_index;
    u8 adr5_index;
    u8 control;

    if(adr3_count == 0u && adr5_count == 0u)
        return;

    for(group_index = 0u; group_index < 22u; ++group_index)
        adr3_tuples[group_index] = 0u;
    for(group_index = 1u; group_index <= adr3_count; ++group_index)
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2AB42D + (fw_addr_t)((u16)((group_index - 1u) << 3) + (u16)((group_index - 1u) << 1));
        u8 tuple_index;

        for(tuple_index = 0u; tuple_index < 6u; ++tuple_index)
        {
            u8 tuple_offset = (u8)(tuple_index + 3u);
            u8 value = FirmwareRead8(FirmwareAddressIncrement24(record, tuple_index < 3u ? tuple_offset : (u8)(tuple_offset + 1u)));

            if(value == 0u)
                continue;
            ++adr3_output_count;
            adr3_tuples[adr3_output_count] = value;
            if(adr3_output_count == 21u)
                break;
        }
        if(adr3_output_count == 21u)
            break;
    }
    adr3_tuples[0] = adr3_output_count;
    if(adr3_tuples[0] == 0u)
        group_count = 0u;
    else if(adr3_tuples[0] < 8u)
        group_count = 1u;
    else if(adr3_tuples[0] < 15u)
        group_count = 2u;
    else
        group_count = 3u;

    control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15F7) | 5u);
    FirmwareWrite8(destination_address, control);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 2u), 0xb1u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 7u), BinaryByteToPackedBcd(adr5_count));
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 8u), BinaryByteToPackedBcd(adr3_tuples[0]));
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 1u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 6u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 5u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 4u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 3u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 9u), 0u);
    destination_address = FirmwareAddressIncrement24(destination_address, 10u);

    for(group_index = 0u; group_index < group_count; ++group_index)
    {
        u8 tuple_offset = (u8)(group_index * 7u + 1u);
        u8 point = (u8)(0xb2u + group_index);

        FirmwareWrite8(destination_address, control);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 2u), point);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 1u), 0u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 3u), BinaryByteToPackedBcd(adr3_tuples[tuple_offset]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 4u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 1u)]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 5u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 2u)]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 6u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 3u)]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 7u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 4u)]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 8u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 5u)]));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 9u), BinaryByteToPackedBcd(adr3_tuples[(u8)(tuple_offset + 6u)]));
        destination_address = FirmwareAddressIncrement24(destination_address, 10u);
    }

    for(adr5_index = 1u; adr5_index <= adr5_count; ++adr5_index)
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2AB5EF + (fw_addr_t)((u16)((adr5_index - 1u) << 3) + (u16)((adr5_index - 1u) << 1));

        FirmwareWrite8(destination_address, control);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 6u), 0u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 1u), 0u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 2u), BinaryByteToPackedBcd(adr5_index));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 3u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 3u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 4u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 4u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 5u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 5u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 7u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 7u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 8u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 8u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination_address, 9u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 9u))));
        destination_address = FirmwareAddressIncrement24(destination_address, 10u);
    }

    /* The RAM:fb5803 entry receives this count through a native stack slot.  Its source
     * adapter passes the non-address sentinel rather than inventing a RAM
     * scratch side effect. */
    if(record_count_address != FW_24BIT_ADDRESS_MASK)
        FirmwareWrite8(record_count_address, (u8)(group_count + 1u + adr5_count));
}

/* Reconstruction helper; mirrors the count written by RAM:fb5cdc without
 * adding a source-level stack-address adapter to the native DR4 output ABI. */
u8 CountPmaAdr3Adr5RawQRecords(void)
{
    u8 adr3_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114A);
    u8 adr5_count = FirmwareReadNear8(FW_NEAR_UNKNOWN_114C);
    u8 tuple_count = 0u;
    u8 record_index;

    if(adr3_count == 0u && adr5_count == 0u)
        return 0u;
    for(record_index = 1u; record_index <= adr3_count; ++record_index)
    {
        fw_addr_t record = FW_HUGE_UNKNOWN_2AB42D + (fw_addr_t)((u16)((record_index - 1u) << 3) + (u16)((record_index - 1u) << 1));
        u8 tuple_index;

        for(tuple_index = 0u; tuple_index < 6u; ++tuple_index)
        {
            u8 tuple_offset = (u8)(tuple_index + 3u);

            if(FirmwareRead8(FirmwareAddressIncrement24(record, tuple_index < 3u ? tuple_offset : (u8)(tuple_offset + 1u))) != 0u)
                ++tuple_count;
            if(tuple_count == 21u)
                return (u8)(4u + adr5_count);
        }
    }
    if(tuple_count == 0u)
        return (u8)(1u + adr5_count);
    return (u8)(1u + (u8)((tuple_count + 6u) / 7u) + adr5_count);
}

/* RAM:fb5803 */
void PLEXTOR_NEAR_FUNCTION BuildTrackTableSecondaryStagedRawQRecordSequence(void) PLEXTOR_REENTRANT
{
    fw_addr_t destination = FW_HUGE_UNKNOWN_2BC519;
    u8 control = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_15F7) | 5u);
    u8 operation = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45);
    u8 emit_b0 = 1u;
    u8 allow_c_record = 0u;
    u8 pma_record_count;
    u8 sequence_count = 0u;

    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0u);
    FirmwareWrite8(destination, control);
    FirmwareWrite8(FirmwareAddressIncrement24(destination, 2u), 0xb0u);
    if(operation == 0x40u)
    {
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), 0xffu);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), 0xffu);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), 0xffu);
    }
    else if(operation == 0xc0u)
    {
        ConvertFramePositionToBinaryMsfTupleFar((s32)(FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD75) + 4500ul), FirmwareAddressIncrement24(destination, 3u));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 3u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 4u))));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(destination, 5u))));
    }
    else if(operation == 0u)
    {
        emit_b0 = 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) != 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), 0xffu);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), 0xffu);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), 0xffu);
            emit_b0 = 1u;
        }
    }
    if(emit_b0 != 0u)
    {
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 7u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113B)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 8u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113C)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 9u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113D)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 6u), 1u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 1u), 0u);
        destination = FirmwareAddressIncrement24(destination, 10u);
        sequence_count = 1u;
    }

    if(operation == 0x40u)
    {
        if(emit_b0 != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_15F7) == 0u)
            allow_c_record = 1u;
    }
    else if(operation == 0u)
    {
        allow_c_record = emit_b0;
    }

    pma_record_count = 0u;
#if defined(PLEXTOR_HOST_TEST)
    /* A native host pointer cannot represent an 80251 24-bit stack address.
     * Suppress the emulated address write and recover the same count through
     * the addressless helper used by the host reconstruction. */
    BuildPmaAdr3Adr5RawQRecordSequence(destination, FW_24BIT_ADDRESS_MASK);
    pma_record_count = CountPmaAdr3Adr5RawQRecords();
#else
    BuildPmaAdr3Adr5RawQRecordSequence(destination, (fw_addr_t)&pma_record_count);
#endif
    destination = FirmwareAddressIncrement24(destination, (u16)pma_record_count * 10u);
    sequence_count = (u8)(sequence_count + pma_record_count);

    if(emit_b0 != 0u && (allow_c_record == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u) && (s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD69) < 0)
    {
        FirmwareWrite8(destination, control);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 2u), 0xc0u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 7u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113F)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 8u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_1140)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 9u), BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_1141)));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7B)) << 1));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) << 1));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7D) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E)) << 1));
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 1u), 0u);
        FirmwareWrite8(FirmwareAddressIncrement24(destination, 6u), 0u);
        ++sequence_count;
        destination = FirmwareAddressIncrement24(destination, 10u);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
        {
            FirmwareWrite8(destination, control);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 2u), 0xc1u);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 3u), (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7F) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80)) << 1));
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 4u), (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB82)) << 1));
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 5u), (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB83) << 1));
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 9u), 0u);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 8u), 0u);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 7u), 0u);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 1u), 0u);
            FirmwareWrite8(FirmwareAddressIncrement24(destination, 6u), 0u);
            ++sequence_count;
        }
    }
    if(emit_b0 != 0u)
        FirmwareWrite8(FW_HUGE_UNKNOWN_2BC51F, PackedBcdByteToBinary(sequence_count));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, sequence_count);
}

/* RAM:fe3277 */
void ClearRawReadPipelineStatusState(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1491, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1492, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1493, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1494, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1495, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1496, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1497, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1498, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1499, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_149A, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142A, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1429, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1428, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142F, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142E, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142D, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142C, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_1434, 0);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_1430, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1439, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1438, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_143A, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_143B, 0);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1454, 0);
}

/* RAM:fcd225 */
void PLEXTOR_NEAR_FUNCTION ResetType2ControllerRecordDetectionState(void)
{
    FirmwareWriteBigEndianU32TrackState(FW_HUGE_UNKNOWN_2BA5A3, 0);
    FirmwareWriteBigEndianU32TrackState(FW_HUGE_ACTIVE_RECORD_UPPER_BOUNDARY, 0);
    FirmwareWriteBigEndianU32TrackState(FW_HUGE_UNKNOWN_2BA59F, 0);
    FirmwareWriteBigEndianU32TrackState(FW_HUGE_ACTIVE_RECORD_MAPPED_POSITION, 0);
    FirmwareWriteBigEndianU32TrackState(FW_HUGE_UNKNOWN_2BA525, 0);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BA524, 0);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_EVENTS) & 0xfeu));
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0x7fu));
    FirmwareWriteNear8(FW_NEAR_HOST_ACQUISITION_COUNTDOWN, 10u);
    FirmwareWriteNear8(FW_NEAR_HOST_COMMAND_STATUS, (u8)(FirmwareReadNear8(FW_NEAR_HOST_COMMAND_STATUS) & 0xf7u));
    FirmwareWriteNear8(FW_NEAR_HOST_FAILURE_COUNTDOWN, (FirmwareReadNear8(FW_NEAR_UNKNOWN_E750) & 1u) != 0 ? 0x3cu : 0x14u);
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfbu));
    FirmwareWriteNear8(FW_NEAR_MEDIA_OPERATION_EVENTS, (u8)(FirmwareReadNear8(FW_NEAR_MEDIA_OPERATION_EVENTS) & 0xfdu));
    FirmwareWriteNear8(FW_NEAR_FORMAT_STATE, 0);
}

/* RAM:fc4e7d */
void InitializeWorkingTrackFlagsFromSelectedTrackMode(void)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) << 4) & 0xf0u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) | 1u));
}

/* RAM:fc4bc8 */
void DeriveWorkingTrackStartFromStatusClassAndDataBlockType(void)
{
    u8 status_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD32);
    u8 data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    u8 read_plan_code;
    u32 frame_adjustment;

    if(status_class == 2u)
    {
        if(data_block_type == 8u)
        {
            read_plan_code = 4u;
            frame_adjustment = 225ul;
        }
        else
        {
            read_plan_code = 6u;
            frame_adjustment = 150ul;
        }
    }
    else if(status_class == 1u)
    {
        if(data_block_type == 8u)
        {
            read_plan_code = 5u;
            frame_adjustment = 150ul;
        }
        else
        {
            read_plan_code = 3u;
            frame_adjustment = 225ul;
        }
    }
    else if(data_block_type == 8u)
    {
        read_plan_code = 4u;
        frame_adjustment = 225ul;
    }
    else
    {
        read_plan_code = 3u;
        frame_adjustment = 225ul;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, read_plan_code);
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + frame_adjustment);
}

/* RAM:fc4d2e */
void SelectWorkingTrackReadPlanCodeByStatusClass(void)
{
    u8 status_class = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD32);
    u8 data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    u8 read_plan_code;

    if(status_class == 2u)
        read_plan_code = data_block_type == 8u ? 4u : 6u;
    else if(status_class == 1u)
        read_plan_code = data_block_type == 8u ? 5u : 3u;
    else
        read_plan_code = data_block_type == 8u ? 4u : 3u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, read_plan_code);
}

/* RAM:fc4c5f */
u8 SelectWorkingTrackReadPlanCode(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 2u)
        {
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 4u)
                return 1u;
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) != 0x50u)
                return 1u;
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 11u);
            return 1u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 2u)
    {
        return 1u;
    }

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) <= 3u)
        return 1u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD2F) == 4u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u ? 8u : 9u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD2F) == 2u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u ? 8u : 9u);
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u)
            SelectWorkingTrackReadPlanCodeByStatusClass();
        else
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 7u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) != 0u ? 2u : 10u);
    return 1u;
}

/* RAM:fc4d7d */
void BuildWorkingTrackAdr1RawQRecord(void)
{
    fw_addr_t record = FW_NEAR_UNKNOWN_2BC11B;

    FirmwareWrite8(record, (u8)(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) << 4) & 0xf0u) | 1u));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 1u), 0u);
    FirmwareWrite8(FirmwareAddressIncrement24(record, 2u), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35));
    ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41), FirmwareAddressIncrement24(record, 3u));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 6u), 0u);
    ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D), FirmwareAddressIncrement24(record, 7u));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 2u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 2u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 3u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 3u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 4u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 4u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 5u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 5u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 7u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 7u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 8u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 8u))));
    FirmwareWrite8(FirmwareAddressIncrement24(record, 9u), BinaryByteToPackedBcd(FirmwareRead8(FirmwareAddressIncrement24(record, 9u))));
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC119, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC11A, 1u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC517, 0u);
    FirmwareWrite8(FW_HUGE_UNKNOWN_2BC518, 0u);
}

/* RAM:fc509b */
u8 InitializeWorkingTrackFromTrackTableHighEntryFromRegisterInputs(u8 inherited_r7, u8 inherited_b)
{
    u8 track_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F);
    fw_addr_t track_record = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD71, (u16)((u16)track_index * 19u));
    u8 update_inherited_r7 = inherited_r7;

    if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 0x16u)) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_index);
    if(track_index == FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 2u);
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD7A));
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD6D, (u16)((u16)track_index * 19u))));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD33, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD75, (u16)((u16)track_index * 19u))));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, ClassifyTrackInformationResultCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD33)));
        if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
        if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 7u)
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);

        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, (u32)ConvertBinaryMsfToAdjustedFramePositionAndCaptureFrameByte(track_record, &update_inherited_r7));
    }

    InitializeWorkingTrackFlagsFromSelectedTrackMode();
    UpdateWorkingTrackDataBlockTypeFields(update_inherited_r7, inherited_b);
    InitializeWorkingTrackFlagsFromSelectedTrackMode();
    UpdateWorkingTrackDataBlockTypeFields(update_inherited_r7, inherited_b);
    return DeriveWorkingTrackStartFrameAndReadPlan();
}

/* RAM:fc4f31 */
u8 LoadTrackInformationAgainstSnapshotFromInheritedB(u8 track_number, u8 inherited_b)
{
    u8 saved_high_index = FirmwareReadNear8(FW_NEAR_UNKNOWN_E4EC);
    fw_addr_t track_record;

    if(saved_high_index < track_number || track_number >= 100u)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    track_record = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD71, (u16)((u16)track_number * 19u));
    if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 0x16u)) != 0u)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    if(track_number == saved_high_index)
    {
        u8 update_inherited_r7 = saved_high_index;

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, saved_high_index);
        if(track_number == FirmwareReadNear8(FW_NEAR_UNKNOWN_E4ED))
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 2u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_E4EE));
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 3u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD30, (u8)(track_number - 1u));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD6D, (u16)((u16)track_number * 19u))));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD33, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD75, (u16)((u16)track_number * 19u))));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, ClassifyTrackInformationResultCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD33)));
            if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
            if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 7u)
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, (u32)ConvertBinaryMsfToAdjustedFramePositionAndCaptureFrameByte(track_record, &update_inherited_r7));
        }

        InitializeWorkingTrackFlagsFromSelectedTrackMode();
        UpdateWorkingTrackDataBlockTypeFields(update_inherited_r7, inherited_b);
        return DeriveWorkingTrackStartFrameAndReadPlan();
    }

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, 4u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD80, (u16)((u16)track_number * 19u))));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD88, (u16)((u16)track_number * 19u))));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, ClassifyTrackInformationResultCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38)));
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD81, (u16)((u16)track_number * 19u))));
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD84, (u16)((u16)track_number * 19u))));
    FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D) - 150ul);
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0x40u) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 10u);
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD37) == 1u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 5u);
    else
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 6u);
    return 1u;
}

/* Reconstruction helper; no direct Ghidra function address. */
static void PopulateWorkingTrackStatusForIndex(u8 track_number)
{
    fw_addr_t track_record = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD71, (u16)((u16)track_number * 19u));

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 3u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD30, (u8)(track_number - 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD31, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD6D, (u16)((u16)track_number * 19u))));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD33, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD75, (u16)((u16)track_number * 19u))));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD32, ClassifyTrackInformationResultCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD33)));
    if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
    if(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 3u)) == 7u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 4u);
}

/* RAM:fc3731 */
u8 LoadTrackInformationByNumberFromInheritedB(u8 track_number, u8 inherited_b)
{
    fw_addr_t track_record;
    u8 working_type;
    u8 update_inherited_r7;
    u32 packet_value;

    if(track_number >= 100u)
    {
        SetSenseData(5u, 0x2400u);
        return 0u;
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD79) == 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) < track_number)
        {
            HandleUnsupportedScsiCommand();
            return 0u;
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) < track_number)
    {
        HandleUnsupportedScsiCommand();
        return 0u;
    }

    ClearWorkingTrackInformation();
    track_record = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD71, (u16)((u16)track_number * 19u));
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 0x16u)) == 4u)
        if(track_number == FindTrackTableEntryByNumber(FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 0x21u))))
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 2u);
        else
            PopulateWorkingTrackStatusForIndex(track_number);
    else if(track_number == FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD2F, 2u);
    else
        PopulateWorkingTrackStatusForIndex(track_number);

    working_type = FirmwareRead8(FirmwareAddressIncrementLow16(track_record, 0x16u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD34, working_type);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD35, track_number);
    update_inherited_r7 = working_type;
    if(track_number == FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7E))
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD7A));
    else
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39, (u32)ConvertBinaryMsfToAdjustedFramePositionAndCaptureFrameByte(track_record, &update_inherited_r7));

    if(working_type == 0u)
    {
        InitializeWorkingTrackFlagsFromSelectedTrackMode();
        UpdateWorkingTrackDataBlockTypeFields(update_inherited_r7, inherited_b);
        if(DeriveWorkingTrackStartFrameAndReadPlan() == 0u)
            return 0u;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64));
        packet_value = (FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) & 0xffffff00ul) | FirmwareReadNear8(FW_NEAR_UNKNOWN_DD64);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD36, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD80, (u16)((u16)track_number * 19u))));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD88, (u16)((u16)track_number * 19u))));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, ClassifyTrackInformationResultCode(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38)));
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD81, (u16)((u16)track_number * 19u))));
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41, (u32)ConvertBinaryMsfToAdjustedFramePosition(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD84, (u16)((u16)track_number * 19u))));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD5F, (u8)((FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD89, (u16)((u16)track_number * 19u))) >> 1) & 1u));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD60, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD92, (u16)((u16)track_number * 19u))));
        packet_value = (FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41) & 0xffffff00ul) | FirmwareReadNear8(FW_NEAR_UNKNOWN_DD60);
        if(SelectWorkingTrackReadPlanCode() == 0u)
            return 0u;
    }

    if(working_type == 0u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        {
            InitializeWorkingTrackWritePacketState();
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD45, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D) - 5ul);
        }
    }
    else if(working_type == 1u || working_type == 3u)
    {
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD45, packet_value);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD89, (u16)((u16)track_number * 19u))) & 1u);
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD57, packet_value);
    }
    else if(working_type == 5u || working_type == 6u)
    {
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD51, packet_value);
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD89, (u16)((u16)track_number * 19u))) & 1u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD57, packet_value);
        }
    }
    else if(working_type == 4u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xd0u) == 0x50u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD55, 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD56, FirmwareRead8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD89, (u16)((u16)track_number * 19u))) & 1u);
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD57, packet_value);
    }

    if(working_type == 1u || working_type == 3u)
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD45));
    else if(working_type == 0u || working_type == 2u)
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39));
    else if(working_type == 5u || working_type == 6u)
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD4D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD51));
    return 1u;
}

/* RAM:fc3ad6 */
void UpdateTrackTableFromWorkingTrack(void)
{
    u8 working_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    u8 track_number;
    fw_addr_t record;
    u8 advance_tail;

    if(working_type == 0u || (working_type != 1u && working_type != 2u && working_type != 3u && working_type != 4u && working_type != 5u && working_type != 6u && working_type != 7u))
        return;

    track_number = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35);
    record = FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD80, (u16)((u16)track_number * 19u));
    FirmwareWrite8(FirmwareAddressIncrementLow16(record, 7u), working_type);
    FirmwareWrite8(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36));
    ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D), FirmwareAddressIncrementLow16(record, 1u));
    ConvertFramePositionToBinaryMsfTupleFar((s32)FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD41), FirmwareAddressIncrementLow16(record, 4u));
    FirmwareWrite8(FirmwareAddressIncrementLow16(record, 8u), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD38));
    FirmwareWrite8(FirmwareAddressIncrementLow16(record, 9u), (u8)((FirmwareRead8(FirmwareAddressIncrementLow16(record, 9u)) & 0xfeu) | (FirmwareReadNear8(FW_NEAR_UNKNOWN_DD56) & 1u)));
    FirmwareWrite8(FirmwareAddressIncrementLow16(record, 18u), FirmwareReadNear8(FW_NEAR_UNKNOWN_DD60));
    FirmwareWrite8(FirmwareAddressIncrementLow16(record, 9u), (u8)((FirmwareRead8(FirmwareAddressIncrementLow16(record, 9u)) & 0xfdu) | ((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD5F) & 1u) << 1)));
    FirmwareWriteBigEndianU32TrackState(FirmwareAddressIncrementLow16(record, 14u),
        working_type == 5u || working_type == 6u ? FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD51) : FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD45));
    FirmwareWriteBigEndianU32TrackState(FirmwareAddressIncrementLow16(record, 10u), FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD57));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD7F) != track_number)
        return;
    advance_tail = (u8)(working_type == 2u || working_type == 3u || working_type == 4u || working_type == 5u || (working_type == 7u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD5F) != 0u));
    if(advance_tail != 0u)
    {
        u8 next_track = (u8)(track_number + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD7F, next_track);
        if(next_track < 100u)
            FirmwareWrite8(FirmwareAddressIncrementLow16(FW_NEAR_UNKNOWN_DD87, (u16)((u16)next_track * 19u)), 0u);
    }
}

/* RAM:fc4a5a */
u8 DeriveWorkingTrackStartFrameAndReadPlan(void)
{
    u16 audio_pause_length;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) != 0u)
        return 1u;

    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) <= 3u)
    {
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + 150ul);
        return 1u;
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD2F) == 4u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
        {
            if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
                FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + 150ul);
            }
            else
            {
                audio_pause_length = FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F);
                if(audio_pause_length < 150u)
                {
                    SetCommandSequenceErrorSense();
                    return 0u;
                }
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 9u);
                FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F));
            }
        }
        else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 2u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + 225ul);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 10u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F));
        }
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD2F) == 2u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 8u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + 150ul);
        }
        else
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 9u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F));
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD31) & 0x40u) != 0u)
    {
        if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
        {
            DeriveWorkingTrackStartFromStatusClassAndDataBlockType();
        }
        else
        {
            audio_pause_length = FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F);
            if(audio_pause_length < 150u)
            {
                SetCommandSequenceErrorSense();
                return 0u;
            }
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 7u);
            FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F));
        }
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) & 4u) != 0u)
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 2u);
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + 225ul);
    }
    else
    {
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD62, 10u);
        FirmwareWriteBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD3D, FirmwareReadBigEndianU32TrackState(FW_NEAR_UNKNOWN_DD39) + FirmwareReadBigEndianU16TrackState(FW_NEAR_UNKNOWN_0C4F));
    }
    return 1u;
}

/* RAM:fc4e93 */
void UpdateWorkingTrackDataBlockTypeFields(u8 inherited_r7, u8 inherited_b)
{
    u8 data_block_type;
    u8 data_block_class;
    u8 data_block_mask;

    /* The native entry consumes this volatile byte once for each result. */
    data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    data_block_class = inherited_r7;
    if(data_block_type <= 3u || data_block_type == 8u)
        data_block_class = 1u;
    else if(data_block_type >= 9u && data_block_type <= 13u)
        data_block_class = 2u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD37, data_block_class);

    data_block_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    data_block_mask = inherited_b;
    if(data_block_type == 8u)
        data_block_mask = 0x40u;
    else if(data_block_type == 9u)
        data_block_mask = 0x20u;
    else if(data_block_type == 10u || data_block_type == 11u || data_block_type == 13u)
        data_block_mask = 0x10u;
    else if(data_block_type == 12u)
        data_block_mask = 0x08u;
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_DD38, data_block_mask);
}
