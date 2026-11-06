#include "lead_out_raw_sector_descriptor_sequence.h"
#include "../platform/arithmetic.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/memory.h"
#include "../platform/raw_sector_capture.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"
#include "raw_read_runtime.h"

#define RAW_TRACK_TABLE_ADDRESS FW_NEAR_UNKNOWN_B4BD
#define STAGED_Q_TABLE_ADDRESS FW_HUGE_UNKNOWN_2BC11B
#define RAW_TRACK_RECORD_SIZE 8u
#define STAGED_Q_RECORD_SIZE 10u

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/lead_out_raw_sector_descriptor_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadLeadOutU16(fw_addr_t address)
{
    return FirmwareReadU16(address);
}

static u32 ReadLeadOutU32(fw_addr_t address)
{
    return FirmwareReadU32(address);
}

static void WriteLeadOutU16(u16 address, u16 value)
{
    FirmwareWriteNearU16(address, value);
}

static void WriteLeadOutU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static fw_addr_t RawTrackRecordAddress(u16 index)
{
    return RAW_TRACK_TABLE_ADDRESS + (fw_addr_t)(index * RAW_TRACK_RECORD_SIZE);
}

static fw_addr_t StagedQRecordAddress(u16 index)
{
    return STAGED_Q_TABLE_ADDRESS + (fw_addr_t)(index * STAGED_Q_RECORD_SIZE);
}

static void CopyThree(u16 destination, u16 source)
{
    FirmwareWrite8(destination, FirmwareRead8(source));
    FirmwareWriteNear8((u16)(destination + 1u), FirmwareReadNear8((u16)(source + 1u)));
    FirmwareWriteNear8((u16)(destination + 2u), FirmwareReadNear8((u16)(source + 2u)));
}

static void CopyCurrentQToArray(u8 *packet)
{
    u8 index;

    for(index = 0u; index != 10u; ++index)
        packet[index] = FirmwareReadNear8(FW_NEAR_UNKNOWN_1272 + index);
}

static void CopyArrayToCurrentQ(const u8 *packet)
{
    u8 index;

    for(index = 0u; index != 10u; ++index)
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272 + index, packet[index]);
}

static void ResetLeadOutDescriptorState(void)
{
    u32 slot_count = ReadLeadOutU32(FW_NEAR_UNKNOWN_156A);

    WriteLeadOutU16(FW_NEAR_UNKNOWN_1298, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1294, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1296, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_1292, 0u);
    WriteLeadOutU16(FW_NEAR_UNKNOWN_129A, (u16)(slot_count - 1ul));
    WriteLeadOutU16(FW_NEAR_UNKNOWN_129C, (u16)(slot_count - 6ul));
    WriteLeadOutU32(FW_NEAR_UNKNOWN_129E, 0x000088bbul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1262, 0ul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1266, 0ul);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_126A, ReadLeadOutU32(FW_NEAR_UNKNOWN_1562));
    WriteLeadOutU32(FW_NEAR_UNKNOWN_126E, 0ul);
}

static fw_addr_t SelectLeadOutRingCursor(void)
{
    u16 descriptor = LEADOUT_LATEST_DESCRIPTOR();
    fw_addr_t cursor = descriptor == 0u ? 0ul : ReadLeadOutU32((u16)(descriptor + 4u)) + 0x930ul;
    u32 span = LEADOUT_MULTIPLY(0x930ul, ReadLeadOutU32(FW_NEAR_UNKNOWN_156A));

    return span - 1ul < cursor ? 0ul : cursor;
}

static void ClearLeadOutAuxiliarySlot(fw_addr_t cursor)
{
    fw_addr_t address = FirmwareAddressIncrement24(cursor, FW_HUGE_BUFFER_BANK_BASE);
    u8 index;

    for(index = 0u; index != 16u; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(address, index), 0u);
}

static void WriteLeadOutAuxiliaryTuple(fw_addr_t cursor, fw_addr_t tuple)
{
    fw_addr_t destination = FirmwareAddressIncrement24(cursor, FW_HUGE_UNKNOWN_10000C);
    u8 index;

    for(index = 0u; index != 4u; ++index)
        FirmwareWrite8(FirmwareAddressIncrement24(destination, index), FirmwareRead8(tuple + index));
}

static void AdvanceLeadOutFrameState(fw_addr_t absolute_tuple, fw_addr_t relative_tuple, fw_addr_t *cursor)
{
    LEADOUT_UPDATE_Q();
    LEADOUT_ENQUEUE();
    WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, ReadLeadOutU32(FW_NEAR_UNKNOWN_1598) + 1ul);
    LEADOUT_INCREMENT((u16)absolute_tuple);
    LEADOUT_INCREMENT((u16)relative_tuple);
    *cursor = LEADOUT_ADVANCE(*cursor);
}

/* RAM:fe6e5b */
u8 PLEXTOR_NEAR_FUNCTION AcquireLeadOutRawSectorDescriptorSequence(u8 restart_after_failure)
{
    fw_addr_t cursor;
    u32 frame_count;
    s32 resume_delta = 0l;
    u16 captured_count = 0u;
    u16 q_group_count = 0u;
    u16 yield_count = 0u;
    u16 toggle_count = 0u;
    u8 first_toggle = 1u;
    u8 alternate_active = 1u;
    u8 long_region;
    u8 mode2_auxiliary;
    u8 initial_q_class;
    u8 saved_q[10];
    u8 alternate_q[10];
#define PLEXTOR_BINDING_SECTION 2
#include "../platform/bindings/media/lead_out_raw_sector_descriptor_sequence_bindings.h"
#undef PLEXTOR_BINDING_SECTION

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 1u);
    if(restart_after_failure == 1u)
        ResetLeadOutDescriptorState();
    FirmwareWrite8(relative_tuple, 0u);
    FirmwareWrite8(relative_tuple + 1u, 0u);
    FirmwareWrite8(relative_tuple + 2u, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0xaau);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1278, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1275, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1276, 0u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1277, 0u);
    cursor = SelectLeadOutRingCursor();

    long_region = (u8)((s32)ReadLeadOutU32(FW_NEAR_UNKNOWN_DD69) < 0l);
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_1008) == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_E5FB) == 1u && FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9) == 1u)
        long_region = 1u;

    if((s32)ReadLeadOutU32(FW_NEAR_UNKNOWN_DD69) >= 0l)
    {
        u16 count = ReadLeadOutU16(FW_HUGE_UNKNOWN_2BC119);
        fw_addr_t last = StagedQRecordAddress((u16)(count - 1u));

        alternate_q[0] = (u8)((FirmwareRead8(last) & 0xf0u) + 5u);
        alternate_q[1] = 0xaau;
        alternate_q[2] = 0xd1u;
        alternate_q[3] = 0u;
        alternate_q[4] = 0u;
        alternate_q[5] = 0u;
        alternate_q[6] = LEADOUT_TO_BCD(FirmwareReadNear8(FW_NEAR_UNKNOWN_12C9));
        LEADOUT_COPY_FIRST((u16)absolute_tuple);
        alternate_q[7] = FirmwareRead8(absolute_tuple);
        alternate_q[8] = FirmwareRead8(absolute_tuple + 1u);
        alternate_q[9] = FirmwareRead8(absolute_tuple + 2u);
    }

    frame_count = long_region != 0u ? 6750ul : 2250ul;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44) == 2u)
    {
        u16 lead_out_index = ReadLeadOutU16(FW_NEAR_UNKNOWN_149F);
        fw_addr_t record = RawTrackRecordAddress(lead_out_index);
        u16 type1_index;

        FirmwareWrite8(absolute_tuple, FirmwareRead8(record + 4u));
        FirmwareWrite8(absolute_tuple + 1u, FirmwareRead8(record + 6u));
        FirmwareWrite8(absolute_tuple + 2u, FirmwareRead8(record + 7u));
        WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, (u32)LEADOUT_TO_FRAME(absolute_tuple));
        if(restart_after_failure == 1u)
        {
            u32 resume = ReadLeadOutU32(FW_NEAR_UNKNOWN_1484);

            resume_delta = (s32)(resume - ReadLeadOutU32(FW_NEAR_UNKNOWN_1598));
            frame_count -= (u32)resume_delta;
            WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, resume);
            LEADOUT_TO_TUPLE((s32)resume, absolute_tuple);
        }
        WriteLeadOutU32(FW_NEAR_UNKNOWN_148C, ReadLeadOutU32(FW_NEAR_UNKNOWN_1598));
        type1_index = LEADOUT_FIND_TRACK(0xaau);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, LEADOUT_DECODE_TYPE(type1_index));
        mode2_auxiliary = (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_1285) > 4u);
        LEADOUT_PACK((u16)absolute_tuple);
        initial_q_class = LEADOUT_FIND_ADJACENT((u16)(lead_out_index - 1u), 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, FirmwareRead8(RawTrackRecordAddress((u16)(lead_out_index - 1u))));
        --frame_count;
    }
    else
    {
        u16 count = ReadLeadOutU16(FW_HUGE_UNKNOWN_2BC119);
        fw_addr_t last = StagedQRecordAddress((u16)(count - 1u));
        u32 start = FirmwareReadNear8(FW_NEAR_UNKNOWN_88B7) == 1u ? ReadLeadOutU32(FW_NEAR_UNKNOWN_E61A) : ReadLeadOutU32(FW_NEAR_UNKNOWN_DD71);

        LEADOUT_TO_TUPLE((s32)start, absolute_tuple);
        WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, start);
        if(restart_after_failure == 1u)
        {
            u32 resume = ReadLeadOutU32(FW_NEAR_UNKNOWN_1484);

            resume_delta = (s32)(resume - start);
            frame_count -= (u32)resume_delta;
            WriteLeadOutU32(FW_NEAR_UNKNOWN_1598, resume);
            LEADOUT_TO_TUPLE((s32)resume, absolute_tuple);
        }
        mode2_auxiliary = (u8)((FirmwareRead8(last) & 0x40u) != 0u);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, mode2_auxiliary != 0u ? FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48) : 0u);
        WriteLeadOutU32(FW_NEAR_UNKNOWN_148C, ReadLeadOutU32(FW_NEAR_UNKNOWN_1598));
        LEADOUT_PACK((u16)absolute_tuple);
        if(mode2_auxiliary != 0u)
        {
            fw_addr_t track_state = FW_NEAR_UNKNOWN_DD88 + (fw_addr_t)FirmwareReadNear8(FW_NEAR_UNKNOWN_1149) * 0x13u;

            initial_q_class = (u8)((FirmwareRead8(track_state) & 0x40u) != 0u ? 1u : 2u);
        }
        else
        {
            initial_q_class = 0u;
        }
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1272, FirmwareRead8(last));
        CopyCurrentQToArray(saved_q);
        CopyThree((u16)relative_tuple, (u16)absolute_tuple);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1273, 0xaau);
        FirmwareWriteNear8(FW_NEAR_UNKNOWN_1274, 1u);
        CopyThree(FW_NEAR_UNKNOWN_1279, (u16)relative_tuple);
        LEADOUT_TO_TUPLE(0l, relative_tuple);
        if(restart_after_failure == 1u)
            LEADOUT_TO_TUPLE(resume_delta, relative_tuple);
        LEADOUT_PACK((u16)relative_tuple);
        CopyThree(FW_NEAR_UNKNOWN_1275, (u16)relative_tuple);
        if((mode2_auxiliary != 0u ? LEADOUT_ACQUIRE_CURRENT(1l, 4u) : LEADOUT_ACQUIRE_UPDATING(1l, 0u)) == 0u)
            return 0u;
        frame_count -= 2ul;
        CopyArrayToCurrentQ(saved_q);
        LEADOUT_INCREMENT((u16)relative_tuple);
        LEADOUT_INCREMENT((u16)absolute_tuple);
        cursor = LEADOUT_ADVANCE(cursor);
    }
    FirmwareWrite8(absolute_tuple + 3u, initial_q_class);

    LEADOUT_CLEAR_BIT35();
    LEADOUT_WRITE_BIT30(1u);
    while((u32)captured_count < frame_count)
    {
        if(alternate_active == 1u)
        {
            CopyThree(FW_NEAR_UNKNOWN_1279, (u16)absolute_tuple);
            CopyThree(FW_NEAR_UNKNOWN_1275, (u16)relative_tuple);
        }
        if((u32)captured_count < ReadLeadOutU32(FW_NEAR_UNKNOWN_156A))
        {
            if(LEADOUT_CAPTURE(absolute_tuple, cursor, 8u) == 0u)
                return 0u;
            if(mode2_auxiliary == 0u)
                ClearLeadOutAuxiliarySlot(cursor);
            if(alternate_active == 0u)
                CopyArrayToCurrentQ(alternate_q);
        }
        else
        {
            if(LEADOUT_WAIT_CAPACITY() == 0u)
            {
                LEADOUT_LATCH_FAILURE();
                return 0u;
            }
            if(mode2_auxiliary == 1u)
            {
                WriteLeadOutAuxiliaryTuple(cursor, absolute_tuple);
                LEADOUT_POPULATE(cursor, 8u);
            }
        }
        AdvanceLeadOutFrameState(absolute_tuple, relative_tuple, &cursor);

        if(captured_count >= 149u)
        {
            if(toggle_count > 18u || first_toggle == 1u)
            {
                FirmwareWriteNear8(FW_NEAR_UNKNOWN_127C, (u8)(FirmwareReadNear8(FW_NEAR_UNKNOWN_127C) != 1u));
                toggle_count = 0u;
                first_toggle = 0u;
            }
            ++toggle_count;
        }
        ++captured_count;
        if(q_group_count == 2u)
        {
            q_group_count = 0u;
            if(long_region == 0u)
            {
                if(alternate_active == 1u)
                {
                    CopyCurrentQToArray(saved_q);
                    CopyArrayToCurrentQ(alternate_q);
                }
                else
                {
                    CopyCurrentQToArray(alternate_q);
                    CopyArrayToCurrentQ(saved_q);
                }
                alternate_active = (u8)(alternate_active == 0u);
            }
        }
        else
        {
            ++q_group_count;
        }
        if(yield_count++ > 30u)
        {
            LEADOUT_YIELD();
            yield_count = 0u;
        }
    }

    LEADOUT_WRITE_BIT30(0u);
    if(LEADOUT_READ_BIT32() != 0u)
        LEADOUT_RESTART();
    for(captured_count = 0u; captured_count != 2u; ++captured_count)
    {
        LEADOUT_COPY_BYTES(FW_NEAR_UNKNOWN_1275, relative_tuple, 3u);
        if(LEADOUT_CAPTURE(absolute_tuple, cursor, 7u) == 0u)
            return 0u;
        if(mode2_auxiliary == 0u)
            ClearLeadOutAuxiliarySlot(cursor);
        AdvanceLeadOutFrameState(absolute_tuple, relative_tuple, &cursor);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1289, 0u);
    WriteLeadOutU32(FW_NEAR_UNKNOWN_15F2, ReadLeadOutU32(FW_NEAR_UNKNOWN_1598) - 1ul);
    return 1u;
}
