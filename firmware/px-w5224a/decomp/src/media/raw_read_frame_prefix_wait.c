#include "raw_read_frame_prefix_wait.h"
#include "../platform/encoding.h"
#include "../platform/firmware_memory.h"
#include "../platform/leaf_workers.h"
#include "../platform/status_mapping.h"
#include "raw_read_frame_sequence.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/raw_read_frame_prefix_wait_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u16 ReadPrefixU16(u16 address)
{
    return FirmwareReadNearU16(address);
}

static u32 ReadPrefixU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WritePrefixU32(u16 address, u32 value)
{
    FirmwareWriteNearU32(address, value);
}

static u16 FindPreviousTypeOneRawReadRecord(void)
{
    u16 index = (u16)(ReadPrefixU16(FW_NEAR_UNKNOWN_149B) - 1u);

    while((FirmwareReadNear8(FW_NEAR_UNKNOWN_B4BD + (u16)(index << 3)) & 0x0fu) != 1u)
        --index;
    return index;
}

static u8 SelectTypeTwoCaptureSource(void)
{
    u16 index = FindPreviousTypeOneRawReadRecord();

    return FirmwareReadNear8(FW_NEAR_UNKNOWN_B4BF + (u16)(index << 3)) == 0u ? 6u : 4u;
}

static u8 WaitForRawReadPrefixCompletion(u8 wait_for_completion)
{
    if(wait_for_completion == 0u)
        return 1u;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_14B3, 0u);
    if(PREFIX_POLL(1u) != 0u)
        return 1u;
    if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) != 0u)
        PREFIX_FAILURE(8u);
    else if(ReadPrefixU16(FW_NEAR_UNKNOWN_1292) == 0u && FirmwareReadNear8(FW_NEAR_UNKNOWN_FBA0) == 0u)
        PREFIX_QUEUE_SENSE();
    else
        PREFIX_FAILURE(2u);
    return 0u;
}

/* RAM:fe3827 */
/* ACC/R7, ERET. */
u8 PrepareRawReadFramePrefixAndOptionallyWait(u8 omit_final_prefix_frame, u8 wait_for_completion)
{
    u8 tuple[3];
    u8 write_type;
    u8 q_type;
    u8 capture_source;
    u16 record_index;
    s32 frame_count;
    u32 target;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B5) != 0u)
        return WaitForRawReadPrefixCompletion(wait_for_completion);

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1284, 1u);
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_127E, 0u);
    target = ReadPrefixU32(FW_NEAR_UNKNOWN_1598);
    PREFIX_BUILD_TUPLE((s32)target, tuple);

    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);
    switch(write_type)
    {
    case 0u:
    case 1u:
        (void)PREFIX_SELECT_CLASS();
        break;
    case 2u:
        record_index = FindPreviousTypeOneRawReadRecord();
        (void)PREFIX_FIND_CLASS(record_index, 0u);
        break;
    case 3u:
        break;
    default:
        PREFIX_SEQUENCE_ERROR();
        return 0u;
    }

    if(write_type == 2u && FirmwareReadNear8(FW_NEAR_UNKNOWN_1273) == 0xaau && ((FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 1u || (FirmwareReadNear8(FW_NEAR_UNKNOWN_1272) & 0x0fu) == 5u))
        return WaitForRawReadPrefixCompletion(wait_for_completion);

    if(write_type == 2u)
    {
        record_index = FindPreviousTypeOneRawReadRecord();
        q_type = PREFIX_DECODE_TYPE(record_index);
    }
    else
    {
        q_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C48);
    }
    FirmwareWriteNear8(FW_NEAR_UNKNOWN_1285, q_type);

    if(q_type < 4u)
    {
        frame_count = omit_final_prefix_frame == 0u ? 3l : 2l;
        capture_source = 0u;
        if(write_type == 2u)
            capture_source = SelectTypeTwoCaptureSource() == 6u ? 1u : 0u;
        if(PREFIX_ACQUIRE_SEQUENCE(frame_count, capture_source) == 0u)
            return 0u;
        target = ReadPrefixU32(FW_NEAR_UNKNOWN_1598);
        if(frame_count == 2l)
        {
            WritePrefixU32(FW_NEAR_UNKNOWN_8000, target);
        }
        else
        {
            --target;
            WritePrefixU32(FW_NEAR_UNKNOWN_1598, target);
            WritePrefixU32(FW_NEAR_UNKNOWN_8000, target);
        }
    }
    else if(omit_final_prefix_frame == 0u)
    {
        if(write_type != 0u || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C46) >> 1) & 1u) != 1u || ReadPrefixU32(FW_NEAR_UNKNOWN_14B8) != 0ul || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) != 0u)
        {
            capture_source = write_type == 2u ? SelectTypeTwoCaptureSource() : 4u;
            if(PREFIX_ACQUIRE_WINDOW(1u, capture_source) == 0u)
                return 0u;
        }
        capture_source = write_type == 2u ? SelectTypeTwoCaptureSource() : 4u;
        if(PREFIX_ACQUIRE_WINDOW(2u, capture_source) == 0u)
            return 0u;
        if(((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) == 0u || write_type != 0u)
        {
            target = ReadPrefixU32(FW_NEAR_UNKNOWN_1598);
            --target;
            WritePrefixU32(FW_NEAR_UNKNOWN_1598, target);
            WritePrefixU32(FW_NEAR_UNKNOWN_8000, target);
        }
    }
    else
    {
        if(PREFIX_ACQUIRE_WINDOW(1u, 4u) == 0u)
            return 0u;
        WritePrefixU32(FW_NEAR_UNKNOWN_8000, ReadPrefixU32(FW_NEAR_UNKNOWN_1598));
    }

    return WaitForRawReadPrefixCompletion(wait_for_completion);
}
