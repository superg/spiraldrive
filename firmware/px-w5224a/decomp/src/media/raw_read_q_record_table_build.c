#include "raw_read_q_record_table_build.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "rid01_identity_block.h"

#define RAW_Q_TABLE_ADDRESS FW_NEAR_UNKNOWN_8012
#define RAW_Q_RECORD_SIZE 10u
#define RAW_Q_RECORD_CAPACITY 220u
#define RAW_TRACK_TABLE_ADDRESS FW_NEAR_UNKNOWN_B4BD
#define RAW_TRACK_RECORD_SIZE 8u
#define CURRENT_Q_ADDRESS FW_NEAR_UNKNOWN_1272

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_q_record_table_build_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadRawQWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadRawQDword(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteRawQWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteRawQDword(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static fw_addr_t RawQRecordAddress(u16 index)
{
    return RAW_Q_TABLE_ADDRESS + (fw_addr_t)(index * RAW_Q_RECORD_SIZE);
}

static fw_addr_t RawTrackRecordAddress(u16 index)
{
    return RAW_TRACK_TABLE_ADDRESS + (fw_addr_t)(index * RAW_TRACK_RECORD_SIZE);
}

static void ClearRawQRecordTable(void)
{
    u16 record_index;

    for(record_index = 0u; record_index != RAW_Q_RECORD_CAPACITY; ++record_index)
    {
        fw_addr_t record = RawQRecordAddress(record_index);
        u8 byte_index;

        for(byte_index = 0u; byte_index != RAW_Q_RECORD_SIZE; ++byte_index)
            FirmwareWrite8(record + byte_index, 0u);
    }
}

static void ClearCurrentRawQPacket(void)
{
    u8 index;

    for(index = 0u; index != RAW_Q_RECORD_SIZE; ++index)
        FirmwareWrite8(CURRENT_Q_ADDRESS + index, 0u);
}

static void CopyRawQRecordToCurrent(u16 record_index)
{
    fw_addr_t source = RawQRecordAddress(record_index);
    u8 index;

    for(index = 0u; index != RAW_Q_RECORD_SIZE; ++index)
        FirmwareWrite8(CURRENT_Q_ADDRESS + index, FirmwareRead8(source + index));
}

static void ConvertFrameToPackedTuple(s32 frame_position, fw_addr_t tuple_address)
{
    RAW_Q_CONVERT(frame_position, tuple_address);
    RAW_Q_PACK((u16)tuple_address);
}

static void LoadRawTrackTuple(u16 record_index, fw_addr_t tuple_address)
{
    fw_addr_t record = RawTrackRecordAddress(record_index);

    FirmwareWrite8(tuple_address, FirmwareRead8(record + 4u));
    FirmwareWrite8(tuple_address + 1u, FirmwareRead8(record + 6u));
    FirmwareWrite8(tuple_address + 2u, FirmwareRead8(record + 7u));
}

static u32 AdvanceRawQMainCursor(u32 cursor)
{
    u32 ring_span = RAW_Q_MULTIPLY(0x930ul, ReadRawQDword(FW_NEAR_UNKNOWN_156A));

    if(cursor + 0x930ul > ring_span - 1ul)
        return 0ul;
    return cursor + 0x930ul;
}

static void ReportRawQCaptureFailure(void)
{
    RAW_Q_SET_SENSE(4u, 0x4400u);
    RAW_Q_SET_INFORMATION(ReadRawQDword(FW_NEAR_UNKNOWN_1598));
}

static void EnqueueRawQSector(u32 cursor, u8 separator)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, separator);
    WriteRawQDword(FW_NEAR_UNKNOWN_126E, cursor);
    RAW_Q_UPDATE();
    RAW_Q_ENQUEUE();
    WriteRawQDword(FW_NEAR_UNKNOWN_1598, ReadRawQDword(FW_NEAR_UNKNOWN_1598) + 1ul);
}

static u8 AppendInitialAdr2Record(u16 *output_index, u16 *expected_count, u16 *capture_count, u16 *remaining_count)
{
    u8 append = 0u;
    fw_addr_t output;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 0u)
        append = 1u;
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5E3) != FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C0) == FirmwareReadNear8(FW_NEAR_UNKNOWN_1149)
            && FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) == 1u)
        append = 1u;
    if(append == 0u)
        return 0u;

    *expected_count = 10u;
    *capture_count = 1u;
    *remaining_count = 1u;
    WriteRawQWord(FW_NEAR_UNKNOWN_8010, 10u);
    WriteRawQWord(FW_NEAR_UNKNOWN_800E, 1u);
    output = RawQRecordAddress(*output_index);
    FirmwareWrite8(output + 3u, RAW_Q_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5DE)));
    FirmwareWrite8(output + 4u, RAW_Q_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5DF)));
    FirmwareWrite8(output + 5u, RAW_Q_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5E0)));
    FirmwareWrite8(output + 8u, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A));
    FirmwareWrite8(output, 2u);
    ++*output_index;
    return 1u;
}

static u8 CaptureSpecialTypeTwoSequence(fw_addr_t tuple_address, u16 expected_count)
{
    u32 cursor = 0ul;
    u16 sector_index;

    WriteRawQWord(FW_NEAR_UNKNOWN_800E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    WriteRawQDword(FW_NEAR_UNKNOWN_1598, ReadRawQDword(FW_NEAR_UNKNOWN_114F) - 10ul);
    ConvertFrameToPackedTuple((s32)ReadRawQDword(FW_NEAR_UNKNOWN_1598), tuple_address);
    RAW_Q_COPY(FW_NEAR_UNKNOWN_1428, (u16)tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 1u);
    ClearCurrentRawQPacket();

    for(sector_index = 0u; sector_index != 10u; ++sector_index)
    {
        if(RAW_Q_WAIT() == 0u)
        {
            RAW_Q_REPORT(3u);
            return 0u;
        }
        RAW_Q_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
        if(RAW_Q_CAPTURE(cursor, 10u) == 0u)
        {
            ReportRawQCaptureFailure();
            return 0u;
        }
        RAW_Q_BUILD_RID01(cursor + 0x10ul);
        RAW_Q_INCREMENT(FW_NEAR_UNKNOWN_1428);
        EnqueueRawQSector(cursor, (u8)sector_index);
        cursor = AdvanceRawQMainCursor(cursor);
    }

    RAW_Q_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(RAW_Q_CAPTURE(cursor, 6u) == 0u)
    {
        ReportRawQCaptureFailure();
        return 0u;
    }
    RAW_Q_BUILD_RID01(cursor + 0x10ul);
    RAW_Q_INCREMENT(FW_NEAR_UNKNOWN_1428);
    EnqueueRawQSector(cursor, (u8)sector_index);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    WriteRawQWord(FW_NEAR_UNKNOWN_8010, (u16)(expected_count + 1u));
    return 1u;
}

static u16 BuildTypeTwoRecords(u16 output_index, fw_addr_t tuple_address)
{
    u16 source_index = 0u;
    u16 source_count = ReadRawQWord(FW_NEAR_UNKNOWN_B4BB);

    while(source_index < source_count)
    {
        fw_addr_t source = RawTrackRecordAddress(source_index);

        if((FirmwareRead8(source) & 0x0fu) == 1u && FirmwareRead8(source + 2u) == 1u && FirmwareRead8(source + 1u) < 0xaau)
        {
            u16 next_index = (u16)(source_index + 1u);
            fw_addr_t output = RawQRecordAddress(output_index);

            while((FirmwareRead8(RawTrackRecordAddress(next_index)) & 0x0fu) != 1u)
                ++next_index;
            FirmwareWrite8(output, FirmwareRead8(source));
            FirmwareWrite8(output + 2u, RAW_Q_TO_BCD(FirmwareRead8(source + 1u)));
            LoadRawTrackTuple(source_index, tuple_address);
            RAW_Q_COPY((u16)(output + 7u), (u16)tuple_address);
            RAW_Q_PACK((u16)(output + 7u));
            LoadRawTrackTuple(next_index, tuple_address);
            RAW_Q_COPY((u16)(output + 3u), (u16)tuple_address);
            RAW_Q_PACK((u16)(output + 3u));
            ++output_index;
        }
        ++source_index;
    }
    return output_index;
}

static u8 AppendModeRecord(u16 output_index, fw_addr_t tuple_address, u16 *expected_count, u16 *capture_count, u16 *remaining_count)
{
    u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_128D);
    fw_addr_t output = RawQRecordAddress(output_index);
    u8 index;

    if(mode == 3u)
    {
        u16 ordinal = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114A) + 1u;

        if(ordinal > 21u)
            goto invalid_pma_count;
        *expected_count = (u16)(*expected_count + 10u);
        ++*remaining_count;
        *capture_count = *remaining_count;
        WriteRawQWord(FW_NEAR_UNKNOWN_8010, *expected_count);
        WriteRawQWord(FW_NEAR_UNKNOWN_800E, *remaining_count);
        ClearCurrentRawQPacket();
        FirmwareWrite8(CURRENT_Q_ADDRESS, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xf0u) + 3u));
        FirmwareWrite8(CURRENT_Q_ADDRESS + 2u, RAW_Q_TO_BCD((u8)ordinal));
        FirmwareWrite8(CURRENT_Q_ADDRESS + 3u, RAW_Q_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35)));
        for(index = 0u; index != RAW_Q_RECORD_SIZE; ++index)
            FirmwareWrite8(output + index, FirmwareRead8(CURRENT_Q_ADDRESS + index));
    }
    else if(mode == 5u)
    {
        u16 ordinal = (u16)FirmwareReadNear8(FW_NEAR_UNKNOWN_114C) + 1u;

        if(ordinal > 40u)
            goto invalid_pma_count;
        *expected_count = (u16)(*expected_count + 10u);
        ++*remaining_count;
        *capture_count = *remaining_count;
        WriteRawQWord(FW_NEAR_UNKNOWN_8010, *expected_count);
        WriteRawQWord(FW_NEAR_UNKNOWN_800E, *remaining_count);
        FirmwareWrite8(CURRENT_Q_ADDRESS, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xf0u) + 5u));
        FirmwareWrite8(CURRENT_Q_ADDRESS + 2u, RAW_Q_TO_BCD((u8)ordinal));
        FirmwareWrite8(output, FirmwareRead8(CURRENT_Q_ADDRESS));
        FirmwareWrite8(output + 2u, FirmwareRead8(CURRENT_Q_ADDRESS + 2u));
        ConvertFrameToPackedTuple((s32)ReadRawQDword(FW_NEAR_UNKNOWN_8000), tuple_address);
        RAW_Q_COPY((u16)(output + 7u), (u16)tuple_address);
        RAW_Q_COPY(FW_NEAR_UNKNOWN_1279, (u16)tuple_address);
        ConvertFrameToPackedTuple((s32)(ReadRawQDword(FW_NEAR_UNKNOWN_8000) - 1ul), tuple_address);
        RAW_Q_COPY((u16)(output + 3u), (u16)tuple_address);
        RAW_Q_COPY(FW_NEAR_UNKNOWN_1275, (u16)tuple_address);
    }
    else if(mode == 0u)
    {
        *expected_count = (u16)(*expected_count + 10u);
        ++*remaining_count;
        *capture_count = *remaining_count;
        WriteRawQWord(FW_NEAR_UNKNOWN_8010, *expected_count);
        WriteRawQWord(FW_NEAR_UNKNOWN_800E, *remaining_count);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128B) == 1u)
        {
            for(index = 0u; index != RAW_Q_RECORD_SIZE; ++index)
            {
                u8 value = FirmwareReadNear8(FW_NEAR_UNKNOWN_2BC11B + index);
                FirmwareWrite8(CURRENT_Q_ADDRESS + index, value);
                FirmwareWrite8(output + index, value);
            }
            RAW_Q_COPY((u16)tuple_address, (u16)(output + 7u));
            RAW_Q_UNPACK((u16)tuple_address);
            WriteRawQDword(FW_NEAR_UNKNOWN_8000, (u32)RAW_Q_TO_FRAME(tuple_address));
            RAW_Q_COPY((u16)tuple_address, (u16)(output + 3u));
            RAW_Q_UNPACK((u16)tuple_address);
            WriteRawQDword(FW_NEAR_UNKNOWN_8000, (u32)RAW_Q_TO_FRAME(tuple_address));
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_128B, 0u);
        }
        else
        {
            s32 tuple2_frame;
            s32 tuple1_frame;

            FirmwareWrite8(CURRENT_Q_ADDRESS, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36) & 0xf0u) + 1u));
            FirmwareWrite8(CURRENT_Q_ADDRESS + 2u, RAW_Q_TO_BCD((u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) + 1u)));
            FirmwareWrite8(output, FirmwareRead8(CURRENT_Q_ADDRESS));
            FirmwareWrite8(output + 2u, FirmwareRead8(CURRENT_Q_ADDRESS + 2u));
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u)
            {
                tuple2_frame = (s32)(ReadRawQDword(FW_NEAR_UNKNOWN_E616) + 151ul);
                tuple1_frame = (s32)ReadRawQDword(FW_NEAR_UNKNOWN_E61A);
            }
            else
            {
                tuple2_frame = (s32)ReadRawQDword(FW_NEAR_UNKNOWN_DD3D);
                tuple1_frame = (s32)ReadRawQDword(FW_NEAR_UNKNOWN_8000);
            }
            ConvertFrameToPackedTuple(tuple2_frame, tuple_address);
            RAW_Q_COPY((u16)(output + 7u), (u16)tuple_address);
            RAW_Q_COPY(FW_NEAR_UNKNOWN_1279, (u16)tuple_address);
            ConvertFrameToPackedTuple(tuple1_frame, tuple_address);
            if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u
                && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u)
            {
                FirmwareWrite8(tuple_address, 0xffu);
                FirmwareWrite8(tuple_address + 1u, 0xffu);
                FirmwareWrite8(tuple_address + 2u, 0xffu);
            }
            RAW_Q_COPY((u16)(output + 3u), (u16)tuple_address);
            RAW_Q_COPY(FW_NEAR_UNKNOWN_1275, (u16)tuple_address);
        }
    }
    return 1u;

invalid_pma_count:
    RAW_Q_LATCH_FAILURE();
    RAW_Q_SET_SENSE(3u, 0x7304u);
    RAW_Q_SET_INFORMATION(ReadRawQDword(FW_NEAR_UNKNOWN_1598));
    return 0u;
}

static u8 CaptureRawQRecordGroups(u16 capture_count, u16 remaining_count, u8 odd_record_count, fw_addr_t tuple_address, u8 terminal_separator)
{
    u32 cursor = 0ul;
    u16 record_index;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    ConvertFrameToPackedTuple((s32)ReadRawQDword(FW_NEAR_UNKNOWN_1598), tuple_address);
    RAW_Q_COPY(FW_NEAR_UNKNOWN_1428, (u16)tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 1u);

    for(record_index = 0u; record_index < capture_count; ++record_index)
    {
        u16 first_sector;
        u16 end_sector;
        u16 sector_index;

        --remaining_count;
        if(odd_record_count == 0u || (s16)remaining_count > 0)
        {
            if((record_index & 1u) == 0u)
            {
                first_sector = 0u;
                end_sector = 5u;
            }
            else
            {
                first_sector = 5u;
                end_sector = 10u;
            }
        }
        else
        {
            first_sector = 0u;
            end_sector = 10u;
        }

        for(sector_index = first_sector; sector_index < end_sector; ++sector_index)
        {
            if(RAW_Q_WAIT() == 0u)
            {
                RAW_Q_REPORT(3u);
                return 0u;
            }
            RAW_Q_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
            if(RAW_Q_CAPTURE(cursor, 10u) == 0u)
            {
                ReportRawQCaptureFailure();
                return 0u;
            }
            RAW_Q_BUILD_RID01(cursor + 0x10ul);
            RAW_Q_INCREMENT(FW_NEAR_UNKNOWN_1428);
            CopyRawQRecordToCurrent(record_index);
            EnqueueRawQSector(cursor, (u8)sector_index);
            cursor = AdvanceRawQMainCursor(cursor);
        }
        terminal_separator = (u8)end_sector;
    }

    RAW_Q_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
    if(RAW_Q_CAPTURE(cursor, 6u) == 0u)
    {
        ReportRawQCaptureFailure();
        return 0u;
    }
    RAW_Q_BUILD_RID01(cursor + 0x10ul);
    RAW_Q_INCREMENT(FW_NEAR_UNKNOWN_1428);
    CopyRawQRecordToCurrent(0u);
    EnqueueRawQSector(cursor, terminal_separator);
    WriteRawQWord(FW_NEAR_UNKNOWN_8010, (u16)(ReadRawQWord(FW_NEAR_UNKNOWN_8010) + 1u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    return 1u;
}

/* RAM:fe3fd0 */
u8 PLEXTOR_NEAR_FUNCTION BuildRawReadQRecordTable(void)
{
    u16 output_index = 0u;
    u16 expected_count = 0u;
    u16 capture_count = 0u;
    u16 remaining_count = 0u;
    u8 odd_record_count = 1u;
    u8 terminal_separator = (u8)RAW_Q_RECORD_CAPACITY;
    u8 write_type;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/raw_read_q_record_table_build_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_128D, 0u);

    WriteRawQDword(FW_NEAR_UNKNOWN_1598, ReadRawQDword(FW_NEAR_UNKNOWN_1143));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 1u);
    WriteRawQWord(FW_NEAR_UNKNOWN_8010, 0u);
    ClearRawQRecordTable();
    AppendInitialAdr2Record(&output_index, &expected_count, &capture_count, &remaining_count);

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    if(write_type == 2u)
    {
        if(RAW_Q_BIT0041() != 0u)
            return 1u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) != 0xc0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) < 2u)
            return CaptureSpecialTypeTwoSequence(tuple_address, expected_count);

        output_index = BuildTypeTwoRecords(output_index, tuple_address);
        capture_count = output_index;
        remaining_count = output_index;
        WriteRawQWord(FW_NEAR_UNKNOWN_800E, output_index);
        if((output_index & 1u) == 0u)
        {
            expected_count = (u16)(output_index * 5u);
            odd_record_count = 0u;
        }
        else
        {
            expected_count = (u16)(output_index * 5u + 5u);
            odd_record_count = 1u;
        }
        WriteRawQWord(FW_NEAR_UNKNOWN_8010, expected_count);
        terminal_separator = (u8)ReadRawQWord(FW_NEAR_UNKNOWN_B4BB);
    }
    else if(write_type == 3u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) != 0u)
            return 1u;
        expected_count = (u16)(expected_count + 10u);
        WriteRawQWord(FW_NEAR_UNKNOWN_8010, expected_count);
        WriteRawQWord(FW_NEAR_UNKNOWN_800E, remaining_count);
        capture_count = remaining_count;
        WriteRawQDword(FW_NEAR_UNKNOWN_1598, ReadRawQDword(FW_NEAR_UNKNOWN_114F) - 10ul);
        ClearCurrentRawQPacket();
        odd_record_count = (u8)(remaining_count & 1u);
    }
    else if(write_type == 0u || write_type == 1u)
    {
        if(AppendModeRecord(output_index, tuple_address, &expected_count, &capture_count, &remaining_count) == 0u)
            return 0u;
        if(remaining_count > 1u)
            remaining_count = 1u;
        odd_record_count = 1u;
    }
    else
    {
        return 0u;
    }

    return CaptureRawQRecordGroups(capture_count, remaining_count, odd_record_count, tuple_address, terminal_separator);
}
