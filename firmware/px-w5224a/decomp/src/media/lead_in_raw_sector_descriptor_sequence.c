#include "lead_in_raw_sector_descriptor_sequence.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"
#include "raw_read_toc_summary.h"
#include "rid01_identity_block.h"
#include "synthetic_raw_read_q_record_table.h"

#define RAW_Q_TABLE_ADDRESS FW_NEAR_UNKNOWN_8012
#define RAW_TRACK_TABLE_ADDRESS FW_NEAR_UNKNOWN_B4BD
#define RAW_Q_RECORD_SIZE 10u
#define RAW_TRACK_RECORD_SIZE 8u

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/lead_in_raw_sector_descriptor_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadLeadInSequenceU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadLeadInSequenceU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteLeadInSequenceU16(fw_addr_t address, u16 value)
{
    FirmwareWriteU16(address, value);
}

static void WriteLeadInSequenceU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t RawTrackRecordAddress(u16 index)
{
    return RAW_TRACK_TABLE_ADDRESS + (fw_addr_t)(index * RAW_TRACK_RECORD_SIZE);
}

static fw_addr_t RawQRecordAddress(u16 index)
{
    return RAW_Q_TABLE_ADDRESS + (fw_addr_t)(index * RAW_Q_RECORD_SIZE);
}

static void ResetLeadInDescriptorState(void)
{
    u32 ring_slot_count = ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_156A);

    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_129A, (u16)(ring_slot_count - 1ul));
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_129C, (u16)(ring_slot_count - 6ul));
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_126A, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1562));
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_126E, 0ul);
}

static void ClearLeadInQState(void)
{
    u16 index;

    for(index = 0u; index != 10u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + index, 0u);
    for(index = 0u; index != 2200u; ++index)
        FirmwareWrite8(RAW_Q_TABLE_ADDRESS + index, 0u);
}

static void BuildPackedTuple(s32 frame_position, fw_addr_t tuple_address)
{
    LEADIN_CONVERT(frame_position, tuple_address);
    LEADIN_PACK((u16)tuple_address);
}

static void CopySelectedQRecordToCurrent(u16 record_index, u16 tuple_address)
{
    fw_addr_t source = RawQRecordAddress(record_index);
    u8 index;

    for(index = 0u; index != RAW_Q_RECORD_SIZE; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + index, FirmwareRead8(source + index));
    if((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u)
        LEADIN_COPY(FW_NEAR_UNKNOWN_1275, tuple_address);
}

static void BuildAdr6QPacket(u16 record_index, u16 tuple_address)
{
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, (u8)((FirmwareRead8(RawQRecordAddress(record_index)) & 0xf0u) | 6u));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 0u);
    LEADIN_COPY(FW_NEAR_UNKNOWN_1275, tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1279, LEADIN_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5DE)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127A, LEADIN_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5DF)));
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127B, LEADIN_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_2BB5E0)));
}

static void ClearLeadInAuxiliarySlot(fw_addr_t main_data_cursor)
{
    fw_addr_t address = FirmwareAddressIncrement24(main_data_cursor, FW_HUGE_BUFFER_BANK_BASE);
    u8 index;

    for(index = 0u; index != 16u; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(address, index), 0u);
}

static void WriteLeadInAuxiliaryTuple(fw_addr_t main_data_cursor, fw_addr_t tuple_address)
{
    fw_addr_t destination = FirmwareAddressIncrement24(main_data_cursor, FW_HUGE_UNKNOWN_10000C);
    u8 index;

    for(index = 0u; index != 4u; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(destination, index), FirmwareRead8(tuple_address + index));
}

static u8 CaptureSpecialLeadInTenFrames(fw_addr_t tuple_address)
{
    fw_addr_t cursor = 0ul;
    u8 sector_index;

    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_800E, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 8u);
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_114F) - 10ul);
    BuildPackedTuple((s32)ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598), tuple_address);
    LEADIN_COPY(FW_NEAR_UNKNOWN_1428, (u16)tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, 1u);

    for(sector_index = 0u; sector_index != 10u; ++sector_index)
    {
        LEADIN_WRITE_BLOCK(FW_NEAR_UNKNOWN_1428);
        if(LEADIN_CAPTURE(cursor, 10u) == 0u)
        {
            LEADIN_SET_SENSE(4u, 0x4400u);
            LEADIN_SET_INFORMATION(ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598));
            return 0u;
        }
        LEADIN_BUILD_RID01(cursor + 0x10ul);
        LEADIN_INCREMENT(FW_NEAR_UNKNOWN_1428);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, sector_index);
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_126E, cursor);
        LEADIN_UPDATE_Q();
        LEADIN_ENQUEUE();
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        cursor = (cursor + 0x930ul > LEADIN_MULTIPLY(0x930ul, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_156A)) - 1ul) ? 0ul : cursor + 0x930ul;
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    LEADIN_WRITE_BIT0041(1u);
    return 1u;
}

static void AdvanceLeadInQSelection(u16 tuple_address)
{
    LEADIN_COPY(FW_NEAR_UNKNOWN_1275, tuple_address);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14C9) == 0u || ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_8008) == ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_800A))
    {
        u16 cursor = (u16)(ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14C1) + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
        if(cursor == ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_8008))
            cursor = 0u;
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C1, cursor);
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C5, cursor);
    }
    else
    {
        u16 cursor = ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14C3);
        u16 next = (u16)(cursor + 1u);

        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 0u);
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C5, cursor);
        if(next == ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_800A))
            next = ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_8008);
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C3, next);
    }
}

static u8 EmitLeadInFrame(fw_addr_t tuple_address, fw_addr_t *cursor, u16 q_record_index, u8 adr6_frame, u16 captured_count)
{
    if(captured_count < ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_156A))
    {
        if(LEADIN_CAPTURE_WITH_Q(tuple_address, *cursor, 7u) == 0u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CA) == 0u)
            ClearLeadInAuxiliarySlot(*cursor);
    }
    else
    {
        if(LEADIN_WAIT_CAPACITY() == 0u)
        {
            LEADIN_LATCH_FAILURE();
            return 0u;
        }
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CA) == 1u)
        {
            WriteLeadInAuxiliaryTuple(*cursor, tuple_address);
            LEADIN_POPULATE_SUBHEADER(*cursor, 7u);
        }
    }

    if(adr6_frame == 0u)
        CopySelectedQRecordToCurrent(q_record_index, (u16)tuple_address);
    else
        BuildAdr6QPacket(q_record_index, (u16)tuple_address);
    LEADIN_UPDATE_Q();
    LEADIN_ENQUEUE();
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    LEADIN_INCREMENT((u16)tuple_address);
    *cursor = LEADIN_ADVANCE_RING(*cursor);
    return 1u;
}

static u8 EmitLeadInTerminalFrames(fw_addr_t tuple_address, fw_addr_t *cursor, u8 frame_count)
{
    u8 index;

    for(index = 0u; index != frame_count; ++index)
    {
        if(LEADIN_CAPTURE_WITH_Q(tuple_address, *cursor, 7u) == 0u)
            return 0u;
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CA) == 0u)
            ClearLeadInAuxiliarySlot(*cursor);
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_126E, *cursor);
        LEADIN_UPDATE_Q();
        LEADIN_ENQUEUE();
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598) + 1ul);
        LEADIN_INCREMENT((u16)tuple_address);
        *cursor = LEADIN_ADVANCE_RING(*cursor);
        LEADIN_COPY(FW_NEAR_UNKNOWN_1275, (u16)tuple_address);
    }
    return 1u;
}

/* RAM:fe63a2 */
u8 PLEXTOR_NEAR_FUNCTION AcquireLeadInRawSectorDescriptorSequence(u8 use_atip_lead_in_start, u8 resume_at_failure_position)
{
    fw_addr_t cursor;
    u32 frame_count;
    u16 captured_count = 0u;
    u16 yield_group_count = 0u;
    u8 initial_q_class;
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/lead_in_raw_sector_descriptor_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14C9, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 1u);
    LEADIN_INITIALIZE_TOC();
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    ResetLeadInDescriptorState();
    if(LEADIN_READ_BIT0028() != 0u)
    {
        LEADIN_CLEAR_BIT0028();
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_FBA0, 0u);
    }
    ClearLeadInQState();

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u
        && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 1u)
            || (FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FA) == 0u)))
        use_atip_lead_in_start = 1u;

    if(use_atip_lead_in_start == 1u)
    {
        s32 start = (s32)ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_114F);

        frame_count = (u32)-start;
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, (u32)start);
        if(resume_at_failure_position == 1u)
        {
            u32 resume = ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1484);
            frame_count -= resume - (u32)start;
            WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, resume);
        }
    }
    else
    {
        u32 start = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u ? ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_E612) : ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_DD69);

        frame_count = 4500ul;
        WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, start);
        if(resume_at_failure_position == 1u)
        {
            u32 resume = ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1484);
            frame_count -= resume - start;
            WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_1598, resume);
        }
    }
    WriteLeadInSequenceU32(FW_NEAR_UNKNOWN_148C, ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 lead_in_index = ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_149D);
        u16 scan_index;

        if(FirmwareRead8(RawTrackRecordAddress(lead_in_index) + 3u) == 0x41u)
        {
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_88B5, 1u);
            FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, 0x41u);
        }
        else if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C45) != 0xc0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) < 2u && LEADIN_READ_BIT0041() == 0u && use_atip_lead_in_start == 1u)
        {
            if(CaptureSpecialLeadInTenFrames(tuple_address) == 0u)
                return 0u;
        }

        LEADIN_BUILD_SYNTHETIC_Q();
        scan_index = (u16)(lead_in_index + 1u);
        while((FirmwareRead8(RawTrackRecordAddress(scan_index)) & 0x0fu) != 1u)
            ++scan_index;
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CA, (u8)((FirmwareRead8(RawTrackRecordAddress(scan_index)) & 0x40u) != 0u));
        scan_index = LEADIN_FIND_TRACK(FirmwareReadNear8(FW_NEAR_UNKNOWN_14A5));
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, LEADIN_DECODE_TYPE(scan_index));
        initial_q_class = LEADIN_FIND_ADJACENT((u16)(lead_in_index + 1u), 1u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, initial_q_class);

        scan_index = ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14A1) == 0u ? 2u : 0u;
        do
        {
            ++scan_index;
        } while((FirmwareRead8(RawTrackRecordAddress(scan_index)) & 0x0fu) != 1u);
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_149B, scan_index);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14A8, FirmwareRead8(RawTrackRecordAddress(scan_index) + 1u));
    }
    else
    {
        LEADIN_MERGE_STAGED_Q();
        initial_q_class = (u8)((FirmwareReadNear8(FW_NEAR_UNKNOWN_2BC139) & 0x40u) != 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, initial_q_class);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, initial_q_class != 0u ? 8u : 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_14CA, initial_q_class);
    }
    FirmwareWrite8(tuple_address + 3u, initial_q_class);

    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C1, 0u);
    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C3, ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_8008));
    BuildPackedTuple((s32)ReadLeadInSequenceU32(FW_NEAR_UNKNOWN_1598), tuple_address);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    LEADIN_COPY(FW_NEAR_UNKNOWN_1279, (u16)tuple_address);
    BuildPackedTuple(0l, tuple_address + 4u);
    LEADIN_COPY(FW_NEAR_UNKNOWN_1275, (u16)(tuple_address + 4u));

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_88B5) != 1u)
    {
        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14CA) == 1u)
        {
            if(LEADIN_FIXED_PREFIX(2u, 7u) == 0u)
                return 0u;
        }
        else if(LEADIN_Q_PREFIX(1l, 0u) == 0u)
        {
            return 0u;
        }
        cursor = LEADIN_RESUME_RING();
        frame_count -= 2ul;
        LEADIN_COPY((u16)tuple_address, 0x1279u);
        WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C5, 0u);

        while((u32)captured_count < frame_count)
        {
            u8 adr6_frame = 0u;

            WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C7, 0u);
            if(LEADIN_READ_BIT0041() != 1u && use_atip_lead_in_start == 1u)
                adr6_frame = 1u;
            while(ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14C7) < 3u && (u32)captured_count < frame_count)
            {
                if(EmitLeadInFrame(tuple_address, &cursor, ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14C5), adr6_frame, captured_count) == 0u)
                    return 0u;
                ++captured_count;
                if(adr6_frame == 0u)
                    WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_14C7, (u16)(ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_14C7) + 1u));
                else
                    adr6_frame = 0u;
            }
            AdvanceLeadInQSelection((u16)tuple_address);
            if(yield_group_count++ > 30u)
            {
                LEADIN_YIELD();
                yield_group_count = 0u;
            }
        }

        if(EmitLeadInTerminalFrames(tuple_address, &cursor, FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u ? 1u : 2u) == 0u)
            return 0u;

        if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
        {
            u16 scan_index = ReadLeadInSequenceU16(FW_NEAR_UNKNOWN_149B);
            u8 first_track = FirmwareReadNear8(FW_NEAR_UNKNOWN_14A5);

            WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_149D, scan_index);
            while((FirmwareRead8(RawTrackRecordAddress(scan_index)) & 0x0fu) != 1u || FirmwareRead8(RawTrackRecordAddress(scan_index) + 1u) != first_track)
                ++scan_index;
            WriteLeadInSequenceU16(FW_NEAR_UNKNOWN_149B, scan_index);
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    }
    return 1u;
}
