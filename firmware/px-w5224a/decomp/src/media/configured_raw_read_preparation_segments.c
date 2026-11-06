#include "configured_raw_read_preparation_segments.h"
#include "../platform/firmware_memory.h"
#include "raw_read_frame_sequence.h"

#define PLEXTOR_BINDING_SECTION 1
#include "../platform/bindings/media/configured_raw_read_preparation_segments_bindings.h"
#undef PLEXTOR_BINDING_SECTION

static u32 ReadConfiguredSegmentsU32(u16 address)
{
    return FirmwareReadNearU32(address);
}

static void WriteConfiguredSegmentsU32(fw_addr_t address, u32 value)
{
    FirmwareWriteU32(address, value);
}

static void DecrementReadPipelineTarget(void)
{
    WriteConfiguredSegmentsU32(FW_NEAR_UNKNOWN_1598, ReadConfiguredSegmentsU32(FW_NEAR_UNKNOWN_1598) - 1ul);
}

static u8 AcquireDirectRange(s32 frame_count, u32 populated_sector_count)
{
    fw_addr_t controller_buffer_address = RESUME_RING();

    if(ACQUIRE_FRAMES(frame_count) == 0u)
        return 0u;
    POPULATE_RANGE(controller_buffer_address, populated_sector_count);
    return 1u;
}

static u8 AcquireStandardRange(s32 frame_count, u8 suppress_prefixed_population)
{
    fw_addr_t controller_buffer_address;

    if(frame_count < 5l)
        return AcquireDirectRange(frame_count, (u32)frame_count);

    controller_buffer_address = RESUME_RING();
    if(ACQUIRE_PREFIX(0u, 6u) == 0u)
        return 0u;
    if(ACQUIRE_FRAMES(frame_count - 5l) == 0u)
        return 0u;
    if(suppress_prefixed_population == 0u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B2) == 0u)
        POPULATE_RANGE(controller_buffer_address, (u32)frame_count);
    return 1u;
}

static u8 AcquireOrdinarySegmentPolicy(u8 state_eligible, s32 first_segment_count, s32 second_segment_count, u8 suppress_prefixed_population)
{
    if(state_eligible != 1u)
        return ACQUIRE_WITH_Q_UPDATES(second_segment_count, 1u) != 0u;
    if(first_segment_count != 0l)
        return AcquireDirectRange(second_segment_count, (u32)second_segment_count);
    return AcquireStandardRange(second_segment_count, suppress_prefixed_population);
}

static u8 AcquireFlaggedSegmentPolicy(u8 state_eligible, s32 first_segment_count, s32 second_segment_count)
{
    fw_addr_t controller_buffer_address;

    if(state_eligible != 1u)
    {
        if(ACQUIRE_WITH_Q_UPDATES(second_segment_count - 4l, 1u) == 0u)
            return 0u;
        DecrementReadPipelineTarget();
        return 1u;
    }

    if(first_segment_count != 0l)
    {
        if(second_segment_count < 7l)
            return AcquireDirectRange(second_segment_count, (u32)second_segment_count);
        controller_buffer_address = RESUME_RING();
        if(ACQUIRE_FRAMES(second_segment_count - 7l) == 0u)
            return 0u;
        if(ACQUIRE_PREFIX(1u, 6u) == 0u)
            return 0u;
        if(ACQUIRE_PREFIX(2u, 4u) == 0u)
            return 0u;
        DecrementReadPipelineTarget();
        POPULATE_RANGE(controller_buffer_address, (u32)(second_segment_count - 5l));
        return 1u;
    }

    if(second_segment_count < 12l)
    {
        if(second_segment_count < 5l || ((FirmwareReadNear8(FW_NEAR_UNKNOWN_0C84) >> 6) & 1u) != 0u)
            return AcquireDirectRange(second_segment_count, (u32)second_segment_count);
        return AcquireStandardRange(second_segment_count, 0u);
    }

    controller_buffer_address = RESUME_RING();
    if(ACQUIRE_PREFIX(0u, 6u) == 0u)
        return 0u;
    if(ACQUIRE_FRAMES(second_segment_count - 12l) == 0u)
        return 0u;
    if(ACQUIRE_PREFIX(1u, 6u) == 0u)
        return 0u;
    if(ACQUIRE_PREFIX(2u, 4u) == 0u)
        return 0u;
    DecrementReadPipelineTarget();
    POPULATE_RANGE(controller_buffer_address, (u32)(second_segment_count - 5l));
    return 1u;
}

static u8 AcquireForcedFiveFramePolicy(u8 state_eligible)
{
    fw_addr_t controller_buffer_address;

    if(state_eligible != 1u)
        return ACQUIRE_WITH_Q_UPDATES(5l, 1u) != 0u;

    controller_buffer_address = RESUME_RING();
    if(ACQUIRE_PREFIX(0u, 6u) == 0u)
        return 0u;
    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_14B2) == 0u)
        POPULATE_RANGE(controller_buffer_address, 5ul);
    return 1u;
}

/* RAM:fea6c4 */
u8 AcquireConfiguredRawReadPreparationSegments(u8 state_eligible, u8 alternate_plan)
{
    u8 write_type;
    u8 working_track_type;
    s32 first_segment_count;
    s32 second_segment_count;

    FirmwareWriteNear8(FW_NEAR_UNKNOWN_142B, SELECT_SECTOR_CLASS(0u, alternate_plan));
    write_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_0C44);

    if(write_type == 3u || write_type > 3u)
        return 1u;

    first_segment_count = (s32)ReadConfiguredSegmentsU32(FW_NEAR_UNKNOWN_1430);
    second_segment_count = (s32)ReadConfiguredSegmentsU32(FW_NEAR_UNKNOWN_1434);

    if(write_type == 1u || write_type == 2u)
        return AcquireOrdinarySegmentPolicy(state_eligible, first_segment_count, second_segment_count, 0u);

    if(write_type != 0u)
        return 1u;

    if(FirmwareReadNear8(FW_NEAR_UNKNOWN_128B) == 1u || FirmwareReadNear8(FW_NEAR_UNKNOWN_14B4) == 1u)
        return AcquireFlaggedSegmentPolicy(state_eligible, first_segment_count, second_segment_count);

    working_track_type = FirmwareReadNear8(FW_NEAR_UNKNOWN_DD34);
    if(working_track_type == 0u || alternate_plan == 1u)
        return AcquireOrdinarySegmentPolicy(state_eligible, first_segment_count, second_segment_count, 1u);
    return AcquireForcedFiveFramePolicy(state_eligible);
}
