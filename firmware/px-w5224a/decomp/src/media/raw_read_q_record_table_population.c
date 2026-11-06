#include "raw_read_q_record_table_population.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_q_record_table_population_bindings.h"
#undef PLEXTOR_BINDING_SECTION

#define RAW_Q_RECORD_COUNT 220u
#define RAW_Q_RECORD_SIZE 10u
#define RAW_Q_TABLE_ADDRESS FW_NEAR_UNKNOWN_8012
#define PMA_ADR1_TABLE_ADDRESS FW_HUGE_UNKNOWN_2BB03B
#define PMA_ADR2_RECORD_ADDRESS FW_NEAR_UNKNOWN_2BB5DB
#define PMA_ADR3_TABLE_ADDRESS FW_NEAR_UNKNOWN_2BB423
#define PMA_ADR4_TABLE_ADDRESS FW_NEAR_UNKNOWN_2BB4FF

static fw_addr_t RawQRecordAddress(u8 record_index)
{
    return RAW_Q_TABLE_ADDRESS + (fw_addr_t)((u16)record_index * 10u);
}

static fw_addr_t PmaRecordAddress(fw_addr_t table, u8 record_index)
{
    return table + (fw_addr_t)((u16)record_index * 10u);
}

static void CopyRawQRecord(fw_addr_t source, u8 destination_index)
{
    fw_addr_t destination = RawQRecordAddress(destination_index);
    u8 byte_index;

    for(byte_index = 0u; byte_index < RAW_Q_RECORD_SIZE; ++byte_index)
        FirmwareWrite8(destination + byte_index, FirmwareRead8(source + byte_index));
}

static void WriteRawQPopulationWord(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static u32 ReadConfigurationDword(const u8 *bytes)
{
    return ((u32)bytes[0] << 24) | ((u32)bytes[1] << 16) | ((u32)bytes[2] << 8) | bytes[3];
}

static void PackFramePositionIntoRecord(fw_addr_t record, u8 tuple_offset, s32 frame_position)
{
    u16 tuple_address = (u16)(record + tuple_offset);

    FRAME_TO_MSF(frame_position, tuple_address);
    PACK_TUPLE(tuple_address);
}

static u8 ShouldPackAdr1Tuples(void)
{
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 1u)
    {
        u8 mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8);

        return (u8)(mode == 6u || mode == 3u);
    }
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
        return 1u;
    return (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u);
}

static void NormalizeCopiedAdr1Record(u8 next_output_index)
{
    fw_addr_t record = RawQRecordAddress((u8)(next_output_index - 1u));

    FirmwareWrite8(record + 2u, BINARY_TO_BCD(FirmwareRead8(record + 2u)));
    if(ShouldPackAdr1Tuples() != 0u)
    {
        PACK_TUPLE((u16)(record + 3u));
        PACK_TUPLE((u16)(record + 7u));
    }
}

static void NormalizeCopiedPmaRecord(u8 next_output_index)
{
    fw_addr_t record = RawQRecordAddress((u8)(next_output_index - 1u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B8) == 6u)
    {
        UNPACK_TUPLE((u16)(record + 3u));
        UNPACK_TUPLE((u16)(record + 7u));
        FirmwareWrite8(record + 2u, BINARY_TO_BCD(FirmwareRead8(record + 2u)));
    }
    else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B6) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 0u)
    {
        PACK_TUPLE((u16)(record + 3u));
        PACK_TUPLE((u16)(record + 7u));
        FirmwareWrite8(record + 2u, BINARY_TO_BCD(FirmwareRead8(record + 2u)));
    }
}

static u8 ClearCompleteTuplesAtOrAboveTrack(u8 first_index, u8 end_index, u8 selected_track)
{
    u8 removed_count = 0u;
    u8 record_index;

    for(record_index = first_index; record_index < end_index; ++record_index)
    {
        fw_addr_t record = RawQRecordAddress(record_index);
        u8 cleared_count = 0u;
        u8 byte_index;

        for(byte_index = 0u; byte_index < 3u; ++byte_index)
        {
            fw_addr_t address = record + 3u + byte_index;

            if(FirmwareRead8(address) >= selected_track)
            {
                FirmwareWrite8(address, 0u);
                ++cleared_count;
            }
        }
        for(byte_index = 0u; byte_index < 3u; ++byte_index)
        {
            fw_addr_t address = record + 7u + byte_index;

            if(FirmwareRead8(address) >= selected_track)
            {
                FirmwareWrite8(address, 0u);
                ++cleared_count;
            }
        }
        if(cleared_count == 6u)
            ++removed_count;
    }
    return removed_count;
}

static u8 AppendPmaTable(fw_addr_t table, u8 record_count, u8 output_index)
{
    u8 source_index;

    for(source_index = 1u; source_index <= record_count; ++source_index)
    {
        CopyRawQRecord(PmaRecordAddress(table, source_index), output_index);
        ++output_index;
        NormalizeCopiedPmaRecord(output_index);
    }
    return output_index;
}

static void AppendSyntheticSeedRecord(u8 output_index)
{
    u8 configuration[4];
    u32 configuration_value;
    u32 product;
    fw_addr_t record = RawQRecordAddress(output_index);
    u16 seed;

    READ_CONFIGURATION(0x7aul, configuration, 4u);
    configuration_value = ReadConfigurationDword(configuration);
    seed = (u16)(((u16)configuration[2] << 8) | configuration[3]);
    RESET_RANDOM(seed);
    product = MULTIPLY_U32((u32)NEXT_RANDOM(), configuration_value);
    FirmwareWrite8(record + 3u, (u8)(product >> 16));
    FirmwareWrite8(record + 4u, (u8)(product >> 8));
    FirmwareWrite8(record + 5u, (u8)product);
    PACK_TUPLE((u16)(record + 3u));
    if((u16)FirmwareRead8(record + 3u) + (u16)FirmwareRead8(record + 4u) + (u16)FirmwareRead8(record + 5u) == 0u)
    {
        FirmwareWrite8(record + 3u, 0x0cu);
        FirmwareWrite8(record + 4u, 0x22u);
        FirmwareWrite8(record + 5u, 0x38u);
    }
    FirmwareWrite8(record + 8u, FirmwareReadNear8(FW_NEAR_FORMAT_STATE) == 0u ? FirmwareReadNear8(FW_NEAR_UNKNOWN_0C4A) : 0x20u);
    FirmwareWrite8(record, 2u);
}

static void AppendWorkingTrackRecord(u8 output_index)
{
    fw_addr_t record = RawQRecordAddress(output_index);
    u8 special_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7);

    FirmwareWrite8(record, FirmwareReadNear8(FW_NEAR_UNKNOWN_DD36));
    FirmwareWrite8(record + 2u, BINARY_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_DD35)));
    PackFramePositionIntoRecord(record, 3u, (s32)(special_mode == 0u ? FirmwareReadNearU32(FW_NEAR_UNKNOWN_8000) : FirmwareReadNearU32(FW_NEAR_UNKNOWN_E61A)));
    if(special_mode == 0u)
    {
        PackFramePositionIntoRecord(record, 7u, (s32)FirmwareReadNearU32(FW_NEAR_UNKNOWN_DD3D));
    }
    else
    {
        u32 frame = FirmwareReadNearU32(FW_NEAR_UNKNOWN_E616);

        PackFramePositionIntoRecord(record, 7u, (s32)(frame + 151ul));
    }
}

/* RAM:fe8b66 */
u8 PLEXTOR_NEAR_FUNCTION PopulateRawReadQRecordTableFromIndex(u8 first_record_index)
{
    u8 output_index = 0u;
    u8 record_index;
    u8 selected_track;
    u8 filter_start;

    for(record_index = first_record_index; record_index < RAW_Q_RECORD_COUNT; ++record_index)
    {
        fw_addr_t record = RawQRecordAddress(record_index);
        u8 byte_index;

        for(byte_index = 0u; byte_index < RAW_Q_RECORD_SIZE; ++byte_index)
            FirmwareWrite8(record + byte_index, 0u);
    }

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_114E) != 0u)
    {
        CopyRawQRecord(PMA_ADR2_RECORD_ADDRESS, output_index);
        ++output_index;
        PACK_TUPLE((u16)(RawQRecordAddress((u8)(output_index - 1u)) + 3u));
        if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
            FirmwareWrite8(RawQRecordAddress((u8)(output_index - 1u)) + 8u, 0x20u);
    }
    else if((FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u)
            || (FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) == 0u) || FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        AppendSyntheticSeedRecord(output_index);
        ++output_index;
    }

    if(FirmwareReadNear8(FW_NEAR_FORMAT_STATE) != 0u)
    {
        fw_addr_t record = RawQRecordAddress(output_index);
        u32 high_frame = FirmwareReadU32(FW_HUGE_UNKNOWN_2BA58D);

        FirmwareWrite8(record, 0x71u);
        FirmwareWrite8(record + 2u, 1u);
        PackFramePositionIntoRecord(record, 3u, (s32)high_frame);
        PackFramePositionIntoRecord(record, 7u, 150l);
        WriteRawQPopulationWord(FW_NEAR_UNKNOWN_800E, 2u);
        WriteRawQPopulationWord(FW_NEAR_UNKNOWN_8010, 20u);
        return 1u;
    }

    selected_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_14CC);
    if(selected_track == 0xbbu)
        selected_track = SELECT_TRACK();
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 0u && selected_track == 0u)
    {
        WriteRawQPopulationWord(FW_NEAR_UNKNOWN_800E, 0u);
        WriteRawQPopulationWord(FW_NEAR_UNKNOWN_8010, 0u);
        return 1u;
    }
    if(selected_track == 0xffu)
        return 0u;

    record_index = 1u;
    for(;;)
    {
        fw_addr_t source = PmaRecordAddress(PMA_ADR1_TABLE_ADDRESS, record_index);
        u8 source_track = FirmwareRead8(source + 2u);
        u8 special_mode = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7);

        if(special_mode == 0u && (record_index > FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) || source_track > selected_track))
            break;
        if(source_track < selected_track)
        {
            CopyRawQRecord(source, output_index);
            ++record_index;
            ++output_index;
            NormalizeCopiedAdr1Record(output_index);
            continue;
        }
        if((source_track == selected_track && first_record_index == 1u) || special_mode == 1u)
        {
            AppendWorkingTrackRecord(output_index);
            ++record_index;
            ++output_index;
            if(special_mode == 1u)
                break;
            continue;
        }
        ++record_index;
    }

    filter_start = output_index;
    output_index = AppendPmaTable(PMA_ADR3_TABLE_ADDRESS, FirmwareReadNear8(FW_NEAR_UNKNOWN_114A), output_index);
    output_index = (u8)(output_index - ClearCompleteTuplesAtOrAboveTrack(filter_start, output_index, selected_track));

    filter_start = output_index;
    output_index = AppendPmaTable(PMA_ADR4_TABLE_ADDRESS, FirmwareReadNear8(FW_NEAR_UNKNOWN_114B), output_index);
    output_index = (u8)(output_index - ClearCompleteTuplesAtOrAboveTrack(filter_start, output_index, selected_track));

    WriteRawQPopulationWord(FW_NEAR_UNKNOWN_800E, output_index);
    WriteRawQPopulationWord(FW_NEAR_UNKNOWN_8010, (u16)output_index * 10u);
    return 1u;
}
