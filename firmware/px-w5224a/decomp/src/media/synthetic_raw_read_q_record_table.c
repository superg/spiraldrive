#include "synthetic_raw_read_q_record_table.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/memory.h"
#include "../platform/status_mapping.h"

#define RAW_TRACK_TABLE_ADDRESS FW_NEAR_UNKNOWN_B4BD
#define RAW_Q_TABLE_ADDRESS FW_NEAR_UNKNOWN_8012
#define RAW_TRACK_RECORD_SIZE 8u
#define RAW_Q_RECORD_SIZE 10u

static fw_addr_t RawTrackRecordAddress(u16 record_index)
{
    return RAW_TRACK_TABLE_ADDRESS + (fw_addr_t)(record_index * RAW_TRACK_RECORD_SIZE);
}

static fw_addr_t RawQRecordAddress(u16 record_index)
{
    return RAW_Q_TABLE_ADDRESS + (fw_addr_t)(record_index * RAW_Q_RECORD_SIZE);
}

static u16 ReadSyntheticQWord(u16 address)
{
    return FirmwareReadNearU16(address);
}

static s32 ReadSyntheticQSignedDword(fw_addr_t address)
{
    return (s32)FirmwareReadU32(address);
}

static void WriteSyntheticQWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void LoadRawTrackMsfTuple(u16 record_index, fw_addr_t tuple_address)
{
    fw_addr_t record = RawTrackRecordAddress(record_index);

    FirmwareWrite8(tuple_address, FirmwareRead8(record + 4u));
    FirmwareWrite8(tuple_address + 1u, FirmwareRead8(record + 6u));
    FirmwareWrite8(tuple_address + 2u, FirmwareRead8(record + 7u));
}

static u8 SyntheticQControl(void)
{
    return (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C47) << 4) | 5u);
}

static void AppendTocSummaryRecord(u16 output_index, u8 point, u8 track_number, u8 write_p_second)
{
    fw_addr_t output = RawQRecordAddress(output_index);
    u8 packed_track = BinaryByteToPackedBcd(track_number);
    u16 raw_track_index = FindType1RawTrackRecordIndexByTrackNumberOrLeadOut(packed_track);

    FirmwareWrite8(output, FirmwareRead8(RawTrackRecordAddress(raw_track_index)));
    FirmwareWrite8(output + 2u, point);
    FirmwareWrite8(output + 7u, packed_track);
    if(write_p_second != 0u)
        FirmwareWrite8(output + 8u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1402));
}

static u16 SkipSyntheticQPrefixRecords(u16 record_index)
{
    fw_addr_t record = RawTrackRecordAddress(record_index);

    while((FirmwareRead8(record) & 0x0fu) == 2u)
    {
        ++record_index;
        record = RawTrackRecordAddress(record_index);
    }
    while(FirmwareRead8(record + 1u) == 0u)
    {
        ++record_index;
        record = RawTrackRecordAddress(record_index);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) == 0x10u)
    {
        while((FirmwareRead8(record) & 0x40u) != 0u && FirmwareRead8(record + 1u) != 0xaau)
        {
            ++record_index;
            record = RawTrackRecordAddress(record_index);
        }
        while(FirmwareRead8(record + 1u) == 1u)
        {
            ++record_index;
            record = RawTrackRecordAddress(record_index);
        }
    }
    return record_index;
}

static u16 FindNextSyntheticQType1Record(u16 record_index)
{
    fw_addr_t record;

    for(;;)
    {
        record = RawTrackRecordAddress(record_index);
        if((FirmwareRead8(record) & 0x0fu) == 1u && FirmwareRead8(record + 2u) != 0u)
            return record_index;
        ++record_index;
    }
}

static u16 AppendSyntheticTrackRecords(u16 output_index, fw_addr_t tuple_address)
{
    u16 record_index = SkipSyntheticQPrefixRecords(0u);
    u8 track_number = FirmwareRead8(RawTrackRecordAddress(record_index) + 1u);

    while(track_number != 0xaau)
    {
        fw_addr_t record;
        fw_addr_t output;
        u16 next_track;

        record_index = FindNextSyntheticQType1Record(record_index);
        record = RawTrackRecordAddress(record_index);
        track_number = FirmwareRead8(record + 1u);
        next_track = (u16)track_number + 1u;

        /* Valid type-one track groups begin with index one. The canonical
         * body retains the previous tuple for later indices. */
        if(FirmwareRead8(record + 2u) == 1u)
            LoadRawTrackMsfTuple(record_index, tuple_address);
        ConvertThreeBinaryBytesToPackedBcd((u16)tuple_address);

        output = RawQRecordAddress(output_index);
        CopyThreeBytes16((u16)(output + 7u), (u16)tuple_address);
        FirmwareWrite8(output, FirmwareRead8(record));
        FirmwareWrite8(output + 1u, 0u);
        FirmwareWrite8(output + 2u, BinaryByteToPackedBcd(track_number));
        FirmwareWrite8(output + 6u, 0u);

        do
        {
            ++record_index;
            record = RawTrackRecordAddress(record_index);
            track_number = FirmwareRead8(record + 1u);
        } while((u16)track_number < next_track);
        ++output_index;
    }
    return output_index;
}

static void BuildB0Tuple(fw_addr_t tuple_address)
{
    u8 mode_bits = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) & 0xc0u);

    if(mode_bits == 0u || mode_bits == 0x40u)
    {
        FirmwareWrite8(tuple_address, 0xffu);
        FirmwareWrite8(tuple_address + 1u, 0xffu);
        FirmwareWrite8(tuple_address + 2u, 0xffu);
    }
    else
    {
        s32 frame_position;

        LoadRawTrackMsfTuple(ReadSyntheticQWord(FW_NEAR_UNKNOWN_149F), tuple_address);
        frame_position = ConvertBinaryMsfToAdjustedFramePosition(tuple_address);
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 0u)
            frame_position += 0x2bf2l;
        else
            frame_position += 0x1a5el;
        ConvertFramePositionToBinaryMsfTuple(frame_position, tuple_address);
        ConvertThreeBinaryBytesToPackedBcd((u16)tuple_address);
    }
}

static u16 AppendB0Record(u16 output_index, fw_addr_t tuple_address, s32 signed_state)
{
    fw_addr_t output = RawQRecordAddress(output_index);
    u8 special_c1 = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 0x10u) != 0u);

    BuildB0Tuple(tuple_address);
    CopyThreeBytes16((u16)(output + 3u), (u16)tuple_address);
    FirmwareWrite8(output + 7u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113B)));
    FirmwareWrite8(output + 8u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113C)));
    FirmwareWrite8(output + 9u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113D)));
    FirmwareWrite8(output, SyntheticQControl());
    FirmwareWrite8(output + 1u, 0u);
    FirmwareWrite8(output + 2u, 0xb0u);
    if(special_c1 != 0u && signed_state < 0)
        FirmwareWrite8(output + 6u, 3u);
    else if(signed_state < 0)
        FirmwareWrite8(output + 6u, 2u);
    else
        FirmwareWrite8(output + 6u, 1u);
    return (u16)(output_index + 1u);
}

static u16 AppendC0Record(u16 output_index)
{
    fw_addr_t output = RawQRecordAddress(output_index);

    FirmwareWrite8(output, SyntheticQControl());
    FirmwareWrite8(output + 2u, 0xc0u);
    FirmwareWrite8(output + 7u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_113F)));
    FirmwareWrite8(output + 8u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_1140)));
    FirmwareWrite8(output + 9u, BinaryByteToPackedBcd(FirmwareReadNear8(FW_NEAR_UNKNOWN_1141)));
    FirmwareWrite8(output + 3u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7A) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7B)) << 1));
    FirmwareWrite8(output + 4u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7C) << 1));
    FirmwareWrite8(output + 5u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7D) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7E)) << 1));
    return (u16)(output_index + 1u);
}

static u16 AppendC1Record(u16 output_index)
{
    fw_addr_t output = RawQRecordAddress(output_index);

    FirmwareWrite8(output, SyntheticQControl());
    FirmwareWrite8(output + 2u, 0xc1u);
    FirmwareWrite8(output + 3u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB7F) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB80)) << 1));
    FirmwareWrite8(output + 4u, (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_FB81) | FirmwareReadNear8(FW_NEAR_UNKNOWN_FB82)) << 1));
    FirmwareWrite8(output + 5u, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_FB83) << 1));
    return (u16)(output_index + 1u);
}

/* RAM:fe04ce */
void PLEXTOR_NEAR_FUNCTION BuildSyntheticRawReadQRecordTableFromTrackState(void)
{
    u16 output_index = 0u;
    fw_addr_t output;
    u16 lead_out_index;
    s32 signed_state;
#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/synthetic_raw_read_q_record_table_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    AppendTocSummaryRecord(output_index, 0xa0u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1400), 1u);
    ++output_index;
    AppendTocSummaryRecord(output_index, 0xa1u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1401), 0u);
    ++output_index;

    output = RawQRecordAddress(output_index);
    FirmwareWrite8(output + 7u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1406));
    FirmwareWrite8(output + 8u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1407));
    FirmwareWrite8(output + 9u, FirmwareReadNear8(FW_NEAR_UNKNOWN_1408));
    lead_out_index = (u16)(ReadSyntheticQWord(FW_NEAR_UNKNOWN_B4BB) - 2u);
    FirmwareWrite8(output, FirmwareRead8(RawTrackRecordAddress(lead_out_index)));
    FirmwareWrite8(output + 2u, 0xa2u);
    ++output_index;

    output_index = AppendSyntheticTrackRecords(output_index, tuple_address);
    WriteSyntheticQWord(FW_NEAR_UNKNOWN_8008, output_index);

    if((((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) & 0xc0u) != 0u) || FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u) && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) != 0x10u)
    {
        signed_state = ReadSyntheticQSignedDword(FW_NEAR_UNKNOWN_DD69);
        output_index = AppendB0Record(output_index, tuple_address, signed_state);
        if(signed_state < 0)
            output_index = AppendC0Record(output_index);
        if(signed_state < 0 && FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && (FirmwareReadNear8(FW_NEAR_UNKNOWN_FB5B) & 0x10u) != 0u)
            output_index = AppendC1Record(output_index);
    }

    WriteSyntheticQWord(FW_NEAR_UNKNOWN_800C, output_index);
    WriteSyntheticQWord(FW_NEAR_UNKNOWN_800A, output_index);
}
